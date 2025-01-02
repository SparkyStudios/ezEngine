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

module MaterialSystem;

import Common;
import Math;

#include <MaterialFlags.h>

__include "MaterialSystem/BRDF/Diffuse.slang";
__include "MaterialSystem/BRDF/Distribution.slang";
__include "MaterialSystem/BRDF/Fresnel.slang";
__include "MaterialSystem/BRDF/IBRDF.slang";
__include "MaterialSystem/BRDF/Specular.slang";
__include "MaterialSystem/BRDF/Visibility.slang";

__include "MaterialSystem/AngularInfo.slang";
__include "MaterialSystem/IMaterial.slang";
__include "MaterialSystem/IShadingTechnique.slang";
__include "MaterialSystem/Light.slang";
__include "MaterialSystem/MaterialData.slang";
__include "MaterialSystem/Streams.slang";
__include "MaterialSystem/Surface.slang";
__include "MaterialSystem/Utils.slang";
__include "MaterialSystem/VertexProcessing.slang";
