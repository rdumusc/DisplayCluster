Documentation {#documentation}
============

This document describes the basic structure and provides pointers to
auxilary documentation.

## Directory Layout

* [CMake](https://github.com/Eyescale/CMake#readme): subdirectory
  included using git externals. See below for details.
* src: Contains the main libraries of the project:
  * core: The core library.
  * dcWebservice: Accepts external commands through the FastCGI protocol.
  * dcstream: Enables streaming pixel content from external applications.
* apps: Applications delivered with the project.
  * DesktopStreamer: A small utility based on dcstream that lets you stream your desktop.
  * DisplayCluster: The main application.
  * LocalStreamer: Used by DisplayCluster to generate content from separate processes (sandboxing).
  * SimpleStreamer: A simple example application which uses the dcstream library.
* tests: Unit tests
* doc: Doxygen and other documentation.
* examples: Example xml configuration files, installed under share/DisplayCluster.
* webservice: Configuration files and static content for the webservice.

## CMakeLists

The top-level CMakeLists is relatively simple due to the delegation of
details into the CMake external. It starts with the project setup which
defines the project name and includes the CMake/common git external.

## CMake

All BBP projects rely on a common
[CMake repository](https://github.com/Eyescale/CMake) which provides
sensible defaults for compilation, documentation and packaging. It is
integrated as a CMake/common subtree as described in the
[Readme](https://github.com/Eyescale/CMake#readme).

## Unit tests

Unit tests are very important. Take a look at the
[coverage report](CoverageReport/index.html).
