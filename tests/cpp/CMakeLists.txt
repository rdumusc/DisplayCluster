
# Copyright (c) 2013-2014, EPFL/Blue Brain Project
#                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>
#
# Generates executables for the unit tests and defines a "tests" make target
# which runs all of them.

# TEST_LIBRARIES is used by CommonCTest.cmake to link tests against them
set(TEST_LIBRARIES dcmock ${DC_LIBRARIES} ${Boost_LIBRARIES})

find_package(X11)
if(NOT X11_FOUND)
  list(APPEND EXCLUDE_FROM_TESTS core/WebbrowserTests.cpp)
endif()

# Recursively compile unit tests for *.cpp files in the current folder,
# linking with TEST_LIBRARIES and excluding EXCLUDE_FROM_TESTS
include(CommonCTest)
