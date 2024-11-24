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

#include <AssetProcessor/Processors/ShaderProcessor.h>

#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Utilities/AssetFileHeader.h>

#include <RPI/Resources/ShaderResource.h>

using namespace RPI;

ezResult spShaderProcessor::Process(ezStringView sFileName, ezStringView sOutputPath)
{
  EZ_LOG_BLOCK("spShaderProcessor::Process", sFileName);

  // TODO: Import shader file and generate Slang IR

  spShaderResourceDescriptor desc;
  // TODO: Populate shader resource descriptor

  ezStringBuilder sOutputFile(sOutputPath);
  sOutputFile.AppendFormat("/{}.spShader", sFileName.GetFileName());

  // Write shader variant resource
  {
    ezFileWriter file;
    if (file.Open(sOutputFile, 1024 * 1024).Failed())
    {
      ezLog::Error("Failed to save mesh asset: '{0}'", sOutputFile);
      return EZ_FAILURE;
    }

    // Write asset header
    ezAssetFileHeader assetHeader;
    assetHeader.SetGenerator("SparkEngine Asset Processor");
    assetHeader.SetFileHashAndVersion(ezHashingUtils::xxHash64String(sFileName), 1);
    EZ_SUCCEED_OR_RETURN(assetHeader.Write(file));

    EZ_SUCCEED_OR_RETURN(desc.Save(file));
  }

  return EZ_SUCCESS;
}

spShaderProcessor::spShaderProcessor(const spShaderProcessorConfig& config)
  : spProcessor(config)
  , m_ShaderImporter({})
{
}
