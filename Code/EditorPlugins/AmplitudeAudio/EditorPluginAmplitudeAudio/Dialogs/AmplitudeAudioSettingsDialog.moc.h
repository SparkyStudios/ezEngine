// Copyright (c) 2022-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
  void on_ButtonBrowseProject_clicked();

private:
  ezResult Save() const;
  void Load();
  void SetCurrentPlatform(const char* szPlatform);
  void StoreCurrentPlatform();
  void PopulatePlatformsList();

  ezString m_sCurrentPlatform;
  ezAmplitudeAssetProfiles m_ConfigsOld;
  ezAmplitudeAssetProfiles m_Configs;

  bool m_bDirty = false;
};
