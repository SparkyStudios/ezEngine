#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Pipeline/Passes/ColorGradingPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/Textures/Texture2DResource.h>
#include <RendererCore/Textures/Texture3DResource.h>

#include <RendererFoundation/Resources/RenderTargetView.h>
#include <RendererFoundation/Resources/Texture.h>

#include "../../../../../../Data/Base/Shaders/Pipeline/ColorGradingConstants.h"

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezColorGradingPass, 1, ezRTTIDefaultAllocator<ezColorGradingPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Color", m_PinInput),
    EZ_MEMBER_PROPERTY("Output", m_PinOutput),
    EZ_MEMBER_PROPERTY("MoodColor", m_MoodColor)->AddAttributes(new ezDefaultValueAttribute(ezColor::Orange)),
    EZ_MEMBER_PROPERTY("MoodStrength", m_fMoodStrength)->AddAttributes(new ezClampValueAttribute(0.0f, {})),
    EZ_MEMBER_PROPERTY("Saturation", m_fSaturation)->AddAttributes(new ezClampValueAttribute(0.0f, 2.0f), new ezDefaultValueAttribute(1.0f)),
    EZ_MEMBER_PROPERTY("Contrast", m_fContrast)->AddAttributes(new ezClampValueAttribute(0.0f, 1.0f)),
    EZ_ACCESSOR_PROPERTY("LUT1", GetLUT1TextureFile, SetLUT1TextureFile)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Texture_3D")),
    EZ_MEMBER_PROPERTY("LUT1Strength", m_fLut1Strength)->AddAttributes(new ezClampValueAttribute(0.0f, 1.0f)),
    EZ_ACCESSOR_PROPERTY("LUT2", GetLUT2TextureFile, SetLUT2TextureFile)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Texture_3D")),
    EZ_MEMBER_PROPERTY("LUT2Strength", m_fLut2Strength)->AddAttributes(new ezClampValueAttribute(0.0f, 1.0f)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezColorGradingPass::ezColorGradingPass()
  : ezRenderPipelinePass("ColorGradingPass", true)
  , m_MoodColor(ezColor::Orange)
  , m_fMoodStrength(0.0f)
  , m_fSaturation(1.0f)
  , m_fContrast(1.0f)
  , m_fLut1Strength(0.0f)
  , m_fLut2Strength(0.0f)
{
  // Loading shaders
  {
    m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/ColorGrading.ezShader");
    EZ_ASSERT_DEV(m_hShader.IsValid(), "Could not load ColorGrading shader!");
  }

  // Loading resources
  {
    m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezColorGradingConstants>();
  }
}

ezColorGradingPass::~ezColorGradingPass()
{
  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
  m_hConstantBuffer.Invalidate();
}

bool ezColorGradingPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  if (inputs[m_PinInput.m_uiInputIndex])
  {
    if (!inputs[m_PinInput.m_uiInputIndex]->m_bAllowShaderResourceView)
    {
      ezLog::Error("'{0}' input must allow shader resource view.", GetName());
      return false;
    }
  }
  else
  {
    ezLog::Error("No input connected to '{0}'.", GetName());
    return false;
  }

  ezGALTextureCreationDescription desc = *inputs[m_PinInput.m_uiInputIndex];
  desc.m_bAllowUAV = true;
  outputs[m_PinOutput.m_uiOutputIndex] = std::move(desc);

  return true;
}

void ezColorGradingPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  const auto* const pInput = inputs[m_PinInput.m_uiInputIndex];
  const auto* const pOutput = outputs[m_PinOutput.m_uiOutputIndex];

  if (pInput == nullptr || pOutput == nullptr)
  {
    return;
  }

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();
  ezResourceManager::ForceLoadResourceNow(m_hShader);

  const bool bAllowAsyncShaderLoading = renderViewContext.m_pRenderContext->GetAllowAsyncShaderLoading();
  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(false);

  ezGALPass* pPass = pDevice->BeginPass(GetName());
  {
    auto pCommandEncoder = ezRenderContext::BeginComputeScope(pPass, renderViewContext);

    ezUInt32 numLUTs = 0;
    ezTexture3DResourceHandle luts[2] = {};
    float lutStrengths[2] = {};

    // Determine how many LUTs are active
    {
      if (m_hLUT1.IsValid())
      {
        luts[numLUTs] = m_hLUT1;
        lutStrengths[numLUTs] = m_fLut1Strength;
        numLUTs++;
      }

      if (m_hLUT2.IsValid())
      {
        luts[numLUTs] = m_hLUT2;
        lutStrengths[numLUTs] = m_fLut2Strength;
        numLUTs++;
      }
    }

    // Update shader constants
    {
      auto* constants = ezRenderContext::GetConstantBufferData<ezColorGradingConstants>(m_hConstantBuffer);
      constants->AutoExposureParams.SetZero();
      constants->MoodColor = m_MoodColor;
      constants->MoodStrength = m_fMoodStrength;
      constants->Saturation = m_fSaturation;
      constants->Lut1Strength = lutStrengths[0];
      constants->Lut2Strength = lutStrengths[1];

      // Pre-calculate factors of a s-shaped polynomial-function
      const float m = (0.5f - 0.5f * m_fContrast) / (0.5f + 0.5f * m_fContrast);
      const float a = 2.0f * m - 2.0f;
      const float b = -3.0f * m + 3.0f;

      constants->ContrastParams = ezVec4(a, b, m, 0.0f);
    }

    renderViewContext.m_pRenderContext->BindShader(m_hShader);

    renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(pInput->m_TextureHandle));
    renderViewContext.m_pRenderContext->BindTexture3D("Lut1Texture", luts[0]);
    renderViewContext.m_pRenderContext->BindTexture3D("Lut2Texture", luts[1]);

    ezGALUnorderedAccessViewHandle hOutput;
    {
      ezGALUnorderedAccessViewCreationDescription desc;
      desc.m_hTexture = pOutput->m_TextureHandle;
      desc.m_uiMipLevelToUse = 0;
      hOutput = pDevice->CreateUnorderedAccessView(desc);
    }

    renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);
    renderViewContext.m_pRenderContext->BindConstantBuffer("ezColorGradingConstants", m_hConstantBuffer);

    ezTempHashedString sLUTModeValues[3] = {"LUT_MODE_NONE", "LUT_MODE_ONE", "LUT_MODE_TWO"};
    renderViewContext.m_pRenderContext->SetShaderPermutationVariable("LUT_MODE", sLUTModeValues[numLUTs]);

    const ezUInt32 uiWidth = pOutput->m_Desc.m_uiWidth;
    const ezUInt32 uiHeight = pOutput->m_Desc.m_uiHeight;

    const ezUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
    const ezUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

    renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 6).IgnoreResult();
  }
  pDevice->EndPass(pPass);

  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(bAllowAsyncShaderLoading);
}

void ezColorGradingPass::SetLUT1TextureFile(const char* szFile)
{
  if (!ezStringUtils::IsNullOrEmpty(szFile))
  {
    m_hLUT1 = ezResourceManager::LoadResource<ezTexture3DResource>(szFile);
  }
}

const char* ezColorGradingPass::GetLUT1TextureFile() const
{
  if (!m_hLUT1.IsValid())
    return "";

  return m_hLUT1.GetResourceID();
}

void ezColorGradingPass::SetLUT2TextureFile(const char* szFile)
{
  if (!ezStringUtils::IsNullOrEmpty(szFile))
  {
    m_hLUT2 = ezResourceManager::LoadResource<ezTexture3DResource>(szFile);
  }
}

const char* ezColorGradingPass::GetLUT2TextureFile() const
{
  if (!m_hLUT2.IsValid())
    return "";

  return m_hLUT2.GetResourceID();
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_ColorGradingPass);
