#include "model_rs.hlsl"
#include "brdf.hlsl"

struct ps_input
{
	float2 uv				: TEXCOORDS;
	float3x3 tbn			: TANGENT_FRAME;
	float3 worldPosition	: POSITION;
	float4 position			: SV_POSITION;
};

ConstantBuffer<pbr_material_cb> material : register(b1);


SamplerState wrapSampler				: register(s0);
SamplerState clampSampler				: register(s1);


Texture2D<float4> albedoTex				: register(t0);
Texture2D<float3> normalTex				: register(t1);
Texture2D<float> roughTex				: register(t2);
Texture2D<float> metalTex				: register(t3);

TextureCube<float4> irradianceTexture	: register(t0, space1);
TextureCube<float4> environmentTexture	: register(t1, space1);

Texture2D<float4> brdf					: register(t0, space2);


// TODO
static const float3 L = normalize(float3(1.f, 0.8f, 0.3f));
static const float3 sunColor = float3(1.f, 1.f, 1.f) * 50.f;
static const float3 cameraPosition = float3(0.f, 0.f, 4.f);

[RootSignature(MODEL_RS)]
float4 main(ps_input IN) : SV_TARGET
{
	uint flags = material.flags;

	float4 albedo = ((flags & USE_ALBEDO_TEXTURE)
		? albedoTex.Sample(wrapSampler, IN.uv)
		: float4(1.f, 1.f, 1.f, 1.f))
		* material.albedoTint;

	float3 N = (flags & USE_NORMAL_TEXTURE)
		? mul(normalTex.Sample(wrapSampler, IN.uv).xyz * 2.f - float3(1.f, 1.f, 1.f), IN.tbn)
		: IN.tbn[2];

	float roughness = (flags & USE_ROUGHNESS_TEXTURE)
		? roughTex.Sample(wrapSampler, IN.uv)
		: material.roughnessOverride;
	roughness = clamp(roughness, 0.01f, 0.99f);

	float metallic = (flags & USE_METALLIC_TEXTURE)
		? metalTex.Sample(wrapSampler, IN.uv)
		: material.metallicOverride;

	float ao = 1.f;// (flags & USE_AO_TEXTURE) ? RMAO.z : 1.f;

	
	float3 camToP = IN.worldPosition - cameraPosition.xyz;
	float3 V = -normalize(camToP);
	float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo.xyz, metallic);

	float4 totalLighting = float4(0.f, 0.f, 0.f, albedo.w);

	totalLighting.xyz += calculateAmbientLighting(albedo.xyz, irradianceTexture, environmentTexture, brdf, clampSampler, N, V, F0, roughness, metallic, ao);

	//float3 L = -sun.worldSpaceDirection.xyz;
	float visibility = 1.f;
	float3 radiance = sunColor.xyz * visibility; // No attenuation for sun.

	totalLighting.xyz += calculateDirectLighting(albedo.xyz, radiance, N, L, V, F0, roughness, metallic);

	return totalLighting;
}