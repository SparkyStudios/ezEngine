#pragma once

#include <SharedPluginAudioSystem/SharedPluginAudioSystemDLL.h>

#include <AudioSystemPlugin/Core/AudioSystemData.h>

#include <Foundation/Basics.h>

class EZ_SHAREDPLUGINAUDIOSYSTEM_DLL ezAudioMiddlewareControlsManager
{
public:
  virtual ~ezAudioMiddlewareControlsManager() = default;

  /// \brief Create controls assets needed for the audio middleware.
  /// \return EZ_SUCCESS on success, otherwise EZ_FAILURE.
  virtual ezResult ReloadControls() = 0;

protected:
  virtual ezResult SerializeTriggerControl(ezStreamWriter* pStream, const ezAudioSystemTriggerData* pControlData) = 0;
  virtual ezResult SerializeRtpcControl(ezStreamWriter* pStream, const ezAudioSystemRtpcData* pControlData) = 0;
  virtual ezResult SerializeSwitchStateControl(ezStreamWriter* pStream, const ezAudioSystemSwitchStateData* pControlData) = 0;

  virtual ezResult CreateTriggerControl(const char* szControlName, const ezAudioSystemTriggerData* pControlData);
  virtual ezResult CreateRtpcControl(const char* szControlName, const ezAudioSystemRtpcData* pControlData);
  virtual ezResult CreateSwitchStateControl(const char* szControlName, const ezAudioSystemSwitchStateData* pControlData);
};
