#include <RAI/RAIPCH.h>

#include <RAI/Mesh.h>

void spMesh::Clear()
{
  m_Data.m_Indices.Clear();
  m_Data.m_Vertices.Clear();

  m_Root.m_Children.Clear();
  m_Root.m_Entries.Clear();
  m_Root.m_sName = "";
  m_Root.m_Transform = {};
}
