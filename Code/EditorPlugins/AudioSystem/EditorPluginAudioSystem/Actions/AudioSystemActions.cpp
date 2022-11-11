#include <EditorPluginAudioSystem/EditorPluginAudioSystemPCH.h>

#include <EditorPluginAudioSystem/Actions/AudioSystemActions.h>
//#include <EditorPluginAudioSystem/Dialogs/AudioSystemProjectSettingsDlg.moc.h>
#include <EditorPluginAudioSystem/Preferences/AudioSystemPreferences.h>

#include <GuiFoundation/Action/ActionManager.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAudioSystemAction, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAudioSystemSliderAction, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

ezActionDescriptorHandle ezAudioSystemActions::s_hCategoryAudioSystem;
ezActionDescriptorHandle ezAudioSystemActions::s_hMuteSound;
ezActionDescriptorHandle ezAudioSystemActions::s_hMasterVolume;

void ezAudioSystemActions::RegisterActions()
{
  s_hCategoryAudioSystem = EZ_REGISTER_CATEGORY("AudioSystem");

  s_hMuteSound = EZ_REGISTER_ACTION_1(
    "AudioSystem.Mute",
    ezActionScope::Document,
    "AudioSystem",
    "",
    ezAudioSystemAction,
    ezAudioSystemAction::ActionType::MuteSound
  );

  s_hMasterVolume = EZ_REGISTER_ACTION_1(
    "AudioSystem.MasterVolume",
    ezActionScope::Document,
    "Volume",
    "",
    ezAudioSystemSliderAction,
    ezAudioSystemSliderAction::ActionType::MasterVolume
  );
}

void ezAudioSystemActions::UnregisterActions()
{
  ezActionManager::UnregisterAction(s_hCategoryAudioSystem);
  ezActionManager::UnregisterAction(s_hMuteSound);
  ezActionManager::UnregisterAction(s_hMasterVolume);
}

void ezAudioSystemActions::MapMenuActions(const char* szMapping)
{
  ezActionMap* pMap = ezActionMapManager::GetActionMap(szMapping);
  EZ_ASSERT_DEV(pMap != nullptr, "Mapping the actions failed!");

  pMap->MapAction(s_hCategoryAudioSystem, "Menu.Editor/ProjectCategory/Menu.ProjectSettings", 9.0f);

  pMap->MapAction(s_hCategoryAudioSystem, "Menu.Scene", 5.0f);
  pMap->MapAction(s_hMuteSound, "Menu.Scene/AudioSystem", 0.0f);
  pMap->MapAction(s_hMasterVolume, "Menu.Scene/AudioSystem", 2.0f);
}

void ezAudioSystemActions::MapToolbarActions(const char* szMapping)
{
  ezActionMap* pSceneMap = ezActionMapManager::GetActionMap(szMapping);
  EZ_ASSERT_DEV(pSceneMap != nullptr, "Mapping the actions failed!");

  pSceneMap->MapAction(s_hCategoryAudioSystem, "", 12.0f);
  pSceneMap->MapAction(s_hMuteSound, "AudioSystem", 0.0f);
}

ezAudioSystemAction::ezAudioSystemAction(const ezActionContext& context, const char* szName, ActionType type)
  : ezButtonAction(context, szName, false, "")
{
  m_Type = type;

  switch (m_Type)
  {
    case ActionType::MuteSound:
    {
      SetCheckable(true);

      const auto* pPreferences = ezPreferences::QueryPreferences<ezAudioSystemProjectPreferences>();
      pPreferences->m_ChangedEvent.AddEventHandler(ezMakeDelegate(&ezAudioSystemAction::OnPreferenceChange, this));

      if (pPreferences->GetMute())
        SetIconPath(":/Icons/SoundOff16.png");
      else
        SetIconPath(":/Icons/SoundOn16.png");

      SetChecked(pPreferences->GetMute());
    }
    break;
  }
}

ezAudioSystemAction::~ezAudioSystemAction()
{
  if (m_Type == ActionType::MuteSound)
  {
    const auto* pPreferences = ezPreferences::QueryPreferences<ezAudioSystemProjectPreferences>();
    pPreferences->m_ChangedEvent.RemoveEventHandler(ezMakeDelegate(&ezAudioSystemAction::OnPreferenceChange, this));
  }
}

void ezAudioSystemAction::Execute(const ezVariant& value)
{
  if (m_Type == ActionType::MuteSound)
  {
    auto* pPreferences = ezPreferences::QueryPreferences<ezAudioSystemProjectPreferences>();
    pPreferences->SetMute(!pPreferences->GetMute());

    if (GetContext().m_pDocument)
    {
      GetContext().m_pDocument->ShowDocumentStatus(ezFmt("Sound is {}", pPreferences->GetMute() ? "muted" : "on"));
    }
  }
}

void ezAudioSystemAction::OnPreferenceChange(ezPreferences* pref)
{
  const auto* pPreferences = ezPreferences::QueryPreferences<ezAudioSystemProjectPreferences>();

  if (m_Type == ActionType::MuteSound)
  {
    if (pPreferences->GetMute())
      SetIconPath(":/Icons/SoundOff16.png");
    else
      SetIconPath(":/Icons/SoundOn16.png");

    SetChecked(pPreferences->GetMute());
  }
}

//////////////////////////////////////////////////////////////////////////

ezAudioSystemSliderAction::ezAudioSystemSliderAction(const ezActionContext& context, const char* szName, ActionType type)
  : ezSliderAction(context, szName)
{
  m_Type = type;

  switch (m_Type)
  {
    case ActionType::MasterVolume:
    {
      const auto* pPreferences = ezPreferences::QueryPreferences<ezAudioSystemProjectPreferences>();
      pPreferences->m_ChangedEvent.AddEventHandler(ezMakeDelegate(&ezAudioSystemSliderAction::OnPreferenceChange, this));
      SetRange(0, 100);
    }
    break;
  }

  UpdateState();
}

ezAudioSystemSliderAction::~ezAudioSystemSliderAction()
{
  switch (m_Type)
  {
    case ActionType::MasterVolume:
    {
      const auto* pPreferences = ezPreferences::QueryPreferences<ezAudioSystemProjectPreferences>();
      pPreferences->m_ChangedEvent.RemoveEventHandler(ezMakeDelegate(&ezAudioSystemSliderAction::OnPreferenceChange, this));
    }
    break;
  }
}

void ezAudioSystemSliderAction::Execute(const ezVariant& value)
{
  const ezInt32 iValue = value.Get<ezInt32>();

  switch (m_Type)
  {
    case ActionType::MasterVolume:
    {
      auto* pPreferences = ezPreferences::QueryPreferences<ezAudioSystemProjectPreferences>();

      pPreferences->SetGain(iValue / 100.0f);

      if (GetContext().m_pDocument)
      {
        GetContext().m_pDocument->ShowDocumentStatus(ezFmt("Sound Volume: {}%%", static_cast<ezInt32>(pPreferences->GetGain() * 100.0f)));
      }
    }
    break;
  }
}

void ezAudioSystemSliderAction::OnPreferenceChange(ezPreferences* pref)
{
  UpdateState();
}

void ezAudioSystemSliderAction::UpdateState()
{
  switch (m_Type)
  {
    case ActionType::MasterVolume:
    {
      const auto* pPreferences = ezPreferences::QueryPreferences<ezAudioSystemProjectPreferences>();
      SetValue(ezMath::Clamp(static_cast<ezInt32>(pPreferences->GetGain() * 100.0f), 0, 100));
    }
    break;
  }
}
