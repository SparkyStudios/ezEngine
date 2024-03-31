#include "Foundation/IO/FileSystem/FileWriter.h"
#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioPCH.h>

#include <EditorPluginAmplitudeAudio/Dialogs/AmplitudeAudioSettingsDialog.moc.h>

#include <AmplitudeAudioPlugin/Core/Common.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OpenDdlWriter.h>
#include <Foundation/Types/Types.h>

#include <QFileDialog>
#include <QInputDialog>

ezQtAmplitudeAudioSettingsDialog::ezQtAmplitudeAudioSettingsDialog(QWidget* parent)
  : QDialog(parent)
{
  setupUi(this);

  ezStringBuilder projectPath;
  if (ezFileSystem::ResolvePath(":project/Sounds/Amplitude/" AMPLITUDE_ASSETS_DIR_NAME, &projectPath, nullptr).Failed())
  {
    ezLog::Error("No Amplitude assets directory available. Cannot customize project settings.");
    return;
  }

  Load();

  for (auto it = m_Configs.m_AssetProfiles.GetIterator(); it.IsValid(); ++it)
  {
    ListPlatforms->addItem(it.Key().GetData());
  }

  ezStringBuilder banksPath(projectPath);
  banksPath.AppendPath(kSoundBanksFolder);

  ezFileSystemIterator fsIt;
  for (fsIt.StartSearch(projectPath, ezFileSystemIteratorFlags::ReportFiles); fsIt.IsValid(); fsIt.Next())
  {
    ezStringBuilder fileName = fsIt.GetStats().m_sName;
    if (fileName.HasExtension(".amconfig"))
    {
      ComboConfig->addItem(fileName.GetData());
    }
  }

  for (fsIt.StartSearch(banksPath, ezFileSystemIteratorFlags::ReportFiles); fsIt.IsValid(); fsIt.Next())
  {
    ezStringBuilder fileName = fsIt.GetStats().m_sName;
    if (fileName.HasExtension(".ambank"))
    {
      ComboBank->addItem(fileName.GetData());
    }
  }

  if (!m_Configs.m_AssetProfiles.IsEmpty())
  {
    if (m_Configs.m_AssetProfiles.Contains("Desktop"))
      SetCurrentPlatform("Desktop");
    else
      SetCurrentPlatform(m_Configs.m_AssetProfiles.GetIterator().Key());
  }
  else
  {
    SetCurrentPlatform("");
  }
}

ezResult ezQtAmplitudeAudioSettingsDialog::Save()
{
  ezFileWriter file;
  EZ_SUCCEED_OR_RETURN(file.Open(":project/Sounds/AudioSystemConfig.ddl"));

  ezOpenDdlWriter ddl;
  ddl.SetOutputStream(&file);

  ddl.BeginObject("Middleware", s_szAmplitudeMiddlewareName);
  {
    if (m_Configs.Save(ddl).Failed())
    {
      ezQtUiServices::GetSingleton()->MessageBoxWarning("Failed to save the Audio System configuration file\n'>project/Sounds/AudioSystemConfig.ddl'");
      return EZ_FAILURE;
    }
  }
  ddl.EndObject();

  return EZ_SUCCESS;
}

void ezQtAmplitudeAudioSettingsDialog::Load()
{
  ezFileReader file;
  if (file.Open(":project/Sounds/AudioSystemConfig.ddl").Failed())
    return;

  ezOpenDdlReader ddl;
  if (ddl.ParseDocument(file).Failed())
    return;

  const ezOpenDdlReaderElement* pRoot = ddl.GetRootElement();
  const ezOpenDdlReaderElement* pChild = pRoot->GetFirstChild();

  while (pChild)
  {
    if (pChild->IsCustomType("Middleware") && pChild->HasName() && pChild->GetName().Compare(s_szAmplitudeMiddlewareName) == 0)
    {
      if (m_Configs.Load(*pChild).Failed())
        ezLog::Error("Failed to load configuration for audio middleware: {0}.", pChild->GetName());

      break;
    }

    pChild = pChild->GetSibling();
  }

  m_ConfigsOld = m_Configs;
}

void ezQtAmplitudeAudioSettingsDialog::SetCurrentPlatform(const char* szPlatform)
{
  StoreCurrentPlatform();

  m_sCurrentPlatform = szPlatform;

  {
    const bool enable = !m_sCurrentPlatform.IsEmpty();
    ComboBank->setEnabled(enable);
    ComboConfig->setEnabled(enable);
    ButtonRemove->setEnabled(enable);
  }

  ezQtScopedBlockSignals bs(ListPlatforms);
  QList<QListWidgetItem*> items = ListPlatforms->findItems(szPlatform, Qt::MatchFlag::MatchExactly);

  ListPlatforms->clearSelection();

  if (items.size() > 0)
  {
    items[0]->setSelected(true);
  }

  m_sCurrentPlatform = szPlatform;

  if (m_sCurrentPlatform.IsEmpty())
    return;

  const auto& cfg = m_Configs.m_AssetProfiles[m_sCurrentPlatform];

  ComboBank->setCurrentText(cfg.m_sInitSoundBank.GetData());
  ComboConfig->setCurrentText(cfg.m_sEngineConfigFileName.GetData());
}

void ezQtAmplitudeAudioSettingsDialog::StoreCurrentPlatform()
{
  if (!m_Configs.m_AssetProfiles.Contains(m_sCurrentPlatform))
    return;

  auto& cfg = m_Configs.m_AssetProfiles[m_sCurrentPlatform];

  cfg.m_sInitSoundBank = ComboBank->currentText().toUtf8().data();
  cfg.m_sEngineConfigFileName = ComboConfig->currentText().toUtf8().data();
}

void ezQtAmplitudeAudioSettingsDialog::on_ButtonBox_clicked(QAbstractButton* pButton)
{
  if (pButton == ButtonBox->button(QDialogButtonBox::Ok))
  {
    StoreCurrentPlatform();

    if (m_ConfigsOld.m_AssetProfiles != m_Configs.m_AssetProfiles)
    {
      if (ezQtUiServices::GetSingleton()->MessageBoxQuestion("Save the changes to the Amplitude Audio configuration?\nYou need to reload the project for the changes to take effect.", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
      {
        ezQtEditorApp::GetSingleton()->AddReloadProjectRequiredReason("Amplitude Audio configuration was modified.");

        if (Save().Failed())
          return;
      }
    }

    accept();
    return;
  }

  if (pButton == ButtonBox->button(QDialogButtonBox::Cancel))
  {
    reject();
    return;
  }
}

void ezQtAmplitudeAudioSettingsDialog::on_ListPlatforms_itemSelectionChanged()
{
  if (ListPlatforms->selectedItems().isEmpty())
  {
    SetCurrentPlatform("");
    return;
  }

  ButtonRemove->setEnabled(true);
  int row = ListPlatforms->selectionModel()->selectedIndexes()[0].row();
  SetCurrentPlatform(ListPlatforms->item(row)->text().toUtf8().data());
}

void ezQtAmplitudeAudioSettingsDialog::on_ButtonAdd_clicked()
{
  QString name = QInputDialog::getText(this, "Add Platform", "Platform Name:");

  if (name.isEmpty())
    return;

  const ezString sName = name.toUtf8().data();

  if (!m_Configs.m_AssetProfiles.Contains(sName))
  {
    // add a new item with default values
    m_Configs.m_AssetProfiles[sName];

    ListPlatforms->addItem(sName.GetData());
  }

  SetCurrentPlatform(sName);
}

void ezQtAmplitudeAudioSettingsDialog::on_ButtonRemove_clicked()
{
  if (ListPlatforms->selectedItems().isEmpty())
    return;

  int row = ListPlatforms->selectionModel()->selectedIndexes()[0].row();
  const ezString sPlatform = ListPlatforms->item(row)->text().toUtf8().data();

  m_Configs.m_AssetProfiles.Remove(sPlatform);
  delete ListPlatforms->item(row);
}
