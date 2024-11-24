// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <AssetProcessor/Importers/RootMaterialImporter.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Utilities/AssetFileHeader.h>

#include <mpack/mpack.h>

using namespace RPI;

ezResult spRootMaterialImporter::Import(ezStringView sAssetPath, ezStringView sOutputPath)
{
  spRootMaterialResourceDescriptor desc;

  // TODO: Generate Slang IR

  return EZ_SUCCESS;
}

spRootMaterialImporter::spRootMaterialImporter(const spRootMaterialImporterConfiguration& config)
  : spImporter<spRootMaterialImporterConfiguration>(config)
{
}
