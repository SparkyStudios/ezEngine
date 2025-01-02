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

implementing MaterialSystem;

public struct AccumulatedLight
{
  public float3 diffuse;
  public float3 specular;
}

AccumulatedLight operator +(AccumulatedLight a, AccumulatedLight b)
{
  AccumulatedLight result;
  result.diffuse  = a.diffuse + b.diffuse;
  result.specular = a.specular + b.specular;

  return result;
}

public interface IShadingTechnique
{
  associatedtype DiffuseBRDF : IBRDF;
  associatedtype SpecularBRDF : IBRDF;

  DiffuseBRDF  prepareDiffuse(in Surface surface, in AngularInfo angularInfo);
  SpecularBRDF prepareSpecular(in Surface surface, in AngularInfo angularInfo);
}

public extension<T : IShadingTechnique> T
{
  public AccumulatedLight process(in Surface surface, in AngularInfo angularInfo)
  {
    AccumulatedLight result;
    result.diffuse  = prepareDiffuse(surface, angularInfo).evaluate(surface, angularInfo);
    result.specular = prepareSpecular(surface, angularInfo).evaluate(surface, angularInfo);

    return result;
  }
}
