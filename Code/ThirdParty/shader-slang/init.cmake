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

  if(EZ_CMAKE_PLATFORM_WINDOWS)
    add_library(shader-slang::slang SHARED IMPORTED)
    set_target_properties(
      shader-slang::slang
      PROPERTIES
      IMPORTED_IMPLIB "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/slang.lib"
      IMPORTED_LOCATION "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/slang.dll"
      IMPORTED_NO_SONAME "TRUE"
      INTERFACE_INCLUDE_DIRECTORIES "${SHADER_SLANG_INCLUDE_DIR}"
    )

    add_library(shader-slang::slang-rt SHARED IMPORTED)
    set_target_properties(
      shader-slang::slang-rt
      PROPERTIES
      IMPORTED_IMPLIB "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/slang-rt.lib"
      IMPORTED_LOCATION "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/slang-rt.dll"
      IMPORTED_NO_SONAME "TRUE"
      INTERFACE_INCLUDE_DIRECTORIES "${SHADER_SLANG_INCLUDE_DIR}"
    )

    target_link_libraries(${TARGET_NAME}
      PUBLIC
      shader-slang::slang shader-slang::slang-rt
    )

    add_custom_command(
      TARGET ${TARGET_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:shader-slang::slang>
      $<TARGET_FILE:shader-slang::slang-rt>
      "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/slang-glslang.dll"
      $<TARGET_FILE_DIR:${TARGET_NAME}>
      COMMAND_EXPAND_LISTS
    )
  endif()
endfunction()
