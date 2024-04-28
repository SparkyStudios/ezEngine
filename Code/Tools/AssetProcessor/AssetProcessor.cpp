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

#include <AssetProcessor/Processors/ImageProcessor.h>
#include <AssetProcessor/Processors/MeshProcessor.h>
#include <AssetProcessor/Processors/RootMaterialProcessor.h>
#include <AssetProcessor/Processors/ShaderProcessor.h>

#include <Foundation/Application/Application.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Logging/ConsoleWriter.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Logging/VisualStudioWriter.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Time/Stopwatch.h>
#include <Foundation/Utilities/CommandLineOptions.h>

/* AssetProcessor command line options:

-mesh <paths>
    Path to one or many mesh files.

    It will generate an spMesh asset with the same file name, in the same path, unless
    the -out option is specified with a different value.

    Example:
      -mesh "path/to/mesh.glb"
      -mesh "path/to/mesh1.glb" "path/to/mesh2.fbx"

-texture <paths>
    Path to one or many texture files.

    It will generate an spTexture asset with the same file name, in the same path, unless
    the -out option is specified with a different value.

    Example:
      -texture "path/to/texture.tga"
      -texture "path/to/texture1.jpg" "path/to/texture2.png"

-shader <paths>
    Paths to one or many folders containing compiled SPSL shader variants files and a single variants.json file.

    It will generate spShader assets from SPSL shader variant files, in the same path, unless
    the -out option is specified with a different value.

    Example:
        -shader "path/to/shader"
        -shader "path/to/shader1" "path/to/shader2"

-material <paths>
     Path to one or many root material files.

    It will generate an spRootMaterial asset from an SPSL root material file with the same file name, in the same path,
    unless the -out option is specified with a different value.

    Example:
      -material "path/to/material.sprm"
      -material "path/to/material1.sprm" "path/to/material2.sprm"

-out <path>
    The output path of the currently processed assets.

    This option always takes a directory path as input. It's in the specified directory that
    the generated asset will be created.

    Example:
      -out "path/to/assets_folder"

Description:
    AssetProcessor converts native files to assets usable by the engine. The type of asset to generate
    is specified by a command line option. It's illegal to process more than one asset type at a time.

Examples:
    AssetProcessor.exe -mesh "C:/DCC/table.glb"
      Processes the "table.glb" mesh and save the generated asset in "C:/DCC/table.spMesh"

    AssetProcessor.exe -texture "C:/DCC/table_nrm.tga" "C:/DCC/table_diff.png" -out "C:/Game/Assets"
      Processes the "table_nrm.tga" and "table_diff.png" images and save the generated assets in "C:/Game/Assets/table_nrm.spTexture" and "C:/Game/Assets/table_diff.spTexture"
*/

#pragma region Mesh processing options

ezCommandLineOptionDoc opt_Meshes("_AssetProcessor", "-mesh", "<paths>", R"(
Path to one or many mesh files.

It will generate an spMesh asset with the same file name, in the same path, unless
the -out option is specified with a different value.
)",
  "");

ezCommandLineOptionFloat opt_Mesh_Scale("_AssetProcessor_Mesh", "-scale", "The global scale to apply on the processed mesh.", 1.0f, 0.0f);

ezCommandLineOptionBool opt_Mesh_WithSkeleton("_AssetProcessor_Mesh", "-with-skeleton", "Whether to generate skeleton assets from the mesh.", false);

ezCommandLineOptionBool opt_Mesh_WithMotions("_AssetProcessor_Mesh", "-with-motions", "Whether to process motion assets from the imported mesh.", false);

ezCommandLineOptionBool opt_Mesh_WithMaterials("_AssetProcessor_Mesh", "-with-materials", "Whether to process material assets from the mesh. Generated material assets will use the default PBR shader.", false);

ezCommandLineOptionBool opt_Mesh_WithMeshes("_AssetProcessor_Mesh", "-with-meshes", "Whether to process mesh assets from the mesh.", true);

ezCommandLineOptionBool opt_Mesh_HasLODs("_AssetProcessor_Mesh", "-has-lods", R"(
Specifies if the mesh support the Spark LOD system.

LODs will be detected and processed. The mesh need to be exported following the specifications
of the Spark LOD system for this option to work.
)",
  false);

ezCommandLineOptionBool opt_Mesh_FlipUVs("_AssetProcessor_Mesh", "-flip-uvs", "Flips the vertex UVs.", false);

ezCommandLineOptionBool opt_Mesh_RecomputeNormals("_AssetProcessor_Mesh", "-recompute-normals", "Whether to recompute the vertex normals. Normals will be recalculated even if present.", false);

ezCommandLineOptionBool opt_Mesh_RecomputeTangents("_AssetsProcessor_Mesh", "-recompute-tangents", "Whether to recompute vertex tangent vectors.", false);

ezCommandLineOptionBool opt_Mesh_FlipWindingNormals("_AssetProcessor_Mesh", "-flip-winding-normals", "Whether to flip in-facing normal vectors.", false);

ezCommandLineOptionBool opt_Mesh_Optimize("_AssetProcessor_Mesh", "-optimize", R"(
Optimizes the mesh for GPU drawing.

This will reorder the index and vertex buffers to reduce the number of data fetch from the GPU.
It's highly recommended to keep this option enabled.
)",
  true);

ezCommandLineOptionEnum opt_Mesh_NormalPrecision("_AssetProcessor_Mesh", "-p-normals", "The precision of a single component in normal vectors.", "10=10|16=16|32=32", 10);

ezCommandLineOptionEnum opt_Mesh_TexCoordPrecision("_AssetProcessor_Mesh", "-p-uvs", "The precision of a single component in texture coordinates.", "16=16|32=32", 16);

ezCommandLineOptionEnum opt_Mesh_BoneWeightPrecision("_AssetProcessor_Mesh", "-p-weights", "The precision of a single component in bone weights.", "8=8|10=10|16=16|32=32", 8);

#pragma endregion

#pragma region Texture processing options

ezCommandLineOptionDoc opt_Textures("_AssetProcessor", "-texture", "<paths>", R"(
Path to one or many texture files.

It will generate an spTexture asset with the same file name, in the same path, unless
the -out option is specified with a different value.
)",
  "");

ezCommandLineOptionEnum opt_Texture_Type("_AssetProcessor_Texture", "-type", "The type of texture.", "Color|Greyscale|NormalMap|HeightMap|Material|HDR|RawData", 0);

ezCommandLineOptionBool opt_Texture_FlipH("_AssetProcessor_Texture", "-flip-horizontal", "Flips the texture horizontally.", false);

ezCommandLineOptionBool opt_Texture_FlipV("_AssetProcessor_Texture", "-flip-vertical", "Flips the texture vertically.", false);

ezCommandLineOptionBool opt_Texture_GenerateMipMaps("_AssetProcessor_Texture", "-generate-mipmaps", "Generates mipmaps for the texture.", false);

ezCommandLineOptionInt opt_Texture_MipLevelCount("_AssetProcessor_Texture", "-mip-level-count", "The number of mipmap levels to generate.", 0, 0);

ezCommandLineOptionEnum opt_Texture_Filter("_AssetProcessor_Texture", "-filter", "The texture filtering mode to use when generating mipmaps.", "Kaiser|Triangle|Box", 0);

ezCommandLineOptionEnum opt_Texture_WrapMode("_AssetProcessor_Texture", "-wrap-mode", "The texture wrapping mode to use when generating mipmaps.", "Clamp|Repeat|Mirror", 0);

ezCommandLineOptionEnum opt_Texture_Compression("_AssetProcessor_Texture", "-compression", "The texture compression level.", "None|Medium|High", 0);

#pragma endregion

#pragma region Shader processing options

ezCommandLineOptionDoc opt_Shaders("_AssetProcessor", "-shader", "<paths>", R"(
Path to one or many directories containing compiled SPSL shader variants.

It will generate spShaderVariant assets from SPSL files the input directory, in the same path, unless
the -out option is specified with a different value.
)",
  "");

ezCommandLineOptionString opt_Shader_Variants("_AssetProcessor_Shader", "-variants", R"(
The shader variants to generate.

This argument takes the names of the shader variants to generate, separated by spaces.
)",
  "");

#pragma endregion

#pragma region Root Material processing options

ezCommandLineOptionDoc opt_Materials("_AssetProcessor_RootMaterial", "-material", "<paths>", R"(
Path to one or many SPSL root materials.

It will generate an spRootMaterial asset from the SPSL file, using the same file name, unless the
-out option is specified with a different value.
)",
  "");

#pragma endregion

ezCommandLineOptionPath opt_Out("_AssetProcessor", "-out", R"(
The output path of the currently processed assets.

This option always takes a directory path as input. It's in the specified directory that
the generated asset will be created.
)",
  "");

class spAssetProcessor : public ezApplication
{
public:
  typedef ezApplication SUPER;

  /// \brief Enumerates available asset types to process.
  enum class AssetType
  {
    /// \brief A mesh asset file.
    ///
    /// This type of asset can generate a *.spMesh file, a *.spSkeleton file,
    /// one or more *.spMotion files and a *.spBlendShape file. This is configured
    /// through the command line options.
    Mesh,

    /// \brief A texture asset file.
    ///
    /// This type of asset will generate a *.spTexture file.
    Texture,

    /// \brief A shader asset file.
    ///
    /// This type of asset will generate a *.spShader file, and many *.spShaderVariant files
    /// according to the variants.json file.
    Shader,

    /// \brief A root material asset file.
    ///
    /// This type of asset will generate a *.spRootMaterial file.
    RootMaterial,

    Unknown = -1
  };

  ezDynamicArray<ezString> m_sInputs;
  ezString m_sOutput;

  AssetType m_eAssetType{AssetType::Unknown};

  spMeshProcessorConfig m_MeshProcessorConfig;

  spImageProcessorConfig m_TextureProcessorConfig;

  spShaderProcessorConfig m_ShaderProcessorConfig;

  spRootMaterialProcessorConfig m_RootMaterialProcessorConfig;

  static const char* GetSortingGroupFromAssetType(AssetType type)
  {
    switch (type)
    {
      case AssetType::Mesh:
        return "_AssetProcessor_Mesh";
      case AssetType::Texture:
        return "_AssetProcessor_Texture";
      case AssetType::Shader:
        return "_AssetProcessor_Shader";
      case AssetType::RootMaterial:
        return "_AssetProcessor_RootMaterial";
      case AssetType::Unknown:
      default:
        return "_AssetProcessor";
    }
  }

  spAssetProcessor()
    : ezApplication("AssetProcessor")
  {
  }

  ezResult ParseArguments()
  {
    if (GetArgumentCount() <= 1)
    {
      ezLog::Error("No argument given. Use -help to get started.");
      return EZ_FAILURE;
    }

    const ezCommandLineUtils& cmd = *ezCommandLineUtils::GetGlobalInstance();

    if (opt_Meshes.IsOptionSpecified())
    {
      m_eAssetType = AssetType::Mesh;
      EZ_SUCCEED_OR_RETURN(ParseMeshArguments());
    }
    else if (opt_Textures.IsOptionSpecified())
    {
      m_eAssetType = AssetType::Texture;
      EZ_SUCCEED_OR_RETURN(ParseTextureArguments());
    }
    else if (opt_Shaders.IsOptionSpecified())
    {
      m_eAssetType = AssetType::Shader;
      EZ_SUCCEED_OR_RETURN(ParseShaderArguments());
    }
    else if (opt_Materials.IsOptionSpecified())
    {
      m_eAssetType = AssetType::RootMaterial;
      EZ_SUCCEED_OR_RETURN(ParseRootMaterialArguments());
    }

    m_sOutput = opt_Out.GetOptionValue(ezCommandLineOption::LogMode::Never);
    m_sOutput = ezOSFile::MakePathAbsoluteWithCWD(m_sOutput);

    const bool isHelp = cmd.HasOption("-help");

    if (!isHelp && m_sInputs.IsEmpty() && !m_sOutput.IsEmpty())
    {
      ezLog::Error("Input asset files not defined. Please provide them.");
      return EZ_FAILURE;
    }

    if (!m_sInputs.IsEmpty())
    {
      ezLog::Info("Inputs:");
      for (const auto& input : m_sInputs)
      {
        ezLog::Info("  {}", input);
      }
    }

    if (!m_sOutput.IsEmpty())
    {
      ezLog::Info("Output:\n  {}", m_sOutput);
    }

    return EZ_SUCCESS;
  }

  ezResult ParseMeshArguments()
  {
    const ezCommandLineUtils& cmd = *ezCommandLineUtils::GetGlobalInstance();

    if (const ezUInt32 args = cmd.GetStringOptionArguments("-mesh"); args > 0)
    {
      if (m_eAssetType != AssetType::Mesh)
      {
        ezLog::Error("Cannot use -mesh option with another asset type.");
        return EZ_FAILURE;
      }

      for (ezUInt32 a = 0; a < args; ++a)
      {
        m_sInputs.PushBack(cmd.GetAbsolutePathOption("-mesh", a));

        if (!ezOSFile::ExistsFile(m_sInputs.PeekBack()))
        {
          ezLog::Error("-mesh input path is not a valid file: '{}'", m_sInputs.PeekBack());
          return EZ_FAILURE;
        }
      }
    }

    m_MeshProcessorConfig.m_bImportSkeleton = opt_Mesh_WithSkeleton.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified);
    m_MeshProcessorConfig.m_bImportMotions = opt_Mesh_WithMotions.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified);
    m_MeshProcessorConfig.m_bImportMaterials = opt_Mesh_WithMaterials.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified);
    m_MeshProcessorConfig.m_bImportMeshes = opt_Mesh_WithMeshes.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified);

    m_MeshProcessorConfig.m_AssimpImporterConfig.m_fScale = opt_Mesh_Scale.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified);
    m_MeshProcessorConfig.m_AssimpImporterConfig.m_bHasLODs = opt_Mesh_HasLODs.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified);
    m_MeshProcessorConfig.m_AssimpImporterConfig.m_bFlipUVs = opt_Mesh_FlipUVs.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified);
    m_MeshProcessorConfig.m_AssimpImporterConfig.m_bRecomputeNormals = opt_Mesh_RecomputeNormals.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified);
    m_MeshProcessorConfig.m_AssimpImporterConfig.m_bRecomputeTangents = opt_Mesh_RecomputeTangents.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified);
    m_MeshProcessorConfig.m_AssimpImporterConfig.m_bFlipWindingNormals = opt_Mesh_FlipWindingNormals.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified);
    m_MeshProcessorConfig.m_AssimpImporterConfig.m_bOptimizeMesh = opt_Mesh_Optimize.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified);
    m_MeshProcessorConfig.m_AssimpImporterConfig.m_eNormalPrecision = static_cast<spAssimpVertexStreamComponentPrecision::Enum>(opt_Mesh_NormalPrecision.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified));
    m_MeshProcessorConfig.m_AssimpImporterConfig.m_eTexCoordPrecision = static_cast<spAssimpVertexStreamComponentPrecision::Enum>(opt_Mesh_TexCoordPrecision.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified));
    m_MeshProcessorConfig.m_AssimpImporterConfig.m_eBoneWeightPrecision = static_cast<spAssimpVertexStreamComponentPrecision::Enum>(opt_Mesh_BoneWeightPrecision.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified));

    return EZ_SUCCESS;
  }

  ezResult ParseTextureArguments()
  {
    const ezCommandLineUtils& cmd = *ezCommandLineUtils::GetGlobalInstance();

    if (const ezUInt32 args = cmd.GetStringOptionArguments("-texture"); args > 0)
    {
      if (m_eAssetType != AssetType::Texture)
      {
        ezLog::Error("Cannot use -texture option with another asset type.");
        return EZ_FAILURE;
      }

      for (ezUInt32 a = 0; a < args; ++a)
      {
        m_sInputs.PushBack(cmd.GetAbsolutePathOption("-texture", a));

        if (!ezOSFile::ExistsFile(m_sInputs.PeekBack()))
        {
          ezLog::Error("-texture input file does not exist: '{}'", m_sInputs.PeekBack());
          return EZ_FAILURE;
        }
      }
    }

    m_TextureProcessorConfig.m_TextureImporterConfig.m_bFlipHorizontal = opt_Texture_FlipH.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified);
    m_TextureProcessorConfig.m_TextureImporterConfig.m_bFlipVertical = opt_Texture_FlipV.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified);
    m_TextureProcessorConfig.m_TextureImporterConfig.m_bGenerateMipMaps = opt_Texture_GenerateMipMaps.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified);
    m_TextureProcessorConfig.m_TextureImporterConfig.m_uiNumMipLevels = opt_Texture_MipLevelCount.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified);
    m_TextureProcessorConfig.m_TextureImporterConfig.m_eMipmapFilter = static_cast<spTextureMipmapFilter::Enum>(opt_Texture_Filter.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified));
    m_TextureProcessorConfig.m_TextureImporterConfig.m_eWrapMode = static_cast<spTextureWrapMode::Enum>(opt_Texture_WrapMode.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified));
    m_TextureProcessorConfig.m_TextureImporterConfig.m_eType = static_cast<spTextureImageType::Enum>(opt_Texture_Type.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified));
    m_TextureProcessorConfig.m_TextureImporterConfig.m_eCompressionLevel = static_cast<spTextureCompressionLevel::Enum>(opt_Texture_Compression.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified));

    return EZ_SUCCESS;
  }

  ezResult ParseShaderArguments()
  {
    const ezCommandLineUtils& cmd = *ezCommandLineUtils::GetGlobalInstance();

    if (const ezUInt32 args = cmd.GetStringOptionArguments("-shader"); args > 0)
    {
      if (m_eAssetType != AssetType::Shader)
      {
        ezLog::Error("Cannot use -shader option with another asset type.");
        return EZ_FAILURE;
      }

      for (ezUInt32 a = 0; a < args; ++a)
      {
        m_sInputs.PushBack(cmd.GetAbsolutePathOption("-shader", a));

        if (!ezOSFile::ExistsFile(m_sInputs.PeekBack()) && !ezOSFile::ExistsDirectory(m_sInputs.PeekBack()))
        {
          ezLog::Error("-shader input file does not exist: '{}'", m_sInputs.PeekBack());
          return EZ_FAILURE;
        }
      }
    }

    opt_Shader_Variants.GetOptionValue(ezCommandLineOption::LogMode::AlwaysIfSpecified).Split(false, m_ShaderProcessorConfig.m_ShaderVariants, " ");

    return EZ_SUCCESS;
  }

  ezResult ParseRootMaterialArguments()
  {
    const ezCommandLineUtils& cmd = *ezCommandLineUtils::GetGlobalInstance();

    if (const ezUInt32 args = cmd.GetStringOptionArguments("-material"); args > 0)
    {
      if (m_eAssetType != AssetType::RootMaterial)
      {
        ezLog::Error("Cannot use -material option with another asset type.");
        return EZ_FAILURE;
      }

      for (ezUInt32 a = 0; a < args; ++a)
      {
        m_sInputs.PushBack(cmd.GetAbsolutePathOption("-material", a));

        if (!ezOSFile::ExistsFile(m_sInputs.PeekBack()))
        {
          ezLog::Error("-material input file does not exist: '{}'", m_sInputs.PeekBack());
          return EZ_FAILURE;
        }
      }
    }

    return EZ_SUCCESS;
  }

  void ProcessMesh(ezHybridArray<ezString, 8>& failedImports)
  {
    for (const auto& input : m_sInputs)
    {
      spMeshProcessor processor(m_MeshProcessorConfig);

      if (processor.Process(input, m_sOutput).Failed())
        failedImports.PushBack(input);
    }
  }

  void ProcessTexture(ezHybridArray<ezString, 8>& failedImports)
  {
    for (const auto& input : m_sInputs)
    {
      spImageProcessor processor(m_TextureProcessorConfig);

      if (processor.Process(input, m_sOutput).Failed())
        failedImports.PushBack(input);
    }
  }

  void ProcessShader(ezHybridArray<ezString, 8>& failedImports)
  {
    for (const auto& input : m_sInputs)
    {
      spShaderProcessor processor(m_ShaderProcessorConfig);

      if (processor.Process(input, m_sOutput).Failed())
        failedImports.PushBack(input);
    }
  }

  void ProcessRootMaterial(ezHybridArray<ezString, 8>& failedImports)
  {
    for (const auto& input : m_sInputs)
    {
      spRootMaterialProcessor processor(m_RootMaterialProcessorConfig);

      if (processor.Process(input, m_sOutput).Failed())
        failedImports.PushBack(input);
    }
  }

  void AfterCoreSystemsStartup() override
  {
    // Add the empty data directory to access files via absolute paths
    ezFileSystem::AddDataDirectory("", "App", ":", ezFileSystem::AllowWrites).IgnoreResult();

    ezGlobalLog::AddLogWriter(ezLogWriter::Console::LogMessageHandler);
    ezGlobalLog::AddLogWriter(ezLogWriter::VisualStudio::LogMessageHandler);
  }

  void BeforeCoreSystemsShutdown() override
  {
    // prevent further output during shutdown
    ezGlobalLog::RemoveLogWriter(ezLogWriter::Console::LogMessageHandler);
    ezGlobalLog::RemoveLogWriter(ezLogWriter::VisualStudio::LogMessageHandler);

    SUPER::BeforeCoreSystemsShutdown();
  }

  Execution Run() override
  {
    if (ParseArguments().Failed())
    {
      SetReturnCode(1);
      return ezApplication::Execution::Quit;
    }

    {
      ezStringBuilder cmdHelp;
      if (ezCommandLineOption::LogAvailableOptionsToBuffer(cmdHelp, ezCommandLineOption::LogAvailableModes::IfHelpRequested, GetSortingGroupFromAssetType(m_eAssetType)))
      {
        ezLog::Print(cmdHelp);
        return ezApplication::Execution::Quit;
      }
    }

    ezHybridArray<ezString, 8> failedImports;

    {
      ezStopwatch sw;

      switch (m_eAssetType)
      {
        case AssetType::Mesh:
        {
          ProcessMesh(failedImports);
          break;
        }

        case AssetType::Texture:
        {
          ProcessTexture(failedImports);
          break;
        }

        case AssetType::Shader:
        {
          ProcessShader(failedImports);
          break;
        }

        case AssetType::RootMaterial:
        {
          ProcessRootMaterial(failedImports);
          break;
        }

        default:
          break;
      }

      sw.Pause();
      ezLog::Info("Finished processing {0} assets in {3}ms, {1} assets failed, {2} assets ignored", m_sInputs.GetCount(), failedImports.GetCount(), 0, ezArgF(sw.GetRunningTotal().GetMilliseconds(), 2));
    }

    if (!failedImports.IsEmpty())
    {
      ezLog::Error("Failed assets:");
      for (const auto& input : failedImports)
        ezLog::Error("  - {0}", input);
    }

    return ezApplication::Execution::Quit;
  }
};

EZ_CONSOLEAPP_ENTRY_POINT(spAssetProcessor);
