Release Notes {#ReleaseNotes}
============

# New in this release {#New}

DisplayCluster 0.6 introduces a new focus mode for windows.

## New Features {#NewFeatures}

* Double tapping a window bring it in focus mode, replacing the old confusing
  switch to and from interaction mode with a tap and hold.
* Animated transitions to and from the focus mode.

## Enhancements {#Enhancements}

* The handling of mouse and touch gestures has been unified and simplified.
* Improved automatic placement of windows.
* Resizing contents using the window borders has been rethought. This operation
  now adjusts the zoom level to preserve a correct pixel aspect ratio.
* Resize with the pinch gesture is now pixel correct and linear for the entire
  operation.

## Optimizations

* Movies are decoded asynchronously for more performance and responsiveness.

## Bug Fixes {#Fixes}

* Touchpads no longer generate undesired gestures on laptops.

- - -

# New in DisplayCluster 0.5

DisplayCluster 0.5 provides the following improvements:

## New Features {#NewFeatures}

* The dcStream library has been moved to a separate project:
  [Deflect](https://github.com/BlueBrain/Deflect).
* The project has been ported from Qt4 to Qt5
* Compilation of external dependencies (Deflect, TUIO) has been simplified
  thanks to the "git subproject" functionality. Dependencies are cloned during
  the CMake configure step and built alongside the project.
* Rendering of contents is done in QML for a better and simplified UI design.
* Windows have a title bar which shows the name of the file (or the stream).
* Windows have customizable action buttons which allow users to close / maximize
  windows and pause movies from the wall.
* Windows have dragable borders for reszing them from the wall.
* Synchronized buffer swap and vsync works for all types of window settings.
* Added pyramidmaker utility to create image pyramids from the command line.
* Image pyramids keep the format of the source image file instead of using jpg.
* Alpha blending (transparency) for SVGs and appropriate image types.

## Enhancements

* The startdisplaycluster script has been greatly improved and handles more MPI
  implementations (OpenMpi, MVAPICH, MPICH2, MPICH v3).
* Window move and resize operations go through a controller to constrain them
  inside a valid range.
* Resizing and zooming inside windows happens around the mouse or touch gesture
  center.
* FPS statistics of PixelStreams are per-stream instead of per-segment.
* Debug display of PixelStream segements show the absolute size of the segments.
* A few extra unit tests (StateSerialization, ContentWindowController,
  DoubleTapGesture).
* Multiple refactorings and code cleanups. All global variables have been
  removed.
* CMake cleanups, using common_library and common_application from CMake/common.
* Improved clarity of log messages

## Optimizations

* PDFs and SVGs have been optimized to move and resize much faster.
* PDFs and SVGs don't redraw constantly in zoomed mode or in settings with
  mutliple windows per node.

## Documentation

* Added a short user guide.
* Removed obsolete manual.pdf.

## Bug Fixes

* The Webbrowser widget uses the correct webpage from the configuration.
* Improved support for decoding Movies.
* Fix session loading from legacy xml.

- - -

# New in DisplayCluster 0.4

DisplayCluster 0.4 provides the following improvements:

## New Features

* Movies play synchronously across all screens.
* The launch procedure relies on a single python script that works out of the
  box on all supported platforms and with the different MPI implementations. It
  is compatible with both Python 2.x and 3.x APIs.
* dc::Stream implements an asyncSend() function for PixelStreams.
* MPIBenchmark application to measure interprocess communication performance.

## Enhancements

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

## Optimizations

* Multithreaded MPI communication between the processes brings significant
  performance improvements.
* PixelStream dispatch rate from master application is based on feedback from
  wall applications for a correct flow control.
* The handling of Content dimensions is greatly simplified and the
  ContentDimensionsRequest has been removed.

## Documentation

* Separate Introduction page from Release Notes for more clarity.

## Bug Fixes

* Fixed DynamicTexture LOD mechanism. Textures are now displayed at the correct
  resolution.
* Fixed startup crash with an empty background.
* Fixed crashes when opening incorrect State files.
* Fix crash on streaming client exit, fix signaling of new events.
* Fixed multi-source PixelStream intialization.
* All the exit(-1) calls have been replaced by std::exceptions.
* Fixed all cppcheck warnings.
* Fixed build with older FFMPEG versions.

## Known Bugs

The following bugs were known at release time:
* Sometimes, once closed the dock may refuse to open again ("Already have a
  window for stream: Dock").
* Touch events may occasionnaly pass through the active window onto the
  background.

- - -

Please file a [Bug Report](https://bbpteam.epfl.ch/project/issues/browse/DISCL)
if you find any other issue with this release.
