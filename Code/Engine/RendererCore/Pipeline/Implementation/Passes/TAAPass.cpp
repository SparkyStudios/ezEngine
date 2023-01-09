#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Passes/TAAPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>

#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/Resources/Texture.h>

#include "../../../../../../Data/Base/Shaders/Pipeline/CopyConstants.h"
#include "../../../../../../Data/Base/Shaders/Pipeline/TAAConstants.h"

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezTAAPass, 1, ezRTTIDefaultAllocator<ezTAAPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Input", m_PinInputColor),
    EZ_MEMBER_PROPERTY("Velocity", m_PinInputVelocity),
    EZ_MEMBER_PROPERTY("DepthStencil", m_PinInputDepth),
    EZ_MEMBER_PROPERTY("Output", m_PinOutput),
    EZ_MEMBER_PROPERTY("UpscaleEnabled", m_bUpsample)->AddAttributes(new ezDefaultValueAttribute(false)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on


ezTAAPass::ezTAAPass()
  : ezRenderPipelinePass("TAAPass", true)
  , m_bUpsample(false)
{
  // Load shader
  {
    m_hTAAShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/TAA.ezShader");
    EZ_ASSERT_DEV(m_hTAAShader.IsValid(), "Could not load TAA Pass shader!");

    m_hCopyShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/Copy_CS.ezShader");
    EZ_ASSERT_DEV(m_hCopyShader.IsValid(), "Could not load the Copy texture shader required for TAA!");
  }

  // Load resources
  {
    m_hTAAConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezTAAConstants>();
    m_hCopyConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezCopyConstants>();
  }
}

ezTAAPass::~ezTAAPass()
{
  ezRenderContext::DeleteConstantBufferStorage(m_hCopyConstantBuffer);
  m_hCopyConstantBuffer.Invalidate();

  ezRenderContext::DeleteConstantBufferStorage(m_hTAAConstantBuffer);
  m_hTAAConstantBuffer.Invalidate();

  if (!m_hHistory.IsInvalidated())
  {
    ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

    pDevice->DestroyTexture(m_hHistory);
    m_hHistory.Invalidate();
  }

  if (!m_hPreviousVelocity.IsInvalidated())
  {
    ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

    pDevice->DestroyTexture(m_hPreviousVelocity);
    m_hPreviousVelocity.Invalidate();
  }
}

bool ezTAAPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  // Color
  if (inputs[m_PinInputColor.m_uiInputIndex])
  {
    if (!inputs[m_PinInputColor.m_uiInputIndex]->m_bAllowShaderResourceView)
    {
      ezLog::Error("'{0}' Color input must allow shader resource view.", GetName());
      return false;
    }

    ezGALTextureCreationDescription desc = *inputs[m_PinInputColor.m_uiInputIndex];
    desc.m_bAllowUAV = true;
    desc.m_bCreateRenderTarget = true;

    if (m_bUpsample)
    {
      desc.m_uiWidth = ezMath::Max(static_cast<ezUInt32>(view.GetTargetViewport().width), desc.m_uiWidth);
      desc.m_uiHeight = ezMath::Max(static_cast<ezUInt32>(view.GetTargetViewport().height), desc.m_uiHeight);
    }

    outputs[m_PinOutput.m_uiOutputIndex] = std::move(desc);
  }
  else
  {
    ezLog::Error("No Color input connected to '{0}'!", GetName());
    return false;
  }

  // Velocity
  if (inputs[m_PinInputVelocity.m_uiInputIndex])
  {
    if (!inputs[m_PinInputVelocity.m_uiInputIndex]->m_bAllowShaderResourceView)
    {
      ezLog::Error("'{0}' Velocity input must allow shader resource view.", GetName());
      return false;
    }
  }
  else
  {
    ezLog::Error("No Velocity input connected to '{0}'!", GetName());
    return false;
  }

  // Depth - Stencil
  if (inputs[m_PinInputDepth.m_uiInputIndex])
  {
    if (!inputs[m_PinInputDepth.m_uiInputIndex]->m_bAllowShaderResourceView)
    {
      ezLog::Error("'{0}' Depth input must allow shader resource view.", GetName());
      return false;
    }
  }
  else
  {
    ezLog::Error("No depth/stencil input connected to pass '{0}'.", GetName());
    return false;
  }

  return true;
}

void ezTAAPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  const auto* const pInputColor = inputs[m_PinInputColor.m_uiInputIndex];
  const auto* const pInputVelocity = inputs[m_PinInputVelocity.m_uiInputIndex];
  const auto* const pInputDepth = inputs[m_PinInputDepth.m_uiInputIndex];
  const auto* const pOutput = outputs[m_PinOutput.m_uiOutputIndex];

  if (pInputColor == nullptr || pInputVelocity == nullptr || pInputDepth == nullptr || pOutput == nullptr)
  {
    return;
  }

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();
  ezResourceManager::ForceLoadResourceNow(m_hTAAShader);

  const bool bAllowAsyncShaderLoading = renderViewContext.m_pRenderContext->GetAllowAsyncShaderLoading();
  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(false);

  ezGALPass* pPass = pDevice->BeginPass(GetName());
  {

    ezGALTextureHandle hPreviousVelocity;
    {
      ezGALTextureCreationDescription desc = pInputVelocity->m_Desc;
      desc.m_bAllowShaderResourceView = true;
      desc.m_bAllowUAV = true;
      hPreviousVelocity = pDevice->CreateTexture(desc);
    }

    ezGALTextureHandle hHistory;
    {
      ezGALTextureCreationDescription desc = pInputColor->m_Desc;
      desc.m_bAllowShaderResourceView = true;
      desc.m_bAllowUAV = true;
      hHistory = pDevice->CreateTexture(desc);
    }

    // TAA Pass
    {
      auto pCommandEncoder = ezRenderContext::BeginComputeScope(pPass, renderViewContext, "TAA");

      renderViewContext.m_pRenderContext->BindShader(m_hTAAShader);

      ezGALUnorderedAccessViewHandle hOutput;
      {
        ezGALUnorderedAccessViewCreationDescription desc;
        desc.m_hTexture = pOutput->m_TextureHandle;
        desc.m_uiMipLevelToUse = 0;
        hOutput = pDevice->CreateUnorderedAccessView(desc);
      }

      renderViewContext.m_pRenderContext->BindUAV("OutputTexture", hOutput);
      renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(pInputColor->m_TextureHandle));
      renderViewContext.m_pRenderContext->BindTexture2D("HistoryTexture", pDevice->GetDefaultResourceView(m_hHistory));
      renderViewContext.m_pRenderContext->BindTexture2D("VelocityTexture", pDevice->GetDefaultResourceView(pInputVelocity->m_TextureHandle));
      renderViewContext.m_pRenderContext->BindTexture2D("PreviousVelocityTexture", pDevice->GetDefaultResourceView(m_hPreviousVelocity));
      renderViewContext.m_pRenderContext->BindTexture2D("SceneDepth", pDevice->GetDefaultResourceView(pInputDepth->m_TextureHandle));
      renderViewContext.m_pRenderContext->BindConstantBuffer("ezTAAConstants", m_hTAAConstantBuffer);

      const ezUInt32 uiWidth = pOutput->m_Desc.m_uiWidth;
      const ezUInt32 uiHeight = pOutput->m_Desc.m_uiHeight;

      const ezUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
      const ezUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

      UpdateTAAConstantBuffer();

      renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();
    }

    // Accumulate Pass
    {
      auto pCommandEncoder = ezRenderContext::BeginComputeScope(pPass, renderViewContext, "Accumulate");

      renderViewContext.m_pRenderContext->BindShader(m_hCopyShader);

      // History
      {
        ezGALUnorderedAccessViewHandle hOutput;
        {
          ezGALUnorderedAccessViewCreationDescription desc;
          desc.m_hTexture = hHistory;
          desc.m_uiMipLevelToUse = 0;
          hOutput = pDevice->CreateUnorderedAccessView(desc);
        }

        renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);
        renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(pOutput->m_TextureHandle));
        renderViewContext.m_pRenderContext->BindConstantBuffer("ezCopyConstants", m_hCopyConstantBuffer);
        renderViewContext.m_pRenderContext->SetShaderPermutationVariable("COPY_TEXTURE_FILTERING", "COPY_TEXTURE_FILTERING_POINT");

        const ezUInt32 uiWidth = pInputColor->m_Desc.m_uiWidth;
        const ezUInt32 uiHeight = pInputColor->m_Desc.m_uiHeight;

        const ezUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
        const ezUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

        UpdateCopyConstantBuffer(ezVec2I32::ZeroVector(), ezVec2U32(uiWidth, uiHeight));

        renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();
      }

      // Velocity
      {
        ezGALUnorderedAccessViewHandle hOutput;
        {
          ezGALUnorderedAccessViewCreationDescription desc;
          desc.m_hTexture = hPreviousVelocity;
          desc.m_uiMipLevelToUse = 0;
          hOutput = pDevice->CreateUnorderedAccessView(desc);
        }

        renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);
        renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(pInputVelocity->m_TextureHandle));
        renderViewContext.m_pRenderContext->BindConstantBuffer("ezCopyConstants", m_hCopyConstantBuffer);
        renderViewContext.m_pRenderContext->SetShaderPermutationVariable("COPY_TEXTURE_FILTERING", "COPY_TEXTURE_FILTERING_POINT");

        const ezUInt32 uiWidth = pInputVelocity->m_Desc.m_uiWidth;
        const ezUInt32 uiHeight = pInputVelocity->m_Desc.m_uiHeight;

        const ezUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
        const ezUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

        UpdateCopyConstantBuffer(ezVec2I32::ZeroVector(), ezVec2U32(uiWidth, uiHeight));

        renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();
      }
    }

    // Clean up
    if (!m_hHistory.IsInvalidated())
      pDevice->DestroyTexture(m_hHistory);

    if (!m_hPreviousVelocity.IsInvalidated())
      pDevice->DestroyTexture(m_hPreviousVelocity);

    m_hHistory = hHistory;
    m_hPreviousVelocity = hPreviousVelocity;
  }
  pDevice->EndPass(pPass);

  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(bAllowAsyncShaderLoading);
}

void ezTAAPass::ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  const auto* const pInput = inputs[m_PinInputColor.m_uiInputIndex];
  const auto* const pOutput = outputs[m_PinOutput.m_uiOutputIndex];

  if (pInput == nullptr || pOutput == nullptr)
  {
    return;
  }

  const ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  const ezGALTexture* pDest = pDevice->GetTexture(pOutput->m_TextureHandle);

  if (const ezGALTexture* pSource = pDevice->GetTexture(pInput->m_TextureHandle); pDest->GetDescription().m_Format != pSource->GetDescription().m_Format)
  {
    // TODO: use a shader when the format doesn't match exactly

    ezLog::Error("Copying textures of different formats is not implemented");
  }
  else
  {
    auto pCommandEncoder = ezRenderContext::BeginPassAndComputeScope(renderViewContext, GetName());

    pCommandEncoder->CopyTexture(pOutput->m_TextureHandle, pInput->m_TextureHandle);
  }
}

void ezTAAPass::UpdateTAAConstantBuffer() const
{
  auto* constants = ezRenderContext::GetConstantBufferData<ezTAAConstants>(m_hTAAConstantBuffer);
  constants->UpsampleEnabled = m_bUpsample;
}

void ezTAAPass::UpdateCopyConstantBuffer(ezVec2I32 offset, ezVec2U32 size) const
{
  auto* constants = ezRenderContext::GetConstantBufferData<ezCopyConstants>(m_hCopyConstantBuffer);
  constants->Offset = offset;
  constants->OutputSize = size;
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_TAAPass);
