#pragma once

#include <RHI/RHIDLL.h>

#include <Foundation/Containers/IdTable.h>

#include <RHI/Core.h>

struct spBufferDescription;
struct spBufferRangeDescription;
struct spCommandListDescription;
struct spComputePipelineDescription;
struct spFenceDescription;
struct spFramebufferDescription;
struct spInputLayoutDescription;
struct spGraphicPipelineDescription;
struct spResourceLayoutDescription;
struct spResourceSetDescription;
struct spSamplerDescription;
struct spShaderDescription;
struct spSwapchainDescription;
struct spTextureDescription;
struct spTextureViewDescription;

class spShader;
class spShaderProgram;
class spTexture;
class spSampler;
class spInputLayout;
class spBuffer;
class spResourceLayout;
class spTextureView;
class spSwapchain;
class spFence;
class spFramebuffer;
class spComputePipeline;
class spGraphicPipeline;
class spResourceSet;
class spDevice;
class spBufferRange;
class spCommandList;

/// \brief Abstract class for a resource on the graphics device.
class SP_RHI_DLL spDeviceResource : public ezReflectedClass, public ezRefCounted
{
  friend class spDeviceResourceManager;

public:
  spDeviceResource() = default;
  ~spDeviceResource() override = default;

  /// \brief Gets the handle of this resource.
  EZ_NODISCARD EZ_ALWAYS_INLINE spResourceHandle GetHandle() const { return m_Handle; }

  /// \brief Gets the debug name of the resource.
  EZ_NODISCARD EZ_ALWAYS_INLINE const ezString& GetDebugName() const { return m_sDebugName; }

  /// \brief Sets the debug name of the resource.
  EZ_ALWAYS_INLINE virtual void SetDebugName(const ezString& debugName) { m_sDebugName = debugName; }

  /// \brief Gets the graphics device in which this resource has been created.
  EZ_NODISCARD EZ_ALWAYS_INLINE spDevice* GetDevice() const { return m_pDevice; }

  /// \brief Releases the resource.
  virtual void ReleaseResource() = 0;

  /// \brief Check whether the resource has been released.
  EZ_NODISCARD virtual bool IsReleased() const = 0;

protected:
  spResourceHandle m_Handle;

  ezString m_sDebugName;
  spDevice* m_pDevice{nullptr};

  bool m_bReleased{false};
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spDeviceResource);

/// \brief Abstract class for device resources which are created only when needed.
class SP_RHI_DLL spDeferredDeviceResource
{
public:
  spDeferredDeviceResource() = default;
  virtual ~spDeferredDeviceResource() = default;

  void EnsureResourceCreated();

  /// \brief Gets whether the resource has been created.
  EZ_NODISCARD EZ_ALWAYS_INLINE bool IsResourceCreated() const { return m_bIsResourceCreated; }

  /// \brief Creates the resource, or do nothing if the resource
  /// is already created.
  virtual void CreateResource() = 0;

protected:
  bool m_bIsResourceCreated{false};
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spDeferredDeviceResource);

/// \brief Abstract class for device resources used in shaders.
class SP_RHI_DLL spShaderResource : public spDeviceResource
{
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spShaderResource);

/// \brief Abstract class for shader resources which can be mapped to
/// CPU memory.
class SP_RHI_DLL spMappableResource : public spShaderResource
{
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spMappableResource);

/// \brief Describes the layout of a mapped resource object.
class SP_RHI_DLL spMappedResource : public ezRefCounted
{
public:
  /// \brief Constructs an empty spMappedResource
  spMappedResource();

  /// \brief Constructs a spMappedResource from a resource handle and a raw pointer.
  /// \param [in] hResource The handle of the resource which should be mapped.
  /// \param [in] eAccess The access mode of the mapped resource.
  /// \param [in] pData Pointer to the mapped data region.
  /// \param [in] uiSize The size in bytes of the mapped data region.
  /// \param [in] uiSubResource The sub-resource which is mapped.
  /// \param [in] uiRowPitch The number of bytes between each row of texels for texture resources.
  /// \param [in] uiDepthPitch The number of bytes between each depth slice for 3D texture resources.
  spMappedResource(const spResourceHandle& hResource, ezEnum<spMapAccess> eAccess, void* pData, ezUInt32 uiSize, ezUInt32 uiSubResource = 0, ezUInt32 uiRowPitch = 0, ezUInt32 uiDepthPitch = 0);

  /// \brief Constructs a spMappedResource from a resource handle and a raw pointer.
  /// \param [in] hResource The handle of the resource which should be mapped.
  /// \param [in] eAccess The access mode of the mapped resource.
  /// \param [in] data A byte array to get data and size from.
  spMappedResource(const spResourceHandle& hResource, ezEnum<spMapAccess> eAccess, const ezByteArrayPtr& data);

  /// \brief Checks whether the mapped resource is valid.
  EZ_NODISCARD EZ_ALWAYS_INLINE bool IsValid() const { return !m_hResource.IsInvalidated() && m_pData != nullptr; }

  /// \brief Gets the mapped resource.
  EZ_NODISCARD const spResourceHandle& GetResource() const;

  /// \brief Gets the mapped resource access mode.
  EZ_NODISCARD ezEnum<spMapAccess> GetAccess() const;

  /// \brief Gets the mapped resource data pointer.
  EZ_NODISCARD void* GetData() const;

  /// \brief Gets the mapped resource size.
  EZ_NODISCARD ezUInt32 GetSize() const;

  /// \brief Gets the mapped resource sub-resource.
  EZ_NODISCARD ezUInt32 GetSubResource() const;

  /// \brief Gets the mapped resource row pitch.
  EZ_NODISCARD ezUInt32 GetRowPitch() const;

  /// \brief Gets the mapped resource depth pitch.
  EZ_NODISCARD ezUInt32 GetDepthPitch() const;

private:
  /// \brief The mapped resource.
  spResourceHandle m_hResource;

  /// \brief The access mode used to map the resource.
  ezEnum<spMapAccess> m_eAccess;

  /// \brief A pointer to the start of the mapped data region.
  void* m_pData{nullptr};

  /// \brief The size in bytes of the mapped data region.
  ezUInt32 m_uiSize{0};

  /// \brief For mapped \see spTexture resources, this is the subresource which is mapped.
  /// For \see spBuffer resources, this field has no meaning.
  ezUInt32 m_uiSubResource{0};

  /// \brief For mapped \see spTexture resources, this is the number of bytes between each row of texels.
  /// For \see spBuffer resources, this field has no meaning.
  ezUInt32 m_uiRowPitch{0};

  /// \brief For mapped \see spTexture resources, this is the number of bytes between each depth slice of a 3D Texture.
  /// For \see spBuffer resources or 2D Textures, this field has no meaning.
  ezUInt32 m_uiDepthPitch{0};
};

/// \brief A typed view of a \see spMappedResource. Provides by-reference structured
/// access to individual elements in the mapped data.
template <typename T>
class SP_RHI_DLL spMappedResourceView
{
public:
  /// \brief The \see spMappedResource viewed by this instance.
  EZ_NODISCARD EZ_ALWAYS_INLINE const spMappedResource& GetMappedResource() const { return m_MappedResource; }

  /// \brief The total size in bytes of the mapped resource.
  EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetSize() const { return m_uiSize; }

  /// \brief The total number of structures that is contained in the mapped resource.
  /// This is effectively the total number of bytes divided by the size of the structure type.
  EZ_NODISCARD EZ_ALWAYS_INLINE ezInt32 GetCount() const { return m_uiCount; }

  /// \brief Creates a typed view of the given \see spMappedResource.
  spMappedResourceView(const spMappedResource& resource)
  {
    m_MappedResource = resource;
    m_uiSize = m_MappedResource.GetSize();
    m_uiCount = m_uiSize / SizeOfT;
  }

  EZ_ALWAYS_INLINE T& operator[](ezUInt32 index)
  {
    EZ_ASSERT_DEV(index < m_uiCount, "Given index ({0}) is out of range.", index);
    return GetAt(index);
  }

  EZ_ALWAYS_INLINE const T& operator[](ezUInt32 index) const
  {
    EZ_ASSERT_DEV(index < m_uiCount, "Given index ({0}) is out of range.", index);
    return GetAt(index);
  }

  EZ_ALWAYS_INLINE T& operator[](ezInt32 index)
  {
    EZ_ASSERT_DEV(index < m_uiCount && index > 0, "Given index ({0}) is out of range.", index);
    return GetAt(static_cast<ezUInt32>(index));
  }

  EZ_ALWAYS_INLINE const T& operator[](ezInt32 index) const
  {
    EZ_ASSERT_DEV(index < m_uiCount && index > 0, "Given index ({0}) is out of range.", index);
    return GetAt(static_cast<ezUInt32>(index));
  }

  EZ_ALWAYS_INLINE T& operator[](ezVec2U32 index)
  {
    return GetAt(index.x, index.y);
  }

  EZ_ALWAYS_INLINE const T& operator[](ezVec2U32 index) const
  {
    return GetAt(index.x, index.y);
  }

  EZ_ALWAYS_INLINE T& operator[](ezVec2I32 index)
  {
    EZ_ASSERT_DEV(index.x > 0 && index.y > 0, "Given index ({0}) is out of range.", index);
    return GetAt(static_cast<ezUInt32>(index.x), static_cast<ezUInt32>(index.y));
  }

  EZ_ALWAYS_INLINE const T& operator[](ezVec2I32 index) const
  {
    EZ_ASSERT_DEV(index.x > 0 && index.y > 0, "Given index ({0}) is out of range.", index);
    return GetAt(static_cast<ezUInt32>(index.x), static_cast<ezUInt32>(index.y));
  }

  EZ_ALWAYS_INLINE T& operator[](ezVec3U32 index)
  {
    return GetAt(index.x, index.y, index.z);
  }

  EZ_ALWAYS_INLINE const T& operator[](ezVec3U32 index) const
  {
    return GetAt(index.x, index.y, index.z);
  }

  EZ_ALWAYS_INLINE T& operator[](ezVec3I32 index)
  {
    EZ_ASSERT_DEV(index.x > 0 && index.y > 0 && index.z > 0, "Given index ({0}) is out of range.", index);
    return GetAt(static_cast<ezUInt32>(index.x), static_cast<ezUInt32>(index.y), static_cast<ezUInt32>(index.z));
  }

  EZ_ALWAYS_INLINE const T& operator[](ezVec3I32 index) const
  {
    EZ_ASSERT_DEV(index.x > 0 && index.y > 0 && index.z > 0, "Given index ({0}) is out of range.", index);
    return GetAt(static_cast<ezUInt32>(index.x), static_cast<ezUInt32>(index.y), static_cast<ezUInt32>(index.z));
  }

private:
  static const constexpr ezUInt64 SizeOfT = sizeof(T);

  EZ_ALWAYS_INLINE T& GetAt(ezUInt32 x, ezUInt32 y = 0, ezUInt32 z = 0)
  {
    ezUInt8* ptr = static_cast<ezUInt8*>(m_MappedResource.GetData()) + (z * m_MappedResource.GetDepthPitch()) + (y * m_MappedResource.GetRowPitch()) + (x * SizeOfT);
    return *reinterpret_cast<T*>(ptr);
  }

  spMappedResource m_MappedResource;
  ezUInt32 m_uiSize;
  ezUInt32 m_uiCount;
};

/// \brief A simple implementation of a cache key used to store mappable resources in
/// a map. Used internally by RHI backends.
struct SP_RHI_DLL spMappedResourceCacheKey : public ezHashableStruct<spMappedResourceCacheKey>
{
  spMappedResourceCacheKey() = default;

  spMappedResourceCacheKey(spMappableResource* pResource, ezUInt32 uiSubResource)
    : ezHashableStruct<spMappedResourceCacheKey>()
  {
    m_pResource = pResource;
    m_uiSubResource = uiSubResource;
  }

  EZ_ALWAYS_INLINE bool operator==(const spMappedResourceCacheKey& other) const
  {
    return m_pResource == other.m_pResource && m_uiSubResource == other.m_uiSubResource;
  }

  EZ_ALWAYS_INLINE bool operator!=(const spMappedResourceCacheKey& other) const
  {
    return !(*this == other);
  }

  EZ_ALWAYS_INLINE bool operator<(const spMappedResourceCacheKey& other) const
  {
    return m_pResource < other.m_pResource || (m_pResource == other.m_pResource && m_uiSubResource < other.m_uiSubResource);
  }

  spMappableResource* m_pResource{nullptr};
  ezUInt32 m_uiSubResource{0};
};

/// \brief Create resources for use in the graphics device.
/// This class must be implemented for each platform implementation.
class SP_RHI_DLL spDeviceResourceFactory
{
  friend class spDevice;

public:
  virtual ~spDeviceResourceFactory() = default;

  /// \brief Gets the graphics device for which resources are created.
  EZ_NODISCARD EZ_ALWAYS_INLINE spDevice* GetDevice() const { return m_pDevice; }

  /// \brief Creates a new spShader resource.
  /// \param [in] description The description of the shader resource to create.
  /// @return An handle to the created shader resource.
  virtual spShader* CreateShader(const spShaderDescription& description) = 0;

  /// \brief Creates a new spShaderProgram resource.
  /// @return An handle to the created shader program resource.
  virtual spShaderProgram* CreateShaderProgram() = 0;

  /// \brief Creates a new spTexture resource.
  /// \param [in] description The description of the texture resource to create.
  /// @return An handle to the created texture resource.
  virtual spTexture* CreateTexture(const spTextureDescription& description) = 0;

  /// \brief Creates a new spSampler resource.
  /// \param [in] description The description of the sampler resource to create.
  /// @return An handle to the created sampler resource.
  virtual spSampler* CreateSampler(const spSamplerDescription& description) = 0;

  /// \brief Creates a new spInputLayout resource.
  /// \param [in] description The description of the input layout resource to create.
  /// \param [in] hShader The handle to the shader resource used by the input layout.
  /// @return An handle to the created input layout resource.
  virtual spInputLayout* CreateInputLayout(const spInputLayoutDescription& description, const spResourceHandle& hShader) = 0;

  /// \brief Creates a new spBuffer resource.
  /// \param [in] description The description of the buffer resource to create.
  /// @return An handle to the created buffer resource.
  virtual spBuffer* CreateBuffer(const spBufferDescription& description) = 0;

  /// \brief Creates a new spBufferRange resource.
  /// \param [in] description The description of the buffer range resource to create.
  /// @return An handle to the created buffer range resource.
  virtual spBufferRange* CreateBufferRange(const spBufferRangeDescription& description) = 0;

  /// \brief Creates a new spResourceLayout resource.
  /// \param [in] description The description of the resource layout resource to create.
  /// @return An handle to the created resource layout resource.
  virtual spResourceLayout* CreateResourceLayout(const spResourceLayoutDescription& description) = 0;

  /// \brief Creates a new spTextureView resource.
  /// \param [in] description The description of the texture view resource to create.
  /// @return An handle to the created texture view resource.
  virtual spTextureView* CreateTextureView(const spTextureViewDescription& description) = 0;

  /// \brief Creates a new spTextureView resource.
  /// \param [in] hTexture The handle to the texture from which create the view.
  /// @return An handle to the created texture view resource.
  virtual spTextureView* CreateTextureView(const spResourceHandle& hTexture) = 0;

  /// \brief Creates a new spSwapchain resource.
  /// \param [in] description The description of the swapchain resource to create.
  /// @return An handle to the created swapchain resource.
  virtual spSwapchain* CreateSwapchain(const spSwapchainDescription& description) = 0;

  /// \brief Creates a new spFence resource.
  /// \param [in] description The description of the fence resource to create.
  /// @return An handle to the created fence resource.
  virtual spFence* CreateFence(const spFenceDescription& description) = 0;

  /// \brief Creates a new spFramebuffer resource.
  /// \param [in] description The description of the framebuffer resource to create.
  /// @return An handle to the created framebuffer resource.
  virtual spFramebuffer* CreateFramebuffer(const spFramebufferDescription& description) = 0;

  /// \brief Creates a new spCommandList resource.
  /// \param [in] description The description of the command list resource to create.
  /// @return An handle to the created command list resource.
  virtual spCommandList* CreateCommandList(const spCommandListDescription& description) = 0;

  /// \brief Creates a new spComputePipeline resource.
  /// \param [in] description The description of the compute pipeline resource to create.
  /// @return An handle to the created compute pipeline resource.
  virtual spComputePipeline* CreateComputePipeline(const spComputePipelineDescription& description) = 0;

  /// \brief Creates a new spGraphicPipeline resource
  /// \param [in] description The description of the graphic pipeline resource to create.
  /// @return An handle to the created graphic pipeline resource.
  virtual spGraphicPipeline* CreateGraphicPipeline(const spGraphicPipelineDescription& description) = 0;

  /// \brief Creates a new spResourceSet resource.
  /// \param [in] description The description of the resource set resource to create.
  /// @return An handle to the created resource set resource.
  virtual spResourceSet* CreateResourceSet(const spResourceSetDescription& description) = 0;

protected:
  /// \brief Creates a new resource factory for the given device.
  /// \param [in] pDevice The device for which the resource factory will create resources.
  explicit spDeviceResourceFactory(spDevice* pDevice)
    : m_pDevice(pDevice)
  {
  }

private:
  spDevice* m_pDevice;
};

/// \brief Manages resources allocation/deallocation within a graphics devices.
class SP_RHI_DLL spDeviceResourceManager : public ezReflectedClass
{
  friend class spDevice;

public:
  /// \brief Creates a new resource manager for the given device.
  /// \param [in] pDevice The graphics device for which the resource manager will manage resources.
  explicit spDeviceResourceManager(spDevice* pDevice);

  virtual ~spDeviceResourceManager();

  /// \brief Gets the graphics device for which resources are created.
  EZ_NODISCARD EZ_ALWAYS_INLINE spDevice* GetGraphicsDevice() const { return m_pDevice; }

  /// \brief Registers a resource in the manager.
  /// \param [in] pResource The resource to register.
  /// \return The handle to the registered resource.
  ///
  /// \note The method is not meant to be called directly, otherwise, create your
  /// resources using the \see spDeviceResourceFactory and it will do it for you.
  spResourceHandle RegisterResource(spDeviceResource* pResource);

  /// \brief Gets a registered resource from the manager using the given handle.
  /// \param [in] hResource The handle to the resource.
  /// \return The pointer of the found resource, or nullptr if the resource is not registered in this manager.
  EZ_NODISCARD spDeviceResource* GetResource(const spResourceHandle& hResource) const;

  /// \brief Gets a registered resource from the manager using the given handle.
  /// The type parameter must be a child class of spDeviceResource.
  /// \return The pointer of the found resource, or nullptr if the resource is not registered in this manager.
  template <class T>
  EZ_ALWAYS_INLINE std::remove_pointer_t<T>* GetResource(const spResourceHandle& hResource) const
  {
    return ezStaticCast<std::remove_pointer_t<T>*>(GetResource(hResource));
  }

  /// \brief Increments the reference count of the given resource.
  /// \param [in] hResource The handle of the resource to increment the reference count.
  /// \return The new reference count.
  EZ_NODISCARD ezUInt32 IncrementResourceRef(const spResourceHandle& hResource) const;

  /// \brief Decrements the reference count of the given resource.
  /// \param [in] hResource The handle of the resource to decrement the reference count.
  /// \return The new reference count.
  EZ_NODISCARD ezUInt32 DecrementResourceRef(const spResourceHandle& hResource) const;

  /// \brief Enqueue a resource for deallocation at the end of the current frame.
  /// \param [in] hResource The resource to be deallocated.
  virtual void EnqueueReleaseResource(const spResourceHandle& hResource) = 0;

  /// \brief Enqueue a resource for deallocation at the end of the current frame.
  /// \param [in] pResource The resource to be deallocated.
  virtual void EnqueueReleaseResource(spDeviceResource* pResource) = 0;

  /// \brief Deallocate a resource now.
  /// \param [in] hResource The resource to be deallocated.
  virtual void ReleaseResource(const spResourceHandle& hResource) = 0;

  /// \brief Releases all the enqueued resources.
  virtual void ReleaseResources() = 0;

protected:
  using spResourceTable = ezIdTable<spResourceHandle::IdType, spDeviceResource*, ezLocalAllocatorWrapper>;

  spDevice* m_pDevice;
  spResourceTable m_RegisteredResources;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spDeviceResourceManager);

/// \brief Default implementation of the resource manager. Use a queue to collect and dispose resources.
class SP_RHI_DLL spDefaultDeviceResourceManager : public spDeviceResourceManager
{
public:
  explicit spDefaultDeviceResourceManager(spDevice* pDevice);
  ~spDefaultDeviceResourceManager() override;

  /// \brief Enqueue a resource for deallocation at the end of the current frame.
  /// \param [in] hResource The resource to be deallocated.
  void EnqueueReleaseResource(const spResourceHandle& hResource) override;

  /// \brief Enqueue a resource for deallocation at the end of the current frame.
  /// \param [in] pResource The resource to be deallocated.
  void EnqueueReleaseResource(spDeviceResource* pResource) override;

  /// \brief Deallocate a resource now.
  /// \param [in] hResource The resource to be deallocated.
  void ReleaseResource(const spResourceHandle& hResource) override;

  /// \brief Releases all the enqueued resources.
  void ReleaseResources() override;

private:
  void ReleaseResource(spDeviceResource* pResource);

  ezDeque<spDeviceResource*> m_ResourcesQueue;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spDefaultDeviceResourceManager);

class SP_RHI_DLL spResourceHelper
{
public:
  static spBufferRange* GetBufferRange(const spDevice* pDevice, spResourceHandle hResource, ezUInt32 uiOffset);
  static spBufferRange* GetBufferRange(const spDevice* pDevice, spShaderResource* pResource, ezUInt32 uiOffset);
};
