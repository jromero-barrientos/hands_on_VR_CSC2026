#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.16...3.27)
project(B01)

#----------------------------------------------------------------------------
# Find Geant4 package
#
find_package(Geant4 REQUIRED)

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
#
include(${Geant4_USE_FILE})

#----------------------------------------------------------------------------
# Project include directories
#
include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${Geant4_INCLUDE_DIR}
)

#----------------------------------------------------------------------------
# Explicit sources/headers (NO GLOB, to avoid editor temp files like .sys.*)
#
set(B01_SOURCES
  ${PROJECT_SOURCE_DIR}/src/B01ActionInitialization.cc
  ${PROJECT_SOURCE_DIR}/src/B01DetectorConstruction.cc
  ${PROJECT_SOURCE_DIR}/src/B01PrimaryGeneratorAction.cc
  ${PROJECT_SOURCE_DIR}/src/B01Run.cc
  ${PROJECT_SOURCE_DIR}/src/B01RunAction.cc
)

set(B01_HEADERS
  ${PROJECT_SOURCE_DIR}/include/B01ActionInitialization.hh
  ${PROJECT_SOURCE_DIR}/include/B01DetectorConstruction.hh
  ${PROJECT_SOURCE_DIR}/include/B01PrimaryGeneratorAction.hh
  ${PROJECT_SOURCE_DIR}/include/B01Run.hh
  ${PROJECT_SOURCE_DIR}/include/B01RunAction.hh
)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(exampleB01
  ${PROJECT_SOURCE_DIR}/exampleB01.cc
  ${B01_SOURCES}
  ${B01_HEADERS}
)

target_link_libraries(exampleB01 ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------
# Copy scripts to the build directory
#
set(B01_SCRIPTS
  exampleB01.out
  init_vis.mac
  vis.mac
  # run.mac
)

foreach(_script ${B01_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
  )
endforeach()

#----------------------------------------------------------------------------
# Convenience target
#
add_custom_target(B01 DEPENDS exampleB01)

#----------------------------------------------------------------------------
# Optional install (keep if you want)
#
install(TARGETS exampleB01 DESTINATION bin)
