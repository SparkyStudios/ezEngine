#include <RHIMTL/RHIMTLPCH.h>

#include <RHIMTL/Core.h>

namespace RHI
{
  constexpr std::initializer_list<MTL::FeatureSet> kMTLFeatureSetList = {
    MTL::FeatureSet_iOS_GPUFamily1_v1,
    MTL::FeatureSet_iOS_GPUFamily2_v1,
    MTL::FeatureSet_iOS_GPUFamily1_v2,
    MTL::FeatureSet_iOS_GPUFamily2_v2,
    MTL::FeatureSet_iOS_GPUFamily3_v1,
    MTL::FeatureSet_iOS_GPUFamily1_v3,
    MTL::FeatureSet_iOS_GPUFamily2_v3,
    MTL::FeatureSet_iOS_GPUFamily3_v2,
    MTL::FeatureSet_iOS_GPUFamily1_v4,
    MTL::FeatureSet_iOS_GPUFamily2_v4,
    MTL::FeatureSet_iOS_GPUFamily3_v3,
    MTL::FeatureSet_iOS_GPUFamily4_v1,
    MTL::FeatureSet_iOS_GPUFamily1_v5,
    MTL::FeatureSet_iOS_GPUFamily2_v5,
    MTL::FeatureSet_iOS_GPUFamily3_v4,
    MTL::FeatureSet_iOS_GPUFamily4_v2,
    MTL::FeatureSet_iOS_GPUFamily5_v1,
    MTL::FeatureSet_macOS_GPUFamily1_v1,
    MTL::FeatureSet_OSX_GPUFamily1_v1,
    MTL::FeatureSet_macOS_GPUFamily1_v2,
    MTL::FeatureSet_OSX_GPUFamily1_v2,
    MTL::FeatureSet_macOS_ReadWriteTextureTier2,
    MTL::FeatureSet_OSX_ReadWriteTextureTier2,
    MTL::FeatureSet_macOS_GPUFamily1_v3,
    MTL::FeatureSet_macOS_GPUFamily1_v4,
    MTL::FeatureSet_macOS_GPUFamily2_v1,
    MTL::FeatureSet_watchOS_GPUFamily1_v1,
    MTL::FeatureSet_WatchOS_GPUFamily1_v1,
    MTL::FeatureSet_watchOS_GPUFamily2_v1,
    MTL::FeatureSet_WatchOS_GPUFamily2_v1,
    MTL::FeatureSet_tvOS_GPUFamily1_v1,
    MTL::FeatureSet_TVOS_GPUFamily1_v1,
    MTL::FeatureSet_tvOS_GPUFamily1_v2,
    MTL::FeatureSet_tvOS_GPUFamily1_v3,
    MTL::FeatureSet_tvOS_GPUFamily2_v1,
    MTL::FeatureSet_tvOS_GPUFamily1_v4,
    MTL::FeatureSet_tvOS_GPUFamily2_v2,
  };

  spMTLSupportedFeatureSet::spMTLSupportedFeatureSet(MTL::Device* pDevice)
  {
    if (pDevice == nullptr)
      return;

    for (auto set : kMTLFeatureSetList)
    {
      if (pDevice->supportsFeatureSet(set))
      {
        m_SupportedFeatureSet.Insert(set);
        m_MaxFeatureSet = set;
      }
    }

    m_bIsMacOS = IsSupported(MTL::FeatureSet_macOS_GPUFamily1_v1) || IsSupported(MTL::FeatureSet_macOS_GPUFamily1_v2) || IsSupported(MTL::FeatureSet_macOS_GPUFamily1_v3);
  }

  bool spMTLSupportedFeatureSet::IsSupported(MTL::FeatureSet featureSet) const
  {
    return m_SupportedFeatureSet.Contains(featureSet);
  }

  bool spMTLSupportedFeatureSet::IsDrawBaseVertexInstanceSupported() const
  {
    return IsSupported(MTL::FeatureSet_iOS_GPUFamily3_v1) || IsSupported(MTL::FeatureSet_iOS_GPUFamily3_v2) || IsSupported(MTL::FeatureSet_iOS_GPUFamily3_v3) || IsSupported(MTL::FeatureSet_iOS_GPUFamily4_v1) || IsSupported(MTL::FeatureSet_tvOS_GPUFamily2_v1) || IsMacOS();
  }

  bool spMTLSupportedFeatureSet::IsMacOS() const
  {
    return m_bIsMacOS;
  }
} // namespace RHI
