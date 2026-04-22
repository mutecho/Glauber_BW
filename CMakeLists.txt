cmake_minimum_required(VERSION 3.20)

project(BlastWave LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

find_package(ROOT CONFIG REQUIRED COMPONENTS Core Graf Gpad Hist RIO Tree)

add_library(blastwave_core
  src/BlastWaveGenerator.cpp
  src/BlastWaveGeneratorGeometry.cpp
  src/BlastWaveGeneratorSampling.cpp
  src/BlastWaveGeneratorValidation.cpp
  src/FlowFieldModel.cpp
  src/MaxwellJuttnerMomentumSampler.cpp
  src/PhysicsUtils.cpp
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
  src/OutputPathUtils.cpp
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
  apps/generate_blastwave/ProgressReporter.cpp
  apps/generate_blastwave/RootEventFileWriter.cpp
  apps/generate_blastwave/RunOptions.cpp
  apps/generate_blastwave_events.cpp
)

target_include_directories(generate_blastwave_events
  PRIVATE
    ${PROJECT_SOURCE_DIR}/apps
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
    blastwave_core
    blastwave_rootio
    ROOT::Hist
)

add_executable(test_maxwell_juttner_sampler
  tests/MaxwellJuttnerMomentumSamplerTest.cpp
)

target_compile_options(test_maxwell_juttner_sampler
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
)

target_link_libraries(test_maxwell_juttner_sampler
  PRIVATE
    blastwave_core
)

add_test(
  NAME test_maxwell_juttner_sampler
  COMMAND $<TARGET_FILE:test_maxwell_juttner_sampler>
)

add_executable(test_output_path_utils
  tests/OutputPathUtilsTest.cpp
)

target_compile_options(test_output_path_utils
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
)

target_link_libraries(test_output_path_utils
  PRIVATE
    blastwave_rootio
)

add_test(
  NAME test_output_path_utils
  COMMAND $<TARGET_FILE:test_output_path_utils>
)

add_executable(test_flow_field_model
  tests/FlowFieldModelTest.cpp
)

target_compile_options(test_flow_field_model
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
)

target_link_libraries(test_flow_field_model
  PRIVATE
    blastwave_core
)

add_test(
  NAME test_flow_field_model
  COMMAND $<TARGET_FILE:test_flow_field_model>
)

add_executable(test_physics_utils
  tests/PhysicsUtilsTest.cpp
)

target_compile_options(test_physics_utils
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
)

target_link_libraries(test_physics_utils
  PRIVATE
    blastwave_core
)

add_test(
  NAME test_physics_utils
  COMMAND $<TARGET_FILE:test_physics_utils>
)

set_target_properties(generate_blastwave_events qa_blastwave_output test_maxwell_juttner_sampler test_output_path_utils test_flow_field_model test_physics_utils
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)
