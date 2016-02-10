/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/*   1. Redistributions of source code must retain the above         */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer.                                                  */
/*                                                                   */
/*   2. Redistributions in binary form must reproduce the above      */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer in the documentation and/or other materials       */
/*      provided with the distribution.                              */
/*                                                                   */
/*    THIS  SOFTWARE IS PROVIDED  BY THE  UNIVERSITY OF  TEXAS AT    */
/*    AUSTIN  ``AS IS''  AND ANY  EXPRESS OR  IMPLIED WARRANTIES,    */
/*    INCLUDING, BUT  NOT LIMITED  TO, THE IMPLIED  WARRANTIES OF    */
/*    MERCHANTABILITY  AND FITNESS FOR  A PARTICULAR  PURPOSE ARE    */
/*    DISCLAIMED.  IN  NO EVENT SHALL THE UNIVERSITY  OF TEXAS AT    */
/*    AUSTIN OR CONTRIBUTORS BE  LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL,  SPECIAL, EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES    */
/*    (INCLUDING, BUT  NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE    */
/*    GOODS  OR  SERVICES; LOSS  OF  USE,  DATA,  OR PROFITS;  OR    */
/*    BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF    */
/*    LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR TORT    */
/*    (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY OUT    */
/*    OF  THE  USE OF  THIS  SOFTWARE,  EVEN  IF ADVISED  OF  THE    */
/*    POSSIBILITY OF SUCH DAMAGE.                                    */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of The University of Texas at Austin.                 */
/*********************************************************************/

#include "DynamicTexture.h"

#include "log.h"
#include "Tile.h"

#include <fstream>
#include <boost/tokenizer.hpp>

#include <QDir>
#include <QImageReader>
#include <QtConcurrentRun>

#define TEXTURE_SIZE 512

namespace
{
const QString PYRAMID_METADATA_FILE_NAME( "pyramid.pyr" );
}

const QString DynamicTexture::pyramidFileExtension = QString( "pyr" );
const QString DynamicTexture::pyramidFolderSuffix = QString( ".pyramid/" );

DynamicTexture::DynamicTexture(const QString& uri, DynamicTexturePtr parent_,
                               const QRectF& parentCoordinates, const int childIndex)
    : _uri(uri)
    , _useImagePyramid(false)
    , _parent(parent_)
    , _imageCoordsInParentImage(parentCoordinates)
    , _depth(0)
{
    // if we're a child...
    if(parent_)
    {
        _depth = parent_->_depth + 1;

        // append childIndex to parent's path to form this object's path
        _treePath = parent_->_treePath;
        _treePath.push_back(childIndex);

        _imageExtension = parent_->_imageExtension;
    }

    // if we're the top-level object
    if(isRoot())
    {
        // this is the top-level object, so its path is 0
        _treePath.push_back(0);

        const QString extension = QString(".").append(pyramidFileExtension);
        if(_uri.endsWith(extension))
            readPyramidMetadataFromFile(_uri);
        else
            readFullImageMetadata(_uri);
    }

    _pendingLoadFutures.setCancelOnWait( true );
}

bool DynamicTexture::isRoot() const
{
    return _depth == 0;
}

bool DynamicTexture::readFullImageMetadata( const QString& uri )
{
    const QImageReader imageReader(uri);
    if( !imageReader.canRead( ))
        return false;

    _imageExtension = QString( imageReader.format( ));
    _imageSize = imageReader.size();
    return true;
}

bool DynamicTexture::readPyramidMetadataFromFile( const QString& uri )
{
    std::ifstream ifs(uri.toLatin1());

    // read the whole line
    std::string lineString;
    getline(ifs, lineString);

    // parse the arguments, allowing escaped characters, quotes, etc., and assign them to a vector
    std::string separator1("\\"); // allow escaped characters
    std::string separator2(" "); // split on spaces
    std::string separator3("\"\'"); // allow quoted arguments

    boost::escaped_list_separator<char> els(separator1, separator2, separator3);
    boost::tokenizer<boost::escaped_list_separator<char> > tokenizer(lineString, els);

    std::vector<std::string> tokens;
    tokens.assign(tokenizer.begin(), tokenizer.end());

    if( tokens.size() != 3 )
    {
        put_flog( LOG_ERROR, "requires 3 arguments, got %i", tokens.size( ));
        return false;
    }

    _imagePyramidPath = QString(tokens[0].c_str());
    if( !determineImageExtension( _imagePyramidPath ))
        return false;

    _imageSize.setWidth(atoi(tokens[1].c_str()));
    _imageSize.setHeight(atoi(tokens[2].c_str()));

    _useImagePyramid = true;

    put_flog( LOG_VERBOSE, "read pyramid file: '%s'', width: %i, height: %i",
              _imagePyramidPath.toLocal8Bit().constData(), _imageSize.width(),
              _imageSize.height( ));

    return true;
}

bool DynamicTexture::determineImageExtension( const QString& imagePyramidPath )
{
    QString extension;
    const QFileInfoList pyramidRootFiles =
            QDir( imagePyramidPath ).entryInfoList( QStringList( "0.*" ));
    if( !pyramidRootFiles.empty( ))
        extension = pyramidRootFiles.first().suffix();
    else
    {
        QString path( imagePyramidPath );
        path.remove( pyramidFolderSuffix );
        extension = path.split(".").last();
    }

    if( !QImageReader().supportedImageFormats().contains( extension.toLatin1( )))
        return false;

    _imageExtension = extension;
    return true;
}

bool DynamicTexture::writeMetadataFile( const QString& pyramidFolder,
                                        const QString& filename ) const
{
    std::ofstream ofs( filename.toStdString().c_str( ));
    if( !ofs.good( ))
    {
        put_flog( LOG_ERROR, "can't write second metadata file: '%s'",
                  filename.toStdString().c_str( ));
        return false;
    }

    ofs << "\"" << pyramidFolder.toStdString() << "\" " << _imageSize.width()
        << " " << _imageSize.height();
    return true;
}

bool DynamicTexture::writePyramidMetadataFiles(const QString& pyramidFolder) const
{
    // First metadata file inside the pyramid folder
    const QString metadataFilename = pyramidFolder + PYRAMID_METADATA_FILE_NAME;

    // Second (more conveniently named) metadata file outside the pyramid folder
    QString secondMetadataFilename = pyramidFolder;
    const int lastIndex = secondMetadataFilename.lastIndexOf(pyramidFolderSuffix);
    secondMetadataFilename.truncate(lastIndex);
    secondMetadataFilename.append(".").append(pyramidFileExtension);

    return writeMetadataFile(pyramidFolder, metadataFilename) &&
           writeMetadataFile(pyramidFolder, secondMetadataFilename);
}

QString DynamicTexture::getPyramidImageFilename() const
{
    QString filename;

    for( unsigned int i=0; i<_treePath.size(); ++i )
    {
        filename.append( QString::number( _treePath[i] ));

        if( i != _treePath.size() - 1 )
            filename.append( "-" );
    }

    filename.append( "." ).append( _imageExtension );

    return filename;
}

bool DynamicTexture::writePyramidImagesRecursive( const QString& pyramidFolder )
{
    _loadImage(); // load this object's scaledImage_

    const QString filename = pyramidFolder + getPyramidImageFilename();
    put_flog( LOG_DEBUG, "saving: '%s'", filename.toLocal8Bit().constData( ));

    if( !_scaledImage.save( filename ))
        return false;
    _scaledImage = QImage(); // no longer need scaled image

    // recursively generate and save children images
    if( _canHaveChildren( ))
    {
        QRectF imageBounds[4];
        imageBounds[0] = QRectF( 0.0, 0.0, 0.5, 0.5 );
        imageBounds[1] = QRectF( 0.5, 0.0, 0.5, 0.5 );
        imageBounds[2] = QRectF( 0.5, 0.5, 0.5, 0.5 );
        imageBounds[3] = QRectF( 0.0, 0.5, 0.5, 0.5 );

#pragma omp parallel for
        for( unsigned int i=0; i<4; ++i )
        {
            DynamicTexturePtr child( new DynamicTexture( "", shared_from_this(),
                                                         imageBounds[i], i ));
            child->writePyramidImagesRecursive( pyramidFolder );
        }
    }
    return true;
}

bool DynamicTexture::loadFullResImage()
{
    if( !fullscaleImage_.load( _uri ))
    {
        put_flog( LOG_ERROR, "error loading: '%s'",
                  _uri.toLocal8Bit().constData( ));
        return false;
    }
    _imageSize = fullscaleImage_.size();
    return true;
}

void DynamicTexture::_loadImage()
{
    if(isRoot())
    {
        if(_useImagePyramid)
        {
            _scaledImage.load(_imagePyramidPath+'/'+getPyramidImageFilename());
        }
        else
        {
            if (!fullscaleImage_.isNull() || loadFullResImage())
                _scaledImage = fullscaleImage_.scaled(TEXTURE_SIZE, TEXTURE_SIZE, Qt::KeepAspectRatio);
        }
    }
    else
    {
        DynamicTexturePtr root = getRoot();

        if(root->_useImagePyramid)
        {
            _scaledImage.load(root->_imagePyramidPath+'/'+getPyramidImageFilename());
        }
        else
        {
            DynamicTexturePtr parentTex(_parent);
            const QImage image = parentTex->getImageFromParent(_imageCoordsInParentImage, this);

            if(!image.isNull())
            {
                _imageSize= image.size();
                _scaledImage = image.scaled(TEXTURE_SIZE, TEXTURE_SIZE, Qt::KeepAspectRatio);
            }
        }
    }

    if( _scaledImage.isNull( ))
    {
        put_flog( LOG_ERROR, "loading failed in DynamicTexture: '%s'",
                  _uri.toLocal8Bit().constData( ));
    }
}

const QSize& DynamicTexture::getSize() const
{
    return _imageSize;
}

QImage DynamicTexture::getRootImage() const
{
    return QImage( _imagePyramidPath+ '/' + getPyramidImageFilename( ));
}

uint DynamicTexture::getMaxLod() const
{
    uint maxLod = 0;
    int maxDim = std::max( _imageSize.width(), _imageSize.height( ));
    while( maxDim > TEXTURE_SIZE )
    {
        maxDim = maxDim >> 1;
        ++maxLod;
    }
    return maxLod;
}

uint DynamicTexture::getLod( const QSize& targetDisplaySize ) const
{
    uint lod = 0;
    QSize nextLOD = _imageSize / 2;
    const uint maxLod = getMaxLod();
    while( targetDisplaySize.width() < nextLOD.width() &&
           targetDisplaySize.height() < nextLOD.height( ) &&
           lod < maxLod )
    {
        nextLOD = nextLOD / 2;
        ++lod;
    }
    return lod;
}

QString DynamicTexture::getTileFilename( const uint tileIndex ) const
{
    uint lod = 0;
    uint firstTileIndex = getFirstTileIndex( lod );
    while( tileIndex < firstTileIndex )
        firstTileIndex = getFirstTileIndex( ++lod );

    const int index = tileIndex - firstTileIndex;
    const QSize tilesCount = getTilesCount( lod );

    int x = index % tilesCount.width();
    int y = index / tilesCount.width();

    QString filename = QString( ".%1" ).arg( _imageExtension );

    const uint maxLod = getMaxLod();
    while( ++lod <= maxLod )
    {
        // The indices go in the order: 0-1
        //                              3-2
        int i = 0;
        if( y % 2 )
            i = 3 - x % 2;
        else
            i = x % 2;

        filename.prepend( QString::number( i )).prepend( '-' );
        x = x >> 1;
        y = y >> 1;
    }
    filename.prepend( '0' );
    filename.prepend( _imagePyramidPath );
    return filename;
}

QImage DynamicTexture::getTileImage( const uint tileIndex ) const
{
    QMutexLocker lock( &_tilesCacheMutex );
    if( !_tilesCache.count( tileIndex ))
        return QImage();
    return _tilesCache[tileIndex];
}

void DynamicTexture::triggerTileLoad( Tile* tile )
{
    if( _tilesCache.count( tile->getIndex( )))
    {
        tile->setVisible( true );
        return;
    }

    _pendingLoadFutures.addFuture(
                           QtConcurrent::run( [&,tile] { _loadTile( tile ); }));
}

void DynamicTexture::cancelPendingTileLoads()
{
    _pendingLoadFutures.waitForFinished();
    _pendingLoadFutures.clearFutures();
}

bool DynamicTexture::hasPendingTileLoads() const
{
    for( const auto& future : _pendingLoadFutures.futures( ))
    {
        if( !future.isFinished( ))
            return true;
    }
    // finished futures are never removed from the future synchronizer, so
    // we do it here when all are done as we are called here as long there are
    // pending futures.
    _pendingLoadFutures.clearFutures();
    return false;
}

void DynamicTexture::_loadTile( Tile* tile )
{
    QImage tileImage( getTileFilename( tile->getIndex( )));
    {
        QMutexLocker lock( &_tilesCacheMutex );
        _tilesCache[tile->getIndex()] = tileImage;
    }
    tile->setVisible( true );
}

int DynamicTexture::getFirstTileIndex( const uint lod ) const
{
    const uint maxLod = getMaxLod();

    if( lod == maxLod )
        return 0;

    const int nextLod = lod + 1;
    return std::pow( 4, maxLod - nextLod ) + getFirstTileIndex( nextLod );
}

QRect
DynamicTexture::getTileCoord( const uint lod, const uint x, const uint y ) const
{
    Q_UNUSED( lod );

    // All tiles have the same size in the current implementation, but this is
    // likely to change in the future
    const QSize size = _imageSize.scaled( TEXTURE_SIZE, TEXTURE_SIZE,
                                          Qt::KeepAspectRatio );
    return QRect( QPoint( x * size.width(), y * size.height( )), size );
}

QSize DynamicTexture::getTilesCount( const uint lod ) const
{
    const int maxDim = std::max( _imageSize.width(), _imageSize.height( ));
    const int tiles = std::ceil( (float)(maxDim >> lod) / TEXTURE_SIZE );
    return QSize( tiles, tiles );
}

QSize DynamicTexture::getTilesArea( const uint lod ) const
{
    return QSize( _imageSize.width() >> lod, _imageSize.height() >> lod );
}

bool DynamicTexture::_canHaveChildren()
{
    return (getRoot()->_imageSize.width() / (1 << _depth) > TEXTURE_SIZE ||
            getRoot()->_imageSize.height() / (1 << _depth) > TEXTURE_SIZE);
}

bool DynamicTexture::makeFolder( const QString& folder )
{
    if( !QDir( folder ).exists ())
    {
        if( !QDir().mkdir( folder ))
        {
            put_flog( LOG_ERROR, "error creating directory: '%s'",
                      folder.toLocal8Bit().constData( ));
            return false;
        }
    }
    return true;
}

bool DynamicTexture::generateImagePyramid( const QString& outputFolder )
{
    assert( isRoot( ));

    const QString imageName( QFileInfo( _uri ).fileName( ));
    const QString pyramidFolder( QDir( outputFolder ).absolutePath() +
                                 "/" + imageName + pyramidFolderSuffix );

    if( !makeFolder( pyramidFolder ))
        return false;

    if( !writePyramidMetadataFiles( pyramidFolder ))
        return false;

    return writePyramidImagesRecursive( pyramidFolder );
}

DynamicTexturePtr DynamicTexture::getRoot()
{
    if(isRoot())
        return shared_from_this();
    else
        return DynamicTexturePtr(_parent)->getRoot();
}

QRectF DynamicTexture::getImageRegionInParentImage(const QRectF& imageRegion) const
{
    QRectF parentRegion;

    parentRegion.setX(_imageCoordsInParentImage.x() + imageRegion.x() * _imageCoordsInParentImage.width());
    parentRegion.setY(_imageCoordsInParentImage.y() + imageRegion.y() * _imageCoordsInParentImage.height());
    parentRegion.setWidth(imageRegion.width() * _imageCoordsInParentImage.width());
    parentRegion.setHeight(imageRegion.height() * _imageCoordsInParentImage.height());

    return parentRegion;
}

QImage DynamicTexture::getImageFromParent( const QRectF& imageRegion,
                                           DynamicTexture* start )
{
    // if we're in the starting node, we must ascend
    if( start == this )
    {
        if( isRoot( ))
        {
            put_flog( LOG_DEBUG, "root object has no parent! In file: '%s'",
                      _uri.toLocal8Bit().constData( ));
            return QImage();
        }

        DynamicTexturePtr parentTex = _parent.lock();
        return parentTex->getImageFromParent(
                    getImageRegionInParentImage( imageRegion ), this );
    }

    if(!fullscaleImage_.isNull())
    {
        // we have a valid image, return the clipped image
        return fullscaleImage_.copy(imageRegion.x()*fullscaleImage_.width(),
                                    imageRegion.y()*fullscaleImage_.height(),
                                    imageRegion.width()*fullscaleImage_.width(),
                                    imageRegion.height()*fullscaleImage_.height());
    }
    else
    {
        // we don't have a valid image
        // if we're the root object, return an empty image
        // otherwise, continue up the tree looking for an image
        if(isRoot())
            return QImage();

        DynamicTexturePtr parentTex = _parent.lock();
        return parentTex->getImageFromParent(getImageRegionInParentImage(imageRegion), start);
    }
}
