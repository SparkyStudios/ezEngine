#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <dxgiformat.h>

#include <RHI/Core.h>

#pragma region Structs

template <typename T>
class spD3D11ScopedResource
{
public:
  explicit spD3D11ScopedResource(T* resource = nullptr)
    : m_pResource(resource)
  {
  }

  ~spD3D11ScopedResource()
  {
    SP_RHI_DX11_RELEASE(m_pResource);
  }

  EZ_ALWAYS_INLINE T* operator*() const
  {
    return m_pResource;
  }

  EZ_ALWAYS_INLINE T** operator&() const
  {
    return &m_pResource;
  }

  EZ_ALWAYS_INLINE T* operator->() const
  {
    return static_cast<T*>(m_pResource);
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
    return static_cast<T*>(m_pResource);
  }

private:
  T* m_pResource;
};

#pragma endregion

#pragma region Conversion Functions

EZ_ALWAYS_INLINE static ezEnum<spPixelFormat> spFromD3D11(DXGI_FORMAT eFormat)
{
  switch (eFormat)
  {
    case DXGI_FORMAT_R8_UNORM:
      return spPixelFormat::R8UNorm;
    case DXGI_FORMAT_R8_SNORM:
      return spPixelFormat::R8SNorm;
    case DXGI_FORMAT_R8_UINT:
      return spPixelFormat::R8UInt;
    case DXGI_FORMAT_R8_SINT:
      return spPixelFormat::R8SInt;

    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_D16_UNORM:
      return spPixelFormat::R16UNorm;
    case DXGI_FORMAT_R16_SNORM:
      return spPixelFormat::R16SNorm;
    case DXGI_FORMAT_R16_UINT:
      return spPixelFormat::R16UInt;
    case DXGI_FORMAT_R16_SINT:
      return spPixelFormat::R16SInt;
    case DXGI_FORMAT_R16_FLOAT:
      return spPixelFormat::R16Float;

    case DXGI_FORMAT_R32_UINT:
      return spPixelFormat::R32UInt;
    case DXGI_FORMAT_R32_SINT:
      return spPixelFormat::R32SInt;
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_D32_FLOAT:
      return spPixelFormat::R32Float;

    case DXGI_FORMAT_R8G8_UNORM:
      return spPixelFormat::R8G8UNorm;
    case DXGI_FORMAT_R8G8_SNORM:
      return spPixelFormat::R8G8SNorm;
    case DXGI_FORMAT_R8G8_UINT:
      return spPixelFormat::R8G8UInt;
    case DXGI_FORMAT_R8G8_SINT:
      return spPixelFormat::R8G8SInt;

    case DXGI_FORMAT_R16G16_UNORM:
      return spPixelFormat::R16G16UNorm;
    case DXGI_FORMAT_R16G16_SNORM:
      return spPixelFormat::R16G16SNorm;
    case DXGI_FORMAT_R16G16_UINT:
      return spPixelFormat::R16G16UInt;
    case DXGI_FORMAT_R16G16_SINT:
      return spPixelFormat::R16G16SInt;
    case DXGI_FORMAT_R16G16_FLOAT:
      return spPixelFormat::R16G16Float;

    case DXGI_FORMAT_R32G32_UINT:
      return spPixelFormat::R32G32UInt;
    case DXGI_FORMAT_R32G32_SINT:
      return spPixelFormat::R32G32SInt;
    case DXGI_FORMAT_R32G32_FLOAT:
      return spPixelFormat::R32G32Float;

    case DXGI_FORMAT_R8G8B8A8_UNORM:
      return spPixelFormat::R8G8B8A8UNorm;
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
      return spPixelFormat::R8G8B8A8UNormSRgb;

    case DXGI_FORMAT_B8G8R8A8_UNORM:
      return spPixelFormat::B8G8R8A8UNorm;
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
      return spPixelFormat::B8G8R8A8UNormSRgb;
    case DXGI_FORMAT_R8G8B8A8_SNORM:
      return spPixelFormat::R8G8B8A8SNorm;
    case DXGI_FORMAT_R8G8B8A8_UINT:
      return spPixelFormat::R8G8B8A8UInt;
    case DXGI_FORMAT_R8G8B8A8_SINT:
      return spPixelFormat::R8G8B8A8SInt;

    case DXGI_FORMAT_R16G16B16A16_UNORM:
      return spPixelFormat::R16G16B16A16UNorm;
    case DXGI_FORMAT_R16G16B16A16_SNORM:
      return spPixelFormat::R16G16B16A16SNorm;
    case DXGI_FORMAT_R16G16B16A16_UINT:
      return spPixelFormat::R16G16B16A16UInt;
    case DXGI_FORMAT_R16G16B16A16_SINT:
      return spPixelFormat::R16G16B16A16SInt;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
      return spPixelFormat::R16G16B16A16Float;

    case DXGI_FORMAT_R32G32B32A32_UINT:
      return spPixelFormat::R32G32B32A32UInt;
    case DXGI_FORMAT_R32G32B32A32_SINT:
      return spPixelFormat::R32G32B32A32SInt;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
      return spPixelFormat::R32G32B32A32Float;

    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_TYPELESS:
      return spPixelFormat::Bc1RgbaUNorm;
    case DXGI_FORMAT_BC2_UNORM:
      return spPixelFormat::Bc2UNorm;
    case DXGI_FORMAT_BC3_UNORM:
      return spPixelFormat::Bc3UNorm;
    case DXGI_FORMAT_BC4_UNORM:
      return spPixelFormat::Bc4UNorm;
    case DXGI_FORMAT_BC4_SNORM:
      return spPixelFormat::Bc4SNorm;
    case DXGI_FORMAT_BC5_UNORM:
      return spPixelFormat::Bc5UNorm;
    case DXGI_FORMAT_BC5_SNORM:
      return spPixelFormat::Bc5SNorm;
    case DXGI_FORMAT_BC7_UNORM:
      return spPixelFormat::Bc7UNorm;

    case DXGI_FORMAT_D24_UNORM_S8_UINT:
      return spPixelFormat::D24UNormS8UInt;
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
      return spPixelFormat::D32FloatS8UInt;

    case DXGI_FORMAT_R10G10B10A2_UINT:
      return spPixelFormat::R10G10B10A2UInt;
    case DXGI_FORMAT_R10G10B10A2_UNORM:
      return spPixelFormat::R10G10B10A2UNorm;
    case DXGI_FORMAT_R11G11B10_FLOAT:
      return spPixelFormat::R11G11B10Float;

    default:
      EZ_ASSERT_NOT_IMPLEMENTED
      return spPixelFormat::Default;
  }
}

/// \brief Coverts a \see spBufferUsage flags into a \see D3D11_BIND_FLAG.
EZ_ALWAYS_INLINE static UINT spToD3D11(ezBitflags<spBufferUsage> eUsage)
{
  UINT flags = 0;

  if (eUsage.IsSet(spBufferUsage::VertexBuffer))
    flags |= D3D11_BIND_VERTEX_BUFFER;

  if (eUsage.IsSet(spBufferUsage::IndexBuffer))
    flags |= D3D11_BIND_INDEX_BUFFER;

  if (eUsage.IsSet(spBufferUsage::ConstantBuffer))
    flags |= D3D11_BIND_CONSTANT_BUFFER;

  if (eUsage.IsAnySet(spBufferUsage::StructuredBufferReadOnly | spBufferUsage::StructuredBufferReadWrite))
    flags |= D3D11_BIND_SHADER_RESOURCE;

  if (eUsage.IsSet(spBufferUsage::StructuredBufferReadWrite))
    flags |= D3D11_BIND_UNORDERED_ACCESS;

  return flags;
}

EZ_ALWAYS_INLINE static DXGI_FORMAT spToD3D11(const ezEnum<spPixelFormat>& eFormat, bool bDepthFormat)
{
  switch (eFormat)
  {
    case spPixelFormat::R8UNorm:
      return DXGI_FORMAT_R8_UNORM;
    case spPixelFormat::R8SNorm:
      return DXGI_FORMAT_R8_SNORM;
    case spPixelFormat::R8UInt:
      return DXGI_FORMAT_R8_UINT;
    case spPixelFormat::R8SInt:
      return DXGI_FORMAT_R8_SINT;

    case spPixelFormat::R16UNorm:
      return bDepthFormat ? DXGI_FORMAT_R16_TYPELESS : DXGI_FORMAT_R16_UNORM;
    case spPixelFormat::R16SNorm:
      return DXGI_FORMAT_R16_SNORM;
    case spPixelFormat::R16UInt:
      return DXGI_FORMAT_R16_UINT;
    case spPixelFormat::R16SInt:
      return DXGI_FORMAT_R16_SINT;
    case spPixelFormat::R16Float:
      return DXGI_FORMAT_R16_FLOAT;

    case spPixelFormat::R32UInt:
      return DXGI_FORMAT_R32_UINT;
    case spPixelFormat::R32SInt:
      return DXGI_FORMAT_R32_SINT;
    case spPixelFormat::R32Float:
      return bDepthFormat ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_R32_FLOAT;

    case spPixelFormat::R8G8UNorm:
      return DXGI_FORMAT_R8G8_UNORM;
    case spPixelFormat::R8G8SNorm:
      return DXGI_FORMAT_R8G8_SNORM;
    case spPixelFormat::R8G8UInt:
      return DXGI_FORMAT_R8G8_UINT;
    case spPixelFormat::R8G8SInt:
      return DXGI_FORMAT_R8G8_SINT;

    case spPixelFormat::R16G16UNorm:
      return DXGI_FORMAT_R16G16_UNORM;
    case spPixelFormat::R16G16SNorm:
      return DXGI_FORMAT_R16G16_SNORM;
    case spPixelFormat::R16G16UInt:
      return DXGI_FORMAT_R16G16_UINT;
    case spPixelFormat::R16G16SInt:
      return DXGI_FORMAT_R16G16_SINT;
    case spPixelFormat::R16G16Float:
      return DXGI_FORMAT_R16G16_FLOAT;

    case spPixelFormat::R32G32UInt:
      return DXGI_FORMAT_R32G32_UINT;
    case spPixelFormat::R32G32SInt:
      return DXGI_FORMAT_R32G32_SINT;
    case spPixelFormat::R32G32Float:
      return DXGI_FORMAT_R32G32_FLOAT;

    case spPixelFormat::R8G8B8A8UNorm:
      return DXGI_FORMAT_R8G8B8A8_UNORM;
    case spPixelFormat::R8G8B8A8UNormSRgb:
      return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case spPixelFormat::R8G8B8A8SNorm:
      return DXGI_FORMAT_R8G8B8A8_SNORM;
    case spPixelFormat::R8G8B8A8UInt:
      return DXGI_FORMAT_R8G8B8A8_UINT;
    case spPixelFormat::R8G8B8A8SInt:
      return DXGI_FORMAT_R8G8B8A8_SINT;

    case spPixelFormat::B8G8R8A8UNorm:
      return DXGI_FORMAT_B8G8R8A8_UNORM;
    case spPixelFormat::B8G8R8A8UNormSRgb:
      return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

    case spPixelFormat::R16G16B16A16UNorm:
      return DXGI_FORMAT_R16G16B16A16_UNORM;
    case spPixelFormat::R16G16B16A16SNorm:
      return DXGI_FORMAT_R16G16B16A16_SNORM;
    case spPixelFormat::R16G16B16A16UInt:
      return DXGI_FORMAT_R16G16B16A16_UINT;
    case spPixelFormat::R16G16B16A16SInt:
      return DXGI_FORMAT_R16G16B16A16_SINT;
    case spPixelFormat::R16G16B16A16Float:
      return DXGI_FORMAT_R16G16B16A16_FLOAT;

    case spPixelFormat::R32G32B32A32UInt:
      return DXGI_FORMAT_R32G32B32A32_UINT;
    case spPixelFormat::R32G32B32A32SInt:
      return DXGI_FORMAT_R32G32B32A32_SINT;
    case spPixelFormat::R32G32B32A32Float:
      return DXGI_FORMAT_R32G32B32A32_FLOAT;

    case spPixelFormat::Bc1RgbUNorm:
    case spPixelFormat::Bc1RgbaUNorm:
      return DXGI_FORMAT_BC1_UNORM;

    case spPixelFormat::Bc1RgbUNormSRgb:
    case spPixelFormat::Bc1RgbaUNormSRgb:
      return DXGI_FORMAT_BC1_UNORM_SRGB;

    case spPixelFormat::Bc2UNorm:
      return DXGI_FORMAT_BC2_UNORM;

    case spPixelFormat::Bc2UNormSRgb:
      return DXGI_FORMAT_BC2_UNORM_SRGB;

    case spPixelFormat::Bc3UNorm:
      return DXGI_FORMAT_BC3_UNORM;

    case spPixelFormat::Bc3UNormSRgb:
      return DXGI_FORMAT_BC3_UNORM_SRGB;

    case spPixelFormat::Bc4UNorm:
      return DXGI_FORMAT_BC4_UNORM;
    case spPixelFormat::Bc4SNorm:
      return DXGI_FORMAT_BC4_SNORM;

    case spPixelFormat::Bc5UNorm:
      return DXGI_FORMAT_BC5_UNORM;
    case spPixelFormat::Bc5SNorm:
      return DXGI_FORMAT_BC5_SNORM;

    case spPixelFormat::Bc7UNorm:
      return DXGI_FORMAT_BC7_UNORM;
    case spPixelFormat::Bc7UNormSRgb:
      return DXGI_FORMAT_BC7_UNORM_SRGB;

    case spPixelFormat::D16UNorm:
      EZ_ASSERT_DEV(bDepthFormat, "Trying to use a depth format without bDepthFormat as true.");
      return DXGI_FORMAT_D16_UNORM;

    case spPixelFormat::D24UNorm:
      EZ_ASSERT_DEV(bDepthFormat, "Trying to use a depth format without bDepthFormat as true.");
      return DXGI_FORMAT_D24_UNORM_S8_UINT;

    case spPixelFormat::D24UNormS8UInt:
      EZ_ASSERT_DEV(bDepthFormat, "Trying to use a depth format without bDepthFormat as true.");
      return DXGI_FORMAT_D24_UNORM_S8_UINT;

    case spPixelFormat::D32Float:
      EZ_ASSERT_DEV(bDepthFormat, "Trying to use a depth format without bDepthFormat as true.");
      return DXGI_FORMAT_D32_FLOAT;

    case spPixelFormat::D32FloatS8UInt:
      EZ_ASSERT_DEV(bDepthFormat, "Trying to use a depth format without bDepthFormat as true.");
      return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

    case spPixelFormat::R10G10B10A2UNorm:
      return DXGI_FORMAT_R10G10B10A2_UNORM;

    case spPixelFormat::R10G10B10A2UInt:
      return DXGI_FORMAT_R10G10B10A2_UINT;

    case spPixelFormat::R11G11B10Float:
      return DXGI_FORMAT_R11G11B10_FLOAT;

    case spPixelFormat::Etc2R8G8B8UNorm:
    case spPixelFormat::Etc2R8G8B8A1UNorm:
    case spPixelFormat::Etc2R8G8B8A8UNorm:
      EZ_ASSERT_DEV(false, "D3D11 doesn't support ETC formats.");

    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      return DXGI_FORMAT_UNKNOWN;
  }
}

EZ_ALWAYS_INLINE static D3D11_COMPARISON_FUNC spToD3D11(const ezEnum<spDepthStencilComparison>& eFunction)
{
  switch (eFunction)
  {
    default:
    case spDepthStencilComparison::None:
    case spDepthStencilComparison::Never:
      return D3D11_COMPARISON_NEVER;
    case spDepthStencilComparison::Equal:
      return D3D11_COMPARISON_EQUAL;
    case spDepthStencilComparison::NotEqual:
      return D3D11_COMPARISON_NOT_EQUAL;
    case spDepthStencilComparison::Less:
      return D3D11_COMPARISON_LESS;
    case spDepthStencilComparison::LessEqual:
      return D3D11_COMPARISON_LESS_EQUAL;
    case spDepthStencilComparison::Greater:
      return D3D11_COMPARISON_GREATER;
    case spDepthStencilComparison::GreaterEqual:
      return D3D11_COMPARISON_GREATER_EQUAL;
    case spDepthStencilComparison::Always:
      return D3D11_COMPARISON_ALWAYS;
  }
}

EZ_ALWAYS_INLINE static D3D11_TEXTURE_ADDRESS_MODE spToD3D11(const ezEnum<spSamplerAddressMode>& eAddressMode)
{
  switch (eAddressMode)
  {
    default:
    case spSamplerAddressMode::None:
      return D3D11_TEXTURE_ADDRESS_CLAMP;
    case spSamplerAddressMode::Repeat:
      return D3D11_TEXTURE_ADDRESS_WRAP;
    case spSamplerAddressMode::ClampToEdge:
      return D3D11_TEXTURE_ADDRESS_CLAMP;
    case spSamplerAddressMode::BorderColor:
      return D3D11_TEXTURE_ADDRESS_BORDER;
    case spSamplerAddressMode::MirroredRepeat:
      return D3D11_TEXTURE_ADDRESS_MIRROR;
  }
}

EZ_ALWAYS_INLINE static D3D11_FILTER spToD3D11(const ezEnum<spSamplerFilter>& eMinFilter, const ezEnum<spSamplerFilter>& eMagFilter, const ezEnum<spSamplerFilter>& eMipFilter, bool bIsComparison)
{
  if (eMipFilter == spSamplerFilter::Linear)
  {
    if (eMinFilter == eMagFilter)
    {
      if (eMinFilter == spSamplerFilter::Point)
        return bIsComparison ? D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR : D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;

      if (eMinFilter == spSamplerFilter::Linear)
        return bIsComparison ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    }
    else
    {
      if (eMinFilter == spSamplerFilter::Point && eMagFilter == spSamplerFilter::Linear)
        return bIsComparison ? D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR : D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;

      if (eMinFilter == spSamplerFilter::Linear && eMagFilter == spSamplerFilter::Point)
        return bIsComparison ? D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR : D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    }
  }
  else
  {
    if (eMinFilter == eMagFilter)
    {
      if (eMinFilter == spSamplerFilter::Point)
        return bIsComparison ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D11_FILTER_MIN_MAG_MIP_POINT;

      if (eMinFilter == spSamplerFilter::Linear)
        return bIsComparison ? D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT : D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    }
    else
    {
      if (eMinFilter == spSamplerFilter::Point && eMagFilter == spSamplerFilter::Linear)
        return bIsComparison ? D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT : D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;

      if (eMinFilter == spSamplerFilter::Linear && eMagFilter == spSamplerFilter::Point)
        return bIsComparison ? D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT : D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
    }
  }

  EZ_ASSERT_NOT_IMPLEMENTED;
  return D3D11_FILTER_MIN_MAG_MIP_POINT;
}

EZ_ALWAYS_INLINE static D3D11_MAP spToD3D11(const ezEnum<spMapAccess>& eMapAccess, bool bIsDynamic)
{
  switch (eMapAccess)
  {
    case spMapAccess::Read:
      return D3D11_MAP_READ;
    case spMapAccess::Write:
      return bIsDynamic ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE;
    case spMapAccess::ReadWrite:
      return D3D11_MAP_READ_WRITE;
    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      return D3D11_MAP_READ;
  }
}

EZ_ALWAYS_INLINE static UINT spToD3D11(const ezBitflags<spColorWriteMask>& eColorWriteMask)
{
  UINT uiEnable = 0;

  if (eColorWriteMask.IsSet(spColorWriteMask::Red))
    uiEnable |= D3D11_COLOR_WRITE_ENABLE_RED;

  if (eColorWriteMask.IsSet(spColorWriteMask::Green))
    uiEnable |= D3D11_COLOR_WRITE_ENABLE_GREEN;

  if (eColorWriteMask.IsSet(spColorWriteMask::Blue))
    uiEnable |= D3D11_COLOR_WRITE_ENABLE_BLUE;

  if (eColorWriteMask.IsSet(spColorWriteMask::Alpha))
    uiEnable |= D3D11_COLOR_WRITE_ENABLE_ALPHA;

  return uiEnable;
}

EZ_ALWAYS_INLINE static D3D11_BLEND spToD3D11(const ezEnum<spBlendFactor>& eBlendFactor)
{
  switch (eBlendFactor)
  {
    case spBlendFactor::Zero:
      return D3D11_BLEND_ZERO;
    case spBlendFactor::One:
      return D3D11_BLEND_ONE;
    case spBlendFactor::SourceColor:
      return D3D11_BLEND_SRC_COLOR;
    case spBlendFactor::InverseSourceColor:
      return D3D11_BLEND_INV_SRC_COLOR;
    case spBlendFactor::SourceAlpha:
      return D3D11_BLEND_SRC_ALPHA;
    case spBlendFactor::InverseSourceAlpha:
      return D3D11_BLEND_INV_SRC_ALPHA;
    case spBlendFactor::DestinationColor:
      return D3D11_BLEND_DEST_COLOR;
    case spBlendFactor::InverseDestinationColor:
      return D3D11_BLEND_INV_DEST_COLOR;
    case spBlendFactor::DestinationAlpha:
      return D3D11_BLEND_DEST_ALPHA;
    case spBlendFactor::InverseDestinationAlpha:
      return D3D11_BLEND_INV_DEST_ALPHA;
    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      return D3D11_BLEND_ZERO;
  }
}

EZ_ALWAYS_INLINE static D3D11_BLEND_OP spToD3D11(const ezEnum<spBlendFunction>& eBlendFunction)
{
  switch (eBlendFunction)
  {
    case spBlendFunction::Add:
      return D3D11_BLEND_OP_ADD;
    case spBlendFunction::Subtract:
      return D3D11_BLEND_OP_SUBTRACT;
    case spBlendFunction::ReverseSubtract:
      return D3D11_BLEND_OP_REV_SUBTRACT;
    case spBlendFunction::Minimum:
      return D3D11_BLEND_OP_MIN;
    case spBlendFunction::Maximum:
      return D3D11_BLEND_OP_MAX;
    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      return D3D11_BLEND_OP_ADD;
  }
}

EZ_ALWAYS_INLINE static D3D11_STENCIL_OP spToD3D11(const ezEnum<spStencilOperation>& eStencilOperation)
{
  switch (eStencilOperation)
  {
    case spStencilOperation::Keep:
      return D3D11_STENCIL_OP_KEEP;
    case spStencilOperation::Zero:
      return D3D11_STENCIL_OP_ZERO;
    case spStencilOperation::Replace:
      return D3D11_STENCIL_OP_REPLACE;
    case spStencilOperation::IncrementClamp:
      return D3D11_STENCIL_OP_INCR_SAT;
    case spStencilOperation::DecrementClamp:
      return D3D11_STENCIL_OP_DECR_SAT;
    case spStencilOperation::Invert:
      return D3D11_STENCIL_OP_INVERT;
    case spStencilOperation::IncrementWrap:
      return D3D11_STENCIL_OP_INCR;
    case spStencilOperation::DecrementWrap:
      return D3D11_STENCIL_OP_DECR;
    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      return D3D11_STENCIL_OP_KEEP;
  }
}

EZ_ALWAYS_INLINE static D3D11_CULL_MODE spToD3D11(const ezEnum<spFaceCullMode>& eCullMode)
{
  switch (eCullMode)
  {
    case spFaceCullMode::Back:
      return D3D11_CULL_BACK;
    case spFaceCullMode::Front:
      return D3D11_CULL_FRONT;
    case spFaceCullMode::None:
      return D3D11_CULL_NONE;
    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      return D3D11_CULL_BACK;
  }
}

EZ_ALWAYS_INLINE static D3D11_FILL_MODE spToD3D11(const ezEnum<spPolygonFillMode>& eFillMode)
{
  switch (eFillMode)
  {
    case spPolygonFillMode::Wireframe:
      return D3D11_FILL_WIREFRAME;
    case spPolygonFillMode::Solid:
      return D3D11_FILL_SOLID;
    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      return D3D11_FILL_SOLID;
  }
}

EZ_ALWAYS_INLINE static DXGI_FORMAT spToD3D11(const ezEnum<spInputElementFormat>& eFormat)
{
  switch (eFormat)
  {
    case spInputElementFormat::Float1:
      return DXGI_FORMAT_R32_FLOAT;
    case spInputElementFormat::Float2:
      return DXGI_FORMAT_R32G32_FLOAT;
    case spInputElementFormat::Float3:
      return DXGI_FORMAT_R32G32B32_FLOAT;
    case spInputElementFormat::Float4:
      return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case spInputElementFormat::Byte2:
      return DXGI_FORMAT_R8G8_UINT;
    case spInputElementFormat::Byte2Norm:
      return DXGI_FORMAT_R8G8_UNORM;
    case spInputElementFormat::Byte4:
      return DXGI_FORMAT_R8G8B8A8_UINT;
    case spInputElementFormat::Byte4Norm:
      return DXGI_FORMAT_R8G8B8A8_UNORM;
    case spInputElementFormat::SByte2:
      return DXGI_FORMAT_R8G8_SINT;
    case spInputElementFormat::SByte2Norm:
      return DXGI_FORMAT_R8G8_SNORM;
    case spInputElementFormat::SByte4:
      return DXGI_FORMAT_R8G8B8A8_SINT;
    case spInputElementFormat::SByte4Norm:
      return DXGI_FORMAT_R8G8B8A8_SNORM;
    case spInputElementFormat::Short2:
      return DXGI_FORMAT_R16G16_SINT;
    case spInputElementFormat::Short2Norm:
      return DXGI_FORMAT_R16G16_SNORM;
    case spInputElementFormat::Short4:
      return DXGI_FORMAT_R16G16B16A16_SINT;
    case spInputElementFormat::Short4Norm:
      return DXGI_FORMAT_R16G16B16A16_SNORM;
    case spInputElementFormat::UShort2:
      return DXGI_FORMAT_R16G16_UINT;
    case spInputElementFormat::UShort2Norm:
      return DXGI_FORMAT_R16G16_UNORM;
    case spInputElementFormat::UShort4:
      return DXGI_FORMAT_R16G16B16A16_UINT;
    case spInputElementFormat::UShort4Norm:
      return DXGI_FORMAT_R16G16B16A16_UNORM;
    case spInputElementFormat::Int1:
      return DXGI_FORMAT_R32_SINT;
    case spInputElementFormat::Int2:
      return DXGI_FORMAT_R32G32_SINT;
    case spInputElementFormat::Int3:
      return DXGI_FORMAT_R32G32B32_SINT;
    case spInputElementFormat::Int4:
      return DXGI_FORMAT_R32G32B32A32_SINT;
    case spInputElementFormat::UInt1:
      return DXGI_FORMAT_R32_UINT;
    case spInputElementFormat::UInt2:
      return DXGI_FORMAT_R32G32_UINT;
    case spInputElementFormat::UInt3:
      return DXGI_FORMAT_R32G32B32_UINT;
    case spInputElementFormat::UInt4:
      return DXGI_FORMAT_R32G32B32A32_UINT;
    case spInputElementFormat::Half1:
      return DXGI_FORMAT_R16_FLOAT;
    case spInputElementFormat::Half2:
      return DXGI_FORMAT_R16G16_FLOAT;
    case spInputElementFormat::Half4:
      return DXGI_FORMAT_R16G16B16A16_FLOAT;
    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      return DXGI_FORMAT_UNKNOWN;
  }
}

EZ_ALWAYS_INLINE static D3D11_PRIMITIVE_TOPOLOGY spToD3D11(const ezEnum<spPrimitiveTopology>& eTopology)
{
  switch (eTopology)
  {
    case spPrimitiveTopology::Points:
      return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    case spPrimitiveTopology::Lines:
      return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
    case spPrimitiveTopology::LineStrip:
      return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
    case spPrimitiveTopology::Triangles:
      return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case spPrimitiveTopology::TriangleStrip:
      return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
  }
}

#pragma endregion

#pragma region Utility Functions

EZ_ALWAYS_INLINE static DXGI_FORMAT spGetTypelessFormat(DXGI_FORMAT eFormat)
{
  switch (eFormat)
  {
    case DXGI_FORMAT_UNKNOWN:
      return DXGI_FORMAT_UNKNOWN;

    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
      return DXGI_FORMAT_R32G32B32A32_TYPELESS;

    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
      return DXGI_FORMAT_R32G32B32_TYPELESS;

    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
      return DXGI_FORMAT_R16G16B16A16_TYPELESS;

    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
      return DXGI_FORMAT_R32G32_TYPELESS;

    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
      return DXGI_FORMAT_R32G8X24_TYPELESS;

    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
      return DXGI_FORMAT_R10G10B10A2_TYPELESS;

    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
      return DXGI_FORMAT_R8G8B8A8_TYPELESS;

    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_R16G16_SINT:
      return DXGI_FORMAT_R16G16_TYPELESS;

    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
      return DXGI_FORMAT_R32_TYPELESS;

    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
      return DXGI_FORMAT_R24G8_TYPELESS;

    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8_SINT:
      return DXGI_FORMAT_R8G8_TYPELESS;

    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_R16_SINT:
      return DXGI_FORMAT_R16_TYPELESS;

    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_R8_SINT:
    case DXGI_FORMAT_A8_UNORM:
      return DXGI_FORMAT_R8_TYPELESS;

    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
      return DXGI_FORMAT_BC1_TYPELESS;

    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
      return DXGI_FORMAT_BC2_TYPELESS;

    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
      return DXGI_FORMAT_BC3_TYPELESS;

    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
      return DXGI_FORMAT_BC4_TYPELESS;

    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
      return DXGI_FORMAT_BC5_TYPELESS;

    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
      return DXGI_FORMAT_B8G8R8A8_TYPELESS;

    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
      return DXGI_FORMAT_B8G8R8X8_TYPELESS;

    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
      return DXGI_FORMAT_BC6H_TYPELESS;

    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
      return DXGI_FORMAT_BC7_TYPELESS;

    default:
      return eFormat;
  }
}

EZ_ALWAYS_INLINE static DXGI_FORMAT spGetViewFormat(DXGI_FORMAT eFormat)
{
  switch (eFormat)
  {
    case DXGI_FORMAT_R16_TYPELESS:
      return DXGI_FORMAT_R16_UNORM;

    case DXGI_FORMAT_R32_TYPELESS:
      return DXGI_FORMAT_R32_FLOAT;

    case DXGI_FORMAT_R24G8_TYPELESS:
      return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

    case DXGI_FORMAT_R32G8X24_TYPELESS:
      return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

    default:
      return eFormat;
  }
}

EZ_ALWAYS_INLINE static DXGI_FORMAT spGetDepthFormat(const ezEnum<spPixelFormat>& eFormat)
{
  switch (eFormat)
  {
    case spPixelFormat::R32Float:
      return DXGI_FORMAT_D32_FLOAT;

    case spPixelFormat::R16Float:
      return DXGI_FORMAT_D16_UNORM;

    case spPixelFormat::D24UNormS8UInt:
      return DXGI_FORMAT_D24_UNORM_S8_UINT;

    case spPixelFormat::D32FloatS8UInt:
      return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

    default:
      EZ_ASSERT_DEV(false, "Invalid depth texture pixel format");
      return DXGI_FORMAT_UNKNOWN;
  }
}

EZ_ALWAYS_INLINE static bool spIsCompressedFormat(const ezEnum<spPixelFormat>& eFormat)
{
  return eFormat == spPixelFormat::Bc1RgbUNorm || eFormat == spPixelFormat::Bc1RgbUNormSRgb || eFormat == spPixelFormat::Bc1RgbaUNorm || eFormat == spPixelFormat::Bc1RgbaUNormSRgb || eFormat == spPixelFormat::Bc2UNorm || eFormat == spPixelFormat::Bc2UNormSRgb || eFormat == spPixelFormat::Bc3UNorm || eFormat == spPixelFormat::Bc3UNormSRgb || eFormat == spPixelFormat::Bc4UNorm || eFormat == spPixelFormat::Bc4SNorm || eFormat == spPixelFormat::Bc5UNorm || eFormat == spPixelFormat::Bc5SNorm || eFormat == spPixelFormat::Bc7UNorm || eFormat == spPixelFormat::Bc7UNormSRgb || eFormat == spPixelFormat::Etc2R8G8B8UNorm || eFormat == spPixelFormat::Etc2R8G8B8A1UNorm || eFormat == spPixelFormat::Etc2R8G8B8A8UNorm;
}

EZ_ALWAYS_INLINE static ezBitflags<spTextureUsage> spGetTextureUsage(UINT bindFlags, UINT cpuAccessFlags, UINT optionFlags)
{
  ezBitflags<spTextureUsage> usage;

  if ((bindFlags & D3D11_BIND_RENDER_TARGET) != 0)
    usage |= spTextureUsage::RenderTarget;

  if ((bindFlags & D3D11_BIND_DEPTH_STENCIL) != 0)
    usage |= spTextureUsage::DepthStencil;

  if ((bindFlags & D3D11_BIND_SHADER_RESOURCE) != 0)
    usage |= spTextureUsage::Sampled;

  if ((bindFlags & D3D11_BIND_UNORDERED_ACCESS) != 0)
    usage |= spTextureUsage::Storage;

  if ((optionFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) != 0)
    usage |= spTextureUsage::Cubemap;

  if ((optionFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS) != 0)
  {
    usage |= spTextureUsage::GenerateMipmaps;
  }

  return usage;
}

#pragma endregion
