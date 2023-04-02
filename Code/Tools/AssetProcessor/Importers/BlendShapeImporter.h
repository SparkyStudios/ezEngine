#pragma once

#include <AssetProcessor/Importers/AssimpImporter.h>

#include <RAI/Resources/BlendShapeResource.h>

class spBlendShapeImporter final : public spAssimpImporter
{
  // spAssimpImporter

public:
  ezResult Import(ezStringView sFilePath, ezStringView sOutputPath) override;

  // spBlendShapeImporter

public:
  explicit spBlendShapeImporter(const spAssimpImporterConfiguration& configuration);
};
