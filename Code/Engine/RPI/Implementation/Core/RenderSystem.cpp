// Copyright (c) 2023-present Sparky Studios. All rights reserved.
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

#include <RPI/RPIPCH.h>

#include <Core/World/World.h>

#include <Foundation/Configuration/CVar.h>

#include <RPI/Core/RenderSystem.h>
#include <RPI/Features/RenderFeature.h>

ezCVarBool cvar_RenderingMultiThreaded("Rendering.MultiThreaded", true, ezCVarFlags::Default, "Enables multi-threaded data update and rendering.");

namespace RPI
{
  EZ_IMPLEMENT_SINGLETON(spRenderSystem);

  spRenderNodeReference spRenderNodeReference::MakeInvalid()
  {
    return spRenderNodeReference(-1);
  }

  spRenderNodeReference::spRenderNodeReference(ezInt32 iReference)
    : m_iRef(iReference)
  {
  }

  ezUInt64 spRenderSystem::s_uiFrameCount = 0;

  bool spRenderSystem::IsMultiThreadedRendering()
  {
    return cvar_RenderingMultiThreaded;
  }

  spRenderSystem::spRenderSystem()
    : m_SingletonRegistrar(this)
  {
  }

  void spRenderSystem::Startup(ezUniquePtr<RHI::spDevice>&& pDevice)
  {
    if (m_bInitialized)
      return;

    m_pDevice = std::move(pDevice);

    m_pRenderThread = EZ_DEFAULT_NEW(spRenderThread);
    m_pRenderThread->Start();

    // TODO: Load the compositor from the resource instead
    m_pCompositor = EZ_NEW(m_pDevice->GetAllocator(), spCompositor);

    m_bInitialized = true;
  }

  void spRenderSystem::Shutdown()
  {
    if (!m_bInitialized)
      return;

    m_bInitialized = false;

    m_pCompositor.Clear();
    m_pCompositor = nullptr;

    m_pRenderThread->Stop();
    m_pRenderThread = nullptr;

    m_pDevice->Destroy();
    m_pDevice = nullptr;
  }

  ezSharedPtr<spCompositor> spRenderSystem::GetCompositor() const
  {
    return m_pCompositor;
  }

  spSceneContext* spRenderSystem::GetSceneContextFromWorld(const ezWorld* pWorld) const
  {
    const auto uiIndex = m_RegisteredWorldScenes.Find(pWorld);
    if (uiIndex == ezInvalidIndex)
      return nullptr;

    return m_RegisteredWorldScenes.GetValue(uiIndex);
  }

  spSceneContext* spRenderSystem::CreateSceneForWorld(ezWorld* pWorld)
  {
    m_RegisteredWorldScenes[pWorld] = EZ_NEW(RHI::spDeviceAllocatorWrapper::GetAllocator(), spSceneContext, m_pDevice.Borrow());
    m_RegisteredWorldScenes[pWorld]->SetWorld(pWorld);

    return m_RegisteredWorldScenes[pWorld];
  }

  void spRenderSystem::RegisterSceneForWorld(const ezWorld* pWorld, spSceneContext* pSceneContext)
  {
    m_RegisteredWorldScenes[pWorld] = pSceneContext;
  }

  void spRenderSystem::UnregisterSceneForWorld(const ezWorld* pWorld)
  {
    m_RegisteredWorldScenes.RemoveAndCopy(pWorld);
  }

  spCameraSlotHandle spRenderSystem::CreateCameraSlot(ezStringView sName)
  {
    spCameraSlot* pSlot = EZ_DEFAULT_NEW(spCameraSlot, sName);
    spCameraSlotHandleId idSlot = m_CameraSlots.Insert(std::move(pSlot));
    pSlot->m_Handle.m_InternalId = idSlot;

    m_CameraSlotsByName.Insert(sName, idSlot);

    return pSlot->m_Handle;
  }

  spCameraSlotHandle spRenderSystem::GetCameraSlotByName(ezStringView sName) const
  {
    ezUInt32 uiIndex = m_CameraSlotsByName.Find(sName);
    if (uiIndex == ezInvalidIndex)
      return spCameraSlotHandle();

    return spCameraSlotHandle(m_CameraSlotsByName.GetValue(uiIndex));
  }

} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Core_RenderSystem);
