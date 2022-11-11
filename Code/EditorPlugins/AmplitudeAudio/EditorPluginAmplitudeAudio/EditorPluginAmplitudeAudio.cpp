#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioPCH.h>

#include <EditorPluginAmplitudeAudio/Actions/AmplitudeAudioActions.h>

#include <EditorFramework/Actions/AssetActions.h>
#include <EditorFramework/Actions/ProjectActions.h>

#include <GuiFoundation/Action/CommandHistoryActions.h>
#include <GuiFoundation/Action/DocumentActions.h>
#include <GuiFoundation/Action/StandardMenus.h>

// BEGIN-DOCS-CODE-SNIPPET: plugin-setup
EZ_PLUGIN_ON_LOADED()
{
  ezAmplitudeAudioActions::RegisterActions();

  // Control Collection
  {
    // Menu Bar
    ezActionMapManager::RegisterActionMap("AudioControlCollectionAssetMenuBar").IgnoreResult();
    ezStandardMenus::MapActions("AudioControlCollectionAssetMenuBar", ezStandardMenuTypes::File | ezStandardMenuTypes::Edit | ezStandardMenuTypes::Panels | ezStandardMenuTypes::Help);
    ezProjectActions::MapActions("AudioControlCollectionAssetMenuBar");
    ezDocumentActions::MapActions("AudioControlCollectionAssetMenuBar", "Menu.File", false);
    ezCommandHistoryActions::MapActions("AudioControlCollectionAssetMenuBar", "Menu.Edit");

    // Tool Bar
    {
      ezActionMapManager::RegisterActionMap("AudioControlCollectionAssetToolBar").IgnoreResult();
      ezDocumentActions::MapActions("AudioControlCollectionAssetToolBar", "", true);
      ezCommandHistoryActions::MapActions("AudioControlCollectionAssetToolBar", "");
      ezAssetActions::MapActions("AudioControlCollectionAssetToolBar", true);
      ezAmplitudeAudioActions::MapToolbarActions("AudioControlCollectionAssetToolBar");
    }
  }

  // Scene
  {
    // Menu Bar
    {
      ezAmplitudeAudioActions::MapMenuActions("EditorPluginScene_DocumentMenuBar");
      ezAmplitudeAudioActions::MapMenuActions("EditorPluginScene_Scene2MenuBar");
      ezAmplitudeAudioActions::MapToolbarActions("EditorPluginScene_DocumentToolBar");
      ezAmplitudeAudioActions::MapToolbarActions("EditorPluginScene_Scene2ToolBar");
    }
  }
}

EZ_PLUGIN_ON_UNLOADED()
{
  ezAmplitudeAudioActions::UnregisterActions();
}
// END-DOCS-CODE-SNIPPET
