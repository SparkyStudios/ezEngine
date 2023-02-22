#pragma once

#include <RPI/RPIDLL.h>

class SP_RPI_DLL spMesh
{
public:
  struct Vertex
  {
    ezVec3 m_vPosition;
    ezVec2 m_vTexCoord0;
    ezVec2 m_vTexCoord1;
    ezColor m_Color0;
    ezColor m_Color1;
    ezVec3 m_vNormal;
    ezVec3 m_vTangent;
    ezVec3 m_vBiTangent;
  };

  struct Data
  {
    ezDynamicArray<Vertex> m_Vertices;
    ezDynamicArray<ezUInt16> m_Indices;
  };

  struct Entry
  {
    ezStringView m_sName;

    ezUInt32 m_uiBaseIndex{0};
    ezUInt32 m_uiIndicesCount{0};

    ezUInt32 m_uiBaseVertex{0};
    ezUInt32 m_uiVerticesCount{0};
  };

  struct Transform
  {
    ezVec3 m_vPosition;
    ezVec3 m_vScale;
    ezVec3 m_vRotation;
  };

  struct Node
  {
    ezStringView m_sName;
    ezDynamicArray<Entry> m_Entries;
    ezDynamicArray<Node> m_Children;
    Transform m_Transform;
    ezStringView m_sMaterial;
  };

  spMesh() = default;

  spMesh(Data meshData, Node rootNode)
    : m_Data(std::move(meshData))
    , m_Root(std::move(rootNode))
  {
  }

  EZ_NODISCARD EZ_ALWAYS_INLINE const Data& GetData() const { return m_Data; }

  EZ_ALWAYS_INLINE void SetData(const Data& data) { m_Data = data; }

  EZ_NODISCARD EZ_ALWAYS_INLINE const Node& GetRootNode() const { return m_Root; }

  EZ_ALWAYS_INLINE void SetRootNode(const Node& rootNode) { m_Root = rootNode; }

private:
  Data m_Data;
  Node m_Root;
};
