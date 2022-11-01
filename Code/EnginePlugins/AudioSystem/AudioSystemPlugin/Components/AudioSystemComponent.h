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

  static void UpdateFunctionName(ezStringBuilder& out_sName);
};

/// \brief Base class for audio system components.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSystemComponent : public ezComponent
{
  EZ_DECLARE_ABSTRACT_COMPONENT_TYPE(ezAudioSystemComponent, ezComponent);

  // ezAudioSystemComponent

protected:
  // Dummy method to hide this component in the editor UI.
  virtual void ezAudioSystemComponentIsAbstract() = 0;
};

/// \brief Base class for audio system components that depends on an audio proxy component.
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
  [[nodiscard]] ezAudioSystemDataID GetEntityId() const;

  class ezAudioProxyComponent* m_pProxyComponent{nullptr};
};

/// \brief Base class for audio system environment components.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSystemEnvironmentComponent : public ezAudioSystemProxyDependentComponent
{
  EZ_DECLARE_ABSTRACT_COMPONENT_TYPE(ezAudioSystemEnvironmentComponent, ezAudioSystemProxyDependentComponent);

  // ezAudioSystemEnvironmentComponent

public:
  ezAudioSystemEnvironmentComponent();

  void OnActivated() override;
  void OnDeactivated() override;

  [[nodiscard]] virtual ezAudioSystemDataID GetEnvironmentId() const = 0;

  [[nodiscard]] virtual float GetEnvironmentAmount(ezAudioProxyComponent* pProxyComponent) const = 0;

  /// \brief Gets the distance from the sphere's origin
  /// at which the environment amount will slightly start to decrease.
  [[nodiscard]] virtual float GetMaxDistance() const;

  /// \brief Sets the distance from the sphere's origin at which
  /// the environment amount will slightly start to decrease.
  virtual void SetMaxDistance(float fFadeDistance);

protected:
  float m_fMaxDistance;
  ezString m_sEnvironmentName;
  ezColor m_ShapeColor;
};

template <typename T>
ezAudioSystemComponentManager<T>::ezAudioSystemComponentManager(ezWorld* pWorld)
  : ezComponentManager<T, ezBlockStorageType::FreeList>(pWorld)
{
}

template <typename T>
void ezAudioSystemComponentManager<T>::Initialize()
{
  ezStringBuilder functionName;
  UpdateFunctionName(functionName);

  auto desc = ezWorldModule::UpdateFunctionDesc(ezWorldModule::UpdateFunction(&ezAudioSystemComponentManager<T>::Update, this), functionName);
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

// static
template <typename T>
void ezAudioSystemComponentManager<T>::UpdateFunctionName(ezStringBuilder& out_sName)
{
  ezStringView sName(EZ_SOURCE_FUNCTION);
  const char* szEnd = sName.FindSubString(",");

  if (szEnd != nullptr && sName.StartsWith("ezAudioSystemComponentManager<class "))
  {
    ezStringView sChoppedName(sName.GetStartPointer() + ezStringUtils::GetStringElementCount("ezAudioSystemComponentManager<class "), szEnd);

    EZ_ASSERT_DEV(!sChoppedName.IsEmpty(), "Chopped name is empty: '{0}'", sName);

    out_sName = sChoppedName;
    out_sName.Append("::Update");
  }
  else
  {
    out_sName = sName;
  }
}
