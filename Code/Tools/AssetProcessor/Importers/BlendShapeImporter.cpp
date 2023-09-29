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

#include <AssetProcessor/Importers/BlendShapeImporter.h>

#include <Core/Assets/AssetFileHeader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>

using namespace RAI;

ezResult spBlendShapeImporter::Import(ezStringView sFilePath, ezStringView sOutputPath)
{
  if (m_pContext == nullptr)
  {
    ezLog::Error("Invalid Assimp Context.");
    return EZ_FAILURE;
  }

  if (m_pContext->m_BlendShapes.IsEmpty())
    return EZ_SUCCESS;

  spBlendShapeResourceDescriptor blendShape;

  const ezUInt32 uiMeshCount = m_pContext->m_Meshes.GetCount();

  for (ezUInt32 uiMeshIndex = 0; uiMeshIndex < uiMeshCount; ++uiMeshIndex)
  {
    for (const auto& blendShapeEntry : m_pContext->m_BlendShapes)
    {
      if (blendShapeEntry.m_uiParentMeshIndex != uiMeshIndex)
        continue;

      spBlendShape entry;
      entry.m_fWeight = blendShapeEntry.m_fWeight;
      entry.m_Vertices = blendShapeEntry.m_BlendShapeData;

      blendShape.AddBlendShape(m_pContext->m_Meshes[uiMeshIndex].m_sName, entry);
    }
  }

  ezStringBuilder sOutputFile(sFilePath.GetFileDirectory());
  sOutputFile.AppendFormat("/{}.spBlendShape", sFilePath.GetFileName());

  ezFileWriter file;
  if (file.Open(sOutputFile, 1024 * 1024).Failed())
  {
    ezLog::Error("Failed to save blend shape asset: '{0}'", sOutputFile);
    return EZ_FAILURE;
  }

  // Write asset header
  ezAssetFileHeader assetHeader;
  assetHeader.SetGenerator("SparkEngine Asset Processor");
  assetHeader.SetFileHashAndVersion(ezHashingUtils::xxHash64String(sFilePath), 1);
  EZ_SUCCEED_OR_RETURN(assetHeader.Write(file));

  EZ_SUCCEED_OR_RETURN(blendShape.Save(file));

  return EZ_SUCCESS;
}

spBlendShapeImporter::spBlendShapeImporter(const spAssimpImporterConfiguration& configuration)
  : spAssimpImporter(configuration)
{
}
