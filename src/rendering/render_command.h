#pragma once

#include "geometry/mesh.h"
#include "shadow_map_cache.h"

struct particle_draw_info
{
	ref<dx_buffer> particleBuffer;
	ref<dx_buffer> aliveList;
	ref<dx_buffer> commandBuffer;
	uint32 aliveListOffset;
	uint32 commandBufferOffset;
	uint32 rootParameterOffset;
};


template <typename render_data_t>
struct render_command
{
	render_data_t data;

	render_command() {}
	render_command(const render_data_t& data)
		: data(data) {}
	render_command(render_data_t&& data)
		: data(data) {}
};

template <typename render_data_t>
struct particle_render_command
{
	dx_vertex_buffer_group_view vertexBuffer;
	dx_index_buffer_view indexBuffer;
	particle_draw_info drawInfo;

	render_data_t data;
};

template <typename render_data_t>
struct depth_only_render_command
{
	uint32 objectID;
	render_data_t data;

	depth_only_render_command() {}
	depth_only_render_command(uint32 objectID, const render_data_t& data)
		: objectID(objectID), data(data) {}
	depth_only_render_command(uint32 objectID, render_data_t&& data)
		: objectID(objectID), data(data) {}
};




