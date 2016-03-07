# Set the minimum version of CMake that is required to build.
cmake_minimum_required(VERSION 2.8)

# Maps to Visual Studio solution file (Phantom3D.sln).
# The solution will have all targets (exe, lib, dll) 
# as Visual Studio projects (.vcproj).
project(PhotonVM)

# Set the version of the engine.
set(PHOTONVM_VERSION 1.0.0)
message("Building PhotonVM V" ${PHOTONVM_VERSION} " - (c) Niklas Grabowski 2015")

# Determine the architecture of the building mashine.
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(SYSTEM_ARCH "x64")
	message("Building for x64 architecture.")
else()
	set(SYSTEM_ARCH "x86")
	message("Building for x86 architecture.")
endif()

# Add the _DEBUG macro if needed. Only on GCC/Clang; No MSVC.
if(NOT WIN32)
	if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
		message("Debug Build. Generating debug data.")
		add_definitions(-D_DEBUG)
	else()
		message("Non Debug Build. No debugging data will be generated!")
	endif()
endif(NOT WIN32)

# Set the output directory of the project.
if(NOT PHOTONVM_OUTPUT_DIR)
	set(PHOTONVM_OUTPUT_DIR ${PhotonVM_SOURCE_DIR}/bin
	CACHE PATH "Choose the root directory that the output will be installed to.")
	message("No PHOTONVM_OUTPUT_DIR specified, defaulting to ${PHOTONVM_OUTPUT_DIR}")
endif(NOT PHOTONVM_OUTPUT_DIR)


# Enable the ability to create folders to organize projects (.vcproj).
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set compiler flags and options.
# Here it is setting the Visual Studio warning level to 3.
# Also we enable the minimal rebuild option and enable the fast floating point model.
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /Zi /fp:fast")
elseif(APPLE)
    # Do nothing
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wno-parentheses -Wno-ignored-qualifiers -Winit-self -Wno-reorder -Wno-attributes -Wno-overloaded-virtual -Wcast-qual -Wextra -Wno-unused-parameter -Wno-write-strings -Wno-missing-field-initializers -Wno-long-long -Wno-unused-variable -fPIC")
    set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
endif()

# Remove the /machine:X86 option from the linker on MSVC.
if(MSVC AND SYSTEM_ARCH STREQUAL "x64")
	string(REPLACE "/machine:X86" "" CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})
	string(REPLACE "/machine:X86" "" CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS})
endif()

# Command to output information to the console.
# Useful for displaying errors, warnings and debugging.
message("cxx Flags: " ${CMAKE_CXX_FLAGS})
message("linker Flags: " ${CMAKE_EXE_LINKER_FLAGS} " shared Flags:" ${CMAKE_SHARED_LINKER_FLAGS})

# Add the sub-directories where the other sub-project CMakeLists.txt exist.
add_subdirectory(src)
