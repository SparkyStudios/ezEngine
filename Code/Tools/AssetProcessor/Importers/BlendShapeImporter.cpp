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

  const auto& blendShapes = m_pContext->m_BlendShapes;

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
