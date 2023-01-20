#pragma once

#include <RHI/RHIDLL.h>

#include <Foundation/Math/Rect.h>

enum SP_RHI_CONSTANTS
{
  /// \brief The maximum number of color targets in a frame buffer.
  SP_RHI_MAX_COLOR_TARGETS = 8,

  /// \brief The maximum multi-buffering mode supported by the RHI.
  SP_RHI_MAX_BUFFERING_LEVEL = 3
};

typedef ezGenericId<28, 4> spResourceHandleId;
typedef ezGenericId<24, 8> spContextHandleId;

/// \brief An handle for every resource created by the graphics device.
class SP_RHI_DLL spResourceHandle
{
  EZ_DECLARE_HANDLE_TYPE(spResourceHandle, spResourceHandleId);
};

/// \brief An handle for every context created by the graphics device.
class SP_RHI_DLL spContextHandle
{
  EZ_DECLARE_HANDLE_TYPE(spContextHandle, spContextHandleId);
};

/// \brief A viewport used for rendering.
struct SP_RHI_DLL spViewport : ezHashableStruct<spViewport>
{
  EZ_ALWAYS_INLINE static spViewport From(ezRectI32 viewport)
  {
    return spViewport(
      viewport.x,
      viewport.y,
      static_cast<ezUInt32>(viewport.width),
      static_cast<ezUInt32>(viewport.height),
      0.0f,
      1.0f);
  }

  /// \brief Creates a new \see Viewport instance.
  /// \param iX The position over the X-axis of the viewport origin.
  /// \param iY The position over the Y-axis of the viewport origin.
  /// \param uiWidth The viewport width.
  /// \param uiHeight The viewport height.
  /// \param fMinDepth The viewport minimum depth.
  /// \param fMaxDepth The viewport maximum depth.
  spViewport(ezInt32 iX, ezInt32 iY, ezUInt32 uiWidth, ezUInt32 uiHeight, float fMinDepth, float fMaxDepth)
    : ezHashableStruct<spViewport>()
    , m_iX(iX)
    , m_iY(iY)
    , m_uiWidth(uiWidth)
    , m_uiHeight(uiHeight)
    , m_fMinDepth(fMinDepth)
    , m_fMaxDepth(fMaxDepth)
  {
  }

  /// \brief Gets the aspect ratio of this viewport.
  EZ_NODISCARD EZ_ALWAYS_INLINE float GetAspectRatio() const { return static_cast<float>(m_uiWidth) / m_uiHeight; }

  /// \brief Compares this \see spViewport to an \a other instance for equality.
  EZ_ALWAYS_INLINE bool operator==(const spViewport& other) const
  {
    return m_iX == other.m_iX && m_iY == other.m_iY && m_uiWidth == other.m_uiWidth && m_uiHeight == other.m_uiHeight && ezMath::IsEqual(m_fMinDepth, other.m_fMinDepth, ezMath::FloatEpsilon<float>()) && ezMath::IsEqual(m_fMaxDepth, other.m_fMaxDepth, ezMath::FloatEpsilon<float>());
  }

  /// \brief Compares this \see spViewport to an \a other instance for inequality.
  EZ_ALWAYS_INLINE bool operator!=(const spViewport& other) const
  {
    return !(*this == other);
  }

  ezInt32 m_iX;
  ezInt32 m_iY;
  ezUInt32 m_uiWidth;
  ezUInt32 m_uiHeight;
  float m_fMinDepth;
  float m_fMaxDepth;
};

/// \brief The list of graphics API usable by the spGraphicsDevice.
struct SP_RHI_DLL spGraphicsApi
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief OpenGL API (Windows, Linux)
    OpenGL,

    /// \brief OpenGLES API (Android)
    OpenGLES,

    /// \brief Direct3D 11 API (Windows, UWP, XBox360)
    Direct3D11,

    /// \brief Direct3D 12 API (Windows, UWP, XBox One, XBox X)
    Direct3D12,

    /// \brief Vulkan API (Windows, Linux, Mac OS X, Android, and many more platforms)
    Vulkan,

    /// \brief Metal API (Mac OS X, iOS)
    Metal,

    Default = OpenGL
  };
};

/// \brief The format of data stored in a spTexture.
///
/// Each name is a compound identifier, where each component denotes a name and a number of bits used to store that
/// component. The final component identifies the storage type of each component. "Float" identifies a signed, floating-point
/// type, UNorm identifies an unsigned integer type which is normalized, meaning it occupies the full space of the integer
/// type. The SRgb suffix for normalized integer formats indicates that the RGB components are stored in sRGB format.
struct SP_RHI_DLL spPixelFormat
{
  typedef ezUInt32 StorageType;

  enum Enum : StorageType
  {
    /// \brief RGBA component order. Each component is an 8-bit unsigned normalized integer.
    R8G8B8A8UNorm,

    /// \brief BGRA component order. Each component is an 8-bit unsigned normalized integer.
    B8G8R8A8UNorm,

    /// \brief Single-channel, 8-bit unsigned normalized integer.
    R8UNorm,

    /// \brief Single-channel, 16-bit unsigned normalized integer. Can be used as a depth format.
    R16UNorm,

    /// \brief RGBA component order. Each component is a 32-bit signed floating-point value.
    R32G32B32A32Float,

    /// \brief Single-channel, 32-bit signed floating-point value. Can be used as a depth format.
    R32Float,

    /// \brief Single-channel, 32-bit signed floating-point value. Used as a depth format.
    D32Float,

    /// \brief Single-channel, 24-bit unsigned normalized integer value. Used as a depth format.
    D24UNorm,

    /// \brief Single-channel, 16-bit unsigned normalized integer value. Used as a depth format.
    D16UNorm,

    /// \brief BC3 block compressed format.
    Bc3UNorm,

    /// \brief A depth-stencil format where the depth is stored in a 24-bit unsigned normalized integer, and the stencil is stored
    /// in an 8-bit unsigned integer.
    D24UNormS8UInt,

    /// \brief A depth-stencil format where the depth is stored in a 32-bit signed floating-point value, and the stencil is stored
    /// in an 8-bit unsigned integer.
    D32FloatS8UInt,

    /// \brief RGBA component order. Each component is a 32-bit unsigned integer.
    R32G32B32A32UInt,

    /// \brief BC1 block compressed format with no alpha.
    Bc1RgbUNorm,

    /// \brief BC1 block compressed format with a single-bit alpha channel.
    Bc1RgbaUNorm,

    /// \brief BC2 block compressed format.
    Bc2UNorm,

    /// \brief A 32-bit packed format. The 10-bit R component occupies bits 0..9, the 10-bit G component occupies bits 10..19,
    /// the 10-bit A component occupies 20..29, and the 2-bit A component occupies bits 30..31. Each value is an unsigned,
    /// normalized integer.
    R10G10B10A2UNorm,

    /// \brief A 32-bit packed format. The 10-bit R component occupies bits 0..9, the 10-bit G component occupies bits 10..19,
    /// the 10-bit A component occupies 20..29, and the 2-bit A component occupies bits 30..31. Each value is an unsigned
    /// integer.
    R10G10B10A2UInt,

    /// \brief A 32-bit packed format. The 11-bit R component occupies bits 0..10, the 11-bit G component occupies bits 11..21,
    /// and the 10-bit B component occupies bits 22..31. Each value is an unsigned floating point value.
    R11G11B10Float,

    /// \brief Single-channel, 8-bit signed normalized integer.
    R8SNorm,

    /// \brief Single-channel, 8-bit unsigned integer.
    R8UInt,

    /// \brief Single-channel, 8-bit signed integer.
    R8SInt,

    /// \brief Single-channel, 16-bit signed normalized integer.
    R16SNorm,

    /// \brief Single-channel, 16-bit unsigned integer.
    R16UInt,

    /// \brief Single-channel, 16-bit signed integer.
    R16SInt,

    /// \brief Single-channel, 16-bit signed floating-point value.
    R16Float,

    /// \brief Single-channel, 32-bit unsigned integer
    R32UInt,

    /// \brief Single-channel, 32-bit signed integer
    R32SInt,

    /// \brief RG component order. Each component is an 8-bit unsigned normalized integer.
    R8G8UNorm,

    /// \brief RG component order. Each component is an 8-bit signed normalized integer.
    R8G8SNorm,

    /// \brief RG component order. Each component is an 8-bit unsigned integer.
    R8G8UInt,

    /// \brief RG component order. Each component is an 8-bit signed integer.
    R8G8SInt,

    /// \brief RG component order. Each component is a 16-bit unsigned normalized integer.
    R16G16UNorm,

    /// \brief RG component order. Each component is a 16-bit signed normalized integer.
    R16G16SNorm,

    /// \brief RG component order. Each component is a 16-bit unsigned integer.
    R16G16UInt,

    /// \brief RG component order. Each component is a 16-bit signed integer.
    R16G16SInt,

    /// \brief RG component order. Each component is a 16-bit signed floating-point value.
    R16G16Float,

    /// \brief RG component order. Each component is a 32-bit unsigned integer.
    R32G32UInt,

    /// \brief RG component order. Each component is a 32-bit signed integer.
    R32G32SInt,

    /// \brief RG component order. Each component is a 32-bit signed floating-point value.
    R32G32Float,

    /// \brief RGB component order. Each component is an 8-bit unsigned normalized integer.
    R8G8B8UNorm,

    /// \brief RGB component order. Each component is an 8-bit signed normalized integer.
    R8G8B8SNorm,

    /// \brief RGB component order. Each component is an 8-bit unsigned integer.
    R8G8B8UInt,

    /// \brief RGB component order. Each component is an 8-bit signed integer.
    R8G8B8SInt,

    /// \brief RGB component order. Each component is a 16-bit unsigned normalized integer.
    R16G16B16UNorm,

    /// \brief RGB component order. Each component is a 16-bit signed normalized integer.
    R16G16B16SNorm,

    /// \brief RGB component order. Each component is a 16-bit unsigned integer.
    R16G16B16UInt,

    /// \brief RGB component order. Each component is a 16-bit signed integer.
    R16G16B16SInt,

    /// \brief RGB component order. Each component is a 16-bit signed floating-point value.
    R16G16B16Float,

    /// \brief RGB component order. Each component is a 32-bit unsigned integer.
    R32G32B32UInt,

    /// \brief RGB component order. Each component is a 32-bit signed integer.
    R32G32B32SInt,

    /// \brief RGB component order. Each component is a 32-bit signed floating-point value.
    R32G32B32Float,

    /// \brief RGBA component order. Each component is an 8-bit signed normalized integer.
    R8G8B8A8SNorm,

    /// \brief RGBA component order. Each component is an 8-bit unsigned integer.
    R8G8B8A8UInt,

    /// \brief RGBA component order. Each component is an 8-bit signed integer.
    R8G8B8A8SInt,

    /// \brief RGBA component order. Each component is a 16-bit unsigned normalized integer.
    R16G16B16A16UNorm,

    /// \brief RGBA component order. Each component is a 16-bit signed normalized integer.
    R16G16B16A16SNorm,

    /// \brief RGBA component order. Each component is a 16-bit unsigned integer.
    R16G16B16A16UInt,

    /// \brief RGBA component order. Each component is a 16-bit signed integer.
    R16G16B16A16SInt,

    /// \brief RGBA component order. Each component is a 16-bit floating-point value.
    R16G16B16A16Float,

    /// \brief RGBA component order. Each component is a 32-bit signed integer.
    R32G32B32A32SInt,

    /// \brief A 64-bit, 4x4 block-compressed format storing unsigned normalized RGB data.
    Etc2R8G8B8UNorm,

    /// \brief A 64-bit, 4x4 block-compressed format storing unsigned normalized RGB data, as well as 1 bit of alpha data.
    Etc2R8G8B8A1UNorm,

    /// \brief A 128-bit, 4x4 block-compressed format storing 64 bits of unsigned normalized RGB data, as well as 64 bits of alpha
    /// data.
    Etc2R8G8B8A8UNorm,

    /// \brief BC4 block compressed format, unsigned normalized values.
    Bc4UNorm,

    /// \brief BC4 block compressed format, signed normalized values.
    Bc4SNorm,

    /// \brief BC5 block compressed format, unsigned normalized values.
    Bc5UNorm,

    /// \brief BC5 block compressed format, signed normalized values.
    Bc5SNorm,

    /// \brief BC7 block compressed format.
    Bc7UNorm,

    /// \brief RGBA component order. Each component is an 8-bit unsigned normalized integer.
    /// This is an sRGB format.
    R8G8B8A8UNormSRgb,

    /// \brief BGRA component order. Each component is an 8-bit unsigned normalized integer.
    /// This is an sRGB format.
    B8G8R8A8UNormSRgb,

    /// \brief BC1 block compressed format with no alpha.
    /// This is an sRGB format.
    Bc1RgbUNormSRgb,

    /// \brief BC1 block compressed format with a single-bit alpha channel.
    /// This is an sRGB format.
    Bc1RgbaUNormSRgb,

    /// \brief BC2 block compressed format.
    /// This is an sRGB format.
    Bc2UNormSRgb,

    /// \brief BC3 block compressed format.
    /// This is an sRGB format.
    Bc3UNormSRgb,

    /// \brief BC7 block compressed format.
    /// This is an sRGB format.
    Bc7UNormSRgb,

    Default = R8G8B8A8UNorm
  };
};

/// \brief The samples count for a texture.
struct SP_RHI_DLL spTextureSampleCount
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief One sample. Disable multi sampling.
    None = 1,

    /// \brief Two samples.
    TwoSamples = 2,

    /// \brief Four samples.
    FourSamples = 4,

    /// \brief Eight samples.
    EightSamples = 8,

    /// \brief Sixteen samples.
    SixteenSamples = 16,

    /// \brief Thirty two samples.
    ThirtyTwoSamples = 32,

    Default = None
  };

  static Enum GetSampleCount(const StorageType samples)
  {
    switch (samples)
    {
      case 1:
        return None;
      case 2:
        return TwoSamples;
      case 4:
        return FourSamples;
      case 8:
        return EightSamples;
      case 16:
        return SixteenSamples;
      case 32:
        return ThirtyTwoSamples;
      default:
        return Default;
    }
  }
};

/// \brief Identifies how a \see spMappedResource will be mapped into CPU address space.
struct SP_RHI_DLL spMapAccess
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief A read-only resource mapping. The mapped data region is not writable, and cannot be used to transfer data into the
    /// graphics resource.
    /// \note This mode can only be used on resources created with the Staging usage flag.
    Read,

    /// \brief A write-only resource mapping. The mapped data region is writable, and will be transferred into the graphics resource
    /// when \see spDevice::UnMap() is called.
    /// \note Upon mapping a buffer with this mode, the previous contents of the resource will be erased.
    /// This mode can only be used to entirely replace the contents of a resource.
    Write,

    /// \brief A read-write resource mapping. The mapped data region is both readable and writable.
    /// \note This mode can only be used on resources created with the Staging usage flag.
    ReadWrite,

    Default = ReadWrite
  };
};

/// \brief Specifies the type of \see spShaderResource bound to a resource layout.
struct SP_RHI_DLL spShaderResourceType
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief A \see spBuffer accessed as a constant buffer. Uploads are synced between CPU and GPU.
    ConstantBuffer,

    /// \brief A \see spBuffer accessed as a read-only structured buffer.
    ReadOnlyStructuredBuffer,

    /// \brief A \see spBuffer accessed as a read-write structured buffer.
    ReadWriteStructuredBuffer,

    /// \brief A \see spTexture accessed as a texture or texture view with read-only capabilities.
    ReadOnlyTexture,

    /// \brief A \see spTexture accessed as a texture or texture view with read-write capabilities.
    ReadWriteTexture,

    /// \brief A \see spSampler.
    Sampler,

    Default = ConstantBuffer
  };
};

/// \brief Specifies the allows shader stage a \see spShaderResource
/// can be bound to.
struct SP_RHI_DLL spShaderStage
{
  typedef ezUInt32 StorageType;

  enum Enum : StorageType
  {
    /// \brief No stages.
    None = 0,

    /// \brief The vertex shader stage.
    VertexShader = EZ_BIT(0),

    /// \brief The pixel shader stage.
    PixelShader = EZ_BIT(1),

    /// \brief The geometry shader stage.
    GeometryShader = EZ_BIT(2),

    /// \brief The tessellation evaluation (domain) shader stage.
    DomainShader = EZ_BIT(3),

    /// \brief The tessellation control (hull) shader stage.
    HullShader = EZ_BIT(4),

    /// \brief The compute shader stage.
    ComputeShader = EZ_BIT(5),

    Default = None
  };

  struct Bits
  {
    StorageType VertexShader : 1;
    StorageType PixelShader : 1;
    StorageType GeometryShader : 1;
    StorageType DomainShader : 1;
    StorageType HullShader : 1;
    StorageType ComputeShader : 1;
  };
};

EZ_DECLARE_FLAGS_OPERATORS(spShaderStage);

/// \brief The data type of a specialization constant.
struct SP_RHI_DLL spShaderSpecializationConstantType
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief A boolean.
    Bool,

    /// \brief A 16-bit unsigned integer.
    UInt16,

    /// \brief A 16-bit signed integer.
    Int16,

    /// \brief A 32-bit unsigned integer.
    UInt32,

    /// \brief A 32-bit signed integer.
    Int32,

    /// \brief A 64-bit unsigned integer.
    UInt64,

    /// \brief A 64-bit signed integer.
    Int64,

    /// \brief A 32-bit floating-point value.
    Float,

    /// \brief A 64-bit floating-point value.
    Double,

    Default = Bool
  };
};

/// \brief A flag that indicates the possible usage for a \see spTexture resource.
struct SP_RHI_DLL spTextureUsage
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief The \see spTexture can be used as the target of a read-only \see spTextureView,
    /// and can be accessed from shaders.
    Sampled = EZ_BIT(0),

    /// \brief The \see spTexture can be used as the target of a read-write \see spTextureView,
    /// and can be accessed from shaders.
    Storage = EZ_BIT(1),

    /// \brief The texture is used as a color target of a \see spFramebuffer.
    RenderTarget = EZ_BIT(2),

    /// \brief The texture is used as a depth target of a \see spFramebuffer.
    DepthStencil = EZ_BIT(3),

    /// \brief The texture is used as a 2D cubemap.
    Cubemap = EZ_BIT(4),

    /// \brief The texture is used as a read-write staging resource for uploading data.
    Staging = EZ_BIT(5),

    /// \brief The texture supports automatic generation of mipmaps.
    GenerateMipmaps = EZ_BIT(6),

    Default = Sampled
  };

  struct Bits
  {
    StorageType Sampled : 1;
    StorageType Storage : 1;
    StorageType RenderTarget : 1;
    StorageType DepthStencil : 1;
    StorageType Cubemap : 1;
    StorageType Staging : 1;
    StorageType GenerateMipmaps : 1;
  };
};

/// \brief Specifies the dimension of a \see spTexture.
struct SP_RHI_DLL spTextureDimension
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief The texture is a 1D texture.
    Texture1D,

    /// \brief The texture is a 2D texture.
    Texture2D,

    /// \brief The texture is a 3D texture.
    Texture3D,

    Default = Texture2D
  };
};

/// \brief Specifies the wrapping mode applied to a \see spSampler.
struct SP_RHI_DLL spSamplerAddressMode
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief No wrapping. It is equivalent to use BorderColor wrapping with black.
    None = 0,

    /// \brief The texture is repeated.
    Repeat,

    /// \brief The texture is clamped with the edge colors.
    ClampToEdge,

    /// \brief The texture is clamped with a custom border color.
    BorderColor,

    /// \brief The texture is repeated with a mirror pattern.
    MirroredRepeat,

    Default = Repeat
  };
};

/// \brief Specifies the filtering mode applied to a \see spSampler.
struct SP_RHI_DLL spSamplerFilter
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief Ignore filtering. The final behavior is determined by the platform-specific
    /// implementation of the RHI.
    None,

    /// \brief Point sampling.
    Point,

    /// \brief Linear sampling.
    Linear,

    Default = Linear
  };
};

/// \brief Specifies how new values are compared with
/// existing values during depth or stencil comparison.
struct SP_RHI_DLL spDepthStencilComparison
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief Do not compare.
    None = 0,

    /// \brief The comparison never succeeds.
    Never,

    /// \brief The comparison succeeds when the new value is equal to the existing value.
    Equal,

    /// \brief The comparison succeeds when the new value is not equal to the existing value.
    NotEqual,

    /// \brief The comparison succeeds when the new value is less than the existing value.
    Less,

    /// \brief The comparison succeeds when the new value is less than or equal to the existing value.
    LessEqual,

    /// \brief The comparison succeeds when the new value is greater than the existing value.
    Greater,

    /// \brief The comparison succeeds when the new value is greater than or equal to the existing value.
    GreaterEqual,

    /// \brief The comparison always succeeds.
    Always,

    Default = None
  };
};

/// \brief A bitmask describing the permitted uses of a buffer.
struct spBufferUsage
{
  using StorageType = ezUInt32;

  enum Enum : StorageType
  {
#pragma region Buffer Types

    /// \brief Indicates that a buffer is can be used as the source of vertex data for drawing commands.
    /// This flag enables the use of a buffer in the spCommandList::SetVertexBuffer() method.
    VertexBuffer = EZ_BIT(0),

    /// \brief Indicates that a buffer is can be used as the source of index data for drawing commands.
    /// This flag enables the use of a buffer in the spCommandList::SetIndexBuffer() method.
    IndexBuffer = EZ_BIT(1),

    /// \brief Indicates that a buffer is can be used as a constant buffer.
    /// This flag enables the use of a buffer in a spResourceSet as a constant buffer.
    ConstantBuffer = EZ_BIT(2),

    /// \brief Indicates that a buffer is can be used as a read-only structured buffer.
    /// This flag enables the use of a buffer in a spResourceSet as a read-only structured buffer.
    /// This flag can only be combined with Dynamic or PersistentMapping.
    StructuredBufferReadOnly = EZ_BIT(3),

    /// \brief Indicates that a buffer is can be used as a read-write structured buffer.
    /// This flag enables the use of a buffer in a ResourceSet as a read-write structured buffer.
    /// This flag can only be combined with PersistentMapping.
    StructuredBufferReadWrite = EZ_BIT(4),

    /// \brief Indicates that a buffer is can be used as a the source of indirect drawing data.
    /// This flag enables the use of a buffer in the spCommandList::*Indirect() methods.
    /// This flag cannot be combined with Dynamic.
    IndirectBuffer = EZ_BIT(5),

#pragma endregion

#pragma region Buffer Usage

    /// \brief Indicates that a buffer will be updated with new data very frequently. Dynamic buffers can be
    /// mapped with spMapMode::Write. This flag cannot be combined with StructuredBufferReadWrite.
    Dynamic = EZ_BIT(6),

    /// \brief Indicates that a buffer will be used as a staging buffer. Staging buffers can be used to transfer data
    /// to-and-from the CPU using spGraphicDevice::Map(). Staging buffers can use any spMapMode values.
    /// This flag can only be combined with PersistentMapping.
    Staging = EZ_BIT(7),

    /// \brief Indicates that a buffer should use persistent mapping if available on the running platform. When PersistentMapping
    /// is enabled, any calls to spGraphicDevice::Map() on this buffer will return the same pointer.
    /// This flag can be combined with any other flags.
    PersistentMapping = EZ_BIT(8),

    /// \brief Indicates that a buffer is double buffered. Double buffering will consume twice the buffer size in memory.
    /// This flag cannot be combined with TripleBuffered.
    DoubleBuffered = EZ_BIT(9),

    /// \brief Indicates that a buffer is triple buffered. Triple buffering will consume three times the buffer size
    /// in memory. This flag cannot be combined with DoubleBuffered.
    TripleBuffered = EZ_BIT(10),

#pragma endregion

    Default = ConstantBuffer
  };

  struct Bits
  {
    StorageType VertexBuffer : 1;
    StorageType IndexBuffer : 1;
    StorageType ConstantBuffer : 1;
    StorageType StructuredBufferReadOnly : 1;
    StorageType StructuredBufferReadWrite : 1;
    StorageType IndirectBuffer : 1;
    StorageType Dynamic : 1;
    StorageType Staging : 1;
    StorageType PersistentMapping : 1;
    StorageType DoubleBuffered : 1;
    StorageType TripleBuffered : 1;
  };
};

EZ_DECLARE_FLAGS_OPERATORS(spBufferUsage);

/// \brief The format of data in an \see spIndexBuffer.
struct spIndexFormat
{
  using StorageType = ezUInt8;

  enum Enum : StorageType
  {
    /// \brief Each index is a 16-bit unsigned integer.
    UInt16,

    /// \brief Each index is a 32-bit unsigned integer.
    UInt32,

    Default = UInt32
  };
};

/// \brief The possible formats of an input element.
struct SP_RHI_DLL spInputElementFormat
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief One 32-bit floating point value.
    Float1,

    /// \brief Two 32-bit floating point values.
    Float2,

    /// \brief Three 32-bit floating point values.
    Float3,

    /// \brief Four 32-bit floating point values.
    Float4,

    /// \brief Two 8-bit unsigned normalized integers.
    Byte2Norm,

    /// \brief Two 8-bit unsigned integers.
    Byte2,

    /// \brief Four 8-bit unsigned normalized integers.
    Byte4Norm,

    /// \brief Four 8-bit unsigned integers.
    Byte4,

    /// \brief Two 8-bit signed normalized integers.
    SByte2Norm,

    /// \brief Two 8-bit signed integers.
    SByte2,

    /// \brief Four 8-bit signed normalized integers.
    SByte4Norm,

    /// \brief Four 8-bit signed integers.
    SByte4,

    /// \brief Two 16-bit unsigned normalized integers.
    UShort2Norm,

    /// \brief Two 16-bit unsigned integers.
    UShort2,

    /// \brief Four 16-bit unsigned normalized integers.
    UShort4Norm,

    /// \brief Four 16-bit unsigned integers.
    UShort4,

    /// \brief Two 16-bit signed normalized integers.
    Short2Norm,

    /// \brief Two 16-bit signed integers.
    Short2,

    /// \brief Four 16-bit signed normalized integers.
    Short4Norm,

    /// \brief Four 16-bit signed integers.
    Short4,

    /// \brief One 32-bit unsigned integer.
    UInt1,

    /// \brief Two 32-bit unsigned integers.
    UInt2,

    /// \brief Three 32-bit unsigned integers.
    UInt3,

    /// \brief Four 32-bit unsigned integers.
    UInt4,

    /// \brief One 32-bit signed integer.
    Int1,

    /// \brief Two 32-bit signed integers.
    Int2,

    /// \brief Three 32-bit signed integers.
    Int3,

    /// \brief Four 32-bit signed integers.
    Int4,

    /// \brief One 16-bit floating point value.
    Half1,

    /// \brief Two 16-bit floating point values.
    Half2,

    /// \brief Four 16-bit floating point values.
    Half4,

    Default = Float4
  };

  /// \brief Gets the number of elements in the given
  /// input format.
  static ezUInt32 GetElementCount(Enum format)
  {
    switch (format)
    {
      case Float1:
      case UInt1:
      case Int1:
      case Half1:
        return 1;

      case Float2:
      case Byte2Norm:
      case Byte2:
      case SByte2Norm:
      case SByte2:
      case UShort2Norm:
      case UShort2:
      case Short2Norm:
      case Short2:
      case UInt2:
      case Int2:
      case Half2:
        return 2;

      case Float3:
      case UInt3:
      case Int3:
        return 3;

      case Float4:
      case Byte4Norm:
      case Byte4:
      case SByte4Norm:
      case SByte4:
      case UShort4Norm:
      case UShort4:
      case Short4Norm:
      case Short4:
      case UInt4:
      case Int4:
      case Half4:
        return 4;

      default:
        EZ_ASSERT_NOT_IMPLEMENTED;
        return 0;
    }
  }

  /// \brief Gets the size in bytes of the given input format.
  static ezUInt32 GetSizeInBytes(Enum format)
  {
    switch (format)
    {
      case Byte2Norm:
      case Byte2:
      case SByte2Norm:
      case SByte2:
      case Half1:
        return 2;

      case Float1:
      case UInt1:
      case Int1:
      case Byte4Norm:
      case Byte4:
      case SByte4Norm:
      case SByte4:
      case UShort2Norm:
      case UShort2:
      case Short2Norm:
      case Short2:
      case Half2:
        return 4;

      case Float2:
      case UInt2:
      case Int2:
      case UShort4Norm:
      case UShort4:
      case Short4Norm:
      case Short4:
      case Half4:
        return 8;

      case Float3:
      case UInt3:
      case Int3:
        return 12;

      case Float4:
      case UInt4:
      case Int4:
        return 16;

      default:
        EZ_ASSERT_NOT_IMPLEMENTED;
        return 0;
    }
  }
};

/// \brief The default location semantics used by input elements.
/// Can be extended by using the \see spInputElementLocationSemantic::Last value
/// and adding the desired offset.
struct SP_RHI_DLL spInputElementLocationSemantic
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief Defines an input layout element storing vertices positions
    /// data. Use the location 0 of the \see ShaderPipeline.
    Position = 0,

    /// \brief Defines an input layout element storing texture coordinates
    /// data. Use the location 2 of the \see ShaderPipeline.
    TextureCoordinate = 1,

    /// \brief Defines an input layout element vertices normal vectors
    /// data. Use the location 3 of the \see ShaderPipeline.
    Normal = 2,

    /// \brief Defines an input layout element vertices tangent vectors
    /// data. Use the location 4 of the \see ShaderPipeline.
    Tangent = 3,

    /// \brief Defines an input layout element vertices bi-tangent vectors
    /// data. Use the location 5 of the \see ShaderPipeline.
    BiTangent = 4,

    /// \brief Defines the last possible value for input layout element managed
    /// by the engine. Any value after this one is user defined.
    Last = BiTangent,

    Default = Position
  };
};

/// \brief Defines some options for a \see spResourceLayoutElementDescription
struct SP_RHI_DLL spResourceLayoutElementOptions
{
  typedef ezUInt32 StorageType;

  enum Enum : StorageType
  {
    /// \brief No special options
    None = 0,

    /// \brief Can be applied to a buffer resource type (\see spShaderResourceType::ReadOnlyBuffer,
    /// \see spShaderResourceType::ReadWriteBuffer, or \see spShaderResourceType::UniformBuffer), allowing it to be
    /// bound with a dynamic offset using \see spCommandList::SetGraphicsResourceSet(ezUInt32, spResourceSet, ezDynamicArray<ezUInt32>).
    /// Offsets specified this way must be a multiple of \see spDevice::GetUniformBufferMinOffsetAlignment() or
    /// \see spDevice::GetStructuredBufferMinOffsetAlignment().
    DynamicBinding = EZ_BIT(0),
  };

  struct Bits
  {
    StorageType DynamicBinding : 1;
  };
};

EZ_DECLARE_FLAGS_OPERATORS(spResourceLayoutElementOptions);

/// \brief Determines how a sequence of vertices is interpreted by the rasterizer.
struct SP_RHI_DLL spPrimitiveTopology
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief A list of isolated, 3-elements triangles.
    Triangles,

    /// \brief A series of connected triangles.
    TriangleStrip,

    /// \brief A series of isolated, 2-elements segments.
    Lines,

    /// \brief A series of connected line segments.
    LineStrip,

    /// \brief A series of isolated points.
    Points,

    Default = Triangles
  };
};

/// \brief Controls the influence of components in a blend operation.
struct SP_RHI_DLL spBlendFactor
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief Each component is multiplied by 0.
    Zero,

    /// \brief Each component is multiplied by 1.
    One,

    /// \brief Each component is multiplied by the source alpha component.
    SourceAlpha,

    /// \brief Each component is multiplied by (1 - source alpha).
    InverseSourceAlpha,

    /// \brief Each component is multiplied by the destination alpha.
    DestinationAlpha,

    /// \brief Each component is multiplied by (1 - destination alpha).
    InverseDestinationAlpha,

    /// \brief Each component is multiplied by the matching component of the source color.
    SourceColor,

    /// \brief Each component is multiplied by (1 - the matching component of the source color).
    InverseSourceColor,

    /// \brief Each component is multiplied by the matching component of the destination color.
    DestinationColor,

    /// \brief Each component is multiplied by (1 - the matching component of the destination color).
    InverseDestinationColor,

    /// \brief Each component is multiplied by the matching component in constant factor as specified in \see spBlendState::m_BlendFactor.
    BlendFactor,

    /// \brief Each component is multiplied by (1 - the matching component in constant factor as specified in \see spBlendState::m_BlendFactor).
    InverseBlendFactor,

    Default = SourceAlpha,
  };
};

/// \brief Controls how the source and destination factors are combined in a blend operation.
struct SP_RHI_DLL spBlendFunction
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief Source and destination are added.
    Add,

    /// \brief Destination is subtracted from source.
    Subtract,

    /// \brief Source is subtracted from destination.
    ReverseSubtract,

    /// \brief The minimum of source and destination is selected.
    Minimum,

    /// \brief The maximum of source and destination is selected.
    Maximum,

    Default = Add
  };
};

/// \brief Specifies an action taken on samples that pass or fail the stencil test.
struct SP_RHI_DLL spStencilOperation
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief Keep the existing value.
    Keep,

    /// \brief Sets the value to 0.
    Zero,

    /// \brief Replaces the existing value with the specified value.
    Replace,

    /// \brief Increments the existing value and clamp it to the maximum representable unsigned value.
    IncrementClamp,

    /// \brief Decrements the existing value and clamp it to 0.
    DecrementClamp,

    /// \brief Bitwise inverts the existing value.
    Invert,

    /// \brief Increments the existing value and wraps it to 0 when it exceeds the maximum representable unsigned value.
    IncrementWrap,

    /// \brief Decrements the existing value and wraps it to the maximum representable unsigned value when it would be reduced below 0.
    DecrementWrap,

    Default = Keep
  };
};

/// \brief Indicates which faces will be culled.
struct SP_RHI_DLL spFaceCullMode
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief No face culling.
    None,

    /// \brief Cull the front face.
    Front,

    /// \brief Cull the back face.
    Back,

    Default = None
  };
};

/// \brief The winding order used to determine the front face of a primitive.
struct SP_RHI_DLL spFrontFace
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief Clockwise winding order.
    Clockwise,

    /// \brief Counter-clockwise winding order.
    CounterClockwise,

    Default = Clockwise
  };
};

/// \brief Indicates how the rasterizer should fill polygons.
struct SP_RHI_DLL spPolygonFillMode
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief Polygons are completely filled.
    Solid,

    /// \brief Polygons are outlined.
    Wireframe,

    /// \brief Only polygon points are drawn.
    Point,

    Default = Solid
  };
};
