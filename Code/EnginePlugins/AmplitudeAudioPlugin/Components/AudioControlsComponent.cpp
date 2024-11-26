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

#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AmplitudeAudioPlugin/Components/AudioControlsComponent.h>
#include <AudioSystemPlugin/Core/AudioSystemRequests.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>

constexpr ezTypeVersion kVersion_AudioControlsComponent = 1;

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezAudioControlsComponent, kVersion_AudioControlsComponent, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("ControlsAsset", m_sControlsAsset)->AddAttributes(new ezAssetBrowserAttribute("Audio Control Collection")),
    EZ_MEMBER_PROPERTY("AutoLoad", m_bAutoLoad),

    EZ_MEMBER_PROPERTY_READ_ONLY("Loaded", m_bLoaded)->AddAttributes(new ezHiddenAttribute()),
  }
  EZ_END_PROPERTIES;

  EZ_BEGIN_FUNCTIONS
  {
    EZ_SCRIPT_FUNCTION_PROPERTY(Load),
    EZ_SCRIPT_FUNCTION_PROPERTY(Unload),
  }
  EZ_END_FUNCTIONS;
}
EZ_END_COMPONENT_TYPE;
// clang-format on

void ezAudioControlsComponent::Initialize()
{
  SUPER::Initialize();

  if (m_bAutoLoad)
    Load();
}

void ezAudioControlsComponent::Deinitialize()
{
  if (m_bLoaded)
    Unload();

  SUPER::Deinitialize();
}

void ezAudioControlsComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  s.WriteVersion(kVersion_AudioControlsComponent);

  s << m_sControlsAsset;
  s << m_bAutoLoad;
}

void ezAudioControlsComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);

  auto& s = stream.GetStream();

  s.ReadVersion(kVersion_AudioControlsComponent);

  s >> m_sControlsAsset;
  s >> m_bAutoLoad;
}

ezAudioControlsComponent::ezAudioControlsComponent()
  : ezAmplitudeComponent()
  , m_bAutoLoad(false)
  , m_bLoaded(false)
{
}

ezAudioControlsComponent::~ezAudioControlsComponent() = default;

bool ezAudioControlsComponent::Load()
{
  if (m_sControlsAsset.IsEmpty())
    return false;

  if (m_bLoaded)
    return true;

  m_hControlsResource = ezResourceManager::LoadResource<ezAmplitudeAudioControlCollectionResource>(m_sControlsAsset);
  if (const ezResourceLock resource(m_hControlsResource, ezResourceAcquireMode::BlockTillLoaded); resource.GetAcquireResult() == ezResourceAcquireResult::MissingFallback)
  {
    ezLog::Error("Failed to load audio control collection '{0}'", m_sControlsAsset);
    return false;
  }

  m_bLoaded = true;
  return true;
}

bool ezAudioControlsComponent::Unload()
{
  if (!m_bLoaded || !m_hControlsResource.IsValid())
    return false;

  ezResourceManager::FreeAllUnusedResources();
  m_hControlsResource.Invalidate();
  ezResourceManager::FreeAllUnusedResources();

  m_bLoaded = false;
  return true;
}

#if defined(AM_WINDOWS_VERSION)
#  include <Foundation/Platform/Win/Utils/IncludeWindows.h>
#endif

EZ_STATICLINK_FILE(AmplitudeAudioPlugin, AmplitudeAudioPlugin_Components_AudioControlsComponent);
