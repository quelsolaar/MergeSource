#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "verse.h"
#include "enough.h"
#include "seduce.h"
#include "la_draw_overlay.h"
#define LO_PREVIEW_SIZE 512

typedef struct {
	uint texture_id;
	uint version;
	float pos[3];
}LoPreviewTexture;

extern boolean draw_persuade_surface_queue_preview(ENode *node);
extern boolean draw_persuade_surface_ready_preview();
extern boolean draw_persuade_surface_draw_preview();

void lo_geo_preview_func(ENode *node, ECustomDataCommand command)
{
	if(command == E_CDC_DESTROY)
	{
		LoPreviewTexture *data;
		data = e_ns_get_custom_data(node, 0);
		if(data->texture_id != -1)
		r_texture_free(data->texture_id);
		free(data);
	}
}

uint lo_preview_render(uint texture_id)
{
	static uint depth_id = -1;
	RMatrix matrix, *save; 
	float size;
	uint x, y;
	void *fbo;
	char *empty = seduce_translate("EMPTY");
	if(texture_id == -1)
		texture_id = r_texture_allocate(R_IF_RGBA_UINT8, LO_PREVIEW_SIZE, LO_PREVIEW_SIZE, 1, TRUE, FALSE, NULL);
	if(depth_id == -1)
		depth_id = r_texture_allocate(R_IF_DEPTH32, LO_PREVIEW_SIZE, LO_PREVIEW_SIZE, 1, TRUE, FALSE, NULL);
	fbo = r_framebuffer_allocate(&texture_id, 1, depth_id, RELINQUISH_TARGET_2D_TEXTURE);
	r_framebuffer_bind(fbo);
	glClearColor(0, 0, 0, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	r_viewport(0, 0, LO_PREVIEW_SIZE, LO_PREVIEW_SIZE);
	r_matrix_identity(&matrix);
	r_matrix_frustum(&matrix, -0.001, 0.001, -0.001, 0.001, 0.01, 100.0);
	r_matrix_translate(&matrix, 0, 0, -10);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	save = r_matrix_get();
	r_matrix_set(&matrix);
	if(!draw_persuade_surface_draw_preview())
	{
		size = seduce_text_line_length(NULL, 1, SEDUCE_T_SPACE, empty, -1);
		seduce_text_line_draw(NULL, -0.75, 0, 1.5 / size, SEDUCE_T_SPACE, empty, 1, 1, 1, 1, -1);
	}
	r_framebuffer_bind(NULL);
//	r_texture_free(depth_id);
	r_framebuffer_free(fbo);
	r_matrix_set(save);
	betray_screen_mode_get(&x, &y, NULL);
	r_viewport(0, 0, x, y);
	return texture_id;
}

void lo_preview_update()
{
	uint node_id;
	LoPreviewTexture *texture;
	ENode *node;
	e_ns_set_custom_func(0, V_NT_GEOMETRY, lo_geo_preview_func);
	if(draw_persuade_surface_ready_preview(&node_id))
	{
		node = e_ns_get_node(0, node_id);
		if(node != NULL)
		{
			texture = e_ns_get_custom_data(node, 0);
			if(texture != NULL)
				texture->texture_id = lo_preview_render(texture->texture_id);
		}
	}
	for(node = e_ns_get_node_next(0, 0, V_NT_GEOMETRY); node != NULL; node = e_ns_get_node_next(e_ns_get_node_id(node) + 1, 0, V_NT_GEOMETRY))
	{
		texture = e_ns_get_custom_data(node, 0);
		if(texture == NULL)
		{
			texture = malloc(sizeof *texture);
			texture->texture_id = -1;
			texture->version = 0;
			texture->pos[0] = 0;
			texture->pos[1] = 0;
			texture->pos[2] = 0;
			e_ns_set_custom_data(node, 0, texture);
		}	
		if(texture->version != e_ns_get_node_version_data(node))
		{
			if(draw_persuade_surface_queue_preview(node))
				texture->version = e_ns_get_node_version_data(node);				
			else
				return;
		}
	}
}
uint lo_preview_texture_get(ENode *node)
{
	LoPreviewTexture *texture;
	texture = e_ns_get_custom_data(node, 0);
	if(texture == NULL)
		return -1;
	return texture->texture_id;
}

void lo_preview_texture_get_pos(ENode *node, float *pos)
{
	LoPreviewTexture *texture;
	texture = e_ns_get_custom_data(node, 0);
	if(texture != NULL)
	{
		pos[0] = texture->pos[0];
		pos[1] = texture->pos[1];
		pos[2] = texture->pos[2];
	}
}

void lo_preview_texture_set_pos(ENode *node, float *pos)
{
	LoPreviewTexture *texture;
	texture = e_ns_get_custom_data(node, 0);
	if(texture != NULL)
	{
		texture->pos[0] = pos[0];
		texture->pos[1] = pos[1];
		texture->pos[2] = pos[2];
	}
}

	