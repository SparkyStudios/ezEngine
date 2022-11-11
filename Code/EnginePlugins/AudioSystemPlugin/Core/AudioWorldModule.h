#pragma once

#include <AudioSystemPlugin/AudioSystemPluginDLL.h>

#include <AudioSystemPlugin/Components/AudioSystemComponent.h>
#include <AudioSystemPlugin/Components/AudioListenerComponent.h>
#include <AudioSystemPlugin/Core/AudioSystemAllocator.h>

#include <Core/World/WorldModule.h>

/// \brief World Module allowing to access audio system features, query environments
///  and environment amounts, and extracting obstruction/occlusion data.
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
  EZ_NODISCARD EnvironmentSet::Iterator GetEnvironments() const;

  void SetDefaultListener(const ezAudioListenerComponent* pListener);
  EZ_NODISCARD const ezAudioListenerComponent* GetDefaultListener() const;

private:
  EnvironmentSet m_lEnvironmentComponents;
  const ezAudioListenerComponent* m_pDefaultListener;

  // TODO: Add events handlers for default listener change
};
