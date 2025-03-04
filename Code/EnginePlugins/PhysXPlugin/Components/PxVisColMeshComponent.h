#pragma once

#include <PhysXPlugin/PhysXPluginDLL.h>
#include <PhysXPlugin/Resources/PxMeshResource.h>
#include <RendererCore/Components/RenderComponent.h>
#include <RendererCore/Meshes/MeshComponent.h>
#include <RendererCore/Meshes/MeshResource.h>
#include <RendererCore/Pipeline/RenderData.h>

class ezPxVisColMeshComponentManager : public ezComponentManager<class ezPxVisColMeshComponent, ezBlockStorageType::Compact>
{
public:
  using SUPER = ezComponentManager<ezPxVisColMeshComponent, ezBlockStorageType::Compact>;

  ezPxVisColMeshComponentManager(ezWorld* pWorld)
    : SUPER(pWorld)
  {
  }

  void Update(const ezWorldModule::UpdateContext& context);
  void EnqueueUpdate(ezComponentHandle hComponent);

private:
  void ResourceEventHandler(const ezResourceEvent& e);

  mutable ezMutex m_Mutex;
  ezDeque<ezComponentHandle> m_RequireUpdate;

protected:
  virtual void Initialize() override;
  virtual void Deinitialize() override;
};

class EZ_PHYSXPLUGIN_DLL ezPxVisColMeshComponent : public ezRenderComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezPxVisColMeshComponent, ezRenderComponent, ezPxVisColMeshComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(ezWorldReader& inout_stream) override;

protected:
  virtual void Initialize() override;


  //////////////////////////////////////////////////////////////////////////
  // ezRenderComponent

public:
  virtual ezResult GetLocalBounds(ezBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, ezMsgUpdateLocalBounds& ref_msg) override;


  //////////////////////////////////////////////////////////////////////////
  // ezPxVisColMeshComponent

public:
  ezPxVisColMeshComponent();
  ~ezPxVisColMeshComponent();

  void SetMeshFile(const char* szFile); // [ property ]
  const char* GetMeshFile() const;      // [ property ]

  void SetMesh(const ezPxMeshResourceHandle& hMesh);
  EZ_ALWAYS_INLINE const ezPxMeshResourceHandle& GetMesh() const { return m_hCollisionMesh; }

protected:
  void OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const;
  void CreateCollisionRenderMesh();

  ezPxMeshResourceHandle m_hCollisionMesh;
  mutable ezMeshResourceHandle m_hMesh;
};
