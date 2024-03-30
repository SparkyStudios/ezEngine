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

list(APPEND CMAKE_MODULE_PATH "$ENV{AM_SDK_PATH}/cmake" "${AM_SDK_PATH}/cmake")
set(AM_SDK_PLATFORM ${VCPKG_TARGET_TRIPLET})
find_package(AmplitudeAudioSDK REQUIRED)

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

    target_link_libraries(${TARGET_NAME} PRIVATE SparkyStudios::Audio::Amplitude::SDK::Shared)
    ez_uwp_mark_import_as_content(SparkyStudios::Audio::Amplitude::SDK::Shared)
#    target_link_libraries(${TARGET_NAME} PRIVATE ezAmplitude::libsamplerate)
endfunction()
