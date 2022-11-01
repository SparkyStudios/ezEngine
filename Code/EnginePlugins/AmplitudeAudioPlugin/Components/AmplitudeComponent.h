#pragma once

#include <Core/World/Component.h>
#include <Core/World/World.h>

#include <AmplitudeAudioPlugin/AmplitudeAudioPluginDLL.h>

/// \brief Base class for all Amplitude components, such that they all have a common ancestor
class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeComponent : public ezComponent
{
  EZ_DECLARE_ABSTRACT_COMPONENT_TYPE(ezAmplitudeComponent, ezComponent);

  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& stream) const override {}
  virtual void DeserializeComponent(ezWorldReader& stream) override {}

  // ezAmplitudeComponent

public:
  ezAmplitudeComponent();
  ~ezAmplitudeComponent();

private:
  // Dummy method to hide this component in the editor UI.
  virtual void ezAmplitudeComponentIsAbstract() = 0;
};
