#include <AssetProcessor/Importers/AssimpImporter.h>

void spAssimpImporterContext::Clear()
{
  m_Nodes.Clear();
  m_pScene = nullptr;
}

spAssimpImporter::spAssimpImporter(const spAssimpImporterConfiguration& configuration)
  : spImporter(configuration)
{
}
