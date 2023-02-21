#pragma once

#include <RPI/RPIDLL.h>

#include <RHI/Buffer.h>
#include <RHI/Device.h>
#include <RHI/RenderTarget.h>
#include <RHI/Sampler.h>
#include <RHI/Texture.h>

#include <RPI/Core.h>
#include <RPI/Pipeline/RenderPipeline.h>

class spRenderGraphNode;
class spRenderGraphBuilder;

struct SP_RPI_DLL spRenderGraphResourceBindType
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    Transient = 0,
    WriteOnly = 1,
    ReadOnly = 2,
    ReadWrite = 3,
    Imported = 4,

    Default = Transient
  };
};

struct SP_RPI_DLL spRenderGraphResourceType
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    Unknown = 0,

    Buffer = 1,
    Texture = 2,
    Sampler = 3,
    RenderTarget = 4,

    Default = Unknown
  };
};

class SP_RPI_DLL spRenderGraphResource : public ezRefCounted
{
public:
  ezSharedPtr<spDeviceResource> m_pResource{nullptr};
  ezEnum<spRenderGraphResourceBindType> m_eBindType{spRenderGraphResourceBindType::Transient};
  ezEnum<spRenderGraphResourceType> m_eType{spRenderGraphResourceType::Unknown};
  spRenderGraphNode* m_pProducer{nullptr};
};

class SP_RPI_DLL spRenderGraphNode : public ezReflectedClass, public ezRefCounted
{
  friend class spRenderGraphBuilder;

  EZ_ADD_DYNAMIC_REFLECTION(spRenderGraphNode, ezReflectedClass);
  EZ_DISALLOW_COPY_AND_ASSIGN(spRenderGraphNode);

public:
  spRenderGraphNode(const ezStringView& sName)
    : m_sName(sName)
  {
  }

  EZ_NODISCARD EZ_ALWAYS_INLINE ezStringView GetName() const { return m_sName; }

  EZ_ALWAYS_INLINE void SetName(ezStringView sName) { m_sName = sName; }

  virtual ezResult Setup(spRenderGraphBuilder* pBuilder, const ezHashTable<ezHashedString, spResourceHandle>& resources) = 0;

  virtual ezUniquePtr<spRenderPass> Compile(spRenderGraphBuilder* pBuilder) = 0;

  virtual bool IsEnabled() const = 0;

protected:
  ezStringView m_sName;
};

class SP_RPI_DLL spRenderGraphBuilder
{
  EZ_DISALLOW_COPY_AND_ASSIGN(spRenderGraphBuilder);

public:
  spRenderGraphBuilder(spDevice* pDevice);

  // --- spDevice shortcuts

  EZ_NODISCARD EZ_ALWAYS_INLINE ezAllocatorBase* GetAllocator() const { return m_pDevice->GetAllocator(); }

  EZ_NODISCARD EZ_ALWAYS_INLINE spDeviceResourceManager* GetResourceManager() const { return m_pDevice->GetResourceManager(); }

  // --- spRenderGraphResource creation

  spResourceHandle Import(ezSharedPtr<spBuffer> pBuffer);
  spResourceHandle Import(ezSharedPtr<spTexture> pTexture);
  spResourceHandle Import(ezSharedPtr<spRenderTarget> pRenderTarget);
  spResourceHandle Import(ezSharedPtr<spSampler> pSampler);

  spResourceHandle Write(spRenderGraphNode* pWriter, spResourceHandle hResource);
  spResourceHandle Read(spRenderGraphNode* pReader, spResourceHandle hResource);

  spResourceHandle CreateTexture(spRenderGraphNode* pProducer, const spTextureDescription& description, const ezEnum<spRenderGraphResourceBindType>& eBindType);
  spResourceHandle CreateRenderTarget(spRenderGraphNode* pProducer, const spRenderTargetDescription& description, const ezEnum<spRenderGraphResourceBindType>& eBindType);
  spResourceHandle CreateBuffer(spRenderGraphNode* pProducer, const spBufferDescription& description, const ezEnum<spRenderGraphResourceBindType>& eBindType);
  spResourceHandle CreateSampler(spRenderGraphNode* pProducer, const spSamplerDescription& description, const ezEnum<spRenderGraphResourceBindType>& eBindType);

  // --- spRenderGraphNode management

  void AddNode(ezStringView sName, ezUniquePtr<spRenderGraphNode>&& node, const ezHashTable<ezHashedString, spResourceHandle>& resources);

  const spRenderGraphNode* GetNode(ezStringView sName) const;

  // --- spRenderGraphBuilder

  ezUniquePtr<spRenderPipeline> Compile();

private:
  spResourceHandle Import(ezSharedPtr<spDeviceResource> pResource, const ezEnum<spRenderGraphResourceType>& eType);

  spDevice* m_pDevice{nullptr};

  ezArrayMap<spResourceHandle, spTextureDescription> m_PendingTextures;
  ezArrayMap<spResourceHandle, spRenderTargetDescription> m_PendingRenderTargets;
  ezArrayMap<spResourceHandle, spBufferDescription> m_PendingBuffers;
  ezArrayMap<spResourceHandle, spSamplerDescription> m_PendingSamplers;

  ezArrayMap<spRenderGraphNode*, spResourceHandle> m_ReadResources;

  spRenderGraphResourcesTable m_GraphResources;
  ezHashTable<ezHashedString, ezUniquePtr<spRenderGraphNode>> m_Nodes;
  ezDynamicArray<ezHashedString> m_OrderedNodes;
};
