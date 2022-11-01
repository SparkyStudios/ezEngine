#pragma once

#include <AmplitudeAudioPlugin/Components/AmplitudeComponent.h>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

typedef ezComponentManagerSimple<class ezAmplitudeListenerComponent, ezComponentUpdateType::WhenSimulating> ezAmplitudeListenerComponentManager;

/// \brief Component that represents a listener in the audio scene.
class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeListenerComponent : public ezAmplitudeComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezAmplitudeListenerComponent, ezAmplitudeComponent, ezAmplitudeListenerComponentManager);

  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& stream) const override;
  virtual void DeserializeComponent(ezWorldReader& stream) override;

  // ezAmplitudeComponent

private:
  virtual void ezAmplitudeComponentIsAbstract() override {}

  // ezAmplitudeListenerComponent

public:
  ezAmplitudeListenerComponent();
  ~ezAmplitudeListenerComponent();

  SparkyStudios::Audio::Amplitude::AmListenerID m_uiListenerId = SparkyStudios::Audio::Amplitude::kAmInvalidObjectId; // [ property ]

protected:
  void Update();
};
