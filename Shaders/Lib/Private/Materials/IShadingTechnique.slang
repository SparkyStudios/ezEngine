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

#pragma once

#include <Lightning/BRDF/IBRDF.slang>

#include <Materials/AngularInfo.slang>
#include <Materials/Surface.slang>

struct AccumulatedLight
{
  float3 diffuse;
  float3 specular;
}

interface IShadingTechnique
{
  associatedtype DiffuseBRDF : IBRDF;
  associatedtype SpecularBRDF : IBRDF;

  DiffuseBRDF  prepareDiffuse(Surface surface, AngularInfo angularInfo);
  SpecularBRDF prepareSpecular(Surface surface, AngularInfo angularInfo);

  AccumulatedLight process(Surface surface, AngularInfo angularInfo);
}
