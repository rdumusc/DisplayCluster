Release Notes {#ReleaseNotes}
============

# New in this release {#New}

DisplayCluster 0.4 provides the following improvements:

## New Features {#NewFeatures}

* Movies play synchronously across all screens.
* The launch procedure relies on a single python script that works out of the
  box on all supported platforms and with the different MPI implementations. It
  is compatible with both Python 2.x and 3.x APIs.
* dc::Stream implements an asyncSend() function for PixelStreams.
* MPIBenchmark application to measure interprocess communication performance.

## Enhancements {#Enhancements}

* DesktopStreamer properly handles AppNap on OSX 10.9.
* DesktopStreamer detects Retina displays automatically (no retina checkbox).
* ImagePyramids use a correct aspect ratio instead of a square size, leading to
  a significant reduction in disk space usage for newly generated pyramids.
* MasterWindow UI menus have been reorganized. Help -> about shows the git
  revision used for the build.
* The DisplayGroup, ContentWindow and related classes have undergone
  profound refactorings and now follow an MVC pattern.
* Use of global variables has been drastically reduced.
* The CMakeLists.txt follows common Eyescale/CMake conventions.
* MPI message headers are now separate from dc::Stream message headers.
* The OpenGL rendering code has been improved, but still needs more attention.

## Optimizations {#Optimizations}

* Multithreaded MPI communication between the processes brings significant
  performance improvements.
* PixelStream dispatch rate from master application is based on feedback from
  wall applications for a correct flow control.
* The handling of Content dimensions is greatly simplified and the
  ContentDimensionsRequest has been removed.

## Documentation {#Documentation}

* Separate Introduction page from Release Notes for more clarity.

## Bug Fixes {#Fixes}

* Fixed DynamicTexture LOD mechanism. Textures are now displayed at the correct
  resolution.
* Fixed startup crash with an empty background.
* Fixed crashes when opening incorrect State files.
* Fix crash on streaming client exit, fix signaling of new events.
* Fixed multi-source PixelStream intialization.
* All the exit(-1) calls have been replaced by std::exceptions.
* Fixed all cppcheck warnings.
* Fixed build with older FFMPEG versions.

## Known Bugs {#Bugs}

The following bugs were known at release time:
* Sometimes, once closed the dock may refuse to open again ("Already have a
  window for stream: Dock").
* Touch events may occasionnaly pass through the active window onto the
  background.

Please file a [Bug Report](https://bbpteam.epfl.ch/project/issues/browse/DISCL)
if you find any other issue with this release.

- - -

# Errata {#Errata}

* Post-release hot fixes go here
