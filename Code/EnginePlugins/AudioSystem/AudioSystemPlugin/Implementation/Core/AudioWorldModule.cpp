#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AudioSystemPlugin/Core/AudioWorldModule.h>

#include <Core/World/World.h>

// clang-format off
EZ_IMPLEMENT_WORLD_MODULE(ezAudioWorldModule);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAudioWorldModule, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezAudioWorldModule::ezAudioWorldModule(ezWorld* pWorld)
  : ezWorldModule(pWorld)
{
}

ezAudioWorldModule::~ezAudioWorldModule()
{
  m_lEnvironmentComponents.Clear();
}

void ezAudioWorldModule::Initialize()
{
  SUPER::Initialize();
}

void ezAudioWorldModule::AddEnvironment(const ezAudioSystemEnvironmentComponent* pComponent)
{
  m_lEnvironmentComponents.Insert(pComponent);
}

void ezAudioWorldModule::RemoveEnvironment(const ezAudioSystemEnvironmentComponent* pComponent)
{
  m_lEnvironmentComponents.Remove(pComponent);
}

ezAudioWorldModule::EnvironmentSet::Iterator ezAudioWorldModule::GetEnvironments() const
{
  return m_lEnvironmentComponents.GetIterator();
}

EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_Implementation_Core_AudioWorldModule);
