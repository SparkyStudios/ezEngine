#pragma once

#include <AssetProcessor/Importers/AssimpImporter.h>

#include <RAI/Resources/SkeletonResource.h>

/// \brief spAssimpImporter implementation for skeletons.
class spSkeletonImporter : public spAssimpImporter
{
  // spAssimpImporter

public:
  ezResult Import(ezStringView sFilePath, ezStringView sOutputPath) override;

  // spSkeletonImporter

public:
  explicit spSkeletonImporter(const spAssimpImporterConfiguration& configuration);

private:
  ezResult ImportSkeleton(const aiScene* pScene, RAI::spSkeletonResourceDescriptor* out_pSkeleton);
  const aiNode* FindRootBone(const aiNode* pNode, const aiNode* pRootNode) const;
  void ImportSkeletonNode(const aiNode* pNode, ezDynamicArray<RAI::spSkeleton::Joint>& out_joints, ezUInt16 uiJointIndex, ezUInt16 uiParentJointIndex);
};
