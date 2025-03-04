#include <ParticlePlugin/ParticlePluginPCH.h>

#include <Core/World/World.h>
#include <ParticlePlugin/Components/ParticleComponent.h>
#include <ParticlePlugin/Effect/ParticleEffectInstance.h>
#include <ParticlePlugin/Events/ParticleEvent.h>
#include <ParticlePlugin/Events/ParticleEventReaction_Effect.h>
#include <ParticlePlugin/Resources/ParticleEffectResource.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleEventReactionFactory_Effect, 1, ezRTTIDefaultAllocator<ezParticleEventReactionFactory_Effect>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Effect", m_sEffect)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Particle_Effect")),
    EZ_ENUM_MEMBER_PROPERTY("Alignment", ezSurfaceInteractionAlignment, m_Alignment),
    EZ_MAP_ACCESSOR_PROPERTY("Parameters", GetParameters, GetParameter, SetParameter, RemoveParameter)->AddAttributes(new ezExposedParametersAttribute("Effect"), new ezExposeColorAlphaAttribute),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleEventReaction_Effect, 1, ezRTTIDefaultAllocator<ezParticleEventReaction_Effect>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezParticleEventReactionFactory_Effect::ezParticleEventReactionFactory_Effect()
{
  m_pParameters = EZ_DEFAULT_NEW(ezParticleEffectParameters);
}

enum class ReactionEffectVersion
{
  Version_0 = 0,
  Version_1,
  Version_2, // added effect parameters
  Version_3, // added alignment

  // insert new version numbers above
  Version_Count,
  Version_Current = Version_Count - 1
};

void ezParticleEventReactionFactory_Effect::Save(ezStreamWriter& inout_stream) const
{
  SUPER::Save(inout_stream);

  const ezUInt8 uiVersion = (int)ReactionEffectVersion::Version_Current;
  inout_stream << uiVersion;

  // Version 1
  inout_stream << m_sEffect;

  // Version 2
  inout_stream << m_pParameters->m_FloatParams.GetCount();
  for (ezUInt32 i = 0; i < m_pParameters->m_FloatParams.GetCount(); ++i)
  {
    inout_stream << m_pParameters->m_FloatParams[i].m_sName;
    inout_stream << m_pParameters->m_FloatParams[i].m_Value;
  }
  inout_stream << m_pParameters->m_ColorParams.GetCount();
  for (ezUInt32 i = 0; i < m_pParameters->m_ColorParams.GetCount(); ++i)
  {
    inout_stream << m_pParameters->m_ColorParams[i].m_sName;
    inout_stream << m_pParameters->m_ColorParams[i].m_Value;
  }

  // Version 3
  inout_stream << m_Alignment;
}

void ezParticleEventReactionFactory_Effect::Load(ezStreamReader& inout_stream)
{
  SUPER::Load(inout_stream);

  ezUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  EZ_ASSERT_DEV(uiVersion <= (int)ReactionEffectVersion::Version_Current, "Invalid version {0}", uiVersion);

  // Version 1
  inout_stream >> m_sEffect;

  if (uiVersion >= 2)
  {
    ezUInt32 numFloats, numColors;

    inout_stream >> numFloats;
    m_pParameters->m_FloatParams.SetCountUninitialized(numFloats);

    for (ezUInt32 i = 0; i < m_pParameters->m_FloatParams.GetCount(); ++i)
    {
      inout_stream >> m_pParameters->m_FloatParams[i].m_sName;
      inout_stream >> m_pParameters->m_FloatParams[i].m_Value;
    }

    inout_stream >> numColors;
    m_pParameters->m_ColorParams.SetCountUninitialized(numColors);

    for (ezUInt32 i = 0; i < m_pParameters->m_ColorParams.GetCount(); ++i)
    {
      inout_stream >> m_pParameters->m_ColorParams[i].m_sName;
      inout_stream >> m_pParameters->m_ColorParams[i].m_Value;
    }
  }

  if (uiVersion >= 3)
  {
    inout_stream >> m_Alignment;
  }
}


const ezRTTI* ezParticleEventReactionFactory_Effect::GetEventReactionType() const
{
  return ezGetStaticRTTI<ezParticleEventReaction_Effect>();
}


void ezParticleEventReactionFactory_Effect::CopyReactionProperties(ezParticleEventReaction* pObject, bool bFirstTime) const
{
  ezParticleEventReaction_Effect* pReaction = static_cast<ezParticleEventReaction_Effect*>(pObject);

  pReaction->m_hEffect.Invalidate();
  pReaction->m_Alignment = m_Alignment;

  if (!m_sEffect.IsEmpty())
    pReaction->m_hEffect = ezResourceManager::LoadResource<ezParticleEffectResource>(m_sEffect);

  pReaction->m_Parameters = m_pParameters;
}

const ezRangeView<const char*, ezUInt32> ezParticleEventReactionFactory_Effect::GetParameters() const
{
  return ezRangeView<const char*, ezUInt32>([this]() -> ezUInt32
    { return 0; },
    [this]() -> ezUInt32
    { return m_pParameters->m_FloatParams.GetCount() + m_pParameters->m_ColorParams.GetCount(); }, [this](ezUInt32& ref_uiIt)
    { ++ref_uiIt; },
    [this](const ezUInt32& uiIt) -> const char*
    {
      if (uiIt < m_pParameters->m_FloatParams.GetCount())
        return m_pParameters->m_FloatParams[uiIt].m_sName.GetData();
      else
        return m_pParameters->m_ColorParams[uiIt - m_pParameters->m_FloatParams.GetCount()].m_sName.GetData();
    });
}

void ezParticleEventReactionFactory_Effect::SetParameter(const char* szKey, const ezVariant& var)
{
  const ezTempHashedString th(szKey);
  if (var.CanConvertTo<float>())
  {
    float value = var.ConvertTo<float>();

    for (ezUInt32 i = 0; i < m_pParameters->m_FloatParams.GetCount(); ++i)
    {
      if (m_pParameters->m_FloatParams[i].m_sName == th)
      {
        if (m_pParameters->m_FloatParams[i].m_Value != value)
        {
          m_pParameters->m_FloatParams[i].m_Value = value;
        }
        return;
      }
    }

    auto& e = m_pParameters->m_FloatParams.ExpandAndGetRef();
    e.m_sName.Assign(szKey);
    e.m_Value = value;

    return;
  }

  if (var.CanConvertTo<ezColor>())
  {
    ezColor value = var.ConvertTo<ezColor>();

    for (ezUInt32 i = 0; i < m_pParameters->m_ColorParams.GetCount(); ++i)
    {
      if (m_pParameters->m_ColorParams[i].m_sName == th)
      {
        if (m_pParameters->m_ColorParams[i].m_Value != value)
        {
          m_pParameters->m_ColorParams[i].m_Value = value;
        }
        return;
      }
    }

    auto& e = m_pParameters->m_ColorParams.ExpandAndGetRef();
    e.m_sName.Assign(szKey);
    e.m_Value = value;

    return;
  }
}

void ezParticleEventReactionFactory_Effect::RemoveParameter(const char* szKey)
{
  const ezTempHashedString th(szKey);

  for (ezUInt32 i = 0; i < m_pParameters->m_FloatParams.GetCount(); ++i)
  {
    if (m_pParameters->m_FloatParams[i].m_sName == th)
    {
      m_pParameters->m_FloatParams.RemoveAtAndSwap(i);
      return;
    }
  }

  for (ezUInt32 i = 0; i < m_pParameters->m_ColorParams.GetCount(); ++i)
  {
    if (m_pParameters->m_ColorParams[i].m_sName == th)
    {
      m_pParameters->m_ColorParams.RemoveAtAndSwap(i);
      return;
    }
  }
}

bool ezParticleEventReactionFactory_Effect::GetParameter(const char* szKey, ezVariant& out_value) const
{
  const ezTempHashedString th(szKey);

  for (const auto& e : m_pParameters->m_FloatParams)
  {
    if (e.m_sName == th)
    {
      out_value = e.m_Value;
      return true;
    }
  }
  for (const auto& e : m_pParameters->m_ColorParams)
  {
    if (e.m_sName == th)
    {
      out_value = e.m_Value;
      return true;
    }
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////

ezParticleEventReaction_Effect::ezParticleEventReaction_Effect() = default;
ezParticleEventReaction_Effect::~ezParticleEventReaction_Effect() = default;

void ezParticleEventReaction_Effect::ProcessEvent(const ezParticleEvent& e)
{
  if (!m_hEffect.IsValid())
    return;

  ezGameObjectDesc god;
  god.m_bDynamic = true;
  god.m_LocalPosition = e.m_vPosition;

  ezVec3 vAlignDir = e.m_vNormal;

  switch (m_Alignment)
  {
    case ezSurfaceInteractionAlignment::IncidentDirection:
      vAlignDir = -e.m_vDirection;
      break;

    case ezSurfaceInteractionAlignment::ReflectedDirection:
      vAlignDir = e.m_vDirection.GetReflectedVector(e.m_vNormal);
      break;

    case ezSurfaceInteractionAlignment::ReverseSurfaceNormal:
      vAlignDir = -e.m_vNormal;
      break;

    case ezSurfaceInteractionAlignment::ReverseIncidentDirection:
      vAlignDir = e.m_vDirection;
      ;
      break;

    case ezSurfaceInteractionAlignment::ReverseReflectedDirection:
      vAlignDir = -e.m_vDirection.GetReflectedVector(e.m_vNormal);
      break;

    case ezSurfaceInteractionAlignment::SurfaceNormal:
      break;
  }

  if (!vAlignDir.IsZero())
  {
    god.m_LocalRotation = ezQuat::MakeShortestRotation(ezVec3(0, 0, 1), vAlignDir);
  }

  ezGameObject* pObject = nullptr;
  m_pOwnerEffect->GetWorld()->CreateObject(god, pObject);

  ezParticleComponent* pComponent = nullptr;
  ezParticleComponent::CreateComponent(pObject, pComponent);

  pComponent->m_uiRandomSeed = m_pOwnerEffect->GetRandomSeed();

  pComponent->m_bIfContinuousStopRightAway = true;
  pComponent->m_OnFinishedAction = ezOnComponentFinishedAction2::DeleteGameObject;
  pComponent->SetParticleEffect(m_hEffect);

  if (!m_Parameters->m_FloatParams.IsEmpty())
  {
    pComponent->m_bFloatParamsChanged = true;
    pComponent->m_FloatParams = m_Parameters->m_FloatParams;
  }

  if (!m_Parameters->m_ColorParams.IsEmpty())
  {
    pComponent->m_bColorParamsChanged = true;
    pComponent->m_ColorParams = m_Parameters->m_ColorParams;
  }
}


EZ_STATICLINK_FILE(ParticlePlugin, ParticlePlugin_Events_ParticleEventReaction_Effect);
