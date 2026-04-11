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

add_library(blastwave_rootio
  src/RootOutputSchema.cpp
)

target_include_directories(blastwave_rootio
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_options(blastwave_rootio
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
)

target_link_libraries(blastwave_rootio
  PUBLIC
    ROOT::Core
    ROOT::RIO
    ROOT::Tree
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
    blastwave_rootio
    ROOT::Graf
    ROOT::Gpad
    ROOT::Hist
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
    blastwave_rootio
    ROOT::Hist
)

set_target_properties(generate_blastwave_events qa_blastwave_output
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)
