# Copyright (c) 2021-present Sparky Studios. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(SP_3RDPARTY_VULKAN_SUPPORT ON CACHE BOOL "Whether to add Vulkan backend for the renderer.")
mark_as_advanced(FORCE SP_3RDPARTY_VULKAN_SUPPORT)

# #####################################
# ## sp_requires_vulkan()
# #####################################
macro(sp_requires_vulkan)
    ez_requires_one_of(EZ_CMAKE_PLATFORM_LINUX EZ_CMAKE_PLATFORM_WINDOWS EZ_CMAKE_PLATFORM_OSX)
    ez_requires(SP_3RDPARTY_VULKAN_SUPPORT)

    if (NOT TARGET unofficial::VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp)
      include(FindVulkan)

      find_package(Vulkan REQUIRED)
      find_package(unofficial-vulkan-memory-allocator-hpp CONFIG REQUIRED)
    endif ()
endmacro()

# #####################################
# ## sp_link_target_vulkan(<target>)
# #####################################
function(sp_link_target_vulkan TARGET_NAME)
    sp_requires_vulkan()

    find_path(VULKAN_HPP_INCLUDE_DIRS "vulkan/vulkan.hpp")
    target_include_directories(${TARGET_NAME} PRIVATE ${VULKAN_HPP_INCLUDE_DIRS})

    target_link_libraries(${TARGET_NAME} PRIVATE Vulkan::Vulkan)
    target_link_libraries(${TARGET_NAME} PRIVATE unofficial::VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp)
endfunction()
