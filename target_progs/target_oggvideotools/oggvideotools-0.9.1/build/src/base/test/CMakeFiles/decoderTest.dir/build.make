# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build

# Include any dependencies generated for this target.
include src/base/test/CMakeFiles/decoderTest.dir/depend.make

# Include the progress variables for this target.
include src/base/test/CMakeFiles/decoderTest.dir/progress.make

# Include the compile flags for this target's objects.
include src/base/test/CMakeFiles/decoderTest.dir/flags.make

src/base/test/CMakeFiles/decoderTest.dir/decoderTest.cpp.o: src/base/test/CMakeFiles/decoderTest.dir/flags.make
src/base/test/CMakeFiles/decoderTest.dir/decoderTest.cpp.o: ../src/base/test/decoderTest.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/base/test/CMakeFiles/decoderTest.dir/decoderTest.cpp.o"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/base/test && /usr/local/bin/afl-g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/decoderTest.dir/decoderTest.cpp.o -c /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/base/test/decoderTest.cpp

src/base/test/CMakeFiles/decoderTest.dir/decoderTest.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/decoderTest.dir/decoderTest.cpp.i"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/base/test && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/base/test/decoderTest.cpp > CMakeFiles/decoderTest.dir/decoderTest.cpp.i

src/base/test/CMakeFiles/decoderTest.dir/decoderTest.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/decoderTest.dir/decoderTest.cpp.s"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/base/test && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/base/test/decoderTest.cpp -o CMakeFiles/decoderTest.dir/decoderTest.cpp.s

src/base/test/CMakeFiles/decoderTest.dir/decoderTest.cpp.o.requires:

.PHONY : src/base/test/CMakeFiles/decoderTest.dir/decoderTest.cpp.o.requires

src/base/test/CMakeFiles/decoderTest.dir/decoderTest.cpp.o.provides: src/base/test/CMakeFiles/decoderTest.dir/decoderTest.cpp.o.requires
	$(MAKE) -f src/base/test/CMakeFiles/decoderTest.dir/build.make src/base/test/CMakeFiles/decoderTest.dir/decoderTest.cpp.o.provides.build
.PHONY : src/base/test/CMakeFiles/decoderTest.dir/decoderTest.cpp.o.provides

src/base/test/CMakeFiles/decoderTest.dir/decoderTest.cpp.o.provides.build: src/base/test/CMakeFiles/decoderTest.dir/decoderTest.cpp.o


# Object files for target decoderTest
decoderTest_OBJECTS = \
"CMakeFiles/decoderTest.dir/decoderTest.cpp.o"

# External object files for target decoderTest
decoderTest_EXTERNAL_OBJECTS =

src/base/test/decoderTest: src/base/test/CMakeFiles/decoderTest.dir/decoderTest.cpp.o
src/base/test/decoderTest: src/base/test/CMakeFiles/decoderTest.dir/build.make
src/base/test/decoderTest: src/base/libovtbase.a
src/base/test/decoderTest: src/misc/libovtmisc.a
src/base/test/decoderTest: src/base/test/CMakeFiles/decoderTest.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable decoderTest"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/base/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/decoderTest.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/base/test/CMakeFiles/decoderTest.dir/build: src/base/test/decoderTest

.PHONY : src/base/test/CMakeFiles/decoderTest.dir/build

src/base/test/CMakeFiles/decoderTest.dir/requires: src/base/test/CMakeFiles/decoderTest.dir/decoderTest.cpp.o.requires

.PHONY : src/base/test/CMakeFiles/decoderTest.dir/requires

src/base/test/CMakeFiles/decoderTest.dir/clean:
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/base/test && $(CMAKE_COMMAND) -P CMakeFiles/decoderTest.dir/cmake_clean.cmake
.PHONY : src/base/test/CMakeFiles/decoderTest.dir/clean

src/base/test/CMakeFiles/decoderTest.dir/depend:
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1 /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/base/test /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/base/test /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/base/test/CMakeFiles/decoderTest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/base/test/CMakeFiles/decoderTest.dir/depend

