#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioPCH.h>

#include <EditorPluginAmplitudeAudio/Actions/AmplitudeAudioActions.h>
#include <EditorPluginAmplitudeAudio/Core/AmplitudeAudioControlsManager.h>
#include <EditorPluginAmplitudeAudio/Dialogs/AmplitudeAudioSettingsDialog.moc.h>

#include <GuiFoundation/Action/ActionManager.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioAction, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

ezActionDescriptorHandle ezAmplitudeAudioActions::s_hCategoryAudioSystem;
ezActionDescriptorHandle ezAmplitudeAudioActions::s_hProjectSettings;
ezActionDescriptorHandle ezAmplitudeAudioActions::s_hReloadControls;

void ezAmplitudeAudioActions::RegisterActions()
{
  s_hCategoryAudioSystem = EZ_REGISTER_CATEGORY("Amplitude");

  s_hProjectSettings = EZ_REGISTER_ACTION_1(
    "AmplitudeAudio.Settings.Project",
    ezActionScope::Document,
    "Amplitude",
    "",
    ezAmplitudeAudioAction,
    ezAmplitudeAudioAction::ActionType::ProjectSettings);

  s_hReloadControls = EZ_REGISTER_ACTION_1(
    "AmplitudeAudio.ReloadControls",
    ezActionScope::Document,
    "Amplitude",
    "",
    ezAmplitudeAudioAction,
    ezAmplitudeAudioAction::ActionType::ReloadControls);
}

void ezAmplitudeAudioActions::UnregisterActions()
{
  ezActionManager::UnregisterAction(s_hCategoryAudioSystem);
  ezActionManager::UnregisterAction(s_hProjectSettings);
  ezActionManager::UnregisterAction(s_hReloadControls);
}

void ezAmplitudeAudioActions::MapMenuActions(const char* szMapping)
{
  ezActionMap* pMap = ezActionMapManager::GetActionMap(szMapping);
  EZ_ASSERT_DEV(pMap != nullptr, "Mapping the actions failed!");

  pMap->MapAction(s_hCategoryAudioSystem, "G.Plugins.Settings", "AudioSystem", 9.0f);
  pMap->MapAction(s_hProjectSettings, "G.Plugins.Settings", "AudioSystem/Amplitude", 0.0f);

  pMap->MapAction(s_hCategoryAudioSystem, "G.Scene", "AudioSystem", 5.0f);
  pMap->MapAction(s_hReloadControls, "G.Scene", "AudioSystem/Amplitude", 1.0f);
}

void ezAmplitudeAudioActions::MapToolbarActions(const char* szMapping)
{
  ezActionMap* pSceneMap = ezActionMapManager::GetActionMap(szMapping);
  EZ_ASSERT_DEV(pSceneMap != nullptr, "Mapping the actions failed!");

  pSceneMap->MapAction(s_hCategoryAudioSystem, "", 12.0f);
  pSceneMap->MapAction(s_hReloadControls, "Amplitude", 0.0f);
}

ezAmplitudeAudioAction::ezAmplitudeAudioAction(const ezActionContext& context, const char* szName, ActionType type)
  : ezButtonAction(context, szName, false, "")
{
  m_Type = type;

  switch (m_Type)
  {
    case ActionType::ProjectSettings:
      SetIconPath(":/AssetIcons/Audio_Control_Collection.png");
      break;

    case ActionType::ReloadControls:
      SetCheckable(false);
      break;
  }
}

ezAmplitudeAudioAction::~ezAmplitudeAudioAction()
{
}

void ezAmplitudeAudioAction::Execute(const ezVariant& value)
{
  if (m_Type == ActionType::ProjectSettings)
  {
    ezQtAmplitudeAudioSettingsDialog dlg(nullptr);
    dlg.exec();
  }

  if (m_Type == ActionType::ReloadControls)
  {
    auto* pControlsManager = ezAmplitudeAudioControlsManager::GetSingleton();
    ezLog::Info("Reload Amplitude Audio Controls {}", pControlsManager->ReloadControls().Succeeded() ? "successful" : "failed");
  }
}
