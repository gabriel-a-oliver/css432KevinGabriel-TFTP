# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/olivergabe/bin/cmake

# The command to remove a file.
RM = /home/olivergabe/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/cmake-build-wsldebug

# Include any dependencies generated for this target.
include CMakeFiles/css432KevinGabriel.dir/depend.make
# Include the progress variables for this target.
include CMakeFiles/css432KevinGabriel.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/css432KevinGabriel.dir/flags.make

CMakeFiles/css432KevinGabriel.dir/server/tftpserver.cpp.o: CMakeFiles/css432KevinGabriel.dir/flags.make
CMakeFiles/css432KevinGabriel.dir/server/tftpserver.cpp.o: ../server/tftpserver.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/cmake-build-wsldebug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/css432KevinGabriel.dir/server/tftpserver.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/css432KevinGabriel.dir/server/tftpserver.cpp.o -c /mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/server/tftpserver.cpp

CMakeFiles/css432KevinGabriel.dir/server/tftpserver.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/css432KevinGabriel.dir/server/tftpserver.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/server/tftpserver.cpp > CMakeFiles/css432KevinGabriel.dir/server/tftpserver.cpp.i

CMakeFiles/css432KevinGabriel.dir/server/tftpserver.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/css432KevinGabriel.dir/server/tftpserver.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/server/tftpserver.cpp -o CMakeFiles/css432KevinGabriel.dir/server/tftpserver.cpp.s

CMakeFiles/css432KevinGabriel.dir/client/tftpclient.cpp.o: CMakeFiles/css432KevinGabriel.dir/flags.make
CMakeFiles/css432KevinGabriel.dir/client/tftpclient.cpp.o: ../client/tftpclient.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/cmake-build-wsldebug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/css432KevinGabriel.dir/client/tftpclient.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/css432KevinGabriel.dir/client/tftpclient.cpp.o -c /mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/client/tftpclient.cpp

CMakeFiles/css432KevinGabriel.dir/client/tftpclient.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/css432KevinGabriel.dir/client/tftpclient.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/client/tftpclient.cpp > CMakeFiles/css432KevinGabriel.dir/client/tftpclient.cpp.i

CMakeFiles/css432KevinGabriel.dir/client/tftpclient.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/css432KevinGabriel.dir/client/tftpclient.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/client/tftpclient.cpp -o CMakeFiles/css432KevinGabriel.dir/client/tftpclient.cpp.s

CMakeFiles/css432KevinGabriel.dir/client/tftp.cpp.o: CMakeFiles/css432KevinGabriel.dir/flags.make
CMakeFiles/css432KevinGabriel.dir/client/tftp.cpp.o: ../client/tftp.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/cmake-build-wsldebug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/css432KevinGabriel.dir/client/tftp.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/css432KevinGabriel.dir/client/tftp.cpp.o -c /mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/client/tftp.cpp

CMakeFiles/css432KevinGabriel.dir/client/tftp.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/css432KevinGabriel.dir/client/tftp.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/client/tftp.cpp > CMakeFiles/css432KevinGabriel.dir/client/tftp.cpp.i

CMakeFiles/css432KevinGabriel.dir/client/tftp.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/css432KevinGabriel.dir/client/tftp.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/client/tftp.cpp -o CMakeFiles/css432KevinGabriel.dir/client/tftp.cpp.s

# Object files for target css432KevinGabriel
css432KevinGabriel_OBJECTS = \
"CMakeFiles/css432KevinGabriel.dir/server/tftpserver.cpp.o" \
"CMakeFiles/css432KevinGabriel.dir/client/tftpclient.cpp.o" \
"CMakeFiles/css432KevinGabriel.dir/client/tftp.cpp.o"

# External object files for target css432KevinGabriel
css432KevinGabriel_EXTERNAL_OBJECTS =

css432KevinGabriel: CMakeFiles/css432KevinGabriel.dir/server/tftpserver.cpp.o
css432KevinGabriel: CMakeFiles/css432KevinGabriel.dir/client/tftpclient.cpp.o
css432KevinGabriel: CMakeFiles/css432KevinGabriel.dir/client/tftp.cpp.o
css432KevinGabriel: CMakeFiles/css432KevinGabriel.dir/build.make
css432KevinGabriel: CMakeFiles/css432KevinGabriel.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/cmake-build-wsldebug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable css432KevinGabriel"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/css432KevinGabriel.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/css432KevinGabriel.dir/build: css432KevinGabriel
.PHONY : CMakeFiles/css432KevinGabriel.dir/build

CMakeFiles/css432KevinGabriel.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/css432KevinGabriel.dir/cmake_clean.cmake
.PHONY : CMakeFiles/css432KevinGabriel.dir/clean

CMakeFiles/css432KevinGabriel.dir/depend:
	cd /mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/cmake-build-wsldebug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel /mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel /mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/cmake-build-wsldebug /mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/cmake-build-wsldebug /mnt/c/Users/olive/Documents/GitHub/css432KevinGabriel/cmake-build-wsldebug/CMakeFiles/css432KevinGabriel.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/css432KevinGabriel.dir/depend

