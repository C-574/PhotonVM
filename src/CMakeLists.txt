# Collect all files for the commandline tool.
file(GLOB PHOTONVM_COMMANDLINE_SOURCES 
	"*.h" 
	"*.cpp")

# Add the files from the include directory to the project.
file(GLOB PHOTONVM_HEADERS 
	"../include/*.h" 
	"../include/*.inl")
	
# Create named folders for the sources within the project file.
# Empty name lists them directly in the root of the .vcproj in a VS project.
source_group("" FILES ${PHOTONVM_COMMANDLINE_SOURCES})
source_group("Photon VM" FILES ${PHOTONVM_HEADERS})

# Set a macro for all sources.
set(PHOTONVM_PROJECT_SOURCES ${PHOTONVM_COMMANDLINE_SOURCES} 
						${PHOTONVM_HEADERS})

# Set the include paths of the project.
include_directories(. 
	${CMAKE_SOURCE_DIR}/include)

# Setup the actual executable output.
add_executable(PvmCmd ${PHOTONVM_PROJECT_SOURCES})


# Creates folder "PhotonVM" and adds target project (PhotonVM.vcproj)	
set_property(TARGET PvmCmd PROPERTY FOLDER "PhotonVM")

# Set some properties for the project such as output name.
set_target_properties(PvmCmd PROPERTIES 
	PREFIX ""
	OUTPUT_NAME "pvm" 
	CLEAN_DIRECT_OUTPUT 1)

# Adds logic to INSTALL.vcproj to copy pvm.exe to destination directory.
install(TARGETS PvmCmd 
		EXPORT DESTINATION ${PHOTONVM_OUTPUT_DIR})
