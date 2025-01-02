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
#include <RHIVK/Device.h>

namespace
{
  ezUInt32 vkDebugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    ezUInt64 object,
    size_t location,
    ezInt32 messageCode,
    const char* pLayerPrefix,
    const char* pMessage,
    void* pUserData)
  {
    if (flags == VK_DEBUG_REPORT_ERROR_BIT_EXT)
      EZ_ASSERT_DEV(false, "A Vulkan validation error was encountered: {}", pMessage);

    ezLog::Debug("[RHIVK] {}", pMessage);
    return 0;
  }
} // namespace

namespace RHI
{
  // clang-format off
  EZ_IMPLEMENT_SINGLETON(spDeviceVK);
  // clang-format on

  spDeviceVK::spDeviceVK(ezAllocator* pAllocator, const spDeviceDescriptionVK& description)
    : spDevice(pAllocator, description)
    , m_SingletonRegistrar(this)
  {
    CreateInstance(description);
    CreatePhysicalDevice();
  }

  bool spDeviceVK::HasSurfaceExtension(ezStringView sExtensionName) const
  {
    return m_SurfaceExtensions.Contains(sExtensionName);
  }

  void spDeviceVK::EnableDebugCallback(vk::DebugReportFlagsEXT flags) const
  {
    ezLog::Debug("Enabling Vulkan Debug callbacks.");
    vk::DebugReportCallbackCreateInfoEXT debugCallbackCI{};
    debugCallbackCI.flags = flags;
    debugCallbackCI.pfnCallback = vkDebugCallback;

    const vk::Result result = m_Instance.createDebugReportCallbackEXT(&debugCallbackCI, nullptr, m_pDebugCallback);
    SP_RHI_VK_CHECK_RESULT(result);
  }

  void spDeviceVK::CreateInstance(const spDeviceDescriptionVK& description)
  {
    m_bIsDebugEnabled = description.m_bDebug;

    const ezSet<ezStringView>& availableLayers = EnumerateInstanceLayers();
    const ezSet<ezStringView>& availableExtensions = GetInstanceExtensions();

    vk::ApplicationInfo appInfo;
    appInfo.sType = vk::StructureType::eApplicationInfo;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "Spark Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Spark Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    vk::InstanceCreateInfo createInfo;
    createInfo.sType = vk::StructureType::eInstanceCreateInfo;
    createInfo.pNext = nullptr;
    createInfo.flags = vk::InstanceCreateFlags(0);
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = 0;

    ezDynamicArray<const char*> instanceExtensions;
    ezDynamicArray<const char*> instanceLayers;

    if (availableExtensions.Contains("VK_KHR_portability_subset"))
      m_SurfaceExtensions.Insert("VK_KHR_portability_subset");

    if (availableExtensions.Contains(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
    {
      instanceExtensions.PushBack(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
      createInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
    }

    if (availableExtensions.Contains(VK_KHR_SURFACE_EXTENSION_NAME))
      m_SurfaceExtensions.Insert(VK_KHR_SURFACE_EXTENSION_NAME);

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS)
    if (availableExtensions.Contains(VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
      m_SurfaceExtensions.Insert(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif EZ_ENABLED(EZ_PLATFORM_LINUX)
    if (availableExtensions.Contains(VK_KHR_XCB_SURFACE_EXTENSION_NAME))
      m_SurfaceExtensions.Insert(VK_KHR_XCB_SURFACE_EXTENSION_NAME);

    if (availableExtensions.Contains(VK_KHR_XLIB_SURFACE_EXTENSION_NAME))
      m_SurfaceExtensions.Insert(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif EZ_ENABLED(EZ_PLATFORM_ANDROID)
    if (availableExtensions.Contains(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME))
      m_SurfaceExtensions.Insert(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif EZ_ENABLED(EZ_PLATFORM_IOS) || EZ_ENABLED(EZ_PLATFORM_OSX)
    if (availableExtensions.Contains(VK_EXT_METAL_SURFACE_EXTENSION_NAME))
    {
      m_SurfaceExtensions.Insert(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
    }
    else
    {
#  if EZ_ENABLED(EZ_PLATFORM_OSX)
      if (availableExtensions.Contains(VK_MVK_MACOS_SURFACE_EXTENSION_NAME))
        m_SurfaceExtensions.Insert(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#  else
      if (availableExtensions.Contains(VK_MVK_IOS_SURFACE_EXTENSION_NAME))
        m_SurfaceExtensions.Insert(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#  endif
    }
#endif

    for (const auto& extension : m_SurfaceExtensions)
    {
      ezStringBuilder sTemp;
      instanceExtensions.PushBack(extension.GetData(sTemp));
    }

    const bool bHasPhysicalDeviceProperties2 = availableExtensions.Contains(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (bHasPhysicalDeviceProperties2)
      instanceExtensions.PushBack(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    for (const auto& extension : description.m_InstanceExtensions)
    {
      EZ_ASSERT_ALWAYS(availableExtensions.Contains(extension), "The required instance extension was not available: {}", extension);

      ezStringBuilder sTemp;
      instanceExtensions.PushBack(extension.GetData(sTemp));
    }

    bool bDebugReportExtensionAvailable = false;
    if (m_bIsDebugEnabled)
    {
      if (availableExtensions.Contains(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
      {
        instanceExtensions.PushBack(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        bDebugReportExtensionAvailable = true;
      }

      if (availableLayers.Contains("VK_LAYER_LUNARG_standard_validation"))
      {
        instanceLayers.PushBack("VK_LAYER_LUNARG_standard_validation");
        m_bStandardValidationLayerAvailable = true;
      }

      if (availableLayers.Contains("VK_LAYER_KHRONOS_validation"))
      {
        instanceLayers.PushBack("VK_LAYER_KHRONOS_validation");
        m_bKhronosValidationLayerAvailable = true;
      }
    }

    createInfo.enabledExtensionCount = instanceExtensions.GetCount();
    createInfo.ppEnabledExtensionNames = instanceExtensions.GetData();

    createInfo.enabledLayerCount = instanceLayers.GetCount();
    createInfo.ppEnabledLayerNames = instanceLayers.GetData();

    const vk::Result result = vk::createInstance(&createInfo, nullptr, &m_Instance);
    SP_RHI_VK_CHECK_RESULT(result);

    if (m_bIsDebugEnabled && bDebugReportExtensionAvailable)
    {
      EnableDebugCallback();
    }

    if (bHasPhysicalDeviceProperties2)
    {
      m_vkGetPhysicalDeviceProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(m_Instance.getProcAddr(EZ_PP_STRINGIFY(vkGetPhysicalDeviceProperties2)));

      if (m_vkGetPhysicalDeviceProperties2 == nullptr)
        m_vkGetPhysicalDeviceProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(m_Instance.getProcAddr(EZ_PP_STRINGIFY(vkGetPhysicalDeviceProperties2KHR)));
    }
  }

  void spDeviceVK::CreatePhysicalDevice()
  {
    ezUInt32 uiDeviceCount = 0;
    const vk::Result result = m_Instance.enumeratePhysicalDevices(&uiDeviceCount, nullptr);
    EZ_ASSERT_DEV(result == vk::Result::eSuccess && uiDeviceCount > 0, "Failed to enumerate physical devices.");

    ezDynamicArray<vk::PhysicalDevice> physicalDevices;
    physicalDevices.SetCount(uiDeviceCount);

    EZ_IGNORE_UNUSED(m_Instance.enumeratePhysicalDevices(&uiDeviceCount, physicalDevices.GetData()));
    m_PhysicalDevice = physicalDevices[0];

    m_PhysicalDevice.getProperties(&m_PhysicalDeviceProperties);

    m_HardwareInfo.m_uiID = m_PhysicalDeviceProperties.deviceID;

    {
      ezStringBuilder sb;
      sb.Append(ezStringView(m_PhysicalDeviceProperties.deviceName.data(), m_PhysicalDeviceProperties.deviceName.size()));
      m_HardwareInfo.m_sName = sb;
    }

    {
      ezStringBuilder sb;
      sb.AppendFormat("id:{}", m_PhysicalDeviceProperties.vendorID);
      m_HardwareInfo.m_sVendor = sb;
    }

    m_ApiVersion = spGraphicsApiVersion(
      VK_API_VERSION_MAJOR(m_PhysicalDeviceProperties.apiVersion),
      VK_API_VERSION_MINOR(m_PhysicalDeviceProperties.apiVersion),
      VK_API_VERSION_PATCH(m_PhysicalDeviceProperties.apiVersion));

    m_PhysicalDevice.getFeatures(&m_PhysicalDeviceFeatures);
    m_PhysicalDevice.getMemoryProperties(&m_PhysicalDeviceMemoryProperties);
  }
} // namespace RHI
