#ifndef SKY_RS_HLSLI
#define SKY_RS_HLSLI

#define TERRAIN_LOD_0_VERTICES_PER_DIMENSION 129
#define TERRAIN_MAX_LOD 5


struct terrain_cb
{
	vec3 minCorner;
	uint32 lod;
	float chunkSize;
	uint32 scaleDownByLODs; // 4 * 8 bit.
};

#define SCALE_DOWN_BY_LODS(negX, posX, negZ, posZ) ((negX << 24) | (posX << 16) | (negZ << 8) | posZ)
#define DECODE_LOD_SCALE(v, negX, posX, negZ, posZ) negX = v >> 24, posX = (v >> 16) & 0xFF, negZ = (v >> 8) & 0xFF, posZ = v & 0xFF;

struct terrain_transform_cb
{
	mat4 vp;
};


#define TERRAIN_RS \
	"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |" \
	"DENY_HULL_SHADER_ROOT_ACCESS |" \
	"DENY_DOMAIN_SHADER_ROOT_ACCESS |" \
	"DENY_GEOMETRY_SHADER_ROOT_ACCESS)," \
	"RootConstants(num32BitConstants=16, b0, visibility=SHADER_VISIBILITY_VERTEX)," \
	"RootConstants(num32BitConstants=6, b1, visibility=SHADER_VISIBILITY_VERTEX)," \
	"DescriptorTable(SRV(t0, numDescriptors=1), visibility=SHADER_VISIBILITY_VERTEX)," \
	"DescriptorTable(SRV(t1, numDescriptors=1), visibility=SHADER_VISIBILITY_PIXEL)," \
	"StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR," \
        "visibility=SHADER_VISIBILITY_PIXEL), " \
	"StaticSampler(s1," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_POINT," \
        "visibility=SHADER_VISIBILITY_PIXEL)"


#define TERRAIN_RS_TRANSFORM	0
#define TERRAIN_RS_CB			1
#define TERRAIN_RS_HEIGHTMAP	2
#define TERRAIN_RS_NORMALMAP	3

#endif

