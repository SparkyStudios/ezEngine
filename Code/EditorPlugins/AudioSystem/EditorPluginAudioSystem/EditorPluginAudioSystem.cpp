#include <EditorPluginAudioSystem/EditorPluginAudioSystemPCH.h>

#include <EditorPluginAudioSystem/Actions/AudioSystemActions.h>
#include <EditorPluginAudioSystem/Preferences/AudioSystemPreferences.h>

#include <EditorFramework/Actions/ProjectActions.h>

static void ToolsProjectEventHandler(const ezToolsProjectEvent& e);

void OnLoadPlugin()
{
  ezToolsProject::s_Events.AddEventHandler(ToolsProjectEventHandler);

  // Scene
  {
    // Menu Bar
    {
      ezAudioSystemActions::RegisterActions();
      ezAudioSystemActions::MapMenuActions("EditorPluginScene_DocumentMenuBar");
      ezAudioSystemActions::MapMenuActions("EditorPluginScene_Scene2MenuBar");
      ezAudioSystemActions::MapToolbarActions("EditorPluginScene_DocumentToolBar");
      ezAudioSystemActions::MapToolbarActions("EditorPluginScene_Scene2ToolBar");
    }
  }
}

void OnUnloadPlugin()
{
  ezAudioSystemActions::UnregisterActions();
  ezToolsProject::s_Events.RemoveEventHandler(ToolsProjectEventHandler);
}

static void ToolsProjectEventHandler(const ezToolsProjectEvent& e)
{
  if (e.m_Type == ezToolsProjectEvent::Type::ProjectOpened)
  {
    ezAudioSystemProjectPreferences* pPreferences = ezPreferences::QueryPreferences<ezAudioSystemProjectPreferences>();
    pPreferences->SyncCVars();
  }
}

EZ_PLUGIN_ON_LOADED()
{
  OnLoadPlugin();
}

EZ_PLUGIN_ON_UNLOADED()
{
  OnUnloadPlugin();
}
