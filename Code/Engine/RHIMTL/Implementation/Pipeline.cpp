#include <RHIMTL/RHIMTLPCH.h>

#include <RHI/Input.h>

#include <RHIMTL/Core.h>
#include <RHIMTL/Device.h>
#include <RHIMTL/Pipeline.h>
#include <RHIMTL/ResourceLayout.h>
#include <RHIMTL/Shader.h>

namespace RHI
{
  static void SetupShaderProgramStage(MTL::RenderPipelineDescriptor* pDescriptor, ezSharedPtr<spShaderProgramMTL> pShaderProgram, const ezEnum<spShaderStage> eStage, ezList<MTL::Function*>& functions)
  {
    const auto pShader = pShaderProgram->Get(eStage).Downcast<spShaderMTL>();
    if (pShader == nullptr)
      return;

    pShader->EnsureResourceCreated();
    MTL::Function* pFunction = pShader->GetMTLShaderFunction();

    if (pShader->HasFunctionConstants())
    {
      NS::Error* pError = nullptr;

      {
        spScopedMTLResource sEntryPoint(NS::String::string(pShader->GetEntryPoint().GetData(), NS::UTF8StringEncoding));
        pFunction = pShader->GetMTLShaderLibrary()->newFunction(*sEntryPoint, nullptr, &pError);
      }

      if (pError != nullptr)
      {
        ezStringBuilder sError;
        sError.SetFormat("Failed to create specialized Metal function: {0}\n{1}", pError->localizedFailureReason()->utf8String(), pError->localizedDescription()->utf8String());

        EZ_LOG_BLOCK("Compute Pipeline Error Message");
        ezLog::Dev("{0}", sError.GetData());

        SP_RHI_MTL_RELEASE(pError);
        return;
      }

      functions.PushBack(pFunction);
    }

    if (eStage == spShaderStage::VertexShader)
      pDescriptor->setVertexFunction(pFunction);
    else if (eStage == spShaderStage::PixelShader)
      pDescriptor->setFragmentFunction(pFunction);
  }

#pragma region spComputePipelineMTL

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spComputePipelineMTL, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spComputePipelineMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    SP_RHI_MTL_RELEASE(m_pPipelineState);
    for (auto it = m_SpecializedFunctions.GetIterator(); it.IsValid(); ++it)
      SP_RHI_MTL_RELEASE(*it);

    m_SpecializedFunctions.Clear();

    m_bReleased = true;
  }

  bool spComputePipelineMTL::IsReleased() const
  {
    return m_bReleased;
  }

  spComputePipelineMTL::spComputePipelineMTL(spDeviceMTL* pDevice, const spComputePipelineDescription& description)
    : spComputePipeline(description)
  {
    m_pDevice = pDevice;
    m_mMTLDevice = pDevice->GetMTLDevice();

    m_ResourceLayouts.EnsureCount(description.m_ResourceLayouts.GetCount());
    for (ezUInt32 i = 0, l = description.m_ResourceLayouts.GetCount(); i < l; ++i)
    {
      auto layout = m_pDevice->GetResourceManager()->GetResource<spResourceLayoutMTL>(description.m_ResourceLayouts[i]);
      EZ_ASSERT_DEV(layout != nullptr, "Invalid resource layout handle {0} in compute pipeline", description.m_hComputeShader.GetInternalID().m_Data);

      m_ResourceLayouts[i] = layout;
    }

    m_DispatchSize = MTL::Size(description.m_uiThreadGroupSizeX, description.m_uiThreadGroupSizeY, description.m_uiThreadGroupSizeZ);

    {
      NS::Error* pError = nullptr;
      spScopedMTLResource desc(MTL::ComputePipelineDescriptor::alloc()->init());

      {
        const auto pShader = m_pDevice->GetResourceManager()->GetResource<spShaderMTL>(description.m_hComputeShader);
        EZ_ASSERT_DEV(pShader != nullptr, "Invalid compute shader resource {0} in compute pipeline", description.m_hComputeShader.GetInternalID().m_Data);

        EZ_IGNORE_UNUSED(pShader->AddRef());
        MTL::Function* pFunction = pShader->GetMTLShaderFunction();

        if (pShader->HasFunctionConstants())
        {
          {
            spScopedMTLResource sEntryPoint(NS::String::string(pShader->GetEntryPoint().GetData(), NS::UTF8StringEncoding));
            pFunction = pShader->GetMTLShaderLibrary()->newFunction(*sEntryPoint, nullptr, &pError);
          }

          if (pError != nullptr)
          {
            ezStringBuilder sError;
            sError.SetFormat("Failed to create specialized Metal function: {0}\n{1}", pError->localizedFailureReason()->utf8String(), pError->localizedDescription()->utf8String());

            EZ_LOG_BLOCK("Compute Pipeline Error Message");
            ezLog::Dev("{0}", sError.GetData());

            SP_RHI_MTL_RELEASE(pError);
            return;
          }

          m_SpecializedFunctions.PushBack(pFunction);
        }

        desc->setComputeFunction(pFunction);
      }

      auto buffers = desc->buffers();
      ezUInt32 uiBufferIndex = 0;

      for (const auto& pLayout : m_ResourceLayouts)
      {
        for (const auto& element : pLayout->GetElements())
        {
          auto eType = element.m_eType;

          if (eType == spShaderResourceType::ConstantBuffer || eType == spShaderResourceType::ReadOnlyStructuredBuffer)
          {
            MTL::PipelineBufferDescriptor* pBufferDescriptor = buffers->object(uiBufferIndex);
            pBufferDescriptor->setMutability(MTL::MutabilityImmutable);
            uiBufferIndex += 1;
          }
          else if (eType == spShaderResourceType::ReadWriteStructuredBuffer)
          {
            MTL::PipelineBufferDescriptor* pBufferDescriptor = buffers->object(uiBufferIndex);
            pBufferDescriptor->setMutability(MTL::MutabilityMutable);
            uiBufferIndex += 1;
          }
        }
      }

      m_pPipelineState = m_mMTLDevice->newComputePipelineState(*desc, MTL::PipelineOptionNone, nullptr, &pError);

      if (pError != nullptr)
      {
        ezStringBuilder sError;
        sError.SetFormat("Failed to create Metal compute pipeline state: {0}\n{1}", pError->localizedFailureReason()->utf8String(), pError->localizedDescription()->utf8String());

        EZ_LOG_BLOCK("Compute Pipeline Error Message");
        ezLog::Dev("{0}", sError.GetData());

        SP_RHI_MTL_RELEASE(pError);
        return;
      }
    }

    m_bReleased = false;
  }

  spComputePipelineMTL::~spComputePipelineMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

#pragma endregion

#pragma region spGraphicPipelineMTL

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spGraphicPipelineMTL, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spGraphicPipelineMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    SP_RHI_MTL_RELEASE(m_pPipelineState);
    for (auto it = m_SpecializedFunctions.GetIterator(); it.IsValid(); ++it)
      SP_RHI_MTL_RELEASE(*it);

    m_SpecializedFunctions.Clear();

    m_bReleased = true;
  }

  bool spGraphicPipelineMTL::IsReleased() const
  {
    return m_bReleased;
  }

  spGraphicPipelineMTL::spGraphicPipelineMTL(spDeviceMTL* pDevice, const spGraphicPipelineDescription& description)
    : spGraphicPipeline(description)
  {
    m_pDevice = pDevice;
    m_pMTLDevice = pDevice->GetMTLDevice();

    m_ResourceLayouts.EnsureCount(description.m_ResourceLayouts.GetCount());
    for (ezUInt32 i = 0, l = description.m_ResourceLayouts.GetCount(); i < l; ++i)
    {
      auto layout = m_pDevice->GetResourceManager()->GetResource<spResourceLayoutMTL>(description.m_ResourceLayouts[i]);
      EZ_ASSERT_DEV(layout != nullptr, "Invalid resource layout handle {0} in graphic pipeline", description.m_ResourceLayouts[i].GetInternalID().m_Data);

      m_ResourceLayouts[i] = layout;
      m_uiNonVertexBufferCount += layout->GetBufferCount();
    }

    const ezUInt32 uiVertexBuffersCount = description.m_ShaderPipeline.m_InputLayouts.GetCount();

    ezDynamicArray<spInputLayoutDescription> inputLayouts;
    inputLayouts.EnsureCount(uiVertexBuffersCount);

    m_InputLayouts.EnsureCount(uiVertexBuffersCount);
    for (ezUInt32 i = 0, l = uiVertexBuffersCount; i < l; ++i)
    {
      auto layout = m_pDevice->GetResourceManager()->GetResource<spInputLayout>(description.m_ShaderPipeline.m_InputLayouts[i]);
      EZ_ASSERT_DEV(layout != nullptr, "Invalid input layout handle {0}", description.m_ShaderPipeline.m_InputLayouts[i].GetInternalID().m_Data);

      m_InputLayouts[i] = layout;

      inputLayouts[i] = layout->GetDescription();
    }

    m_PrimitiveType = spToMTL(description.m_ePrimitiveTopology);
    m_CullMode = spToMTL(description.m_RenderingState.m_RasterizerState.m_eFaceCulling);
    m_Winding = spToMTL(description.m_RenderingState.m_RasterizerState.m_eFrontFace);
    m_FillMode = spToMTL(description.m_RenderingState.m_RasterizerState.m_ePolygonFillMode);
    m_fDepthBias = description.m_RenderingState.m_RasterizerState.m_fDepthBias;
    m_fDepthBiasClamp = description.m_RenderingState.m_RasterizerState.m_fDepthBiasClamp;
    m_fSlopeScaledDepthBias = description.m_RenderingState.m_RasterizerState.m_fSlopeScaledDepthBias;

    m_bScissorTestEnabled = description.m_RenderingState.m_RasterizerState.m_bScissorTestEnabled;

    spOutputDescription outputDescription = description.m_Output;

    {
      spScopedMTLResource desc(MTL::RenderPipelineDescriptor::alloc()->init());

      const auto pShaderProgram = m_pDevice->GetResourceManager()->GetResource<spShaderProgramMTL>(m_hShaderProgram);
      EZ_ASSERT_DEV(pShaderProgram != nullptr, "Invalid shader program resource {0}", m_hShaderProgram.GetInternalID().m_Data);

      EZ_IGNORE_UNUSED(pShaderProgram->AddRef());

      SetupShaderProgramStage(*desc, pShaderProgram, spShaderStage::VertexShader, m_SpecializedFunctions);
      SetupShaderProgramStage(*desc, pShaderProgram, spShaderStage::GeometryShader, m_SpecializedFunctions);
      SetupShaderProgramStage(*desc, pShaderProgram, spShaderStage::HullShader, m_SpecializedFunctions);
      SetupShaderProgramStage(*desc, pShaderProgram, spShaderStage::DomainShader, m_SpecializedFunctions);
      SetupShaderProgramStage(*desc, pShaderProgram, spShaderStage::PixelShader, m_SpecializedFunctions);

      const auto inputDesc = desc->vertexDescriptor();

      for (ezUInt32 i = 0, l = description.m_ShaderPipeline.m_InputLayouts.GetCount(); i < l; ++i)
      {
        const ezUInt32 uiLayoutIndex = m_uiNonVertexBufferCount + i;
        MTL::VertexBufferLayoutDescriptor* pLayoutDescriptor = inputDesc->layouts()->object(uiLayoutIndex);
        pLayoutDescriptor->setStride(inputLayouts[i].m_uiStride);
        const ezUInt32 uiStepRate = inputLayouts[i].m_uiInstanceStepRate;
        pLayoutDescriptor->setStepFunction(uiStepRate == 0 ? MTL::VertexStepFunctionPerVertex : MTL::VertexStepFunctionPerInstance);
        pLayoutDescriptor->setStepRate(ezMath::Max<ezUInt32>(1, uiStepRate));
      }

      ezUInt32 uiElement = 0;
      for (ezUInt32 i = 0, l = inputLayouts.GetCount(); i < l; i++)
      {
        ezUInt32 offset = 0;
        const auto& inputLayoutDescription = inputLayouts[i];

        for (ezUInt32 j = 0, m = inputLayoutDescription.m_Elements.GetCount(); j < m; j++)
        {
          const auto& elementDesc = inputLayoutDescription.m_Elements[j];
          MTL::VertexAttributeDescriptor* pAttributeDescriptor = inputDesc->attributes()->object(uiElement);
          pAttributeDescriptor->setBufferIndex(m_uiNonVertexBufferCount + i);
          pAttributeDescriptor->setFormat(spToMTL(elementDesc.m_eFormat));
          pAttributeDescriptor->setOffset(elementDesc.m_uiOffset != 0 ? elementDesc.m_uiOffset : offset);
          offset += spPixelFormatHelper::GetSizeInBytes(elementDesc.m_eFormat);
          uiElement += 1;
        }
      }

      m_uiVertexBufferCount = inputLayouts.GetCount();

      const auto& blendStateDesc = description.m_RenderingState.m_BlendState;
      m_BlendColor = blendStateDesc.m_BlendColor;

      if (outputDescription.m_eSampleCount != spTextureSampleCount::None)
      {
        desc->setSampleCount(outputDescription.m_eSampleCount);
      }

      if (outputDescription.m_DepthAttachment.m_eFormat != spPixelFormat::Unknown)
      {
        const MTL::PixelFormat eFormat = spToMTL(outputDescription.m_DepthAttachment.m_eFormat, true);
        desc->setDepthAttachmentPixelFormat(eFormat);

        if (spPixelFormatHelper::IsStencilFormat(outputDescription.m_DepthAttachment.m_eFormat))
        {
          m_bHasStencil = true;
          desc->setStencilAttachmentPixelFormat(eFormat);
        }
      }

      for (uint i = 0, l = outputDescription.m_ColorAttachments.GetCount(); i < l; i++)
      {
        const auto& attachmentBlendDesc = blendStateDesc.m_AttachmentStates[i];
        MTL::RenderPipelineColorAttachmentDescriptor* colorDesc = desc->colorAttachments()->object(i);
        colorDesc->setPixelFormat(spToMTL(outputDescription.m_ColorAttachments[i].m_eFormat, false));
        colorDesc->setBlendingEnabled(attachmentBlendDesc.m_bEnabled);
        colorDesc->setWriteMask(spToMTL(attachmentBlendDesc.m_eColorWriteMask));
        colorDesc->setAlphaBlendOperation(spToMTL(attachmentBlendDesc.m_eAlphaBlendFunction));
        colorDesc->setSourceAlphaBlendFactor(spToMTL(attachmentBlendDesc.m_eSourceAlphaBlendFactor));
        colorDesc->setDestinationAlphaBlendFactor(spToMTL(attachmentBlendDesc.m_eDestinationAlphaBlendFactor));

        colorDesc->setRgbBlendOperation(spToMTL(attachmentBlendDesc.m_eColorBlendFunction));
        colorDesc->setSourceRGBBlendFactor(spToMTL(attachmentBlendDesc.m_eSourceColorBlendFactor));
        colorDesc->setDestinationRGBBlendFactor(spToMTL(attachmentBlendDesc.m_eDestinationColorBlendFactor));
      }

      desc->setAlphaToCoverageEnabled(blendStateDesc.m_bAlphaToCoverage);

      NS::Error* pError = nullptr;
      m_pPipelineState = m_pMTLDevice->newRenderPipelineState(*desc, MTL::PipelineOptionNone, nullptr, &pError);

      if (pError != nullptr)
      {
        ezStringBuilder sError;
        sError.SetFormat("Failed to create Metal render pipeline state: {0}\n{1}", pError->localizedFailureReason()->utf8String(), pError->localizedDescription()->utf8String());

        EZ_LOG_BLOCK("Graphic Pipeline Error Message");
        ezLog::Dev("{0}", sError.GetData());

        SP_RHI_MTL_RELEASE(pError);
        return;
      }
    }

    if (outputDescription.m_DepthAttachment.m_eFormat != spPixelFormat::Unknown)
    {
      spScopedMTLResource depthDescriptor(MTL::DepthStencilDescriptor::alloc()->init());
      depthDescriptor->setDepthCompareFunction(spToMTL(description.m_RenderingState.m_DepthState.m_eDepthStencilComparison));
      depthDescriptor->setDepthWriteEnabled(description.m_RenderingState.m_DepthState.m_bDepthMaskEnabled);

      if (description.m_RenderingState.m_StencilState.m_bEnabled)
      {
        m_uiStencilReference = description.m_RenderingState.m_StencilState.m_uiReference;

        {
          const auto& frontStencilDesc = description.m_RenderingState.m_StencilState.m_Front;
          spScopedMTLResource front(MTL::StencilDescriptor::alloc()->init());
          front->setReadMask(description.m_RenderingState.m_StencilState.m_uiReadMask);
          front->setWriteMask(description.m_RenderingState.m_StencilState.m_uiWriteMask);
          front->setDepthFailureOperation(spToMTL(frontStencilDesc.m_eDepthFail));
          front->setStencilFailureOperation(spToMTL(frontStencilDesc.m_eFail));
          front->setDepthStencilPassOperation(spToMTL(frontStencilDesc.m_ePass));
          front->setStencilCompareFunction(spToMTL(frontStencilDesc.m_eComparison));
          depthDescriptor->setFrontFaceStencil(*front);
        }

        {
          const auto& backStencilDesc = description.m_RenderingState.m_StencilState.m_Back;
          spScopedMTLResource back(MTL::StencilDescriptor::alloc()->init());
          back->setReadMask(description.m_RenderingState.m_StencilState.m_uiReadMask);
          back->setWriteMask(description.m_RenderingState.m_StencilState.m_uiWriteMask);
          back->setDepthFailureOperation(spToMTL(backStencilDesc.m_eDepthFail));
          back->setStencilFailureOperation(spToMTL(backStencilDesc.m_eFail));
          back->setDepthStencilPassOperation(spToMTL(backStencilDesc.m_ePass));
          back->setStencilCompareFunction(spToMTL(backStencilDesc.m_eComparison));
          depthDescriptor->setBackFaceStencil(*back);
        }
      }

      m_pDepthStencilState = m_pMTLDevice->newDepthStencilState(*depthDescriptor);
    }

    m_DepthClipMode = description.m_RenderingState.m_DepthState.m_bDepthTestEnabled ? MTL::DepthClipModeClip : MTL::DepthClipModeClamp;

    m_bReleased = false;
  }

  spGraphicPipelineMTL::~spGraphicPipelineMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

#pragma endregion
} // namespace RHI

EZ_STATICLINK_FILE(RHIMTL, RHIMTL_Implementation_Pipeline);
