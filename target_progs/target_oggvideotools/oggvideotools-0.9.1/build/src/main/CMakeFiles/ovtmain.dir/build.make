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
include src/main/CMakeFiles/ovtmain.dir/depend.make

# Include the progress variables for this target.
include src/main/CMakeFiles/ovtmain.dir/progress.make

# Include the compile flags for this target's objects.
include src/main/CMakeFiles/ovtmain.dir/flags.make

src/main/CMakeFiles/ovtmain.dir/audioConverter.cpp.o: src/main/CMakeFiles/ovtmain.dir/flags.make
src/main/CMakeFiles/ovtmain.dir/audioConverter.cpp.o: ../src/main/audioConverter.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/main/CMakeFiles/ovtmain.dir/audioConverter.cpp.o"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ovtmain.dir/audioConverter.cpp.o -c /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/audioConverter.cpp

src/main/CMakeFiles/ovtmain.dir/audioConverter.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ovtmain.dir/audioConverter.cpp.i"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/audioConverter.cpp > CMakeFiles/ovtmain.dir/audioConverter.cpp.i

src/main/CMakeFiles/ovtmain.dir/audioConverter.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ovtmain.dir/audioConverter.cpp.s"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/audioConverter.cpp -o CMakeFiles/ovtmain.dir/audioConverter.cpp.s

src/main/CMakeFiles/ovtmain.dir/audioConverter.cpp.o.requires:

.PHONY : src/main/CMakeFiles/ovtmain.dir/audioConverter.cpp.o.requires

src/main/CMakeFiles/ovtmain.dir/audioConverter.cpp.o.provides: src/main/CMakeFiles/ovtmain.dir/audioConverter.cpp.o.requires
	$(MAKE) -f src/main/CMakeFiles/ovtmain.dir/build.make src/main/CMakeFiles/ovtmain.dir/audioConverter.cpp.o.provides.build
.PHONY : src/main/CMakeFiles/ovtmain.dir/audioConverter.cpp.o.provides

src/main/CMakeFiles/ovtmain.dir/audioConverter.cpp.o.provides.build: src/main/CMakeFiles/ovtmain.dir/audioConverter.cpp.o


src/main/CMakeFiles/ovtmain.dir/audioHook.cpp.o: src/main/CMakeFiles/ovtmain.dir/flags.make
src/main/CMakeFiles/ovtmain.dir/audioHook.cpp.o: ../src/main/audioHook.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/main/CMakeFiles/ovtmain.dir/audioHook.cpp.o"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ovtmain.dir/audioHook.cpp.o -c /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/audioHook.cpp

src/main/CMakeFiles/ovtmain.dir/audioHook.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ovtmain.dir/audioHook.cpp.i"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/audioHook.cpp > CMakeFiles/ovtmain.dir/audioHook.cpp.i

src/main/CMakeFiles/ovtmain.dir/audioHook.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ovtmain.dir/audioHook.cpp.s"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/audioHook.cpp -o CMakeFiles/ovtmain.dir/audioHook.cpp.s

src/main/CMakeFiles/ovtmain.dir/audioHook.cpp.o.requires:

.PHONY : src/main/CMakeFiles/ovtmain.dir/audioHook.cpp.o.requires

src/main/CMakeFiles/ovtmain.dir/audioHook.cpp.o.provides: src/main/CMakeFiles/ovtmain.dir/audioHook.cpp.o.requires
	$(MAKE) -f src/main/CMakeFiles/ovtmain.dir/build.make src/main/CMakeFiles/ovtmain.dir/audioHook.cpp.o.provides.build
.PHONY : src/main/CMakeFiles/ovtmain.dir/audioHook.cpp.o.provides

src/main/CMakeFiles/ovtmain.dir/audioHook.cpp.o.provides.build: src/main/CMakeFiles/ovtmain.dir/audioHook.cpp.o


src/main/CMakeFiles/ovtmain.dir/hookHandler.cpp.o: src/main/CMakeFiles/ovtmain.dir/flags.make
src/main/CMakeFiles/ovtmain.dir/hookHandler.cpp.o: ../src/main/hookHandler.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/main/CMakeFiles/ovtmain.dir/hookHandler.cpp.o"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ovtmain.dir/hookHandler.cpp.o -c /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/hookHandler.cpp

src/main/CMakeFiles/ovtmain.dir/hookHandler.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ovtmain.dir/hookHandler.cpp.i"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/hookHandler.cpp > CMakeFiles/ovtmain.dir/hookHandler.cpp.i

src/main/CMakeFiles/ovtmain.dir/hookHandler.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ovtmain.dir/hookHandler.cpp.s"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/hookHandler.cpp -o CMakeFiles/ovtmain.dir/hookHandler.cpp.s

src/main/CMakeFiles/ovtmain.dir/hookHandler.cpp.o.requires:

.PHONY : src/main/CMakeFiles/ovtmain.dir/hookHandler.cpp.o.requires

src/main/CMakeFiles/ovtmain.dir/hookHandler.cpp.o.provides: src/main/CMakeFiles/ovtmain.dir/hookHandler.cpp.o.requires
	$(MAKE) -f src/main/CMakeFiles/ovtmain.dir/build.make src/main/CMakeFiles/ovtmain.dir/hookHandler.cpp.o.provides.build
.PHONY : src/main/CMakeFiles/ovtmain.dir/hookHandler.cpp.o.provides

src/main/CMakeFiles/ovtmain.dir/hookHandler.cpp.o.provides.build: src/main/CMakeFiles/ovtmain.dir/hookHandler.cpp.o


src/main/CMakeFiles/ovtmain.dir/videoHook.cpp.o: src/main/CMakeFiles/ovtmain.dir/flags.make
src/main/CMakeFiles/ovtmain.dir/videoHook.cpp.o: ../src/main/videoHook.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/main/CMakeFiles/ovtmain.dir/videoHook.cpp.o"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ovtmain.dir/videoHook.cpp.o -c /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/videoHook.cpp

src/main/CMakeFiles/ovtmain.dir/videoHook.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ovtmain.dir/videoHook.cpp.i"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/videoHook.cpp > CMakeFiles/ovtmain.dir/videoHook.cpp.i

src/main/CMakeFiles/ovtmain.dir/videoHook.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ovtmain.dir/videoHook.cpp.s"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/videoHook.cpp -o CMakeFiles/ovtmain.dir/videoHook.cpp.s

src/main/CMakeFiles/ovtmain.dir/videoHook.cpp.o.requires:

.PHONY : src/main/CMakeFiles/ovtmain.dir/videoHook.cpp.o.requires

src/main/CMakeFiles/ovtmain.dir/videoHook.cpp.o.provides: src/main/CMakeFiles/ovtmain.dir/videoHook.cpp.o.requires
	$(MAKE) -f src/main/CMakeFiles/ovtmain.dir/build.make src/main/CMakeFiles/ovtmain.dir/videoHook.cpp.o.provides.build
.PHONY : src/main/CMakeFiles/ovtmain.dir/videoHook.cpp.o.provides

src/main/CMakeFiles/ovtmain.dir/videoHook.cpp.o.provides.build: src/main/CMakeFiles/ovtmain.dir/videoHook.cpp.o


src/main/CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.o: src/main/CMakeFiles/ovtmain.dir/flags.make
src/main/CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.o: ../src/main/oggBOSExtractorFactory.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object src/main/CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.o"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.o -c /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/oggBOSExtractorFactory.cpp

src/main/CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.i"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/oggBOSExtractorFactory.cpp > CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.i

src/main/CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.s"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/oggBOSExtractorFactory.cpp -o CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.s

src/main/CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.o.requires:

.PHONY : src/main/CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.o.requires

src/main/CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.o.provides: src/main/CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.o.requires
	$(MAKE) -f src/main/CMakeFiles/ovtmain.dir/build.make src/main/CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.o.provides.build
.PHONY : src/main/CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.o.provides

src/main/CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.o.provides.build: src/main/CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.o


src/main/CMakeFiles/ovtmain.dir/streamMux.cpp.o: src/main/CMakeFiles/ovtmain.dir/flags.make
src/main/CMakeFiles/ovtmain.dir/streamMux.cpp.o: ../src/main/streamMux.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object src/main/CMakeFiles/ovtmain.dir/streamMux.cpp.o"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ovtmain.dir/streamMux.cpp.o -c /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/streamMux.cpp

src/main/CMakeFiles/ovtmain.dir/streamMux.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ovtmain.dir/streamMux.cpp.i"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/streamMux.cpp > CMakeFiles/ovtmain.dir/streamMux.cpp.i

src/main/CMakeFiles/ovtmain.dir/streamMux.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ovtmain.dir/streamMux.cpp.s"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/streamMux.cpp -o CMakeFiles/ovtmain.dir/streamMux.cpp.s

src/main/CMakeFiles/ovtmain.dir/streamMux.cpp.o.requires:

.PHONY : src/main/CMakeFiles/ovtmain.dir/streamMux.cpp.o.requires

src/main/CMakeFiles/ovtmain.dir/streamMux.cpp.o.provides: src/main/CMakeFiles/ovtmain.dir/streamMux.cpp.o.requires
	$(MAKE) -f src/main/CMakeFiles/ovtmain.dir/build.make src/main/CMakeFiles/ovtmain.dir/streamMux.cpp.o.provides.build
.PHONY : src/main/CMakeFiles/ovtmain.dir/streamMux.cpp.o.provides

src/main/CMakeFiles/ovtmain.dir/streamMux.cpp.o.provides.build: src/main/CMakeFiles/ovtmain.dir/streamMux.cpp.o


src/main/CMakeFiles/ovtmain.dir/streamSerializer.cpp.o: src/main/CMakeFiles/ovtmain.dir/flags.make
src/main/CMakeFiles/ovtmain.dir/streamSerializer.cpp.o: ../src/main/streamSerializer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object src/main/CMakeFiles/ovtmain.dir/streamSerializer.cpp.o"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ovtmain.dir/streamSerializer.cpp.o -c /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/streamSerializer.cpp

src/main/CMakeFiles/ovtmain.dir/streamSerializer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ovtmain.dir/streamSerializer.cpp.i"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/streamSerializer.cpp > CMakeFiles/ovtmain.dir/streamSerializer.cpp.i

src/main/CMakeFiles/ovtmain.dir/streamSerializer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ovtmain.dir/streamSerializer.cpp.s"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/streamSerializer.cpp -o CMakeFiles/ovtmain.dir/streamSerializer.cpp.s

src/main/CMakeFiles/ovtmain.dir/streamSerializer.cpp.o.requires:

.PHONY : src/main/CMakeFiles/ovtmain.dir/streamSerializer.cpp.o.requires

src/main/CMakeFiles/ovtmain.dir/streamSerializer.cpp.o.provides: src/main/CMakeFiles/ovtmain.dir/streamSerializer.cpp.o.requires
	$(MAKE) -f src/main/CMakeFiles/ovtmain.dir/build.make src/main/CMakeFiles/ovtmain.dir/streamSerializer.cpp.o.provides.build
.PHONY : src/main/CMakeFiles/ovtmain.dir/streamSerializer.cpp.o.provides

src/main/CMakeFiles/ovtmain.dir/streamSerializer.cpp.o.provides.build: src/main/CMakeFiles/ovtmain.dir/streamSerializer.cpp.o


src/main/CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.o: src/main/CMakeFiles/ovtmain.dir/flags.make
src/main/CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.o: ../src/main/cmdlineextractor.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object src/main/CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.o"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.o -c /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/cmdlineextractor.cpp

src/main/CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.i"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/cmdlineextractor.cpp > CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.i

src/main/CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.s"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && /usr/local/bin/afl-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main/cmdlineextractor.cpp -o CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.s

src/main/CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.o.requires:

.PHONY : src/main/CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.o.requires

src/main/CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.o.provides: src/main/CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.o.requires
	$(MAKE) -f src/main/CMakeFiles/ovtmain.dir/build.make src/main/CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.o.provides.build
.PHONY : src/main/CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.o.provides

src/main/CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.o.provides.build: src/main/CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.o


# Object files for target ovtmain
ovtmain_OBJECTS = \
"CMakeFiles/ovtmain.dir/audioConverter.cpp.o" \
"CMakeFiles/ovtmain.dir/audioHook.cpp.o" \
"CMakeFiles/ovtmain.dir/hookHandler.cpp.o" \
"CMakeFiles/ovtmain.dir/videoHook.cpp.o" \
"CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.o" \
"CMakeFiles/ovtmain.dir/streamMux.cpp.o" \
"CMakeFiles/ovtmain.dir/streamSerializer.cpp.o" \
"CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.o"

# External object files for target ovtmain
ovtmain_EXTERNAL_OBJECTS =

src/main/libovtmain.a: src/main/CMakeFiles/ovtmain.dir/audioConverter.cpp.o
src/main/libovtmain.a: src/main/CMakeFiles/ovtmain.dir/audioHook.cpp.o
src/main/libovtmain.a: src/main/CMakeFiles/ovtmain.dir/hookHandler.cpp.o
src/main/libovtmain.a: src/main/CMakeFiles/ovtmain.dir/videoHook.cpp.o
src/main/libovtmain.a: src/main/CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.o
src/main/libovtmain.a: src/main/CMakeFiles/ovtmain.dir/streamMux.cpp.o
src/main/libovtmain.a: src/main/CMakeFiles/ovtmain.dir/streamSerializer.cpp.o
src/main/libovtmain.a: src/main/CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.o
src/main/libovtmain.a: src/main/CMakeFiles/ovtmain.dir/build.make
src/main/libovtmain.a: src/main/CMakeFiles/ovtmain.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Linking CXX static library libovtmain.a"
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && $(CMAKE_COMMAND) -P CMakeFiles/ovtmain.dir/cmake_clean_target.cmake
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ovtmain.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/main/CMakeFiles/ovtmain.dir/build: src/main/libovtmain.a

.PHONY : src/main/CMakeFiles/ovtmain.dir/build

src/main/CMakeFiles/ovtmain.dir/requires: src/main/CMakeFiles/ovtmain.dir/audioConverter.cpp.o.requires
src/main/CMakeFiles/ovtmain.dir/requires: src/main/CMakeFiles/ovtmain.dir/audioHook.cpp.o.requires
src/main/CMakeFiles/ovtmain.dir/requires: src/main/CMakeFiles/ovtmain.dir/hookHandler.cpp.o.requires
src/main/CMakeFiles/ovtmain.dir/requires: src/main/CMakeFiles/ovtmain.dir/videoHook.cpp.o.requires
src/main/CMakeFiles/ovtmain.dir/requires: src/main/CMakeFiles/ovtmain.dir/oggBOSExtractorFactory.cpp.o.requires
src/main/CMakeFiles/ovtmain.dir/requires: src/main/CMakeFiles/ovtmain.dir/streamMux.cpp.o.requires
src/main/CMakeFiles/ovtmain.dir/requires: src/main/CMakeFiles/ovtmain.dir/streamSerializer.cpp.o.requires
src/main/CMakeFiles/ovtmain.dir/requires: src/main/CMakeFiles/ovtmain.dir/cmdlineextractor.cpp.o.requires

.PHONY : src/main/CMakeFiles/ovtmain.dir/requires

src/main/CMakeFiles/ovtmain.dir/clean:
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main && $(CMAKE_COMMAND) -P CMakeFiles/ovtmain.dir/cmake_clean.cmake
.PHONY : src/main/CMakeFiles/ovtmain.dir/clean

src/main/CMakeFiles/ovtmain.dir/depend:
	cd /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1 /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/src/main /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main /home/wws/Music/Fuzz/target_progs/target_oggvideotools/oggvideotools-0.9.1/build/src/main/CMakeFiles/ovtmain.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/main/CMakeFiles/ovtmain.dir/depend

