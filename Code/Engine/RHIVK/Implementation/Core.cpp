// Copyright (c) 2024-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <RHIVK/RHIVKPCH.h>

#include <RHIVK/Core.h>

namespace
{
  ezSet<ezStringView> EnumerateInstanceExtensions()
  {
    if (!RHI::IsVulkanLoaded())
      return {};

    ezUInt32 uiPropCount = 0;
    const vk::Result result = vk::enumerateInstanceExtensionProperties(nullptr, &uiPropCount, nullptr);
    SP_RHI_VK_CHECK_RESULT_RETURN(result, {});

    if (uiPropCount == 0)
      return {};

    ezDynamicArray<vk::ExtensionProperties> props;
    props.SetCount(uiPropCount);
    EZ_IGNORE_UNUSED(vk::enumerateInstanceExtensionProperties(nullptr, &uiPropCount, props.GetData()));

    ezSet<ezStringView> names;

    for (ezUInt32 i = 0; i < uiPropCount; ++i)
      names.Insert(ezStringView(props[i].extensionName.data(), props[i].extensionName.size()));

    return names;
  }

  bool TryLoadVulkan()
  {
    const vk::DynamicLoader loader{};
    return loader.success();
  }

  spLazy<bool> s_bVulkanLoaded{TryLoadVulkan};
  spLazy<ezSet<ezStringView>> s_InstanceExtensions{EnumerateInstanceExtensions};
} // namespace

namespace RHI
{
  ezSet<ezStringView> EnumerateInstanceLayers()
  {
    ezUInt32 uiPropCount = 0;

    const vk::Result result = vk::enumerateInstanceLayerProperties(&uiPropCount, nullptr);
    SP_RHI_VK_CHECK_RESULT(result);

    if (uiPropCount == 0)
      return {};

    ezDynamicArray<vk::LayerProperties> props;
    props.SetCount(uiPropCount);
    EZ_IGNORE_UNUSED(vk::enumerateInstanceLayerProperties(&uiPropCount, props.GetData()));

    ezSet<ezStringView> names;

    for (ezUInt32 i = 0; i < uiPropCount; ++i)
      names.Insert(ezStringView(props[i].layerName.data(), props[i].layerName.size()));

    return names;
  }

  ezSet<ezStringView> GetInstanceExtensions()
  {
    return s_InstanceExtensions.Get();
  }

  bool IsVulkanLoaded()
  {
    return s_bVulkanLoaded.Get();
  }
} // namespace RHI
