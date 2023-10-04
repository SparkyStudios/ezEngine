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

#include <Foundation/IO/OSFile.h>

using namespace RAI;

ezResult spShaderProcessor::Process(ezStringView sFileName, ezStringView sOutputPath)
{
  EZ_LOG_BLOCK("spShaderProcessor::Process", sFileName);

  if (ezPathUtils::HasAnyExtension(sFileName))
  {
    if (!m_Configuration.m_ShaderVariants.IsEmpty())
      ezLog::Warning("Shader variants are not supported by the shader processor.");

    return m_ShaderImporter.Import(sFileName, sOutputPath);
  }
  else
  {
    ezSet<ezStringView> filesToProcess;

    for (auto it = m_Configuration.m_ShaderVariants.GetIterator(); it.IsValid(); it.Next())
      filesToProcess.Insert(*it);

    ezFileSystemIterator it;
    for (it.StartSearch(sFileName, ezFileSystemIteratorFlags::ReportFiles); it.IsValid(); it.Next())
    {
      ezStringBuilder filePath = it.GetCurrentPath();
      filePath.AppendPath(it.GetStats().m_sName);

      if (filesToProcess.IsEmpty() || filesToProcess.Contains(filePath))
        m_ShaderImporter.Import(filePath, sOutputPath).IgnoreResult();
    }

    return EZ_SUCCESS;
  }
}

spShaderProcessor::spShaderProcessor(const spShaderProcessorConfig& config)
  : spProcessor(config)
  , m_ShaderImporter({})
{
}
