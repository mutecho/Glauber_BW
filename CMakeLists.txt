cmake_minimum_required(VERSION 3.20)

project(BlastWave LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ROOT CONFIG REQUIRED COMPONENTS Core Graf Gpad Hist RIO Tree)

add_library(blastwave_core
  src/BlastWaveGenerator.cpp
)

target_include_directories(blastwave_core
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_options(blastwave_core
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
)

add_executable(generate_blastwave_events
  apps/generate_blastwave_events.cpp
)

target_compile_options(generate_blastwave_events
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
)

target_link_libraries(generate_blastwave_events
  PRIVATE
    blastwave_core
    ROOT::Core
    ROOT::Graf
    ROOT::Gpad
    ROOT::Hist
    ROOT::RIO
    ROOT::Tree
)

add_executable(qa_blastwave_output
  apps/qa_blastwave_output.cpp
)

target_compile_options(qa_blastwave_output
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
)

target_link_libraries(qa_blastwave_output
  PRIVATE
    ROOT::Core
    ROOT::Hist
    ROOT::RIO
    ROOT::Tree
)

set_target_properties(generate_blastwave_events qa_blastwave_output
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)
