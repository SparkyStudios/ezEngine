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

/// \brief Base class for a file processor.
///
/// A file processor can generate more than one asset as output. The processor
/// behavior is configured through the TConfig type parameter which is defined
/// for each implementation.
///
/// \tparam TConfig The configuration type.
template <typename TConfig>
class spProcessor
{
public:
  virtual ~spProcessor() = default;

  /// \brief Processes the given file.
  /// \param [in] sFilename The path to the file to process.
  /// \param [in] sOutputPath The path to the output directory.
  virtual ezResult Process(ezStringView sFilename, ezStringView sOutputPath) = 0;

protected:
  explicit spProcessor(TConfig configuration)
    : m_Configuration(std::move(configuration))
  {
  }

  TConfig m_Configuration;
};
