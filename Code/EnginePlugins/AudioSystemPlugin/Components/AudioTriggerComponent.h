#pragma once

#include <AudioSystemPlugin/AudioSystemPluginDLL.h>

#include <AudioSystemPlugin/Components/AudioSystemComponent.h>
#include <AudioSystemPlugin/Core/AudioSystemData.h>

class ezPhysicsWorldModuleInterface;

constexpr ezUInt32 k_MaxOcclusionRaysCount = 32;

class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioTriggerComponentManager : public ezComponentManager<class ezAudioTriggerComponent, ezBlockStorageType::FreeList>
{
public:
  explicit ezAudioTriggerComponentManager(ezWorld* pWorld);

  void Initialize() override;
  void Deinitialize() override;

private:
  friend class ezAudioTriggerComponent;


  struct ObstructionOcclusionValue
  {
    /// \brief The new value target.
    float m_fTarget;

    /// \brief The current value.
    float m_fValue;

    /// \brief Gets the current value.
    [[nodiscard]] float GetValue() const;

    /// \brief Sets the new value's target.
    /// \param fTarget The value's target.
    /// \param bReset Specifies if the value should be reset
    /// to the new target.
    void SetTarget(float fTarget, bool bReset = false);

    /// \brief Updates the current value by moving it to the target
    /// value with a fSmoothFactor speed.
    /// \param fSmoothFactor The smooth factor, if not defined, will
    /// use the default one from CVar.
    void Update(float fSmoothFactor = -1.0f);

    /// \brief Resets the value and target to the given one.
    /// \param fInitialValue The new initial value.
    void Reset(float fInitialValue = 0.0f);
  };

  struct ObstructionOcclusionState
  {
    /// \brief The audio trigger component owner of this state
    ezAudioTriggerComponent* m_pComponent{nullptr};

    ezUInt8 m_uiNextRayIndex{0};

    ObstructionOcclusionValue m_ObstructionValue;
    ObstructionOcclusionValue m_OcclusionValue;

    ezStaticArray<float, k_MaxOcclusionRaysCount> m_ObstructionRaysValues;
  };

  ezUInt32 AddObstructionOcclusionState(ezAudioTriggerComponent* pComponent);
  void RemoveObstructionOcclusionState(ezUInt32 uiIndex);
  [[nodiscard]] const ObstructionOcclusionState& GetObstructionOcclusionState(ezUInt32 uiIndex) const { return m_ObstructionOcclusionStates[uiIndex]; }

  void ShootOcclusionRays(ObstructionOcclusionState& state, ezVec3 listenerPos, ezUInt32 uiNumRays, const ezPhysicsWorldModuleInterface* pPhysicsWorldModule);
  void CastRay(ObstructionOcclusionState& state, ezVec3 sourcePos, ezVec3 direction, ezUInt8 collisionLayer, const ezPhysicsWorldModuleInterface* pPhysicsWorldModule, ezUInt32 rayIndex);
  void ProcessOcclusion(const ezWorldModule::UpdateContext& context);
  void Update(const ezWorldModule::UpdateContext& context);

  ezDynamicArray<ObstructionOcclusionState> m_ObstructionOcclusionStates;
};

/// \brief Audio System Component that triggers an audio event.
///
/// This component takes as properties a mandatory play trigger an optional stop trigger.
/// The user should specify the name of the triggers as defined by the available audio controls loaded in the audio system.
/// If the stop trigger is left empty, the component will send a StopEvent request to the audio system, that means
/// the event triggered by the play trigger should be stoppable that way.
///
/// The component also exposes a property that allows to access the internal state through scripting.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioTriggerComponent : public ezAudioSystemProxyDependentComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezAudioTriggerComponent, ezAudioSystemProxyDependentComponent, ezAudioTriggerComponentManager);

  // ezComponent

public:
  void Initialize() override;
  void OnActivated() override;
  void OnSimulationStarted() override;
  void OnDeactivated() override;
  void Deinitialize() override;
  void SerializeComponent(ezWorldWriter& stream) const override;
  void DeserializeComponent(ezWorldReader& stream) override;

  // ezAudioSystemComponent

private:
  void ezAudioSystemComponentIsAbstract() override {}

  // ezAudioTriggerComponent

public:
  ezAudioTriggerComponent();
  ~ezAudioTriggerComponent() override;

  /// \brief Sets the collision layer on which rays should hit when calculating
  /// obstruction and occlusion.
  void SetOcclusionCollisionLayer(ezUInt8 uiCollisionLayer);

  /// \brief Gets the current occlusion collision layer.
  [[nodiscard]] ezUInt8 GetOcclusionCollisionLayer() const { return m_uiOcclusionCollisionLayer; }

  /// \brief Sets the name of the play trigger. If the provided name is the same as the
  /// current name, nothing will happen.
  ///
  /// When setting a new name, the current event will be stopped if playing, but the new event will
  /// not be triggered automatically.
  ///
  /// \param sName The name of the play trigger.
  void SetPlayTrigger(ezString sName);

  /// \brief Gets the name of the current play trigger.
  [[nodiscard]] const ezString& GetPlayTrigger() const;

  /// \brief Sets the name of the stop trigger. If the provided name is the same as the
  /// current name, nothing will happen.
  ///
  /// \param sName The name of the stop trigger.
  void SetStopTrigger(ezString sName);

  /// \brief Gets the name of the current stop trigger.
  [[nodiscard]] const ezString& GetStopTrigger() const;

  /// \brief Gets the internal state of the play trigger.
  /// \returns An ezEnum with the value of the play trigger state.
  [[nodiscard]] const ezEnum<ezAudioSystemTriggerState>& GetState() const;

  /// \brief Returns whether the play trigger is currently being loaded.
  /// \returns True if the play trigger is currently being loaded, false otherwise.
  [[nodiscard]] bool IsLoading() const;

  /// \brief Returns whether the play trigger is ready to be activated.
  /// \returns True if the play trigger is ready to be activated, false otherwise.
  [[nodiscard]] bool IsReady() const;

  /// \brief Returns whether the play trigger is being activated.
  /// \returns True if the play trigger is being activated, false otherwise.
  [[nodiscard]] bool IsStarting() const;

  /// \brief Returns whether the play trigger has been activated and is currently playing.
  /// \returns True if the play trigger is currently playing, false otherwise.
  [[nodiscard]] bool IsPlaying() const;

  /// \brief Returns whether the event is being stopped, either by activating the stop trigger
  /// if defined, or by stopping the event directly.
  /// \returns True if the event is being stopped, false otherwise.
  [[nodiscard]] bool IsStopping() const;

  /// \brief Returns whether the play trigger has been activated and is currently stopped.
  /// \returns True if the play trigger is currently stopped, false otherwise.
  [[nodiscard]] bool IsStopped() const;

  /// \brief Returns whether the play trigger is being unloaded.
  /// \returns True if the play trigger is currently being unloaded, false otherwise.
  [[nodiscard]] bool IsUnloading() const;

  /// \brief Activates the play trigger. If the play trigger was not loaded on initialization, this will
  /// load the play trigger the first time it's called.
  ///
  /// \param bSync Whether the request should be executed synchronously or asynchronously.
  void Play(bool bSync = false);

  /// \brief If a stop trigger is defined, this will activate it. Otherwise, the triggered event will be stopped.
  ///
  /// \param bSync Whether the request should be executed synchronously or asynchronously.
  void Stop(bool bSync = false);

private:
  void LoadPlayTrigger(bool bSync);
  void LoadStopTrigger(bool bSync, bool bDeinit);
  void UnloadPlayTrigger(bool bSync, bool bDeinit = false);
  void UnloadStopTrigger(bool bSync, bool bDeinit = false);

  void UpdateOcclusion();
  void Update();

  void StopInternal(bool bSync = false, bool bDeinit = false);

  ezEnum<ezAudioSystemTriggerState> m_eState;

  ezAudioSystemDataID m_uiPlayEventId = 0;
  ezAudioSystemDataID m_uiStopEventId = 0;

  ezString m_sPlayTrigger;
  ezString m_sStopTrigger;

  ezEnum<ezAudioSystemSoundObstructionType> m_eObstructionType;
  ezUInt8 m_uiOcclusionCollisionLayer;
  ezUInt32 m_uiObstructionOcclusionStateIndex;

  bool m_bLoadOnInit;
  bool m_bPlayOnActivate;

  bool m_bPlayTriggerLoaded{false};
  bool m_bStopTriggerLoaded{false};

  bool m_bHasPlayedOnActivate{false};

  bool m_bCanPlay{false};
};
