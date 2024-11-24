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

#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Logging/Log.h>

#include <mpack/mpack.h>

/// \brief Base class for all RPI assets importers.
/// \tparam TConfig The configuration type of the importer.
template <typename TConfig>
class spImporter
{
public:
  /// \brief Initializes a new instance of the importer.
  /// \param configuration The importer configuration to use.
  explicit spImporter(TConfig configuration)
    : m_Configuration(std::move(configuration))
  {
  }

  virtual ~spImporter() = default;

  /// \brief Imports the asset from the specified root node.
  ///
  /// This will typically convert the raw data in each nodes starting
  /// at the root node to asset data. Node data are assumed to be already
  /// properly parsed from mesh files and all transformations (tangent space
  /// calculation, mesh optimization, etc.) are applied.
  ///
  /// \param [in] sFilePath The path to the input file.
  /// \param [in] sOutputPath The path to the output directory.
  virtual ezResult Import(ezStringView sFilePath, ezStringView sOutputPath) = 0;

  /// \brief The current importer configuration.
  [[nodiscard]] EZ_ALWAYS_INLINE TConfig GetConfiguration() const { return m_Configuration; }

protected:
  TConfig m_Configuration;
};

// Utilities
void mpack_node_hstr(const mpack_node_t& node, ezHashedString& out_str);
