#pragma once

#include <AudioSystemPlugin/AudioSystemPluginDLL.h>

#include <AudioSystemPlugin/Components/AudioSystemComponent.h>

typedef ezAudioSystemComponentManager<class ezAudioBoxEnvironmentComponent> ezAudioBoxEnvironmentComponentManager;

/// \brief Component that applies environmental effects in a box shape.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioBoxEnvironmentComponent : public ezAudioSystemEnvironmentComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezAudioBoxEnvironmentComponent, ezAudioSystemEnvironmentComponent, ezAudioBoxEnvironmentComponentManager);

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
  [[nodiscard]] float GetEnvironmentAmount(ezAudioProxyComponent* pProxyComponent) const override;

  // ezAudioBoxEnvironmentComponent

public:
  /// \brief Gets the radius of the sphere that
  /// specifies the environment.
  [[nodiscard]] const ezVec3& GetHalfExtends() const;

  /// \brief Sets the radius of the sphere that
  /// specifies the environment.
  void SetHalfExtends(const ezVec3& vHalfExtends);

protected:
  void Update();

private:
  ezVec3 m_vHalfExtends;

  ezBoundingBox m_Box;
};
