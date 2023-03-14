#pragma once

#include "core/math.h"
#include "material.h"
#include "render_command.h"

struct dx_command_list;

enum pbr_material_shader
{
	pbr_material_shader_default,
	pbr_material_shader_double_sided,
	pbr_material_shader_alpha_cutout,

	pbr_material_shader_count,
};

static const char* pbrMaterialShaderNames[] =
{
	"Default",
	"Double sided",
	"Alpha cutout",
};

struct pbr_material
{
	pbr_material() = default;
	pbr_material(ref<dx_texture> albedo, 
		ref<dx_texture> normal, 
		ref<dx_texture> roughness, 
		ref<dx_texture> metallic, 
		const vec4& emission, 
		const vec4& albedoTint, 
		float roughnessOverride, 
		float metallicOverride,
		pbr_material_shader shader,
		float uvScale,
		float translucency)
		: albedo(albedo), 
		normal(normal), 
		roughness(roughness), 
		metallic(metallic), 
		emission(emission), 
		albedoTint(albedoTint), 
		roughnessOverride(roughnessOverride), 
		metallicOverride(metallicOverride),
		shader(shader),
		uvScale(uvScale),
		translucency(translucency) {}

	ref<dx_texture> albedo;
	ref<dx_texture> normal;
	ref<dx_texture> roughness;
	ref<dx_texture> metallic;

	vec4 emission;
	vec4 albedoTint;
	float roughnessOverride;
	float metallicOverride;
	pbr_material_shader shader;
	float uvScale;
	float translucency;
};

struct pbr_render_data
{
	mat4 transform;
	dx_vertex_buffer_group_view vertexBuffer;
	dx_index_buffer_view indexBuffer;
	submesh_info submesh;

	ref<pbr_material> material;
};

struct pbr_pipeline
{
	using render_data_t = pbr_render_data;

	static void initialize();

	PIPELINE_RENDER_DECL;

	struct opaque;
	struct opaque_double_sided;
	struct transparent;

protected:
	static void setupPBRCommon(dx_command_list* cl, const common_render_data& common);
};

struct pbr_pipeline::opaque : pbr_pipeline
{
	PIPELINE_SETUP_DECL;
};

struct pbr_pipeline::opaque_double_sided : pbr_pipeline
{
	PIPELINE_SETUP_DECL;
};

struct pbr_pipeline::transparent : pbr_pipeline
{
	PIPELINE_SETUP_DECL;
};



ref<pbr_material> createPBRMaterial(const fs::path& albedoTex, const fs::path& normalTex, const fs::path& roughTex, const fs::path& metallicTex,
	const vec4& emission = vec4(0.f), const vec4& albedoTint = vec4(1.f), float roughOverride = 1.f, float metallicOverride = 0.f, 
	pbr_material_shader shader = pbr_material_shader_default, float uvScale = 1.f, float translucency = 0.f, bool disableTextureCompression = false);
ref<pbr_material> createPBRMaterial(const ref<dx_texture>& albedoTex, const ref<dx_texture>& normalTex, const ref<dx_texture>& roughTex, const ref<dx_texture>& metallicTex,
	const vec4& emission = vec4(0.f), const vec4& albedoTint = vec4(1.f), float roughOverride = 1.f, float metallicOverride = 0.f, 
	pbr_material_shader shader = pbr_material_shader_default, float uvScale = 1.f, float translucency = 0.f);

ref<pbr_material> getDefaultPBRMaterial();
