#pragma once

#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioDLL.h>

#include <Foundation/Configuration/CVar.h>

#include <GuiFoundation/Action/BaseActions.h>
#include <GuiFoundation/GuiFoundationDLL.h>

class ezPreferences;

class EZ_EDITORPLUGINAMPLITUDEAUDIO_DLL ezAmplitudeAudioActions
{
public:
  static void RegisterActions();
  static void UnregisterActions();

  static void MapMenuActions(const char* szMapping);
  static void MapToolbarActions(const char* szMapping);

  static ezActionDescriptorHandle s_hCategoryAudioSystem;
  static ezActionDescriptorHandle s_hProjectSettings;
  static ezActionDescriptorHandle s_hReloadControls;
};


class EZ_EDITORPLUGINAMPLITUDEAUDIO_DLL ezAmplitudeAudioAction : public ezButtonAction
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioAction, ezButtonAction);

public:
  enum class ActionType
  {
    ProjectSettings,
    ReloadControls
  };

  ezAmplitudeAudioAction(const ezActionContext& context, const char* szName, ActionType type);
  ~ezAmplitudeAudioAction() override;

  virtual void Execute(const ezVariant& value) override;

private:
  void OnPreferenceChange(ezPreferences* pref);

  ActionType m_Type;
};
