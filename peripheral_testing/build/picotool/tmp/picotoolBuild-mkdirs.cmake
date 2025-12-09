# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/rishi-patil/OpenFC2040_FYP/peripheral_testing/build/_deps/picotool-src"
  "/home/rishi-patil/OpenFC2040_FYP/peripheral_testing/build/_deps/picotool-build"
  "/home/rishi-patil/OpenFC2040_FYP/peripheral_testing/build/_deps"
  "/home/rishi-patil/OpenFC2040_FYP/peripheral_testing/build/picotool/tmp"
  "/home/rishi-patil/OpenFC2040_FYP/peripheral_testing/build/picotool/src/picotoolBuild-stamp"
  "/home/rishi-patil/OpenFC2040_FYP/peripheral_testing/build/picotool/src"
  "/home/rishi-patil/OpenFC2040_FYP/peripheral_testing/build/picotool/src/picotoolBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/rishi-patil/OpenFC2040_FYP/peripheral_testing/build/picotool/src/picotoolBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/rishi-patil/OpenFC2040_FYP/peripheral_testing/build/picotool/src/picotoolBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
