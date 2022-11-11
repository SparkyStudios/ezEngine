#include <EditorPluginAudioSystem/EditorPluginAudioSystemPCH.h>

#include <EditorPluginAudioSystem/Preferences/AudioSystemPreferences.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAudioSystemProjectPreferences, 1, ezRTTIDefaultAllocator<ezAudioSystemProjectPreferences>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Mute", m_bMute),
    EZ_MEMBER_PROPERTY("Gain", m_fGain)->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.0f, 1.0f)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezAudioSystemProjectPreferences::ezAudioSystemProjectPreferences()
  : ezPreferences(Domain::Project, "AudioSystem")
{
  ezEditorEngineProcessConnection::s_Events.AddEventHandler(ezMakeDelegate(&ezAudioSystemProjectPreferences::ProcessEventHandler, this));
}

ezAudioSystemProjectPreferences::~ezAudioSystemProjectPreferences()
{
  ezEditorEngineProcessConnection::s_Events.RemoveEventHandler(ezMakeDelegate(&ezAudioSystemProjectPreferences::ProcessEventHandler, this));
}

void ezAudioSystemProjectPreferences::SetMute(bool bMute)
{
  m_bMute = bMute;
  SyncCVars();
}

void ezAudioSystemProjectPreferences::SetGain(float fGain)
{
  m_fGain = fGain;
  SyncCVars();
}

void ezAudioSystemProjectPreferences::SyncCVars()
{
  TriggerPreferencesChangedEvent();

  {
    ezChangeCVarMsgToEngine msg;
    msg.m_sCVarName = "Audio.MasterGain";
    msg.m_NewValue = m_fGain;

    ezEditorEngineProcessConnection::GetSingleton()->SendMessage(&msg);
  }

  {
    ezChangeCVarMsgToEngine msg;
    msg.m_sCVarName = "Audio.Mute";
    msg.m_NewValue = m_bMute;

    ezEditorEngineProcessConnection::GetSingleton()->SendMessage(&msg);
  }
}

void ezAudioSystemProjectPreferences::ProcessEventHandler(const ezEditorEngineProcessConnection::Event& e)
{
  if (e.m_Type == ezEditorEngineProcessConnection::Event::Type::ProcessRestarted)
  {
    SyncCVars();
  }
}
