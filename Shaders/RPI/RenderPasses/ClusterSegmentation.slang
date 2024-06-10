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

#include <Common.slang>

#define CLUSTERS_X_THREADS 16
#define CLUSTERS_Y_THREADS 8
#define CLUSTERS_Z_THREADS 4

#define CLUSTERS_X 16
#define CLUSTERS_Y 8
#define CLUSTERS_Z 24

#define sp_WorkGroupSize uint3(CLUSTERS_X_THREADS, CLUSTERS_Y_THREADS, CLUSTERS_Z_THREADS)

struct Cluster
{
  float4 MinPoint;
  float4 MaxPoint;
}

// The writeonly buffer of segmented clusters.
writeonly RWStructuredBuffer<Cluster> Clusters;

// The size of a cluster in screen coordinates.
uniform uint2 ClusterSize;

// Calculates light cluster min/max AABB in eye space
// Largely inspired by http://www.aortiz.me/2018/12/21/CG.html
// and http://advances.realtimerendering.com/s2016/Siggraph2016_idTech6.pdf for the z-subdivision
[shader("compute")]
[numthreads(CLUSTERS_X_THREADS, CLUSTERS_Y_THREADS, CLUSTERS_Z_THREADS)]
void ClusterSegmentation(uint3 sp_DispatchThreadId: SV_DispatchThreadID)
{
  // Index calculation must match the inverse operation in the fragment shader (see getClusterIndex)
  uint clusterIndex = sp_DispatchThreadId.z * sp_WorkGroupSize.x * sp_WorkGroupSize.y +
                      sp_DispatchThreadId.y * sp_WorkGroupSize.x +
                      sp_DispatchThreadId.x;

  // Calculating the min and max point in screen space
  float4 maxPoint_sS = float4((sp_DispatchThreadId.xy + float2(1.0f)) * ClusterSize.xy, 1.0f, 1.0f); // Top Right
  float4 minPoint_sS = float4(sp_DispatchThreadId.xy * ClusterSize.xy, 1.0f, 1.0f);                  // Bottom Left

  // Pass min and max to view space
  float3 maxPoint_vS = ScreenToView(maxPoint_sS).xyz;
  float3 minPoint_vS = ScreenToView(minPoint_sS).xyz;

  // Near and far values of the cluster in view space
  float clusterNear = Buffer_PerView.NearClipPlane * pow(Buffer_PerView.FarClipPlane / Buffer_PerView.NearClipPlane, sp_DispatchThreadId.z / float(CLUSTERS_Z));
  float clusterFar  = Buffer_PerView.NearClipPlane * pow(Buffer_PerView.FarClipPlane / Buffer_PerView.NearClipPlane, (sp_DispatchThreadId.z + 1) / float(CLUSTERS_Z));

  // Finding the 4 intersection points made from the MaxPoint to the cluster near/far plane
  float3 minPointNear = minPoint_vS * clusterNear / minPoint_vS.z;
  float3 minPointFar  = minPoint_vS * clusterFar / minPoint_vS.z;
  float3 maxPointNear = maxPoint_vS * clusterNear / maxPoint_vS.z;
  float3 maxPointFar  = maxPoint_vS * clusterFar / maxPoint_vS.z;

  float3 minPointAABB = min(min(minPointNear, minPointFar), min(maxPointNear, maxPointFar));
  float3 maxPointAABB = max(max(minPointNear, minPointFar), max(maxPointNear, maxPointFar));

  // Getting the cluster bounding box
  Clusters[clusterIndex].MinPoint = float4(minPointAABB, 1.0f);
  Clusters[clusterIndex].MaxPoint = float4(maxPointAABB, 1.0f);
}
