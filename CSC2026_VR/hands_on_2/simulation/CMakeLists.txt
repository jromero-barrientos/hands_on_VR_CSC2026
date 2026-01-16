#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.16...3.27)
project(GB02project)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
  find_package(Geant4 REQUIRED)
  
#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
#
include(${Geant4_USE_FILE})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
include_directories(${PROJECT_SOURCE_DIR}/include 
                    ${Geant4_INCLUDE_DIR})

#----------------------------------------------------------------------------
# Explicit sources/headers (NO GLOB, to avoid editor temp files like .sys.*)
#
set(GB02project_SOURCES
  ${PROJECT_SOURCE_DIR}/src/GB02ActionInitialization.cc
  ${PROJECT_SOURCE_DIR}/src/GB02DetectorConstruction.cc
  ${PROJECT_SOURCE_DIR}/src/GB02PrimaryGeneratorAction.cc
  ${PROJECT_SOURCE_DIR}/src/GB02SteppingAction.cc
  ${PROJECT_SOURCE_DIR}/src/GB02EventAction.cc
  ${PROJECT_SOURCE_DIR}/src/GB02BOptrMultiParticleForceCollision.cc
  ${PROJECT_SOURCE_DIR}/src/GB02RunAction.cc
)

set(GB02project_HEADERS
  ${PROJECT_SOURCE_DIR}/include/GB02ActionInitialization.hh
  ${PROJECT_SOURCE_DIR}/include/GB02DetectorConstruction.hh
  ${PROJECT_SOURCE_DIR}/include/GB02PrimaryGeneratorAction.hh
  ${PROJECT_SOURCE_DIR}/include/GB02SteppingAction.hh
  ${PROJECT_SOURCE_DIR}/include/GB02EventAction.hh
  ${PROJECT_SOURCE_DIR}/include/GB02BOptrMultiParticleForceCollision.hh
  ${PROJECT_SOURCE_DIR}/include/GB02RunAction.hh
)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(exampleGB02 exampleGB02.cc ${GB02project_SOURCES} ${GB02project_HEADERS})
target_link_libraries(exampleGB02 ${Geant4_LIBRARIES} )

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we build
# GB02project. This is so that we can run the executable directly
# because it relies on these scripts being in the current working directory.
#
set(GB02project_SCRIPTS
  exampleGB02.in
  exampleGB02.out
  run_10k_seedA.mac
  run_10k_seedB.mac
  run_10k_seedC.mac
  run_100k_seedA.mac
  run_50k_seedA.mac
  run_30k_seedA.mac
  )

foreach(_script ${GB02project_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()

#----------------------------------------------------------------------------
# Add program to the project targets
# (this avoids the need of typing the program name after make)
#
add_custom_target(GB02project DEPENDS exampleGB02)

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS exampleGB02 DESTINATION bin)

