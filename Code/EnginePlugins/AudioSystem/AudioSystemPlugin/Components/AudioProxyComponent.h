#pragma once

#include <AudioSystemPlugin/AudioSystemPluginDLL.h>

#include <AudioSystemPlugin/Components/AudioSystemComponent.h>

typedef ezAudioSystemComponentManager<class ezAudioProxyComponent> ezAudioProxyComponentManager;

class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioProxyComponent : public ezAudioSystemComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezAudioProxyComponent, ezAudioSystemComponent, ezAudioProxyComponentManager);

  // ezComponent

public:
  void Initialize() override;
  void Deinitialize() override;

  // ezAudioSystemComponent

private:
  void ezAudioSystemComponentIsAbstract() override {}

  // ezAudioProxyComponent

public:
  ezAudioProxyComponent();
  ~ezAudioProxyComponent() override;

  /// \brief Gets the assigned Audio System Entity ID to this game object.
  ///
  /// Entity ids are automatically assigned at the start of the simulation. They may change each time
  /// the game is simulated. You should not assert that an ID will always be assigned to the same game object every time.
  ezAudioSystemDataID GetEntityId() const;

protected:
  void Update();

private:
  friend class ezAudioSystemProxyDependentComponent;

  ezAudioSystemDataID m_uiEntityId{0};
  ezUInt32 m_uiRefCount{0};

  ezAudioSystemTransform m_LastTransform;
};
