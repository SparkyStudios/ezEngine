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

#include <AssetProcessor/Importers/SkeletonImporter.h>

#include <Core/Assets/AssetFileHeader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>

#include <assimp/scene.h>

using namespace RAI;

ezResult spSkeletonImporter::Import(ezStringView sFilePath, ezStringView sOutputPath)
{
  if (m_pContext == nullptr)
  {
    ezLog::Error("Invalid Assimp Context.");
    return EZ_FAILURE;
  }

  spSkeletonResourceDescriptor skeleton;

  EZ_SUCCEED_OR_RETURN(ImportSkeleton(m_pContext->m_pScene, &skeleton));

  ezStringBuilder sOutputFile(sOutputPath);
  sOutputFile.AppendFormat("/{}.spSkeleton", sFilePath.GetFileName());

  ezFileWriter file;
  if (file.Open(sOutputFile, 1024 * 1024).Failed())
  {
    ezLog::Error("Failed to save skeleton asset: '{0}'", sOutputFile);
    return EZ_FAILURE;
  }

  // Write asset header
  ezAssetFileHeader assetHeader;
  assetHeader.SetGenerator("SparkEngine Asset Processor");
  assetHeader.SetFileHashAndVersion(ezHashingUtils::xxHash64String(sFilePath), 1);
  EZ_SUCCEED_OR_RETURN(assetHeader.Write(file));

  EZ_SUCCEED_OR_RETURN(skeleton.Save(file));

  return EZ_SUCCESS;
}

spSkeletonImporter::spSkeletonImporter(const spAssimpImporterConfiguration& configuration)
  : spAssimpImporter(configuration)
{
}

ezResult spSkeletonImporter::ImportSkeleton(const aiScene* pScene, spSkeletonResourceDescriptor* out_pSkeleton)
{
  if (pScene->mNumMeshes == 0)
    return EZ_FAILURE;

  const aiNode* pRootNode = nullptr;

  for (ezUInt32 meshIndex = 0; meshIndex < pScene->mNumMeshes; ++meshIndex)
  {
    const aiMesh* pMesh = pScene->mMeshes[meshIndex];
    if (pMesh->mNumBones == 0)
      continue;

    const aiBone* pBone = pMesh->mBones[0];
    pRootNode = FindRootBone(pBone->mArmature, pScene->mRootNode);
    break;
  }

  if (pRootNode == nullptr)
  {
    ezLog::Error("Unable to find the root joint of the skeleton.");
    return EZ_FAILURE;
  }

  spSkeleton skeleton;
  ImportSkeletonNode(pRootNode, skeleton.GetJoints(), 0, spSkeletonInvalidJointIndex);

  out_pSkeleton->SetSkeleton(skeleton);

  return EZ_SUCCESS;
}

const aiNode* spSkeletonImporter::FindRootBone(const aiNode* pNode, const aiNode* pRootNode) const
{
  if (pNode->mParent->mName == pRootNode->mName)
    return pNode;

  return FindRootBone(pNode->mParent, pRootNode);
}

void spSkeletonImporter::ImportSkeletonNode(const aiNode* pNode, ezDynamicArray<spSkeleton::Joint>& out_joints, ezUInt16 uiJointIndex, ezUInt16 uiParentJointIndex)
{
  const ezMat4& transform = spFromAssimp(pNode->mTransformation);

  spSkeleton::Joint joint;
  joint.m_uiIndex = uiJointIndex;
  joint.m_sName.Assign(pNode->mName.C_Str());
  joint.m_Transform = ezTransform::MakeFromMat4(transform);
  joint.m_uiParentIndex = uiParentJointIndex;

  out_joints.PushBack(joint);

  for (ezUInt32 i = 0; i < pNode->mNumChildren; ++i)
    ImportSkeletonNode(pNode->mChildren[i], out_joints, static_cast<ezUInt16>(out_joints.GetCount() - 1), joint.m_uiIndex);
}
