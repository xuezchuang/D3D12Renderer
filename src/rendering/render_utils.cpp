#include "pch.h"
#include "render_utils.h"
#include "core/random.h"
#include "dx/dx_context.h"
#include "animation/skinning.h"
#include "texture_preprocessing.h"
#include "dx/dx_pipeline.h"
#include "render_resources.h"
#include "render_algorithms.h"
#include "pbr.h"
#include "scene/particle_systems.h"
#include "bitonic_sort.h"


static vec2 haltonSequence[128];
static uint64 skinningFence;


void initializeRenderUtils()
{
	initializeTexturePreprocessing();
	initializeSkinning();
	loadCommonShaders();

	pbr_material::initializePipeline();
	particle_system::initializePipeline();
	initializeBitonicSort();
	loadAllParticleSystemPipelines();


	createAllPendingReloadablePipelines();
	render_resources::initializeGlobalResources();

	for (uint32 i = 0; i < arraysize(haltonSequence); ++i)
	{
		haltonSequence[i] = halton23(i) * 2.f - vec2(1.f);
	}
}

void endFrameCommon()
{
	checkForChangedPipelines();
	skinningFence = performSkinning();
}

void buildCameraConstantBuffer(const render_camera& camera, float cameraJitterStrength, camera_cb& outCB)
{
	if (cameraJitterStrength > 0.f)
	{
		vec2 jitter = haltonSequence[dxContext.frameID % arraysize(haltonSequence)] / vec2((float)camera.width, (float)camera.height) * cameraJitterStrength;
		buildCameraConstantBuffer(camera.getJitteredVersion(jitter), jitter, outCB);
	}
	else
	{
		buildCameraConstantBuffer(camera, vec2(0.f, 0.f), outCB);
	}
}

void buildCameraConstantBuffer(const render_camera& camera, vec2 jitter, camera_cb& outCB)
{
	outCB.prevFrameViewProj = outCB.viewProj;
	outCB.viewProj = camera.viewProj;
	outCB.view = camera.view;
	outCB.proj = camera.proj;
	outCB.invViewProj = camera.invViewProj;
	outCB.invView = camera.invView;
	outCB.invProj = camera.invProj;
	outCB.position = vec4(camera.position, 1.f);
	outCB.forward = vec4(camera.rotation * vec3(0.f, 0.f, -1.f), 0.f);
	outCB.right = vec4(camera.rotation * vec3(1.f, 0.f, 0.f), 0.f);
	outCB.up = vec4(camera.rotation * vec3(0.f, 1.f, 0.f), 0.f);
	outCB.projectionParams = vec4(camera.nearPlane, camera.farPlane, camera.farPlane / camera.nearPlane, 1.f - camera.farPlane / camera.nearPlane);
	outCB.screenDims = vec2((float)camera.width, (float)camera.height);
	outCB.invScreenDims = vec2(1.f / camera.width, 1.f / camera.height);
	outCB.prevFrameJitter = outCB.jitter;
	outCB.jitter = jitter;
}

void assignSunShadowMapViewports(const sun_shadow_render_pass* sunShadowRenderPass, directional_light_cb& sun)
{
	if (sunShadowRenderPass)
	{
		for (uint32 i = 0; i < sun.numShadowCascades; ++i)
		{
			auto vp = sunShadowRenderPass->viewports[i];
			sun.viewports[i] = vec4(vp.x, vp.y, vp.size, vp.size) / vec4((float)SHADOW_MAP_WIDTH, (float)SHADOW_MAP_HEIGHT, (float)SHADOW_MAP_WIDTH, (float)SHADOW_MAP_HEIGHT);
		}
	}
}

void waitForSkinningToFinish()
{
	if (skinningFence)
	{
		dxContext.renderQueue.waitForOtherQueue(dxContext.computeQueue); // Wait for GPU skinning.
	}
}
