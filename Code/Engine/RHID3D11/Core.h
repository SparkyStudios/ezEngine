#pragma once

#include <d3d11.h>

#include <RHI/Core.h>

/// \brief Coverts a \see spBufferUsage flags into a \see D3D11_BIND_FLAG.
static UINT spToD3D11(ezBitflags<spBufferUsage> eUsage)
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
/*
static DXGI_FORMAT spToD3D11(const ezEnum<spPixelFormat>& eFormat, bool bDepthFormat)
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
*/
static D3D11_COMPARISON_FUNC spToD3D11(const ezEnum<spDepthStencilComparison>& eFunction)
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

static D3D11_TEXTURE_ADDRESS_MODE spToD3D11(const ezEnum<spSamplerAddressMode>& eAddressMode)
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

static D3D11_FILTER spToD3D11(const ezEnum<spSamplerFilter>& eMinFilter, const ezEnum<spSamplerFilter>& eMagFilter, const ezEnum<spSamplerFilter>& eMipFilter, bool bIsComparison)
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
