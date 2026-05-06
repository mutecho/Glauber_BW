cmake_minimum_required(VERSION 3.20)

project(BlastWave LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

# The generator writes saved canvases, and the differential-flow payload uses
# TVectorD metadata plus histogram painters, so those ROOT components must be
# linked eagerly instead of relying on delayed autoload.
find_package(ROOT CONFIG REQUIRED COMPONENTS Core Graf Gpad Hist HistPainter Matrix RIO Tree)

add_library(blastwave_core
  src/BlastWaveGenerator.cpp
  src/BlastWaveGeneratorGeometry.cpp
  src/BlastWaveGeneratorSampling.cpp
  src/BlastWaveGeneratorValidation.cpp
  src/EmissionSampler.cpp
  src/EventMedium.cpp
  src/FlowFieldDensity.cpp
  src/FlowFieldGeometry.cpp
  src/FlowFieldModel.cpp
  src/MaxwellJuttnerMomentumSampler.cpp
  src/PhysicsUtils.cpp
  src/DifferentialFlowCumulant.cpp
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
  src/DifferentialFlowRootPayload.cpp
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
    ROOT::Graf
    ROOT::Gpad
    ROOT::Hist
    ROOT::HistPainter
    ROOT::Matrix
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
    ROOT::HistPainter
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
)

add_executable(analyze_blastwave_vnpt
  apps/analyze_blastwave_vnpt.cpp
)

target_compile_options(analyze_blastwave_vnpt
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
)

target_link_libraries(analyze_blastwave_vnpt
  PRIVATE
    blastwave_core
    blastwave_rootio
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

add_executable(test_emission_sampler
  tests/EmissionSamplerTest.cpp
)

target_compile_options(test_emission_sampler
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
)

target_link_libraries(test_emission_sampler
  PRIVATE
    blastwave_core
)

add_test(
  NAME test_emission_sampler
  COMMAND $<TARGET_FILE:test_emission_sampler>
)

add_executable(test_run_options
  apps/generate_blastwave/RunOptions.cpp
  tests/RunOptionsTest.cpp
)

target_include_directories(test_run_options
  PRIVATE
    ${PROJECT_SOURCE_DIR}/apps
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_options(test_run_options
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
)

target_link_libraries(test_run_options
  PRIVATE
    blastwave_core
)

add_test(
  NAME test_run_options
  COMMAND $<TARGET_FILE:test_run_options>
)

add_executable(test_differential_flow_cumulant
  tests/DifferentialFlowCumulantTest.cpp
)

target_compile_options(test_differential_flow_cumulant
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
)

target_link_libraries(test_differential_flow_cumulant
  PRIVATE
    blastwave_core
)

add_test(
  NAME test_differential_flow_cumulant
  COMMAND $<TARGET_FILE:test_differential_flow_cumulant>
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

add_executable(test_harmonic_geometry
  tests/HarmonicGeometryTest.cpp
)

target_compile_options(test_harmonic_geometry
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
)

target_link_libraries(test_harmonic_geometry
  PRIVATE
    blastwave_core
)

add_test(
  NAME test_harmonic_geometry
  COMMAND $<TARGET_FILE:test_harmonic_geometry>
)

add_executable(test_blast_wave_generator_response
  tests/BlastWaveGeneratorResponseTest.cpp
)

target_compile_options(test_blast_wave_generator_response
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
)

target_link_libraries(test_blast_wave_generator_response
  PRIVATE
    blastwave_core
)

add_test(
  NAME test_blast_wave_generator_response
  COMMAND $<TARGET_FILE:test_blast_wave_generator_response>
)

set_target_properties(generate_blastwave_events qa_blastwave_output analyze_blastwave_vnpt test_maxwell_juttner_sampler test_output_path_utils test_flow_field_model test_emission_sampler test_run_options test_differential_flow_cumulant test_physics_utils test_harmonic_geometry test_blast_wave_generator_response
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)
