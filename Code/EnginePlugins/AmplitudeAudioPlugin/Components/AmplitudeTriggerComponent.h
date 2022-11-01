#pragma once

#include <Core/Messages/EventMessage.h>
#include <Core/ResourceManager/Resource.h>
#include <GameEngine/VisualScript/VisualScriptNode.h>

#include <AmplitudeAudioPlugin/Components/AmplitudeComponent.h>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

class ezPhysicsWorldModuleInterface;

class ezAmplitudeTriggerComponentManager : public ezComponentManager<class ezAmplitudeTriggerComponent, ezBlockStorageType::FreeList>
{
public:
  ezAmplitudeTriggerComponentManager(ezWorld* pWorld);

  virtual void Initialize() override;
  virtual void Deinitialize() override;

private:
  friend class ezAmplitudeTriggerComponent;

  struct ObstructionOcclusionState;
  ezDynamicArray<ObstructionOcclusionState> m_ObstructionOcclusionStates;

  ezUInt32 AddObstructionOcclusionState(ezAmplitudeTriggerComponent* pComponent);
  void RemoveObstructionOcclusionState(ezUInt32 uiIndex);
  [[nodiscard]] const ObstructionOcclusionState& GetObstructionOcclusionState(ezUInt32 uiIndex) const { return m_ObstructionOcclusionStates[uiIndex]; }

  void ShootOcclusionRays(ObstructionOcclusionState& state, ezVec3 listenerPos, ezUInt32 uiNumRays, const ezPhysicsWorldModuleInterface* pPhysicsWorldModule, ezTime deltaTime);
  void UpdateOcclusion(const ezWorldModule::UpdateContext& context);
  void UpdateTriggers(const ezWorldModule::UpdateContext& context);

  void ResourceEventHandler(const ezResourceEvent& e);
};

/// \brief Sent when a sound is finished playing.
struct EZ_AMPLITUDEAUDIOPLUGIN_DLL ezMsgAmplitudeSoundEnded : public ezEventMessage
{
  EZ_DECLARE_MESSAGE_TYPE(ezMsgAmplitudeSoundEnded, ezEventMessage);
};

class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeTriggerComponent : public ezAmplitudeComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezAmplitudeTriggerComponent, ezAmplitudeComponent, ezAmplitudeTriggerComponentManager);

  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& stream) const override;
  virtual void DeserializeComponent(ezWorldReader& stream) override;

protected:
  virtual void OnSimulationStarted() override;
  virtual void OnDeactivated() override;

  // ezAmplitudeComponent

private:
  friend class ezComponentManagerSimple<class ezAmplitudeTriggerComponent, ezComponentUpdateType::WhenSimulating>;

  virtual void ezAmplitudeComponentIsAbstract() override
  {
  }

  // ezAmplitudeTriggerComponent

public:
  ezAmplitudeTriggerComponent();
  ~ezAmplitudeTriggerComponent();

  void SetSoundObjectName(const char* szSoundObjectName);
  const char* GetSoundObjectName() const;

  void SetGain(float gain);
  [[nodiscard]] float GetGain() const;

  void SetOcclusionCollisionLayer(ezUInt8 uiCollisionLayer);                         // [ property ]
  ezUInt8 GetOcclusionCollisionLayer() const { return m_uiOcclusionCollisionLayer; } // [ property ]

  [[nodiscard]] bool IsPlaying() const;

  void Play();

  void Stop(double dDuration = SparkyStudios::Audio::Amplitude::kMinFadeDuration);

  void Pause(double dDuration = SparkyStudios::Audio::Amplitude::kMinFadeDuration);

  void Resume(double dDuration = SparkyStudios::Audio::Amplitude::kMinFadeDuration);

  ezEnum<ezOnComponentFinishedAction> m_OnFinishedAction; // [ property ]

protected:
  void OnMsgDeleteGameObject(ezMsgDeleteGameObject& msg); // [ msg handler ]

  void Update();
  void UpdateEntity();
  void UpdateOcclusion();

  ezString m_sSoundObjectName;
  float m_fGain;
  ezUInt8 m_uiOcclusionCollisionLayer;

  ezUInt32 m_uiObstructionOcclusionStateIndex = ezInvalidIndex;

  ezEventMessageSender<ezMsgAmplitudeSoundEnded> m_SoundEndedEventSender; // [ event ]

private:
  SparkyStudios::Audio::Amplitude::Channel m_TriggerChannel;
  SparkyStudios::Audio::Amplitude::Entity m_TriggerEntity;
};
