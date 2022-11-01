#pragma once

#include <AudioSystemPlugin/AudioSystemPluginDLL.h>

#include <AudioSystemPlugin/Components/AudioSystemComponent.h>

/// \brief Component manager for audio system environment components.
typedef ezAudioSystemComponentManager<class ezAudioSphereEnvironmentComponent> ezAudioSystemEnvironmentComponentManager;

class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSphereEnvironmentComponent : public ezAudioSystemEnvironmentComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezAudioSphereEnvironmentComponent, ezAudioSystemEnvironmentComponent, ezAudioSystemEnvironmentComponentManager);

  // ezComponent

public:
  void Initialize() override;
  void Deinitialize() override;
  void SerializeComponent(ezWorldWriter& stream) const override;
  void DeserializeComponent(ezWorldReader& stream) override;

  // ezAudioSystemComponent

protected:
  void ezAudioSystemComponentIsAbstract() override {}

  // ezAudioSystemEnvironmentComponent

public:
  [[nodiscard]] ezAudioSystemDataID GetEnvironmentId() const override;

  [[nodiscard]] float GetEnvironmentAmount(ezAudioProxyComponent* pProxyComponent) const override;

  // ezAudioSphereEnvironmentComponent

public:
  ezAudioSphereEnvironmentComponent();

  /// \brief Gets the radius of the sphere that
  /// specifies the environment.
  [[nodiscard]] float GetRadius() const;

  /// \brief Sets the radius of the sphere that
  /// specifies the environment.
  void SetRadius(float fRadius);

protected:
  void Update();

private:
  ezBoundingSphere m_Sphere;
};
