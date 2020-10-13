# Install script for directory: /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/docs

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/wws/Music/Fuzz/target_progs/target_oggvideotools/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/man/man1" TYPE FILE FILES
    "/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/docs/oggTranscode.1"
    "/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/docs/oggSlideshow.1"
    "/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/docs/oggThumb.1"
    "/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/docs/oggSplit.1"
    "/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/docs/oggJoin.1"
    "/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/docs/oggCut.1"
    "/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/docs/oggCat.1"
    "/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/docs/oggSilence.1"
    "/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/docs/oggDump.1"
    "/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/docs/oggLength.1"
    "/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/docs/mkThumbs.1"
    )
endif()

