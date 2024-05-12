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
# ## sp_requires_slang()
# #####################################
macro(sp_requires_slang)
# noop
endmacro()

# #####################################
# ## sp_link_target_slang(<target>)
# #####################################
function(sp_link_target_slang TARGET_NAME)
  sp_requires_slang()

  find_path(SHADER_SLANG_INCLUDE_DIR NAMES slang.h)
  target_include_directories(${TARGET_NAME}
    PUBLIC
      ${SHADER_SLANG_INCLUDE_DIR}
  )

  target_link_directories(${TARGET_NAME}
    PUBLIC
      ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib
  )

  target_link_libraries(${TARGET_NAME}
    PUBLIC
      slang slang-glslang
  )
endfunction()
