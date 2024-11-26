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

  void Execute(const ezVariant& value) override;

  ActionType m_Type;
};
