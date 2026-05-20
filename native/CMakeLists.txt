cmake_minimum_required(VERSION 3.14)
project(justifier_audio C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# ---------------------------------------------------------------------------
# Faust codegen — optional regeneration from .dsp sources
# ---------------------------------------------------------------------------
option(JUSTIFIER_REGEN_FAUST "Re-run Faust compiler to regenerate DSP C++ files" OFF)

set(FAUST_DSP_FILES sine triangle saw square pulse white_noise pink_noise brown_noise lfnoise0 lfnoise1 lfnoise2 fm reverb delay chorus phaser flanger parametric_eq saturation)
set(FAUST_COMMITTED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/generated")

if(JUSTIFIER_REGEN_FAUST)
    find_program(FAUST_EXECUTABLE faust)
    if(NOT FAUST_EXECUTABLE)
        message(FATAL_ERROR
            "Faust compiler not found (required with -DJUSTIFIER_REGEN_FAUST=ON). Install with:\n"
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
    file(TO_CMAKE_PATH "${FAUST_INCLUDE_DIR}" FAUST_INCLUDE_DIR)
    message(STATUS "Faust include dir: ${FAUST_INCLUDE_DIR}")

    set(FAUST_DSP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dsp")
    set(FAUST_GEN_DIR "${FAUST_COMMITTED_DIR}")

    foreach(DSP_NAME ${FAUST_DSP_FILES})
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
else()
    # Use pre-committed generated files — no Faust compiler needed
    message(STATUS "Using pre-committed Faust C++ files from src/generated/")

    # Locate Faust headers: check system paths, fall back to bundled if needed
    find_path(FAUST_INCLUDE_DIR faust/dsp/dsp.h
        HINTS /usr/include /usr/local/include $ENV{HOME}/.local/include
    )
    if(NOT FAUST_INCLUDE_DIR)
        message(FATAL_ERROR "Faust headers (faust/dsp/dsp.h) not found. Install faust development headers.")
    endif()
    message(STATUS "Faust include dir: ${FAUST_INCLUDE_DIR}")

    set(FAUST_GEN_DIR "${FAUST_COMMITTED_DIR}")
endif()

# ---------------------------------------------------------------------------
# Library target
# ---------------------------------------------------------------------------
add_library(justifier_audio SHARED
    src/audio_engine.cpp
    src/faust_wrapper.cpp
)

# Only export justifier_* symbols, hide everything else
set_target_properties(justifier_audio PROPERTIES
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# Mark this TU as the library build (triggers dllexport on Windows)
target_compile_definitions(justifier_audio PRIVATE JUSTIFIER_BUILDING)



# Generated DSP files are #included directly by faust_wrapper.cpp, not compiled
# as independent TUs. They depend on Faust base class headers (dsp::dsp, UI, Meta)
# which faust_wrapper.cpp includes first.
if(JUSTIFIER_REGEN_FAUST)
    add_custom_target(generate_faust_dsp DEPENDS ${GENERATED_DSP_SOURCES})
    add_dependencies(justifier_audio generate_faust_dsp)
endif()

target_include_directories(justifier_audio PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${FAUST_GEN_DIR}
    ${FAUST_INCLUDE_DIR}
)

# Unix-specific libraries (pthread and libm don't exist on Windows)
if(UNIX)
    target_link_libraries(justifier_audio PRIVATE
        pthread
        m
    )
endif()

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
# Link stdc++ on non-Windows (MSVC links C++ stdlib automatically)
if(NOT WIN32)
    target_link_libraries(test_audio PRIVATE stdc++)
endif()
