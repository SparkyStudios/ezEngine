#pragma once

#include <AudioSystemPlugin/AudioSystemPluginDLL.h>

#include <AudioSystemPlugin/Core/AudioSystemData.h>

#include <Core/World/Component.h>
#include <Core/World/World.h>

/// \brief Base class for audio system component manager which need to update their states (eg. AudioListenerComponent).
template <typename T>
class ezAudioSystemComponentManager final : public ezComponentManager<T, ezBlockStorageType::FreeList>
{
  // ezComponentManager

public:
  void Initialize() override;

  // ezAudioSystemComponentManager

public:
  explicit ezAudioSystemComponentManager(ezWorld* pWorld);

private:
  /// \brief A simple update function that iterates over all components and calls Update() on every component
  void Update(const ezWorldModule::UpdateContext& context);
};

/// \brief Base class for audio system components.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSystemComponent : public ezComponent
{
  EZ_DECLARE_ABSTRACT_COMPONENT_TYPE(ezAudioSystemComponent, ezComponent);

  // ezAudioSystemComponent

private:
  // Dummy method to hide this component in the editor UI.
  virtual void ezAudioSystemComponentIsAbstract() = 0;
};

class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSystemProxyDependentComponent : public ezAudioSystemComponent
{
  EZ_DECLARE_ABSTRACT_COMPONENT_TYPE(ezAudioSystemProxyDependentComponent, ezAudioSystemComponent);

  // ezComponent

public:
  void Initialize() override;
  void Deinitialize() override;

  // ezAudiSystemProxyDependentComponent

protected:
  /// \brief Get the ID of the entity referenced by the proxy.
  ezAudioSystemDataID GetEntityId() const;

  class ezAudioProxyComponent* m_pProxyComponent{nullptr};
};

template <typename T>
ezAudioSystemComponentManager<T>::ezAudioSystemComponentManager(ezWorld* pWorld)
  : ezComponentManager<T, ezBlockStorageType::FreeList>(pWorld)
{
}

template <typename T>
void ezAudioSystemComponentManager<T>::Initialize()
{
  auto desc = ezWorldModule::UpdateFunctionDesc(ezWorldModule::UpdateFunction(&ezAudioSystemComponentManager<T>::Update, this), "ezAudioSystemComponentManager::Update");
  desc.m_bOnlyUpdateWhenSimulating = true;
  desc.m_Phase = ezWorldModule::UpdateFunctionDesc::Phase::PostTransform; // Should we apply entity transform after game object transform?

  this->RegisterUpdateFunction(desc);
}

template <typename T>
void ezAudioSystemComponentManager<T>::Update(const ezWorldModule::UpdateContext& context)
{
  for (auto it = this->m_ComponentStorage.GetIterator(context.m_uiFirstComponentIndex, context.m_uiComponentCount); it.IsValid(); ++it)
  {
    T* pComponent = it;
    if (pComponent->IsActiveAndInitialized())
    {
      pComponent->Update();
    }
  }
}
