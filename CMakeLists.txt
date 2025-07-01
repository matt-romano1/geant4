cmake_minimum_required(VERSION 3.16...3.27)

project(CosmicMuonTomography)

# Find required package Geant4
find_package(Geant4 REQUIRED ui_all vis_all)

# Setup Geant4 include directories and compile definitions
include(${Geant4_USE_FILE})

# Locate sources and headers for this project
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.hh)

# Add the executable, and link it to the Geant4 libraries
add_executable(cosmicMuonTomography ${sources} ${headers})
target_link_libraries(cosmicMuonTomography ${Geant4_LIBRARIES})


