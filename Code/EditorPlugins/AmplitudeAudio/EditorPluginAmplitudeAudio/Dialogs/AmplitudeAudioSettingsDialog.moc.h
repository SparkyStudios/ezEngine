#pragma once

#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioDLL.h>
#include <EditorPluginAmplitudeAudio/ui_AmplitudeAudioSettingsDialog.h>

#include <AmplitudeAudioPlugin/AmplitudeAudioSingleton.h>

#include <QDialog>

class ezQtAmplitudeAudioSettingsDialog : public QDialog, public Ui_AmplitudeAudioSettingsDialog
{
public:
  Q_OBJECT

public:
  ezQtAmplitudeAudioSettingsDialog(QWidget* parent);

private Q_SLOTS:
  void on_ButtonBox_clicked(QAbstractButton* pButton);
  void on_ListPlatforms_itemSelectionChanged();
  void on_ButtonAdd_clicked();
  void on_ButtonRemove_clicked();

private:
  ezResult Save();
  void Load();
  void SetCurrentPlatform(const char* szPlatform);
  void StoreCurrentPlatform();

  ezString m_sCurrentPlatform;
  ezAmplitudeAssetProfiles m_ConfigsOld;
  ezAmplitudeAssetProfiles m_Configs;
};
