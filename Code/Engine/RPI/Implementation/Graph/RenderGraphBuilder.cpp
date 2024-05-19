#include <RPI/RPIPCH.h>

#include <RPI/Graph/RenderGraph.h>
#include <RPI/Pipeline/RenderPass.h>

using namespace RHI;

namespace RPI
{
  spRenderGraphBuilder::spRenderGraphBuilder(spDevice* pDevice)
    : m_pDevice(pDevice)
  {
  }

  spResourceHandle spRenderGraphBuilder::Import(const ezSharedPtr<spBuffer>& pBuffer)
  {
    return Import(pBuffer, spRenderGraphResourceType::Buffer);
  }

  spResourceHandle spRenderGraphBuilder::Import(const ezSharedPtr<spTexture>& pTexture)
  {
    return Import(pTexture, spRenderGraphResourceType::Texture);
  }

  spResourceHandle spRenderGraphBuilder::Import(const ezSharedPtr<spRenderTarget>& pRenderTarget)
  {
    return Import(pRenderTarget, spRenderGraphResourceType::RenderTarget);
  }

  spResourceHandle spRenderGraphBuilder::Import(const ezSharedPtr<spSampler>& pSampler)
  {
    return Import(pSampler, spRenderGraphResourceType::Sampler);
  }

  spResourceHandle spRenderGraphBuilder::Write(spRenderGraphNode* pWriter, spResourceHandle hResource)
  {
    spRenderGraphResource* resource;
    if (!m_GraphResources.TryGetValue(hResource.GetInternalID(), resource))
      ezLog::Error("Resource with ID '{}' does not exist.", hResource.GetInternalID().m_Data);

    if (resource->m_eBindType != spRenderGraphResourceBindType::WriteOnly && resource->m_eBindType != spRenderGraphResourceBindType::ReadWrite && resource->m_eBindType != spRenderGraphResourceBindType::Imported)
    {
      ezLog::Error("Resource with ID '{}' is not writable.", hResource.GetInternalID().m_Data);
      return {};
    }

    EZ_IGNORE_UNUSED(resource->m_pProducer->AddRef());
    return spResourceHandle(m_GraphResources.Insert(*resource));
  }

  spResourceHandle spRenderGraphBuilder::Read(spRenderGraphNode* pReader, spResourceHandle hResource)
  {
    spRenderGraphResource* resource;
    if (!m_GraphResources.TryGetValue(hResource.GetInternalID(), resource))
      ezLog::Error("Resource with ID '{}' does not exist.", hResource.GetInternalID().m_Data);

    if (resource->m_eBindType != spRenderGraphResourceBindType::ReadOnly && resource->m_eBindType != spRenderGraphResourceBindType::ReadWrite && resource->m_eBindType != spRenderGraphResourceBindType::Imported)
    {
      ezLog::Error("Resource with ID '{}' is not readable.", hResource.GetInternalID().m_Data);
      return {};
    }

    EZ_IGNORE_UNUSED(resource->AddRef());
    m_ReadResources.Insert(pReader, hResource);

    return hResource;
  }

  spResourceHandle spRenderGraphBuilder::CreateTexture(spRenderGraphNode* pProducer, const spTextureDescription& description, const ezEnum<spRenderGraphResourceBindType>& eBindType)
  {
    EZ_ASSERT_DEV(pProducer != nullptr, "Cannot create a resource without specifying a producer node.");

    spRenderGraphResource resource;
    resource.m_pProducer = pProducer;
    resource.m_eBindType = eBindType;
    resource.m_eType = spRenderGraphResourceType::Texture;

    const auto hResource = spResourceHandle(m_GraphResources.Insert(std::move(resource)));
    m_PendingTextures.Insert(hResource, description);

    if (eBindType == spRenderGraphResourceBindType::Transient)
      EZ_IGNORE_UNUSED(m_GraphResources[hResource.GetInternalID()].AddRef()); // Transient resources lives inside the producer, so they should have at least (only?) one reference

    return hResource;
  }

  spResourceHandle spRenderGraphBuilder::CreateRenderTarget(spRenderGraphNode* pProducer, const spRenderTargetDescription& description, const ezEnum<spRenderGraphResourceBindType>& eBindType)
  {
    EZ_ASSERT_DEV(pProducer != nullptr, "Cannot create a resource without specifying a producer node.");

    spRenderGraphResource resource;
    resource.m_pProducer = pProducer;
    resource.m_eBindType = eBindType;
    resource.m_eType = spRenderGraphResourceType::RenderTarget;

    const auto hResource = spResourceHandle(m_GraphResources.Insert(std::move(resource)));
    m_PendingRenderTargets.Insert(hResource, description);

    if (eBindType == spRenderGraphResourceBindType::Transient)
      EZ_IGNORE_UNUSED(m_GraphResources[hResource.GetInternalID()].AddRef()); // Transient resources lives inside the producer, so they should have at least (only?) one reference

    return hResource;
  }

  spResourceHandle spRenderGraphBuilder::CreateBuffer(spRenderGraphNode* pProducer, const spBufferDescription& description, const ezEnum<spRenderGraphResourceBindType>& eBindType)
  {
    EZ_ASSERT_DEV(pProducer != nullptr, "Cannot create a resource without specifying a producer node.");

    spRenderGraphResource resource;
    resource.m_pProducer = pProducer;
    resource.m_eBindType = eBindType;
    resource.m_eType = spRenderGraphResourceType::Buffer;

    const auto hResource = spResourceHandle(m_GraphResources.Insert(std::move(resource)));
    m_PendingBuffers.Insert(hResource, description);

    if (eBindType == spRenderGraphResourceBindType::Transient)
      EZ_IGNORE_UNUSED(m_GraphResources[hResource.GetInternalID()].AddRef()); // Transient resources lives inside the producer, so they should have at least (only?) one reference

    return hResource;
  }

  spResourceHandle spRenderGraphBuilder::CreateSampler(spRenderGraphNode* pProducer, const spSamplerDescription& description, const ezEnum<spRenderGraphResourceBindType>& eBindType)
  {
    EZ_ASSERT_DEV(pProducer != nullptr, "Cannot create a resource without specifying a producer node.");

    spRenderGraphResource resource;
    resource.m_pProducer = pProducer;
    resource.m_eBindType = eBindType;
    resource.m_eType = spRenderGraphResourceType::Sampler;

    const auto hResource = spResourceHandle(m_GraphResources.Insert(std::move(resource)));
    m_PendingSamplers.Insert(hResource, description);

    if (eBindType == spRenderGraphResourceBindType::Transient)
      EZ_IGNORE_UNUSED(m_GraphResources[hResource.GetInternalID()].AddRef()); // Transient resources lives inside the producer, so they should have at least (only?) one reference

    return hResource;
  }

  void spRenderGraphBuilder::AddNode(ezStringView sName, ezUniquePtr<spRenderGraphNode>&& node, const spRenderGraphResourceMap& resources)
  {
    ezHashedString sNameHash;
    sNameHash.Assign(sName);

    if (node->Setup(this, resources).Succeeded())
    {
      node->SetName(sName);
      m_Nodes.Insert(sNameHash, std::move(node));
      m_OrderedNodes.PushBack(sNameHash);
      return;
    }

    ezLog::Error("Failed to setup node '{0}'", sName);
  }

  const spRenderGraphNode* spRenderGraphBuilder::GetNode(ezStringView sName) const
  {
    ezHashedString sNameHash;
    sNameHash.Assign(sName);

    if (ezUniquePtr<spRenderGraphNode>* pNode = nullptr; m_Nodes.TryGetValue(sNameHash, pNode))
      return pNode->Borrow();

    return nullptr;
  }

  ezUniquePtr<spRenderPipeline> spRenderGraphBuilder::Compile()
  {
    // --- Cull unused resources and nodes

    ezDeque<spRenderGraphResource> culledResources;

    const auto releaseResources = [&](const spRenderGraphNode* pNode) -> void
    {
      // Release resources read by the given node
      for (auto& readResource : m_ReadResources)
      {
        auto& res = m_GraphResources[readResource.value.GetInternalID()];

        if (readResource.key == pNode)
          EZ_IGNORE_UNUSED(res.ReleaseRef());

        if (res.m_eBindType != spRenderGraphResourceBindType::Imported && res.GetRefCount() <= 0)
        {
          culledResources.PushBack(res);
          m_GraphResources.Remove(readResource.value.GetInternalID());
        }
      }

      while (m_ReadResources.RemoveAndCopy(pNode))
      {
      }

      // Release resources produced by the given node
      auto it = m_GraphResources.GetIterator();

      while (it.IsValid())
      {
        auto& res = it.Value();

        if (res.m_pProducer == pNode)
          EZ_IGNORE_UNUSED(res.ReleaseRef());

        if (res.m_eBindType != spRenderGraphResourceBindType::Imported && res.GetRefCount() <= 0)
        {
          culledResources.PushBack(res);
          m_GraphResources.Remove(it.Id());
          continue;
        }

        it.Next();
      }
    };

    // Cull disabled nodes and all their resources
    for (auto nodeIt = m_Nodes.GetIterator(); nodeIt.IsValid(); nodeIt.Next())
    {
      ezUniquePtr<spRenderGraphNode>& pNode = nodeIt.Value();

      if (pNode->IsEnabled())
        EZ_IGNORE_UNUSED(pNode->AddRef());

      if (pNode->GetRefCount() == 0)
        releaseResources(pNode.Borrow());
    }

    while (!culledResources.IsEmpty())
    {
      const spRenderGraphResource& res = culledResources.PeekBack();

      if (res.m_pProducer != nullptr && res.m_pProducer->GetRefCount() > 0)
      {
        EZ_IGNORE_UNUSED(res.m_pProducer->ReleaseRef());

        if (res.m_pProducer->GetRefCount() == 0)
          releaseResources(res.m_pProducer);
      }

      culledResources.PopBack();
    }

    // --- Create pending resources - Some of them may be deferred

    for (const auto& pending : m_PendingTextures)
    {
      if (spRenderGraphResource* resource = nullptr; m_GraphResources.TryGetValue(pending.key.GetInternalID(), resource))
        resource->m_pRHIResource = m_pDevice->GetResourceFactory()->CreateTexture(pending.value);
    }

    for (const auto& pending : m_PendingRenderTargets)
    {
      if (spRenderGraphResource* resource = nullptr; m_GraphResources.TryGetValue(pending.key.GetInternalID(), resource))
        resource->m_pRHIResource = m_pDevice->GetResourceFactory()->CreateRenderTarget(pending.value);
    }

    for (const auto& pending : m_PendingBuffers)
    {
      if (spRenderGraphResource* resource = nullptr; m_GraphResources.TryGetValue(pending.key.GetInternalID(), resource))
        resource->m_pRHIResource = m_pDevice->GetResourceFactory()->CreateBuffer(pending.value);
    }

    for (const auto& pending : m_PendingSamplers)
    {
      if (spRenderGraphResource* resource = nullptr; m_GraphResources.TryGetValue(pending.key.GetInternalID(), resource))
        resource->m_pRHIResource = m_pDevice->GetResourceFactory()->CreateSampler(pending.value);
    }

    // --- Compile nodes and setup pipeline passes

    ezUniquePtr<spRenderPipeline> renderPipeline = EZ_NEW(m_pDevice->GetAllocator(), spRenderPipeline, std::move(m_GraphResources));

    for (auto& node : m_OrderedNodes)
    {
      const ezUniquePtr<spRenderGraphNode>* pNode = m_Nodes.GetValue(node);
      if ((*pNode)->GetRefCount() == 0)
        continue;

      ezUniquePtr<spRenderPass> pass = (*pNode)->Compile(this);
      renderPipeline->AddPass(node, std::move(pass));
    }

    return renderPipeline;
  }

  spResourceHandle spRenderGraphBuilder::Import(const ezSharedPtr<spDeviceResource>& pResource, const ezEnum<spRenderGraphResourceType>& eType)
  {
    EZ_ASSERT_DEV(pResource != nullptr, "Tried to import an invalid resource.");

    spRenderGraphResource resource;
    resource.m_eBindType = spRenderGraphResourceBindType::Imported;
    resource.m_eType = eType;
    resource.m_pRHIResource = pResource;

    return spResourceHandle(m_GraphResources.Insert(std::move(resource)));
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Pipeline_Graph_RenderGraphBuilder);
