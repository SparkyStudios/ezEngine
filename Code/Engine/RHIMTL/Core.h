#pragma once

#include <RHIMTL/RHIMTLDLL.h>

#include <RHI/Core.h>

namespace RHI
{
  struct spUnalignedBufferCopyData
  {
    ezUInt32 m_uiSourceOffset = 0;
    ezUInt32 m_uiDestinationOffset = 0;
    ezUInt32 m_uiSize = 0;
  };

  struct SP_RHIMTL_DLL spMTLSupportedFeatureSet
  {
  public:
    explicit spMTLSupportedFeatureSet(MTL::Device* pDevice);

    EZ_NODISCARD bool IsSupported(MTL::FeatureSet feature) const;
    EZ_NODISCARD bool IsDrawBaseVertexInstanceSupported() const;
    EZ_NODISCARD bool IsMacOS() const;

    EZ_NODISCARD EZ_ALWAYS_INLINE MTL::FeatureSet GetMaxFeatureSet() const { return m_MaxFeatureSet; }

  private:
    ezSet<MTL::FeatureSet> m_SupportedFeatureSet;

    MTL::FeatureSet m_MaxFeatureSet{0};
    bool m_bIsMacOS{false};
  };

  template <typename T>
  class spScopedMTLResource
  {
  public:
    explicit spScopedMTLResource(T* pResource = nullptr, bool bOwn = false)
      : m_pResource(pResource)
    {
      if (bOwn)
        AddRef();
    }

    ~spScopedMTLResource()
    {
      ReleaseRef();
    }

    EZ_ALWAYS_INLINE const T* operator*() const
    {
      return m_pResource;
    }

    EZ_ALWAYS_INLINE const T** operator&() const
    {
      return &m_pResource;
    }

    EZ_ALWAYS_INLINE const T* operator->() const
    {
      return m_pResource;
    }

    EZ_ALWAYS_INLINE T* operator*()
    {
      return m_pResource;
    }

    EZ_ALWAYS_INLINE T** operator&()
    {
      return &m_pResource;
    }

    EZ_ALWAYS_INLINE T* operator->()
    {
      return m_pResource;
    }

  private:
    EZ_ALWAYS_INLINE void AddRef()
    {
      SP_RHI_MTL_RETAIN(m_pResource);
    }

    EZ_ALWAYS_INLINE void ReleaseRef()
    {
      SP_RHI_MTL_RELEASE(m_pResource);
    }

    T* m_pResource;
  };

#pragma region Conversion Functions

  EZ_ALWAYS_INLINE static MTL::PixelFormat spToMTL(const ezEnum<spPixelFormat>& eFormat, bool bDepthFormat)
  {
    switch (eFormat)
    {
      case spPixelFormat::R8UNorm:
        return MTL::PixelFormatR8Unorm;
      case spPixelFormat::R8SNorm:
        return MTL::PixelFormatR8Snorm;
      case spPixelFormat::R8UInt:
        return MTL::PixelFormatR8Uint;
      case spPixelFormat::R8SInt:
        return MTL::PixelFormatR8Sint;

      case spPixelFormat::R16UNorm:
        return bDepthFormat ? MTL::PixelFormatDepth16Unorm : MTL::PixelFormatR16Unorm;
      case spPixelFormat::R16SNorm:
        return MTL::PixelFormatR16Snorm;
      case spPixelFormat::R16UInt:
        return MTL::PixelFormatR16Uint;
      case spPixelFormat::R16SInt:
        return MTL::PixelFormatR16Sint;
      case spPixelFormat::R16Float:
        return MTL::PixelFormatR16Float;

      case spPixelFormat::R32UInt:
        return MTL::PixelFormatR32Uint;
      case spPixelFormat::R32SInt:
        return MTL::PixelFormatR32Sint;
      case spPixelFormat::R32Float:
        return bDepthFormat ? MTL::PixelFormatDepth32Float : MTL::PixelFormatR32Float;

      case spPixelFormat::R8G8UNorm:
        return MTL::PixelFormatRG8Unorm;
      case spPixelFormat::R8G8SNorm:
        return MTL::PixelFormatRG8Snorm;
      case spPixelFormat::R8G8UInt:
        return MTL::PixelFormatRG8Uint;
      case spPixelFormat::R8G8SInt:
        return MTL::PixelFormatRG8Sint;

      case spPixelFormat::R16G16UNorm:
        return MTL::PixelFormatRG16Unorm;
      case spPixelFormat::R16G16SNorm:
        return MTL::PixelFormatRG16Snorm;
      case spPixelFormat::R16G16UInt:
        return MTL::PixelFormatRG16Uint;
      case spPixelFormat::R16G16SInt:
        return MTL::PixelFormatRG16Sint;
      case spPixelFormat::R16G16Float:
        return MTL::PixelFormatRG16Float;

      case spPixelFormat::R32G32UInt:
        return MTL::PixelFormatRG32Uint;
      case spPixelFormat::R32G32SInt:
        return MTL::PixelFormatRG32Sint;
      case spPixelFormat::R32G32Float:
        return MTL::PixelFormatRG32Float;

      case spPixelFormat::R8G8B8UNorm:
      case spPixelFormat::R8G8B8A8UNorm:
        return MTL::PixelFormatRGBA8Unorm;
      case spPixelFormat::R8G8B8A8UNormSRgb:
        return MTL::PixelFormatRGBA8Unorm_sRGB;
      case spPixelFormat::B8G8R8A8UNorm:
        return MTL::PixelFormatBGRA8Unorm;
      case spPixelFormat::B8G8R8A8UNormSRgb:
        return MTL::PixelFormatBGRA8Unorm_sRGB;
      case spPixelFormat::R8G8B8SNorm:
      case spPixelFormat::R8G8B8A8SNorm:
        return MTL::PixelFormatRGBA8Snorm;
      case spPixelFormat::R8G8B8UInt:
      case spPixelFormat::R8G8B8A8UInt:
        return MTL::PixelFormatRGBA8Uint;
      case spPixelFormat::R8G8B8SInt:
      case spPixelFormat::R8G8B8A8SInt:
        return MTL::PixelFormatRGBA8Sint;

      case spPixelFormat::R16G16B16UNorm:
      case spPixelFormat::R16G16B16A16UNorm:
        return MTL::PixelFormatRGBA16Unorm;
      case spPixelFormat::R16G16B16SNorm:
      case spPixelFormat::R16G16B16A16SNorm:
        return MTL::PixelFormatRGBA16Snorm;
      case spPixelFormat::R16G16B16UInt:
      case spPixelFormat::R16G16B16A16UInt:
        return MTL::PixelFormatRGBA16Uint;
      case spPixelFormat::R16G16B16SInt:
      case spPixelFormat::R16G16B16A16SInt:
        return MTL::PixelFormatRGBA16Sint;
      case spPixelFormat::R16G16B16Float:
      case spPixelFormat::R16G16B16A16Float:
        return MTL::PixelFormatRGBA16Float;

      case spPixelFormat::R32G32B32UInt:
      case spPixelFormat::R32G32B32A32UInt:
        return MTL::PixelFormatRGBA32Uint;
      case spPixelFormat::R32G32B32SInt:
      case spPixelFormat::R32G32B32A32SInt:
        return MTL::PixelFormatRGBA32Sint;
      case spPixelFormat::R32G32B32Float:
      case spPixelFormat::R32G32B32A32Float:
        return MTL::PixelFormatRGBA32Float;

      case spPixelFormat::Bc1RgbUNorm:
      case spPixelFormat::Bc1RgbaUNorm:
        return MTL::PixelFormatBC1_RGBA;

      case spPixelFormat::Bc1RgbUNormSRgb:
      case spPixelFormat::Bc1RgbaUNormSRgb:
        return MTL::PixelFormatBC1_RGBA_sRGB;

      case spPixelFormat::Bc2UNorm:
        return MTL::PixelFormatBC2_RGBA;
      case spPixelFormat::Bc2UNormSRgb:
        return MTL::PixelFormatBC2_RGBA_sRGB;

      case spPixelFormat::Bc3UNorm:
        return MTL::PixelFormatBC3_RGBA;
      case spPixelFormat::Bc3UNormSRgb:
        return MTL::PixelFormatBC3_RGBA_sRGB;

      case spPixelFormat::Bc4UNorm:
        return MTL::PixelFormatBC4_RUnorm;
      case spPixelFormat::Bc4SNorm:
        return MTL::PixelFormatBC4_RSnorm;

      case spPixelFormat::Bc5UNorm:
        return MTL::PixelFormatBC5_RGUnorm;
      case spPixelFormat::Bc5SNorm:
        return MTL::PixelFormatBC5_RGSnorm;

      case spPixelFormat::Bc6HUFloat:
        return MTL::PixelFormatBC6H_RGBUfloat;
      case spPixelFormat::Bc6HSFloat:
        return MTL::PixelFormatBC6H_RGBFloat;

      case spPixelFormat::Bc7UNorm:
        return MTL::PixelFormatBC7_RGBAUnorm;
      case spPixelFormat::Bc7UNormSRgb:
        return MTL::PixelFormatBC7_RGBAUnorm_sRGB;

      case spPixelFormat::Etc2R8G8B8UNorm:
        return MTL::PixelFormatETC2_RGB8;
      case spPixelFormat::Etc2R8G8B8A1UNorm:
        return MTL::PixelFormatETC2_RGB8A1;
      case spPixelFormat::Etc2R8G8B8A8UNorm:
        return MTL::PixelFormatEAC_RGBA8;

      case spPixelFormat::D16UNorm:
        EZ_ASSERT_DEV(bDepthFormat, "Trying to use a depth format without bDepthFormat as true.");
        return MTL::PixelFormatDepth16Unorm;
      case spPixelFormat::D24UNorm:
      case spPixelFormat::D24UNormS8UInt:
        EZ_ASSERT_DEV(bDepthFormat, "Trying to use a depth format without bDepthFormat as true.");
        return MTL::PixelFormatDepth24Unorm_Stencil8;
      case spPixelFormat::D32Float:
        EZ_ASSERT_DEV(bDepthFormat, "Trying to use a depth format without bDepthFormat as true.");
        return MTL::PixelFormatDepth32Float;
      case spPixelFormat::D32FloatS8UInt:
        EZ_ASSERT_DEV(bDepthFormat, "Trying to use a depth format without bDepthFormat as true.");
        return MTL::PixelFormatDepth32Float_Stencil8;

      case spPixelFormat::R10G10B10A2UNorm:
        return MTL::PixelFormatRGB10A2Unorm;
      case spPixelFormat::R10G10B10A2UInt:
        return MTL::PixelFormatRGB10A2Uint;
      case spPixelFormat::R11G11B10Float:
        return MTL::PixelFormatRG11B10Float;

      case spPixelFormat::Unknown:
        EZ_ASSERT_DEV(false, "Trying to use an unknown pixel format.");
        return MTL::PixelFormatInvalid;
    }
  }

  EZ_ALWAYS_INLINE static ezEnum<spPixelFormat> spFromMTL(MTL::PixelFormat eFormat)
  {
    switch (eFormat)
    {
      case MTL::PixelFormatR8Unorm:
        return spPixelFormat::R8UNorm;
      case MTL::PixelFormatR8Snorm:
        return spPixelFormat::R8SNorm;
      case MTL::PixelFormatR8Uint:
        return spPixelFormat::R8UInt;
      case MTL::PixelFormatR8Sint:
        return spPixelFormat::R8SInt;

      case MTL::PixelFormatR16Unorm:
        return spPixelFormat::R16UNorm;
      case MTL::PixelFormatR16Snorm:
        return spPixelFormat::R16SNorm;
      case MTL::PixelFormatR16Uint:
        return spPixelFormat::R16UInt;
      case MTL::PixelFormatR16Sint:
        return spPixelFormat::R16SInt;
      case MTL::PixelFormatR16Float:
        return spPixelFormat::R16Float;

      case MTL::PixelFormatR32Uint:
        return spPixelFormat::R32UInt;
      case MTL::PixelFormatR32Sint:
        return spPixelFormat::R32SInt;
      case MTL::PixelFormatR32Float:
        return spPixelFormat::R32Float;

      case MTL::PixelFormatRG8Unorm:
        return spPixelFormat::R8G8UNorm;
      case MTL::PixelFormatRG8Snorm:
        return spPixelFormat::R8G8SNorm;
      case MTL::PixelFormatRG8Uint:
        return spPixelFormat::R8G8UInt;
      case MTL::PixelFormatRG8Sint:
        return spPixelFormat::R8G8SInt;

      case MTL::PixelFormatRG16Unorm:
        return spPixelFormat::R16G16UNorm;
      case MTL::PixelFormatRG16Snorm:
        return spPixelFormat::R16G16SNorm;
      case MTL::PixelFormatRG16Uint:
        return spPixelFormat::R16G16UInt;
      case MTL::PixelFormatRG16Sint:
        return spPixelFormat::R16G16SInt;
      case MTL::PixelFormatRG16Float:
        return spPixelFormat::R16G16Float;

      case MTL::PixelFormatRG32Uint:
        return spPixelFormat::R32G32UInt;
      case MTL::PixelFormatRG32Sint:
        return spPixelFormat::R32G32SInt;
      case MTL::PixelFormatRG32Float:
        return spPixelFormat::R32G32Float;

      case MTL::PixelFormatRGBA8Unorm:
        return spPixelFormat::R8G8B8A8UNorm;
      case MTL::PixelFormatRGBA8Unorm_sRGB:
        return spPixelFormat::R8G8B8A8UNormSRgb;
      case MTL::PixelFormatBGRA8Unorm:
        return spPixelFormat::B8G8R8A8UNorm;
      case MTL::PixelFormatBGRA8Unorm_sRGB:
        return spPixelFormat::B8G8R8A8UNormSRgb;
      case MTL::PixelFormatRGBA8Snorm:
        return spPixelFormat::R8G8B8A8SNorm;
      case MTL::PixelFormatRGBA8Uint:
        return spPixelFormat::R8G8B8A8UInt;
      case MTL::PixelFormatRGBA8Sint:
        return spPixelFormat::R8G8B8A8SInt;

      case MTL::PixelFormatRGBA16Unorm:
        return spPixelFormat::R16G16B16A16UNorm;
      case MTL::PixelFormatRGBA16Snorm:
        return spPixelFormat::R16G16B16A16SNorm;
      case MTL::PixelFormatRGBA16Uint:
        return spPixelFormat::R16G16B16A16UInt;
      case MTL::PixelFormatRGBA16Sint:
        return spPixelFormat::R16G16B16A16SInt;
      case MTL::PixelFormatRGBA16Float:
        return spPixelFormat::R16G16B16A16Float;

      case MTL::PixelFormatRGBA32Uint:
        return spPixelFormat::R32G32B32A32UInt;
      case MTL::PixelFormatRGBA32Sint:
        return spPixelFormat::R32G32B32A32SInt;
      case MTL::PixelFormatRGBA32Float:
        return spPixelFormat::R32G32B32A32Float;

      case MTL::PixelFormatBC1_RGBA:
        return spPixelFormat::Bc1RgbaUNorm;

      case MTL::PixelFormatBC1_RGBA_sRGB:
        return spPixelFormat::Bc1RgbaUNormSRgb;

      case MTL::PixelFormatBC2_RGBA:
        return spPixelFormat::Bc2UNorm;
      case MTL::PixelFormatBC2_RGBA_sRGB:
        return spPixelFormat::Bc2UNormSRgb;

      case MTL::PixelFormatBC3_RGBA:
        return spPixelFormat::Bc3UNorm;
      case MTL::PixelFormatBC3_RGBA_sRGB:
        return spPixelFormat::Bc3UNormSRgb;

      case MTL::PixelFormatBC4_RUnorm:
        return spPixelFormat::Bc4UNorm;
      case MTL::PixelFormatBC4_RSnorm:
        return spPixelFormat::Bc4SNorm;

      case MTL::PixelFormatBC5_RGUnorm:
        return spPixelFormat::Bc5UNorm;
      case MTL::PixelFormatBC5_RGSnorm:
        return spPixelFormat::Bc5SNorm;

      case MTL::PixelFormatBC6H_RGBUfloat:
        return spPixelFormat::Bc6HUFloat;
      case MTL::PixelFormatBC6H_RGBFloat:
        return spPixelFormat::Bc6HSFloat;

      case MTL::PixelFormatBC7_RGBAUnorm:
        return spPixelFormat::Bc7UNorm;
      case MTL::PixelFormatBC7_RGBAUnorm_sRGB:
        return spPixelFormat::Bc7UNormSRgb;

      case MTL::PixelFormatETC2_RGB8:
        return spPixelFormat::Etc2R8G8B8UNorm;
      case MTL::PixelFormatETC2_RGB8A1:
        return spPixelFormat::Etc2R8G8B8A1UNorm;
      case MTL::PixelFormatEAC_RGBA8:
        return spPixelFormat::Etc2R8G8B8A8UNorm;

      case MTL::PixelFormatDepth16Unorm:
        return spPixelFormat::D16UNorm;
      case MTL::PixelFormatDepth24Unorm_Stencil8:
        return spPixelFormat::D24UNormS8UInt;
      case MTL::PixelFormatDepth32Float:
        return spPixelFormat::D32Float;
      case MTL::PixelFormatDepth32Float_Stencil8:
        return spPixelFormat::D32FloatS8UInt;

      case MTL::PixelFormatRGB10A2Unorm:
        return spPixelFormat::R10G10B10A2UNorm;
      case MTL::PixelFormatRGB10A2Uint:
        return spPixelFormat::R10G10B10A2UInt;
      case MTL::PixelFormatRG11B10Float:
        return spPixelFormat::R11G11B10Float;

      default:
        EZ_ASSERT_NOT_IMPLEMENTED;
        return spPixelFormat::Default;
    }
  }

  EZ_ALWAYS_INLINE static MTL::TriangleFillMode spToMTL(const ezEnum<spPolygonFillMode>& eFillMode)
  {
    switch (eFillMode)
    {
      case spPolygonFillMode::Solid:
        return MTL::TriangleFillModeFill;
      case spPolygonFillMode::Wireframe:
        return MTL::TriangleFillModeLines;
      default:
        EZ_ASSERT_DEV(false, "Invalid Polygon Fill Mode, using Solid as fallback.");
        return MTL::TriangleFillModeFill;
    }
  }

  EZ_ALWAYS_INLINE static MTL::Winding spToMTL(const ezEnum<spFrontFace>& eFrontFace)
  {
    return eFrontFace == spFrontFace::CounterClockwise ? MTL::WindingCounterClockwise : MTL::WindingClockwise;
  }

  EZ_ALWAYS_INLINE static MTL::SamplerMinMagFilter spToMTL(const ezEnum<spSamplerFilter>& eFilter, MTL::SamplerMinMagFilter& out_eFilter)
  {
    switch (eFilter)
    {
      case spSamplerFilter::Linear:
        return out_eFilter = MTL::SamplerMinMagFilterLinear;
      case spSamplerFilter::Point:
        return out_eFilter = MTL::SamplerMinMagFilterNearest;
      default:
        EZ_ASSERT_DEV(false, "Invalid Sampler Filter, using Linear as fallback.");
        return out_eFilter = MTL::SamplerMinMagFilterLinear;
    }
  }

  EZ_ALWAYS_INLINE static MTL::SamplerMipFilter spToMTL(const ezEnum<spSamplerFilter>& eFilter, MTL::SamplerMipFilter& out_eFilter)
  {
    switch (eFilter)
    {
      case spSamplerFilter::None:
        return out_eFilter = MTL::SamplerMipFilterNotMipmapped;
      case spSamplerFilter::Linear:
        return out_eFilter = MTL::SamplerMipFilterLinear;
      case spSamplerFilter::Point:
        return out_eFilter = MTL::SamplerMipFilterNearest;
    }
  }

  EZ_ALWAYS_INLINE static MTL::TextureType spToMTL(const ezEnum<spTextureDimension>& eDimension, ezUInt32 uiArrayLayers, bool bMultiSampled, bool bCube)
  {
    switch (eDimension)
    {
      case spTextureDimension::Texture1D:
        return uiArrayLayers > 1 ? MTL::TextureType1DArray : MTL::TextureType1D;
      case spTextureDimension::Texture2D:
        if (bCube)
          return uiArrayLayers > 1 ? MTL::TextureTypeCubeArray : MTL::TextureTypeCube;

        if (bMultiSampled)
          return MTL::TextureType2DMultisample;

        return uiArrayLayers > 1 ? MTL::TextureType2DArray : MTL::TextureType2D;
      case spTextureDimension::Texture3D:
        return MTL::TextureType3D;
      default:
        EZ_ASSERT_NOT_IMPLEMENTED;
    }
  }

  EZ_ALWAYS_INLINE static ezEnum<spTextureDimension> spFromMTL(MTL::TextureType eDimension)
  {
    switch (eDimension)
    {
      case MTL::TextureType1DArray:
      case MTL::TextureType1D:
        return spTextureDimension::Texture1D;
      case MTL::TextureTypeCubeArray:
      case MTL::TextureTypeCube:
      case MTL::TextureType2DMultisample:
      case MTL::TextureType2DArray:
      case MTL::TextureType2D:
        return spTextureDimension::Texture2D;
      case MTL::TextureType3D:
        return spTextureDimension::Texture3D;
      default:
        EZ_ASSERT_NOT_IMPLEMENTED;
        return spTextureDimension::Texture2D;
    }
  }

  EZ_ALWAYS_INLINE static MTL::BlendFactor spToMTL(const ezEnum<spBlendFactor>& eBlendFactor)
  {
    switch (eBlendFactor)
    {
      case spBlendFactor::Zero:
        return MTL::BlendFactorZero;
      case spBlendFactor::One:
        return MTL::BlendFactorOne;
      case spBlendFactor::SourceAlpha:
        return MTL::BlendFactorSourceAlpha;
      case spBlendFactor::InverseSourceAlpha:
        return MTL::BlendFactorOneMinusSourceAlpha;
      case spBlendFactor::DestinationAlpha:
        return MTL::BlendFactorDestinationAlpha;
      case spBlendFactor::InverseDestinationAlpha:
        return MTL::BlendFactorOneMinusDestinationAlpha;
      case spBlendFactor::SourceColor:
        return MTL::BlendFactorSourceColor;
      case spBlendFactor::InverseSourceColor:
        return MTL::BlendFactorOneMinusSourceColor;
      case spBlendFactor::DestinationColor:
        return MTL::BlendFactorDestinationColor;
      case spBlendFactor::InverseDestinationColor:
        return MTL::BlendFactorOneMinusDestinationColor;
      case spBlendFactor::BlendFactor:
        return MTL::BlendFactorBlendColor;
      case spBlendFactor::InverseBlendFactor:
        return MTL::BlendFactorOneMinusBlendColor;
    }
  }

  EZ_ALWAYS_INLINE static MTL::BlendOperation spToMTL(const ezEnum<spBlendFunction>& eOperation)
  {
    switch (eOperation)
    {
      case spBlendFunction::Add:
        return MTL::BlendOperationAdd;
      case spBlendFunction::Maximum:
        return MTL::BlendOperationMax;
      case spBlendFunction::Minimum:
        return MTL::BlendOperationMin;
      case spBlendFunction::ReverseSubtract:
        return MTL::BlendOperationReverseSubtract;
      case spBlendFunction::Subtract:
        return MTL::BlendOperationSubtract;
    }
  }

  EZ_ALWAYS_INLINE static MTL::ColorWriteMask spToMTL(const ezBitflags<spColorWriteMask>& eMask)
  {
    MTL::ColorWriteMask mask = MTL::ColorWriteMaskNone;

    if (eMask.IsSet(spColorWriteMask::Red))
      mask |= MTL::ColorWriteMaskRed;
    if (eMask.IsSet(spColorWriteMask::Green))
      mask |= MTL::ColorWriteMaskGreen;
    if (eMask.IsSet(spColorWriteMask::Blue))
      mask |= MTL::ColorWriteMaskBlue;
    if (eMask.IsSet(spColorWriteMask::Alpha))
      mask |= MTL::ColorWriteMaskAlpha;

    return mask;
  }

  EZ_ALWAYS_INLINE static MTL::CompareFunction spToMTL(const ezEnum<spDepthStencilComparison>& eComparison)
  {
    switch (eComparison)
    {
      case spDepthStencilComparison::Always:
        return MTL::CompareFunctionAlways;
      case spDepthStencilComparison::Equal:
        return MTL::CompareFunctionEqual;
      case spDepthStencilComparison::Greater:
        return MTL::CompareFunctionGreater;
      case spDepthStencilComparison::GreaterEqual:
        return MTL::CompareFunctionGreaterEqual;
      case spDepthStencilComparison::Less:
        return MTL::CompareFunctionLess;
      case spDepthStencilComparison::LessEqual:
        return MTL::CompareFunctionLessEqual;
      default:
      case spDepthStencilComparison::None:
      case spDepthStencilComparison::Never:
        return MTL::CompareFunctionNever;
      case spDepthStencilComparison::NotEqual:
        return MTL::CompareFunctionNotEqual;
    }
  }

  EZ_ALWAYS_INLINE static MTL::CullMode spToMTL(const ezEnum<spFaceCullMode>& eMode)
  {
    switch (eMode)
    {
      case spFaceCullMode::Front:
        return MTL::CullModeFront;
      case spFaceCullMode::Back:
        return MTL::CullModeBack;
      case spFaceCullMode::None:
        return MTL::CullModeNone;
    }
  }

  EZ_ALWAYS_INLINE static MTL::SamplerBorderColor spToMTL(const ezEnum<spSamplerBorderColor>& eColor)
  {
    switch (eColor)
    {
      case spSamplerBorderColor::TransparentBlack:
        return MTL::SamplerBorderColorTransparentBlack;
      case spSamplerBorderColor::OpaqueBlack:
        return MTL::SamplerBorderColorOpaqueBlack;
      case spSamplerBorderColor::OpaqueWhite:
        return MTL::SamplerBorderColorOpaqueWhite;
    }
  }

  EZ_ALWAYS_INLINE static MTL::SamplerAddressMode spToMTL(const ezEnum<spSamplerAddressMode>& eMode)
  {
    switch (eMode)
    {
      case spSamplerAddressMode::BorderColor:
        return MTL::SamplerAddressModeClampToBorderColor;
      case spSamplerAddressMode::ClampToEdge:
        return MTL::SamplerAddressModeClampToEdge;
      case spSamplerAddressMode::MirroredRepeat:
        return MTL::SamplerAddressModeMirrorRepeat;
      case spSamplerAddressMode::Repeat:
        return MTL::SamplerAddressModeRepeat;
      case spSamplerAddressMode::None:
        return MTL::SamplerAddressModeClampToZero;
    }
  }

  EZ_ALWAYS_INLINE static MTL::PrimitiveType spToMTL(const ezEnum<spPrimitiveTopology>& eTopology)
  {
    switch (eTopology)
    {
      case spPrimitiveTopology::Lines:
        return MTL::PrimitiveTypeLine;
      case spPrimitiveTopology::LineStrip:
        return MTL::PrimitiveTypeLineStrip;
      case spPrimitiveTopology::Triangles:
        return MTL::PrimitiveTypeTriangle;
      case spPrimitiveTopology::TriangleStrip:
        return MTL::PrimitiveTypeTriangleStrip;
      case spPrimitiveTopology::Points:
        return MTL::PrimitiveTypePoint;
    }
  }

  EZ_ALWAYS_INLINE static MTL::TextureUsage spToMTL(const ezBitflags<spTextureUsage>& eUsage)
  {
    MTL::TextureUsage ret = MTL::TextureUsageUnknown;

    if (eUsage.IsSet(spTextureUsage::Sampled))
    {
      ret |= MTL::TextureUsageShaderRead;
    }
    if (eUsage.IsSet(spTextureUsage::Storage))
    {
      ret |= MTL::TextureUsageShaderWrite;
    }
    if (eUsage.IsSet(spTextureUsage::DepthStencil) || eUsage.IsSet(spTextureUsage::RenderTarget))
    {
      ret |= MTL::TextureUsageRenderTarget;
    }

    return ret;
  }

  EZ_ALWAYS_INLINE static ezBitflags<spTextureUsage> spFromMTL(MTL::TextureUsage eUsage, bool bDepthStencil)
  {
    ezBitflags<spTextureUsage> ret;

    if ((eUsage & MTL::TextureUsageShaderRead) != 0)
    {
      ret |= spTextureUsage::Sampled;
    }
    if ((eUsage & MTL::TextureUsageShaderWrite) != 0)
    {
      ret |= spTextureUsage::Storage;
    }
    if ((eUsage & MTL::TextureUsageRenderTarget) != 0)
    {
      if (bDepthStencil)
        ret |= spTextureUsage::DepthStencil;
      else
        ret |= spTextureUsage::RenderTarget;
    }

    return ret;
  }

  EZ_ALWAYS_INLINE static MTL::VertexFormat spToMTL(const ezEnum<spInputElementFormat>& eFormat)
  {
    switch (eFormat)
    {
      case spInputElementFormat::Byte2Norm:
        return MTL::VertexFormatUChar2Normalized;
      case spInputElementFormat::Byte2:
        return MTL::VertexFormatUChar2;
      case spInputElementFormat::Byte4Norm:
        return MTL::VertexFormatUChar4Normalized;
      case spInputElementFormat::Byte4:
        return MTL::VertexFormatUChar4;
      case spInputElementFormat::SByte2Norm:
        return MTL::VertexFormatChar2Normalized;
      case spInputElementFormat::SByte2:
        return MTL::VertexFormatChar2;
      case spInputElementFormat::SByte4Norm:
        return MTL::VertexFormatChar4Normalized;
      case spInputElementFormat::SByte4:
        return MTL::VertexFormatChar4;
      case spInputElementFormat::UShort2Norm:
        return MTL::VertexFormatUShort2Normalized;
      case spInputElementFormat::UShort2:
        return MTL::VertexFormatUShort2;
      case spInputElementFormat::Short2Norm:
        return MTL::VertexFormatShort2Normalized;
      case spInputElementFormat::Short2:
        return MTL::VertexFormatShort2;
      case spInputElementFormat::UShort4Norm:
        return MTL::VertexFormatUShort4Normalized;
      case spInputElementFormat::UShort4:
        return MTL::VertexFormatUShort4;
      case spInputElementFormat::Short4Norm:
        return MTL::VertexFormatShort4Normalized;
      case spInputElementFormat::Short4:
        return MTL::VertexFormatShort4;
      case spInputElementFormat::UInt1:
        return MTL::VertexFormatUInt;
      case spInputElementFormat::UInt2:
        return MTL::VertexFormatUInt2;
      case spInputElementFormat::UInt3:
        return MTL::VertexFormatUInt3;
      case spInputElementFormat::UInt4:
        return MTL::VertexFormatUInt4;
      case spInputElementFormat::Int1:
        return MTL::VertexFormatInt;
      case spInputElementFormat::Int2:
        return MTL::VertexFormatInt2;
      case spInputElementFormat::Int3:
        return MTL::VertexFormatInt3;
      case spInputElementFormat::Int4:
        return MTL::VertexFormatInt4;
      case spInputElementFormat::Float1:
        return MTL::VertexFormatFloat;
      case spInputElementFormat::Float2:
        return MTL::VertexFormatFloat2;
      case spInputElementFormat::Float3:
        return MTL::VertexFormatFloat3;
      case spInputElementFormat::Float4:
        return MTL::VertexFormatFloat4;
      case spInputElementFormat::Half1:
        return MTL::VertexFormatHalf;
      case spInputElementFormat::Half2:
        return MTL::VertexFormatHalf2;
      case spInputElementFormat::Half4:
        return MTL::VertexFormatHalf4;
      case spInputElementFormat::R10G10B10A2UNorm:
        return MTL::VertexFormatUInt1010102Normalized;
    }
  }

  EZ_ALWAYS_INLINE static MTL::IndexType spToMTL(const ezEnum<spIndexFormat>& eFormat)
  {
    return eFormat == spIndexFormat::UInt16 ? MTL::IndexTypeUInt16 : MTL::IndexTypeUInt32;
  }

  EZ_ALWAYS_INLINE static MTL::StencilOperation spToMTL(const ezEnum<spStencilOperation>& eOperation)
  {
    switch (eOperation)
    {
      default:
      case spStencilOperation::Keep:
        return MTL::StencilOperationKeep;
      case spStencilOperation::Zero:
        return MTL::StencilOperationZero;
      case spStencilOperation::Replace:
        return MTL::StencilOperationReplace;
      case spStencilOperation::IncrementClamp:
        return MTL::StencilOperationIncrementClamp;
      case spStencilOperation::DecrementClamp:
        return MTL::StencilOperationDecrementClamp;
      case spStencilOperation::Invert:
        return MTL::StencilOperationInvert;
      case spStencilOperation::IncrementWrap:
        return MTL::StencilOperationIncrementWrap;
      case spStencilOperation::DecrementWrap:
        return MTL::StencilOperationDecrementWrap;
    }
  }

  EZ_ALWAYS_INLINE static MTL::SamplerMinMagFilter spToMTL(const ezEnum<spSamplerFilter>& eFilter)
  {
    switch (eFilter)
    {
      default:
      case spSamplerFilter::None:
      case spSamplerFilter::Linear:
        return MTL::SamplerMinMagFilterLinear;
      case spSamplerFilter::Point:
        return MTL::SamplerMinMagFilterNearest;
    }
  }

  EZ_ALWAYS_INLINE static MTL::SamplerMipFilter spToMTLMipFilter(const ezEnum<spSamplerFilter>& eFilter)
  {
    switch (eFilter)
    {
      default:
      case spSamplerFilter::None:
        return MTL::SamplerMipFilterNotMipmapped;
      case spSamplerFilter::Linear:
        return MTL::SamplerMipFilterLinear;
      case spSamplerFilter::Point:
        return MTL::SamplerMipFilterNearest;
    }
  }

#pragma endregion

#pragma region Getters

  EZ_ALWAYS_INLINE static ezUInt32 GetMaxTexture1DSize(MTL::FeatureSet eFeatureSet)
  {
    switch (eFeatureSet)
    {
      default:
      case MTL::FeatureSet_iOS_GPUFamily1_v1:
      case MTL::FeatureSet_iOS_GPUFamily2_v1:
        return 4096;
      case MTL::FeatureSet_iOS_GPUFamily1_v2:
      case MTL::FeatureSet_iOS_GPUFamily2_v2:
      case MTL::FeatureSet_iOS_GPUFamily1_v3:
      case MTL::FeatureSet_iOS_GPUFamily2_v3:
      case MTL::FeatureSet_iOS_GPUFamily1_v4:
      case MTL::FeatureSet_iOS_GPUFamily2_v4:
      case MTL::FeatureSet_tvOS_GPUFamily1_v1:
      case MTL::FeatureSet_tvOS_GPUFamily1_v2:
      case MTL::FeatureSet_tvOS_GPUFamily1_v3:
        return 8192;
      case MTL::FeatureSet_iOS_GPUFamily3_v1:
      case MTL::FeatureSet_iOS_GPUFamily3_v2:
      case MTL::FeatureSet_iOS_GPUFamily3_v3:
      case MTL::FeatureSet_iOS_GPUFamily4_v1:
      case MTL::FeatureSet_tvOS_GPUFamily2_v1:
      case MTL::FeatureSet_macOS_GPUFamily1_v1:
      case MTL::FeatureSet_macOS_GPUFamily1_v2:
      case MTL::FeatureSet_macOS_GPUFamily1_v3:
        return 16384;
    }
  }

  EZ_ALWAYS_INLINE static ezUInt32 GetMaxTexture2DSize(MTL::FeatureSet eFeatureSet)
  {
    switch (eFeatureSet)
    {
      default:
      case MTL::FeatureSet_iOS_GPUFamily1_v1:
      case MTL::FeatureSet_iOS_GPUFamily2_v1:
        return 4096;
      case MTL::FeatureSet_iOS_GPUFamily1_v2:
      case MTL::FeatureSet_iOS_GPUFamily2_v2:
      case MTL::FeatureSet_iOS_GPUFamily1_v3:
      case MTL::FeatureSet_iOS_GPUFamily2_v3:
      case MTL::FeatureSet_iOS_GPUFamily1_v4:
      case MTL::FeatureSet_iOS_GPUFamily2_v4:
      case MTL::FeatureSet_tvOS_GPUFamily1_v1:
      case MTL::FeatureSet_tvOS_GPUFamily1_v2:
      case MTL::FeatureSet_tvOS_GPUFamily1_v3:
        return 8192;
      case MTL::FeatureSet_iOS_GPUFamily3_v1:
      case MTL::FeatureSet_iOS_GPUFamily3_v2:
      case MTL::FeatureSet_iOS_GPUFamily3_v3:
      case MTL::FeatureSet_iOS_GPUFamily4_v1:
      case MTL::FeatureSet_tvOS_GPUFamily2_v1:
      case MTL::FeatureSet_macOS_GPUFamily1_v1:
      case MTL::FeatureSet_macOS_GPUFamily1_v2:
      case MTL::FeatureSet_macOS_GPUFamily1_v3:
        return 16384;
    }
  }

  EZ_ALWAYS_INLINE static ezUInt32 GetMaxTextureCubeSize(MTL::FeatureSet eFeatureSet)
  {
    switch (eFeatureSet)
    {
      default:
      case MTL::FeatureSet_iOS_GPUFamily1_v1:
      case MTL::FeatureSet_iOS_GPUFamily2_v1:
        return 4096;
      case MTL::FeatureSet_iOS_GPUFamily1_v2:
      case MTL::FeatureSet_iOS_GPUFamily2_v2:
      case MTL::FeatureSet_iOS_GPUFamily1_v3:
      case MTL::FeatureSet_iOS_GPUFamily2_v3:
      case MTL::FeatureSet_iOS_GPUFamily1_v4:
      case MTL::FeatureSet_iOS_GPUFamily2_v4:
      case MTL::FeatureSet_tvOS_GPUFamily1_v1:
      case MTL::FeatureSet_tvOS_GPUFamily1_v2:
      case MTL::FeatureSet_tvOS_GPUFamily1_v3:
        return 8192;
      case MTL::FeatureSet_iOS_GPUFamily3_v1:
      case MTL::FeatureSet_iOS_GPUFamily3_v2:
      case MTL::FeatureSet_iOS_GPUFamily3_v3:
      case MTL::FeatureSet_iOS_GPUFamily4_v1:
      case MTL::FeatureSet_tvOS_GPUFamily2_v1:
      case MTL::FeatureSet_macOS_GPUFamily1_v1:
      case MTL::FeatureSet_macOS_GPUFamily1_v2:
      case MTL::FeatureSet_macOS_GPUFamily1_v3:
        return 16384;
    }
  }

  EZ_ALWAYS_INLINE static ezUInt32 GetMaxTextureVolume(MTL::FeatureSet eFeatureSet)
  {
    return 2048;
  }

  EZ_ALWAYS_INLINE static bool spIsPixelFormatSupported(const ezEnum<spPixelFormat>& eFormat, const ezBitflags<spTextureUsage>& eTextureUsage, const spMTLSupportedFeatureSet& features)
  {
    switch (eFormat)
    {
      case spPixelFormat::Bc1RgbUNorm:
      case spPixelFormat::Bc1RgbUNormSRgb:
      case spPixelFormat::Bc1RgbaUNorm:
      case spPixelFormat::Bc1RgbaUNormSRgb:
      case spPixelFormat::Bc2UNorm:
      case spPixelFormat::Bc2UNormSRgb:
      case spPixelFormat::Bc3UNorm:
      case spPixelFormat::Bc3UNormSRgb:
      case spPixelFormat::Bc4UNorm:
      case spPixelFormat::Bc4SNorm:
      case spPixelFormat::Bc5UNorm:
      case spPixelFormat::Bc5SNorm:
      case spPixelFormat::Bc7UNorm:
      case spPixelFormat::Bc7UNormSRgb:
        return features.IsSupported(MTL::FeatureSet_macOS_GPUFamily1_v1) || features.IsSupported(MTL::FeatureSet_macOS_GPUFamily1_v2) || features.IsSupported(MTL::FeatureSet_macOS_GPUFamily1_v3);

      case spPixelFormat::Etc2R8G8B8UNorm:
      case spPixelFormat::Etc2R8G8B8A1UNorm:
      case spPixelFormat::Etc2R8G8B8A8UNorm:
        return features.IsSupported(MTL::FeatureSet_iOS_GPUFamily1_v1) || features.IsSupported(MTL::FeatureSet_iOS_GPUFamily2_v1) || features.IsSupported(MTL::FeatureSet_iOS_GPUFamily3_v1) || features.IsSupported(MTL::FeatureSet_iOS_GPUFamily4_v1);

      case spPixelFormat::R16UNorm:
        return (eTextureUsage.IsSet(spTextureUsage::DepthStencil) || features.IsSupported(MTL::FeatureSet_macOS_GPUFamily1_v2) || features.IsSupported(MTL::FeatureSet_macOS_GPUFamily1_v3));

      default:
        return true;
    }
  }

  EZ_ALWAYS_INLINE static bool spIsDepthFormat(MTL::PixelFormat eFormat)
  {
    switch (eFormat)
    {
      case MTL::PixelFormatStencil8:
      case MTL::PixelFormatX32_Stencil8:
      case MTL::PixelFormatX24_Stencil8:
      case MTL::PixelFormatDepth16Unorm:
      case MTL::PixelFormatDepth24Unorm_Stencil8:
      case MTL::PixelFormatDepth32Float:
      case MTL::PixelFormatDepth32Float_Stencil8:
        return true;
      default:
        return false;
    }
  }

#pragma endregion
} // namespace RHI
