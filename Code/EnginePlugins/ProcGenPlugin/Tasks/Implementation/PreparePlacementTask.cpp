#include <ProcGenPlugin/ProcGenPluginPCH.h>

#include <Core/Interfaces/PhysicsWorldModule.h>
#include <ProcGenPlugin/Tasks/PlacementData.h>
#include <ProcGenPlugin/Tasks/PreparePlacementTask.h>
#include <ProcGenPlugin/Tasks/Utils.h>

using namespace ezProcGenInternal;

PreparePlacementTask::PreparePlacementTask(PlacementData* pData, const char* szName)
  : m_pData(pData)
{
  ConfigureTask(szName, ezTaskNesting::Maybe);
}

PreparePlacementTask::~PreparePlacementTask() = default;

void PreparePlacementTask::Execute()
{
  const ezWorld& world = *m_pData->m_pWorld;
  const ezBoundingBox& box = m_pData->m_TileBoundingBox;
  const Output& output = *m_pData->m_pOutput;

  ezProcGenInternal::ExtractVolumeCollections(world, box, output, m_pData->m_VolumeCollections, m_pData->m_GlobalData);
  ezProcGenInternal::SetInstanceSeed(m_pData->m_uiTileSeed, m_pData->m_GlobalData);
}
