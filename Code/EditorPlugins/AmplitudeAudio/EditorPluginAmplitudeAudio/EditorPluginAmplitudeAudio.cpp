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
    ezStandardMenus::MapActions("AudioControlCollectionAssetMenuBar", ezStandardMenuTypes::Default | ezStandardMenuTypes::Edit);
    ezProjectActions::MapActions("AudioControlCollectionAssetMenuBar");
    ezDocumentActions::MapMenuActions("AudioControlCollectionAssetMenuBar");
    ezCommandHistoryActions::MapActions("AudioControlCollectionAssetMenuBar");

    // Tool Bar
    {
      ezActionMapManager::RegisterActionMap("AudioControlCollectionAssetToolBar").IgnoreResult();
      ezDocumentActions::MapToolbarActions("AudioControlCollectionAssetToolBar");
      ezCommandHistoryActions::MapActions("AudioControlCollectionAssetToolBar", "");
      ezAssetActions::MapToolBarActions("AudioControlCollectionAssetToolBar", true);
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
