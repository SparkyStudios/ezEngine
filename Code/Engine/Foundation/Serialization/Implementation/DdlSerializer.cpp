#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlUtils.h>
#include <Foundation/IO/OpenDdlWriter.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Serialization/DdlSerializer.h>
#include <Foundation/Serialization/GraphVersioning.h>

namespace
{
  ezSerializedBlock* FindBlock(ezHybridArray<ezSerializedBlock, 3>& ref_blocks, ezStringView sName)
  {
    for (auto& block : ref_blocks)
    {
      if (block.m_Name == sName)
      {
        return &block;
      }
    }
    return nullptr;
  }

  ezSerializedBlock* FindHeaderBlock(ezHybridArray<ezSerializedBlock, 3>& ref_blocks, ezInt32& out_iVersion)
  {
    ezStringBuilder sHeaderName = "HeaderV";
    out_iVersion = 0;
    for (auto& block : ref_blocks)
    {
      if (block.m_Name.StartsWith(sHeaderName))
      {
        ezResult res = ezConversionUtils::StringToInt(block.m_Name.GetData() + sHeaderName.GetElementCount(), out_iVersion);
        if (res.Failed())
        {
          ezLog::Error("Failed to parse version from header name '{0}'", block.m_Name);
        }
        return &block;
      }
    }
    return nullptr;
  }

  ezSerializedBlock* GetOrCreateBlock(ezHybridArray<ezSerializedBlock, 3>& ref_blocks, ezStringView sName)
  {
    ezSerializedBlock* pBlock = FindBlock(ref_blocks, sName);
    if (!pBlock)
    {
      pBlock = &ref_blocks.ExpandAndGetRef();
      pBlock->m_Name = sName;
    }
    if (!pBlock->m_Graph)
    {
      pBlock->m_Graph = EZ_DEFAULT_NEW(ezAbstractObjectGraph);
    }
    return pBlock;
  }
} // namespace

static void WriteGraph(ezOpenDdlWriter& ref_writer, const ezAbstractObjectGraph* pGraph, const char* szName)
{
  ezMap<ezStringView, const ezVariant*> SortedProperties;

  ref_writer.BeginObject(szName);

  const auto& Nodes = pGraph->GetAllNodes();
  for (auto itNode = Nodes.GetIterator(); itNode.IsValid(); ++itNode)
  {
    const auto& node = *itNode.Value();

    ref_writer.BeginObject("o");

    {

      ezOpenDdlUtils::StoreUuid(ref_writer, node.GetGuid(), "id");
      ezOpenDdlUtils::StoreString(ref_writer, node.GetType(), "t");
      ezOpenDdlUtils::StoreUInt32(ref_writer, node.GetTypeVersion(), "v");

      if (!node.GetNodeName().IsEmpty())
        ezOpenDdlUtils::StoreString(ref_writer, node.GetNodeName(), "n");

      ref_writer.BeginObject("p");
      {
        for (const auto& prop : node.GetProperties())
          SortedProperties[prop.m_sPropertyName] = &prop.m_Value;

        for (auto it = SortedProperties.GetIterator(); it.IsValid(); ++it)
        {
          ezOpenDdlUtils::StoreVariant(ref_writer, *it.Value(), it.Key());
        }

        SortedProperties.Clear();
      }
      ref_writer.EndObject();
    }
    ref_writer.EndObject();
  }

  ref_writer.EndObject();
}

void ezAbstractGraphDdlSerializer::Write(ezStreamWriter& inout_stream, const ezAbstractObjectGraph* pGraph, const ezAbstractObjectGraph* pTypesGraph,
  bool bCompactMmode, ezOpenDdlWriter::TypeStringMode typeMode)
{
  ezOpenDdlWriter writer;
  writer.SetOutputStream(&inout_stream);
  writer.SetCompactMode(bCompactMmode);
  writer.SetFloatPrecisionMode(ezOpenDdlWriter::FloatPrecisionMode::Exact);
  writer.SetPrimitiveTypeStringMode(typeMode);

  if (typeMode != ezOpenDdlWriter::TypeStringMode::Compliant)
    writer.SetIndentation(-1);

  Write(writer, pGraph, pTypesGraph);
}


void ezAbstractGraphDdlSerializer::Write(
  ezOpenDdlWriter& ref_writer, const ezAbstractObjectGraph* pGraph, const ezAbstractObjectGraph* pTypesGraph /*= nullptr*/)
{
  WriteGraph(ref_writer, pGraph, "Objects");
  if (pTypesGraph)
  {
    WriteGraph(ref_writer, pTypesGraph, "Types");
  }
}

static void ReadGraph(ezAbstractObjectGraph* pGraph, const ezOpenDdlReaderElement* pRoot)
{
  ezStringBuilder tmp, tmp2;
  ezVariant varTmp;

  for (const ezOpenDdlReaderElement* pObject = pRoot->GetFirstChild(); pObject != nullptr; pObject = pObject->GetSibling())
  {
    const ezOpenDdlReaderElement* pGuid = pObject->FindChildOfType(ezOpenDdlPrimitiveType::Custom, "id");
    const ezOpenDdlReaderElement* pType = pObject->FindChildOfType(ezOpenDdlPrimitiveType::String, "t");
    const ezOpenDdlReaderElement* pTypeVersion = pObject->FindChildOfType(ezOpenDdlPrimitiveType::UInt32, "v");
    const ezOpenDdlReaderElement* pName = pObject->FindChildOfType(ezOpenDdlPrimitiveType::String, "n");
    const ezOpenDdlReaderElement* pProps = pObject->FindChildOfType("p");

    if (pGuid == nullptr || pType == nullptr || pProps == nullptr)
    {
      EZ_REPORT_FAILURE("Object contains invalid elements");
      continue;
    }

    ezUuid guid;
    if (ezOpenDdlUtils::ConvertToUuid(pGuid, guid).Failed())
    {
      EZ_REPORT_FAILURE("Object has an invalid guid");
      continue;
    }

    tmp = pType->GetPrimitivesString()[0];

    if (pName)
      tmp2 = pName->GetPrimitivesString()[0];
    else
      tmp2.Clear();

    ezUInt32 uiTypeVersion = 0;
    if (pTypeVersion)
    {
      uiTypeVersion = pTypeVersion->GetPrimitivesUInt32()[0];
    }

    auto* pNode = pGraph->AddNode(guid, tmp, uiTypeVersion, tmp2);

    for (const ezOpenDdlReaderElement* pProp = pProps->GetFirstChild(); pProp != nullptr; pProp = pProp->GetSibling())
    {
      if (!pProp->HasName())
        continue;

      if (ezOpenDdlUtils::ConvertToVariant(pProp, varTmp).Failed())
        continue;

      pNode->AddProperty(pProp->GetName(), varTmp);
    }
  }
}

ezResult ezAbstractGraphDdlSerializer::Read(
  ezStreamReader& inout_stream, ezAbstractObjectGraph* pGraph, ezAbstractObjectGraph* pTypesGraph, bool bApplyPatches)
{
  ezOpenDdlReader reader;
  if (reader.ParseDocument(inout_stream, 0, ezLog::GetThreadLocalLogSystem()).Failed())
  {
    ezLog::Error("Failed to parse DDL graph");
    return EZ_FAILURE;
  }

  return Read(reader.GetRootElement(), pGraph, pTypesGraph, bApplyPatches);
}


ezResult ezAbstractGraphDdlSerializer::Read(const ezOpenDdlReaderElement* pRootElement, ezAbstractObjectGraph* pGraph,
  ezAbstractObjectGraph* pTypesGraph /*= nullptr*/, bool bApplyPatches /*= true*/)
{
  const ezOpenDdlReaderElement* pObjects = pRootElement->FindChildOfType("Objects");
  if (pObjects != nullptr)
  {
    ReadGraph(pGraph, pObjects);
  }
  else
  {
    ezLog::Error("DDL graph does not contain an 'Objects' root object");
    return EZ_FAILURE;
  }

  ezAbstractObjectGraph* pTempTypesGraph = pTypesGraph;
  if (pTempTypesGraph == nullptr)
  {
    pTempTypesGraph = EZ_DEFAULT_NEW(ezAbstractObjectGraph);
  }
  const ezOpenDdlReaderElement* pTypes = pRootElement->FindChildOfType("Types");
  if (pTypes != nullptr)
  {
    ReadGraph(pTempTypesGraph, pTypes);
  }

  if (bApplyPatches)
  {
    if (pTempTypesGraph)
      ezGraphVersioning::GetSingleton()->PatchGraph(pTempTypesGraph);
    ezGraphVersioning::GetSingleton()->PatchGraph(pGraph, pTempTypesGraph);
  }

  if (pTypesGraph == nullptr)
    EZ_DEFAULT_DELETE(pTempTypesGraph);

  return EZ_SUCCESS;
}

ezResult ezAbstractGraphDdlSerializer::ReadBlocks(ezStreamReader& stream, ezHybridArray<ezSerializedBlock, 3>& blocks)
{
  ezOpenDdlReader reader;
  if (reader.ParseDocument(stream, 0, ezLog::GetThreadLocalLogSystem()).Failed())
  {
    ezLog::Error("Failed to parse DDL graph");
    return EZ_FAILURE;
  }

  const ezOpenDdlReaderElement* pRoot = reader.GetRootElement();
  for (const ezOpenDdlReaderElement* pChild = pRoot->GetFirstChild(); pChild != nullptr; pChild = pChild->GetSibling())
  {
    ezSerializedBlock* pBlock = GetOrCreateBlock(blocks, pChild->GetCustomType());
    ReadGraph(pBlock->m_Graph.Borrow(), pChild);
  }
  return EZ_SUCCESS;
}

#define EZ_DOCUMENT_VERSION 2

void ezAbstractGraphDdlSerializer::WriteDocument(ezStreamWriter& inout_stream, const ezAbstractObjectGraph* pHeader, const ezAbstractObjectGraph* pGraph,
  const ezAbstractObjectGraph* pTypes, bool bCompactMode, ezOpenDdlWriter::TypeStringMode typeMode)
{
  ezOpenDdlWriter writer;
  writer.SetOutputStream(&inout_stream);
  writer.SetCompactMode(bCompactMode);
  writer.SetFloatPrecisionMode(ezOpenDdlWriter::FloatPrecisionMode::Exact);
  writer.SetPrimitiveTypeStringMode(typeMode);

  if (typeMode != ezOpenDdlWriter::TypeStringMode::Compliant)
    writer.SetIndentation(-1);

  ezStringBuilder sHeaderVersion;
  sHeaderVersion.SetFormat("HeaderV{0}", (int)EZ_DOCUMENT_VERSION);
  WriteGraph(writer, pHeader, sHeaderVersion);
  WriteGraph(writer, pGraph, "Objects");
  WriteGraph(writer, pTypes, "Types");
}

ezResult ezAbstractGraphDdlSerializer::ReadDocument(ezStreamReader& inout_stream, ezUniquePtr<ezAbstractObjectGraph>& ref_pHeader,
  ezUniquePtr<ezAbstractObjectGraph>& ref_pGraph, ezUniquePtr<ezAbstractObjectGraph>& ref_pTypes, bool bApplyPatches)
{
  ezHybridArray<ezSerializedBlock, 3> blocks;
  if (ReadBlocks(inout_stream, blocks).Failed())
  {
    return EZ_FAILURE;
  }

  ezInt32 iVersion = 2;
  ezSerializedBlock* pHB = FindHeaderBlock(blocks, iVersion);
  ezSerializedBlock* pOB = FindBlock(blocks, "Objects");
  ezSerializedBlock* pTB = FindBlock(blocks, "Types");
  if (!pOB)
  {
    ezLog::Error("No 'Objects' block in document");
    return EZ_FAILURE;
  }
  if (!pTB && !pHB)
  {
    iVersion = 0;
  }
  else if (!pHB)
  {
    iVersion = 1;
  }
  if (iVersion < 2)
  {
    // Move header into its own graph.
    ezStringBuilder sHeaderVersion;
    sHeaderVersion.SetFormat("HeaderV{0}", iVersion);
    pHB = GetOrCreateBlock(blocks, sHeaderVersion);
    ezAbstractObjectGraph& graph = *pOB->m_Graph.Borrow();
    if (auto* pHeaderNode = graph.GetNodeByName("Header"))
    {
      ezAbstractObjectGraph& headerGraph = *pHB->m_Graph.Borrow();
      /*auto* pNewHeaderNode =*/headerGraph.CopyNodeIntoGraph(pHeaderNode);
      // pNewHeaderNode->AddProperty("DocVersion", iVersion);
      graph.RemoveNode(pHeaderNode->GetGuid());
    }
  }

  if (bApplyPatches && pTB)
  {
    ezGraphVersioning::GetSingleton()->PatchGraph(pTB->m_Graph.Borrow());
    ezGraphVersioning::GetSingleton()->PatchGraph(pHB->m_Graph.Borrow(), pTB->m_Graph.Borrow());
    ezGraphVersioning::GetSingleton()->PatchGraph(pOB->m_Graph.Borrow(), pTB->m_Graph.Borrow());
  }

  ref_pHeader = std::move(pHB->m_Graph);
  ref_pGraph = std::move(pOB->m_Graph);
  if (pTB)
  {
    ref_pTypes = std::move(pTB->m_Graph);
  }

  return EZ_SUCCESS;
}

// This is a handcrafted DDL reader that ignores everything that is not an 'AssetInfo' object
// The purpose is to speed up reading asset information by skipping everything else
//
// Version 0 and 1:
// The reader 'knows' the file format details and uses them.
// Top-level (ie. depth 0) there is an "Objects" object -> we need to enter that
// Inside that (depth 1) there is the "AssetInfo" object -> need to enter that as well
// All objects inside that must be stored
// Once the AssetInfo object is left everything else can be skipped
//
// Version 2:
// The very first top level object is "Header" and only that is read and parsing is stopped afterwards.
class HeaderReader : public ezOpenDdlReader
{
public:
  HeaderReader() = default;

  bool m_bHasHeader = false;
  ezInt32 m_iDepth = 0;

  virtual void OnBeginObject(ezStringView sType, ezStringView sName, bool bGlobalName) override
  {
    //////////////////////////////////////////////////////////////////////////
    // New document format has header block.
    if (m_iDepth == 0 && sType.StartsWith("HeaderV"))
    {
      m_bHasHeader = true;
    }
    if (m_bHasHeader)
    {
      ++m_iDepth;
      ezOpenDdlReader::OnBeginObject(sType, sName, bGlobalName);
      return;
    }

    //////////////////////////////////////////////////////////////////////////
    // Old header is stored in the object block.
    // not yet entered the "Objects" group
    if (m_iDepth == 0 && sType == "Objects")
    {
      ++m_iDepth;

      ezOpenDdlReader::OnBeginObject(sType, sName, bGlobalName);
      return;
    }

    // not yet entered the "AssetInfo" group, but inside "Objects"
    if (m_iDepth == 1 && sType == "AssetInfo")
    {
      ++m_iDepth;

      ezOpenDdlReader::OnBeginObject(sType, sName, bGlobalName);
      return;
    }

    // inside "AssetInfo"
    if (m_iDepth > 1)
    {
      ++m_iDepth;
      ezOpenDdlReader::OnBeginObject(sType, sName, bGlobalName);
      return;
    }

    // ignore everything else
    SkipRestOfObject();
  }


  virtual void OnEndObject() override
  {
    --m_iDepth;
    if (m_bHasHeader)
    {
      if (m_iDepth == 0)
      {
        m_iDepth = -1;
        StopParsing();
      }
    }
    else
    {
      if (m_iDepth <= 1)
      {
        // we were inside "AssetInfo" or "Objects" and returned from it, so now skip the rest
        m_iDepth = -1;
        StopParsing();
      }
    }
    ezOpenDdlReader::OnEndObject();
  }
};

ezResult ezAbstractGraphDdlSerializer::ReadHeader(ezStreamReader& inout_stream, ezAbstractObjectGraph* pGraph)
{
  HeaderReader reader;
  if (reader.ParseDocument(inout_stream, 0, ezLog::GetThreadLocalLogSystem()).Failed())
  {
    EZ_REPORT_FAILURE("Failed to parse DDL graph");
    return EZ_FAILURE;
  }

  const ezOpenDdlReaderElement* pObjects = nullptr;
  if (reader.m_bHasHeader)
  {
    pObjects = reader.GetRootElement()->GetFirstChild();
  }
  else
  {
    pObjects = reader.GetRootElement()->FindChildOfType("Objects");
  }

  if (pObjects != nullptr)
  {
    ReadGraph(pGraph, pObjects);
  }
  else
  {
    EZ_REPORT_FAILURE("DDL graph does not contain an 'Objects' root object");
    return EZ_FAILURE;
  }
  return EZ_SUCCESS;
}
