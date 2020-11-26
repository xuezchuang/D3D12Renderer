#pragma once

#include "bounding_volumes.h"
#include "dx_render_primitives.h"

struct aiScene;
struct pbr_material;


struct single_mesh
{
	submesh_info submesh;
	bounding_box boundingBox;
	ref<pbr_material> material;
	std::string name;
};

struct lod_mesh
{
	uint32 firstMesh;
	uint32 numMeshes;
};

struct composite_mesh
{
	std::vector<single_mesh> singleMeshes;
	std::vector<lod_mesh> lods;
	std::vector<float> lodDistances;
	dx_mesh mesh;
};


const aiScene* loadAssimpSceneFile(const char* filepath);
void freeAssimpScene(const aiScene* scene);
composite_mesh loadMeshFromScene(const aiScene* scene, uint32 flags, bool loadMaterials = false);
composite_mesh loadMeshFromFile(const char* sceneFilename, uint32 flags, bool loadMaterials = false);
