#include "cs.hlsl"
#include "camera.hlsl"
#include "light_culling.hlsl"

#define BLOCK_SIZE 16

ConstantBuffer<camera_cb> camera	: register(b0);
ConstantBuffer<frusta_cb> cb	    : register(b1);
RWStructuredBuffer<light_culling_view_frustum> outFrusta  : register(u0);


static light_culling_frustum_plane getPlane(float3 p0, float3 p1, float3 p2)
{
    light_culling_frustum_plane plane;

    float3 v0 = p1 - p0;
    float3 v2 = p2 - p0;

    plane.N = normalize(cross(v0, v2));
    plane.d = dot(plane.N, p0);

    return plane;
}

[RootSignature(WORLD_SPACE_TILED_FRUSTA_RS)]
[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void main(cs_input IN)
{
    float3 cameraPos = camera.position.xyz;

    float2 invScreenDims = camera.invScreenDims;

    float2 screenTL = invScreenDims * (IN.dispatchThreadID.xy * cb.tileSize);
    float2 screenTR = invScreenDims * (float2(IN.dispatchThreadID.x + 1, IN.dispatchThreadID.y) * cb.tileSize);
    float2 screenBL = invScreenDims * (float2(IN.dispatchThreadID.x, IN.dispatchThreadID.y + 1) * cb.tileSize);
    float2 screenBR = invScreenDims * (float2(IN.dispatchThreadID.x + 1, IN.dispatchThreadID.y + 1) * cb.tileSize);

    // Points on near plane.
    float3 tl = restoreWorldSpacePosition(camera.invVP, screenTL, 0.f);
    float3 tr = restoreWorldSpacePosition(camera.invVP, screenTR, 0.f);
    float3 bl = restoreWorldSpacePosition(camera.invVP, screenBL, 0.f);
    float3 br = restoreWorldSpacePosition(camera.invVP, screenBR, 0.f);

    light_culling_view_frustum frustum;
    frustum.planes[0] = getPlane(cameraPos, bl, tl);
    frustum.planes[1] = getPlane(cameraPos, tr, br);
    frustum.planes[2] = getPlane(cameraPos, tl, tr);
    frustum.planes[3] = getPlane(cameraPos, br, bl);

    if (IN.dispatchThreadID.x < cb.numThreadsX && IN.dispatchThreadID.y < cb.numThreadsY)
    {
        uint index = IN.dispatchThreadID.y * cb.numThreadsX + IN.dispatchThreadID.x;
        outFrusta[index] = frustum;
    }
}