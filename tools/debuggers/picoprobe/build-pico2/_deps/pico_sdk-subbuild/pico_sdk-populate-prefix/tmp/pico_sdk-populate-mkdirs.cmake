# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/rishi-patil/Desktop/FYP/picoprobe/build-pico2/_deps/pico_sdk-src"
  "/home/rishi-patil/Desktop/FYP/picoprobe/build-pico2/_deps/pico_sdk-build"
  "/home/rishi-patil/Desktop/FYP/picoprobe/build-pico2/_deps/pico_sdk-subbuild/pico_sdk-populate-prefix"
  "/home/rishi-patil/Desktop/FYP/picoprobe/build-pico2/_deps/pico_sdk-subbuild/pico_sdk-populate-prefix/tmp"
  "/home/rishi-patil/Desktop/FYP/picoprobe/build-pico2/_deps/pico_sdk-subbuild/pico_sdk-populate-prefix/src/pico_sdk-populate-stamp"
  "/home/rishi-patil/Desktop/FYP/picoprobe/build-pico2/_deps/pico_sdk-subbuild/pico_sdk-populate-prefix/src"
  "/home/rishi-patil/Desktop/FYP/picoprobe/build-pico2/_deps/pico_sdk-subbuild/pico_sdk-populate-prefix/src/pico_sdk-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/rishi-patil/Desktop/FYP/picoprobe/build-pico2/_deps/pico_sdk-subbuild/pico_sdk-populate-prefix/src/pico_sdk-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/rishi-patil/Desktop/FYP/picoprobe/build-pico2/_deps/pico_sdk-subbuild/pico_sdk-populate-prefix/src/pico_sdk-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
