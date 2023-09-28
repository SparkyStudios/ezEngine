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

set (SP_3RDPARTY_MESSAGEPACK_SUPPORT ON CACHE BOOL "Whether to add support for MessagePack serialization/deserialization.")
mark_as_advanced(FORCE SP_3RDPARTY_MESSAGEPACK_SUPPORT)

######################################
### sp_requires_mpack()
######################################
macro(sp_requires_mpack)
    ez_requires(SP_3RDPARTY_MESSAGEPACK_SUPPORT)
endmacro()

######################################
### sp_link_target_mpack(<target>)
######################################
function(sp_link_target_mpack TARGET_NAME)
    sp_requires_mpack()

    target_link_libraries(${TARGET_NAME} PRIVATE mpack)
endfunction()
