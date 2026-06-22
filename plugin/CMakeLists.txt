#==================================
#Animated LPG Plugin CMake
#==================================

#==================================
#Set platform export directories
#==================================
if(APPLE)
    set(VST3_COPY_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/VST3")
    set(AU_COPY_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/Components")
elseif(WIN32)
    set(VST3_COPY_DIR "$ENV{COMMONPROGRAMFILES}/VST3")
endif()

#==================================
#Binary assets
#==================================
#juce_add_binary_data(AnimatedLpgAssets
#        NAMESPACE BinaryData
#        HEADER_NAME BinaryData.h
#        SOURCES
#        Assets/ERIK-JOURGENSEN_ICON_B_W.png
#)

#==================================
#Add plugin
#==================================
juce_add_plugin(AnimatedLPG
        PLUGIN_NAME "Animated LPG"
        COMPANY_NAME AnimatedInstruments
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        PLUGIN_MANUFACTURER_CODE JOUR
        PLUGIN_CODE ANIL
        FORMATS AU VST3 Standalone
        COPY_PLUGIN_AFTER_BUILD TRUE
        VST3_COPY_DIR "${VST3_COPY_DIR}"
        AU_COPY_DIR "${AU_COPY_DIR}"
)

#==================================
#Compiler settings
#==================================
set_target_properties(AnimatedLPG PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        COMPILE_WARNING_AS_ERROR ON
)

if (MSVC)
    target_compile_options(AnimatedLPG PRIVATE /W4)
else()
    target_compile_options(AnimatedLPG PRIVATE -Wall -Wextra -pedantic)
endif()

#==================================
#Bring in WebGpu
#==================================
target_copy_webgpu_binaries(AnimatedLPG)

if (APPLE)
    set_target_properties(AnimatedLPG PROPERTIES
            XCODE_GENERATE_SCHEME ON
            XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal"
            XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME "YES"
    )
endif()

#==================================
#Cpp file locations
#==================================
target_sources(AnimatedLPG
        PRIVATE
        source/AnimatedLpgEditor.cpp
        source/AnimatedLpgProcessor.cpp
        dsp/AnimatedLPG.cpp
        dsp/AnimatedVactrol.cpp
        ui/resourceManager.cpp
        ui/WebGpuWindow.cpp
        ui/scene.cpp
)

if (APPLE)
    target_sources(AnimatedLPG PRIVATE source/GpuSurface.mm)
    target_link_libraries(AnimatedLPG PRIVATE "-framework QuartzCore" "-framework Metal")
endif()

#==================================
#Include directories
#==================================
target_include_directories(AnimatedLPG
        PRIVATE
        include
        source
        dsp
        ui
        procedural
#       shaders
#       utilities
)

#==================================
#Link libraries
#==================================
target_link_libraries(AnimatedLPG
        PRIVATE
        webgpu
#        AnimatedLpgAssets
        juce::juce_audio_utils
        juce::juce_gui_extra
        juce::juce_dsp
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)


#==================================
#
#==================================
target_compile_definitions(AnimatedLPG
        PUBLIC
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

option(DEV_MODE "Set up development helper settings" ON)

if(DEV_MODE)
    target_compile_definitions(AnimatedLPG PRIVATE
            RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/geometries"
    )

else()
    target_compile_definitions(AnimatedLPG PRIVATE
            RESOURCE_DIR="geometries"
    )
endif()

#==================================
# Shader files (concatenated in order)
#==================================
set(SHADER_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/common.wgsl"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/lighting.wgsl"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/mat_floor.wgsl"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/vs_main.wgsl"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/fs_main.wgsl"
)

set(SHADER_SOURCE "")
foreach(SHADER_FILE ${SHADER_FILES})
    file(READ "${SHADER_FILE}" SHADER_CHUNK)
    string(APPEND SHADER_SOURCE "${SHADER_CHUNK}\n")
endforeach()

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ui/shader.wgsl.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/shader.wgsl.h"
)

target_compile_definitions(AnimatedLPG PRIVATE
        DEBUG_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)

target_include_directories(AnimatedLPG PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")