#pragma once

#include <AudioSystemPlugin/AudioSystemPluginDLL.h>

#include <AudioSystemPlugin/Components/AudioSystemComponent.h>
#include <AudioSystemPlugin/Core/AudioSystemAllocator.h>

#include <Core/World/WorldModule.h>

class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioWorldModule : public ezWorldModule
{
  EZ_DECLARE_WORLD_MODULE();
  EZ_ADD_DYNAMIC_REFLECTION(ezAudioWorldModule, ezWorldModule);

  using EnvironmentSet = ezSet<const ezAudioSystemEnvironmentComponent*, ezCompareHelper<const ezAudioSystemEnvironmentComponent*>, ezAudioSystemAllocatorWrapper>;

public:
  explicit ezAudioWorldModule(ezWorld* pWorld);
  ~ezAudioWorldModule() override;

  void Initialize() override;

  void AddEnvironment(const ezAudioSystemEnvironmentComponent* pComponent);
  void RemoveEnvironment(const ezAudioSystemEnvironmentComponent* pComponent);
  [[nodiscard]] EnvironmentSet::Iterator GetEnvironments() const;

private:
  EnvironmentSet m_lEnvironmentComponents;
};
