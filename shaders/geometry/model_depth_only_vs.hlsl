#include "model_rs.hlsli"


ConstantBuffer<depth_only_transform_cb> transform : register(b0);

struct vs_input
{
	float3 position		: POSITION;
	float2 uv			: TEXCOORDS;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
};

struct vs_output
{
	float4 position			: SV_POSITION;
};

[RootSignature(MODEL_DEPTH_ONLY_RS)]
vs_output main(vs_input IN)
{
	vs_output OUT;
	float4x4 mvp = transform.mvp; // This local transform is necessary, because we do the same in the normal vertex shader. This makes the floating point math be the same in both passes.
	OUT.position = mul(mvp, float4(IN.position, 1.f));
	return OUT;
}
