# Copyright (c) 2021-present Sparky Studios. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

###########################################
### Amplitude Audio Support for EZ Engine
###########################################

set (EZ_3RDPARTY_AMPLITUDE_SUPPORT ON CACHE BOOL "Whether to add support for the Amplitude Audio Engine.")
mark_as_advanced(FORCE EZ_3RDPARTY_AMPLITUDE_SUPPORT)

# Amplitude installation path
set(SS_AMPLITUDE_SDK_PATH "" CACHE PATH "The path to Amplitude Audio SDK libraries.")

# Check for a known file in the SDK path to verify the path
function(is_valid_sdk sdk_path is_valid)
    set(${is_valid} FALSE PARENT_SCOPE)
    if(EXISTS ${sdk_path})
        set(sdk_file ${sdk_path}/sdk/include/SparkyStudios/Audio/Amplitude/Amplitude.h)
        if(EXISTS ${sdk_file})
            set(${is_valid} TRUE PARENT_SCOPE)
        endif()
    endif()
endfunction()

# Paths that will be checked, in order:
# - CMake cache variable
# - A Environment Variable
set(AMPLITUDE_SDK_PATHS
    "${SS_AMPLITUDE_SDK_PATH}"
    "$ENV{SS_AMPLITUDE_ROOT_PATH}"
)

set(found_sdk FALSE)
foreach(candidate_path ${AMPLITUDE_SDK_PATHS})
    is_valid_sdk(${candidate_path} found_sdk)
    if(found_sdk)
        # Update the Amplitude installation path variable internally
        set(SS_AMPLITUDE_SDK_PATH "${candidate_path}")
        break()
    endif()
endforeach()

if(NOT found_sdk)
    # If we don't find a path that appears to be a valid Amplitude install, we can bail here.
    # No 3rdParty::AmplitudeAudioSDK target will exist, so that can be checked elsewhere.
    return()
endif()

message(STATUS "Using Amplitude Audio SDK at ${SS_AMPLITUDE_SDK_PATH}")

set(AMPLITUDE_COMPILE_DEFINITIONS
    $<IF:$<CONFIG:Shipping>,AMPLITUDE_NO_ASSERTS,>
    $<IF:$<CONFIG:Shipping>,AM_NO_MEMORY_STATS,>
)

# Use these to get the parent path and folder name before adding the external 3rd party target.
get_filename_component(AMPLITUDE_INSTALL_ROOT ${SS_AMPLITUDE_SDK_PATH} DIRECTORY)
get_filename_component(AMPLITUDE_FOLDER ${SS_AMPLITUDE_SDK_PATH} NAME)

if(MSVC)
    set(AMPLITUDE_LIB_OS "win")
    set(AMPLITUDE_LIB_NAME "Amplitude.lib")
    set(AMPLITUDE_LIB_NAME_DEBUG "Amplitude_d.lib")
    set(LIBSAMPLERATE_LIB_NAME "samplerate.lib")
    set(LIBSAMPLERATE_LIB_NAME_DEBUG "samplerate_d.lib")
elseif(APPLE)
    set(AMPLITUDE_LIB_OS "osx")
else()
    set(AMPLITUDE_LIB_OS "linux")
    set(AMPLITUDE_LIB_NAME "libAmplitude.a")
    set(AMPLITUDE_LIB_NAME_DEBUG "libAmplitude_d.a")
    set(LIBSAMPLERATE_LIB_NAME "libsamplerate.a")
    set(LIBSAMPLERATE_LIB_NAME_DEBUG "libsamplerate_d.a")
endif()

add_library(ezAmplitude::SDK STATIC IMPORTED GLOBAL)
set_target_properties(ezAmplitude::SDK PROPERTIES IMPORTED_LOCATION "${AMPLITUDE_INSTALL_ROOT}/${AMPLITUDE_FOLDER}/sdk/lib/${AMPLITUDE_LIB_OS}/${AMPLITUDE_LIB_NAME}")
set_target_properties(ezAmplitude::SDK PROPERTIES IMPORTED_LOCATION_DEBUG "${AMPLITUDE_INSTALL_ROOT}/${AMPLITUDE_FOLDER}/sdk/lib/${AMPLITUDE_LIB_OS}/${AMPLITUDE_LIB_NAME_DEBUG}")
set_target_properties(ezAmplitude::SDK PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${AMPLITUDE_INSTALL_ROOT}/${AMPLITUDE_FOLDER}/sdk/include")
ez_uwp_mark_import_as_content(ezAmplitude::SDK)

add_library(ezAmplitude::libsamplerate STATIC IMPORTED GLOBAL)
set_target_properties(ezAmplitude::libsamplerate PROPERTIES IMPORTED_LOCATION "${AMPLITUDE_INSTALL_ROOT}/${AMPLITUDE_FOLDER}/sdk/lib/${AMPLITUDE_LIB_OS}/${LIBSAMPLERATE_LIB_NAME}")
set_target_properties(ezAmplitude::libsamplerate PROPERTIES IMPORTED_LOCATION_DEBUG "${AMPLITUDE_INSTALL_ROOT}/${AMPLITUDE_FOLDER}/sdk/lib/${AMPLITUDE_LIB_OS}/${LIBSAMPLERATE_LIB_NAME_DEBUG}")
ez_uwp_mark_import_as_content(ezAmplitude::libsamplerate)

######################################
### ez_requires_amplitude()
######################################

macro(ez_requires_amplitude)
    ez_requires(EZ_3RDPARTY_AMPLITUDE_SUPPORT)
endmacro()

######################################
### ez_link_target_amplitude(<target>)
######################################

function(ez_link_target_amplitude TARGET_NAME)
    ez_requires_amplitude()

    target_link_libraries(${TARGET_NAME} PRIVATE ezAmplitude::SDK)
    target_link_libraries(${TARGET_NAME} PRIVATE ezAmplitude::libsamplerate)
endfunction()
