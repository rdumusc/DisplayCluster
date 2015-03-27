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

#ifndef DYNAMIC_TEXTURE_H
#define DYNAMIC_TEXTURE_H

#include "WallContent.h"

#include "GLTexture2D.h"
#include "GLQuad.h"

#include <QImage>
#include <QFuture>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

/**
 * A dynamically loaded large scale image.
 *
 * It can work with two types of image files:
 * (1) A custom precomuted image pyramid (recommended)
 * (2) Direct reading from a large image
 * @see generateImagePyramid()
 */
class DynamicTexture : public boost::enable_shared_from_this<DynamicTexture>,
        public WallContent
{
public:
    /**
     * Constructor
     * @param uri The uri of an image or of a Pyramid metadata file
     * @param parent Internal use: child objects need to keep a weak pointer to
     *        their parent
     * @param parentCoordinates Internal use: texture coordinates in the parent
     *        texture
     * @param childIndex Internal use: index of the child object
     */
    DynamicTexture( const QString& uri = "",
                    DynamicTexturePtr parent = DynamicTexturePtr(),
                    const QRectF& parentCoordinates = QRectF(),
                    const int childIndex = 0 );

    /** Destructor */
    ~DynamicTexture();

    /** The exension of pyramid metadata files */
    static const QString pyramidFileExtension;

    /** The standard suffix for pyramid image folders */
    static const QString pyramidFolderSuffix;

    /** Get the size of the full resolution texture */
    const QSize& getSize() const;

    /** Render the dynamic texture. */
    void render() override;

    /** Render the preview. */
    void renderPreview() override;

    /** Pre render step. */
    void preRenderUpdate( ContentWindowPtr window,
                          const QRect& wallArea ) override;

    /** Post render step. */
    void postRenderSync( WallToWallChannel& wallToWallChannel ) override;

    /** Get the root image of the pyramid. */
    QImage getRootImage() const;

    /**
     * Generate an image Pyramid from the current uri and save it to the disk.
     * @param outputFolder The folder in which the metadata and pyramid images
     *        will be created.
     */
    bool generateImagePyramid( const QString& outputFolder );

    /**
     * Load the image for this part of the texture
     * @throw boost::bad_weak_ptr exception if a parent object is deleted during
     *        thread execution
     * @internal asynchronous loading thread needs access to this method
     */
    void loadImage();

    /**
     * Decrement the global count of loading threads.
     * @throw boost::bad_weak_ptr exception if a parent object is deleted during
     *        thread execution
     * @internal asynchronous loading thread needs access to this method
     */
    void decrementGlobalThreadCount();

private:
    /* for root only: */

    QString uri_;
    QString imageExtension_;

    QString imagePyramidPath_;
    bool useImagePyramid_;

    int threadCount_;
    QMutex threadCountMutex_;

    QImage fullscaleImage_;

    QRectF zoomRect_;

    /* for children only: */

    boost::weak_ptr<DynamicTexture> parent_;
    QRectF imageCoordsInParentImage_;

    /* for all objects: */

    std::vector<int> treePath_; // To construct the image name for each object
    int depth_; // The depth of the object in the image pyramid

    mutable QFuture<void> loadImageThread_;
    bool loadImageThreadStarted_;

    QSize imageSize_; // full scale image dimensions
    QImage scaledImage_; // for texture upload to GPU
    GLTexture2D texture_;
    GLQuad quad_;
    GLQuad quadBorder_;

    std::vector<DynamicTexturePtr> children_; // Children in the image pyramid
    bool renderedChildren_; // Used for garbage-collecting unused child objects

    bool isVisibleInCurrentGLView();
    bool isResolutionSufficientForCurrentGLView();
    bool canHaveChildren();

    /** Recursively clear children which have not been rendered recently. */
    void clearOldChildren(); // @All

    /**
     * Render the dynamic texture.
     * @param texCoords The area of the full scale texture to render
     */
    void render( const QRectF& texCoords );

    /**
     * Render the dynamic texture.
     * This function is also called from child objects to render a low-res
     * texture when the high-res one is not loaded yet.
     * @param texCoords The area of the full scale texture to render
     */
    void drawTexture( const QRectF& texCoords ); // @All

    /** Is this object the root element. */
    bool isRoot() const;  // @All

    /**
     * Get the root object,
     * @return A valid DynamicTexturePtr to the root element
     * @throw boost::bad_weak_ptr exception if the root object is deleted during
     *        thread execution
     */
    DynamicTexturePtr getRoot(); // @Child only

    bool readFullImageMetadata( const QString& uri );

    bool readPyramidMetadataFromFile( const QString& uri ); // @Root only
    bool determineImageExtension( const QString& imagePyramidPath );
    bool makeFolder(const QString& folder ); // @Root only
    bool writeMetadataFile( const QString& pyramidFolder,
                            const QString& filename ) const; // @Root only
    // @Root only
    bool writePyramidMetadataFiles( const QString& pyramidFolder ) const;
    QString getPyramidImageFilename() const; // @All

    bool writePyramidImagesRecursive( const QString& pyramidFolder ); // @All

    QRectF getImageRegionInParentImage( const QRectF& imageRegion ) const;

    void loadImageAsync(); // @All
    bool loadFullResImage(); // @Root only
    QImage getImageFromParent( const QRectF& imageRegion,
                               DynamicTexture* start ); // @Child only
    void generateTexture(); // @All

    void renderChildren( const QRectF& texCoords ); // @All
    void renderTextureBorder(); // @All
    void renderTexturedUnitQuad( const QRectF& texCoords ); // @All

    bool getThreadsDoneDescending(); // @Root


    int getGlobalThreadCount(); // @Root
    void incrementGlobalThreadCount(); // @Root

    // @TODO-Remove
    QRect getRootImageCoordinates( float x, float y, float w, float h );

    // @TODO-Remove
    /**
     * Get the region spanned by a unit rectangle {(0;0),(1;1)} in the current
     * GL view.
     * The region is in screen coordinates with the origin at the viewport's
     * top-left corner.
     * @param clampToViewportBorders Clamp to the visible part of the region.
     * @return The region in pixel units.
     */
    static QRectF getProjectedPixelRect( const bool clampToViewportBorders );
};

#endif
