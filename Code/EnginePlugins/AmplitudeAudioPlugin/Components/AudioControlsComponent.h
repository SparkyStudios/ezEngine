#pragma once

#include <AmplitudeAudioPlugin/AmplitudeAudioPluginDLL.h>

#include <AmplitudeAudioPlugin/Components/AmplitudeComponent.h>
#include <AmplitudeAudioPlugin/Resources/AudioControlCollectionResource.h>
#include <AudioSystemPlugin/Core/AudioSystemData.h>

typedef ezComponentManager<class ezAudioControlsComponent, ezBlockStorageType::FreeList> ezAudioControlsComponentManager;

/// \brief Component used to load and unload a set of audio controls.
///
/// The audio controls are provided by the selected audio control collection.
class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAudioControlsComponent : public ezAmplitudeComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezAudioControlsComponent, ezAmplitudeComponent, ezAudioControlsComponentManager);

  // ezComponent

public:
  void Initialize() override;
  void Deinitialize() override;
  void SerializeComponent(ezWorldWriter& stream) const override;
  void DeserializeComponent(ezWorldReader& stream) override;

  // ezAmplitudeComponent

private:
  void ezAmplitudeComponentIsAbstract() override {}

  // ezAudioControlsComponent

public:
  ezAudioControlsComponent();
  ~ezAudioControlsComponent() override;

  /// \brief Load the audio controls from the given collection.
  /// This is automatically called on component initialization when
  /// the AutoLoad property is set to true.
  bool Load();

  /// \brief Unloads the audio controls.
  bool Unload();

private:
  ezString m_sControlsAsset;
  bool m_bAutoLoad;

  bool m_bLoaded;
  ezAudioControlCollectionResourceHandle m_hControlsResource;
};
