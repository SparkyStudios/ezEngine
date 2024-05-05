# Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

# #####################################
# ## sp_requires_d3d12()
# #####################################
macro(sp_requires_d3d12)
  ez_requires_windows()

  get_property(SP_DX12_LIBRARIES GLOBAL PROPERTY SP_DX12_LIBRARIES)

  if(NOT SP_DX12_LIBRARIES)
    find_package(D3D12 REQUIRED)

    if(D3D12_FOUND)
      set_property(GLOBAL PROPERTY SP_DX12_LIBRARIES ${D3D12_LIBRARIES})
      set_property(GLOBAL PROPERTY SP_DX12_INCL_DIRS ${D3D12_INCLUDE_DIRS})
    endif()
  endif()

  find_package(d3d12-memory-allocator CONFIG REQUIRED)
endmacro()

# #####################################
# ## sp_link_target_d3d12(<target>)
# #####################################
function(sp_link_target_d3d12 TARGET_NAME)
  sp_requires_d3d12()

  get_property(SP_DX12_LIBRARIES GLOBAL PROPERTY SP_DX12_LIBRARIES)
  get_property(SP_DX12_INCL_DIRS GLOBAL PROPERTY SP_DX12_INCL_DIRS)

  target_link_libraries(${TARGET_NAME}
    PRIVATE
    ${SP_DX12_LIBRARIES}
    unofficial::D3D12MemoryAllocator
  )

  target_include_directories(${TARGET_NAME}
    PRIVATE
    ${SP_DX12_INCL_DIRS}
  )
endfunction()
