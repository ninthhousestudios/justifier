cmake_minimum_required(VERSION 3.14)
project(justifier_audio C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# ---------------------------------------------------------------------------
# Find Faust compiler and include directory
# ---------------------------------------------------------------------------
find_program(FAUST_EXECUTABLE faust)
if(NOT FAUST_EXECUTABLE)
    message(FATAL_ERROR
        "Faust compiler not found. Install with:\n"
        "  Arch Linux: sudo pacman -S faust\n"
        "  Ubuntu/Debian: sudo apt install faust\n"
        "  macOS: brew install faust\n"
        "  Windows: download installer from https://github.com/grame-cncm/faust/releases"
    )
endif()

execute_process(
    COMMAND ${FAUST_EXECUTABLE} --includedir
    OUTPUT_VARIABLE FAUST_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE FAUST_INCLUDEDIR_RESULT
)
if(NOT FAUST_INCLUDEDIR_RESULT EQUAL 0)
    message(FATAL_ERROR "Could not determine Faust include directory via 'faust --includedir'")
endif()
message(STATUS "Faust include dir: ${FAUST_INCLUDE_DIR}")

# ---------------------------------------------------------------------------
# Faust codegen via add_custom_command (Pattern 4 from RESEARCH.md)
# Uses -i flag to inline all library code, -cn for unique class names.
# Does NOT use -a minimal.cpp — include Faust dsp/dsp.h from include dir directly.
# ---------------------------------------------------------------------------
set(FAUST_DSP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dsp")
set(FAUST_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${FAUST_GEN_DIR})

set(FAUST_DSP_FILES sine triangle saw square pulse noise fm)

foreach(DSP_NAME ${FAUST_DSP_FILES})
    # Capitalize first letter for class name (e.g. sine -> SineDSP)
    string(SUBSTRING ${DSP_NAME} 0 1 DSP_FIRST)
    string(TOUPPER ${DSP_FIRST} DSP_FIRST_UPPER)
    string(SUBSTRING ${DSP_NAME} 1 -1 DSP_REST)
    set(DSP_CLASS_NAME "${DSP_FIRST_UPPER}${DSP_REST}DSP")

    add_custom_command(
        OUTPUT  "${FAUST_GEN_DIR}/${DSP_NAME}_dsp.cpp"
        COMMAND ${FAUST_EXECUTABLE}
                -lang cpp
                -cn ${DSP_CLASS_NAME}
                -i
                "${FAUST_DSP_DIR}/${DSP_NAME}.dsp"
                -o "${FAUST_GEN_DIR}/${DSP_NAME}_dsp.cpp"
        DEPENDS "${FAUST_DSP_DIR}/${DSP_NAME}.dsp"
        COMMENT "Compiling Faust DSP: ${DSP_NAME}.dsp -> ${DSP_CLASS_NAME}"
    )
    list(APPEND GENERATED_DSP_SOURCES "${FAUST_GEN_DIR}/${DSP_NAME}_dsp.cpp")
endforeach()

# ---------------------------------------------------------------------------
# Library target
# ---------------------------------------------------------------------------
add_library(justifier_audio STATIC
    src/audio_engine.c
    src/faust_wrapper.cpp
    ${GENERATED_DSP_SOURCES}
)

target_include_directories(justifier_audio PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${FAUST_GEN_DIR}
    ${FAUST_INCLUDE_DIR}
)

target_link_libraries(justifier_audio PRIVATE
    pthread
    m
)

# Linux: link ALSA for miniaudio ALSA backend
if(UNIX AND NOT APPLE)
    target_link_libraries(justifier_audio PRIVATE asound dl)
endif()

# macOS: link CoreAudio frameworks
if(APPLE)
    target_link_libraries(justifier_audio PRIVATE
        "-framework CoreAudio"
        "-framework AudioToolbox"
        "-framework CoreFoundation"
    )
endif()

# ---------------------------------------------------------------------------
# Test harness target
# ---------------------------------------------------------------------------
add_executable(test_audio test/test_audio.c)
target_link_libraries(test_audio PRIVATE justifier_audio)
target_include_directories(test_audio PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
# Link stdc++ because faust_wrapper.cpp and generated code are C++
target_link_libraries(test_audio PRIVATE stdc++)
