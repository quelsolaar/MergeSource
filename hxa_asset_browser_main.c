#include <math.h>
#include <stdlib.h>
#include <stdio.h> 
#include "betray.h"
#include "imagine.h"
#include "seduce.h"
#include "hxa.h"
#include "hxa_utils.h"

#define HXA_NAME_SIZE_MAX 128
#define HXA_FILE_EXTENTION_SIZE 5
#define HXA_PATH_SIZE_MAX (256 + HXA_NAME_SIZE_MAX + HXA_FILE_EXTENTION_SIZE) 

typedef struct{	
	char path[HXA_PATH_SIZE_MAX];
	char name[HXA_NAME_SIZE_MAX];
	HXAFile *file;
	uint texture_id;
	float size;
	float center[2];
	float x;
	float y;
}HxAAsset;

typedef struct{
	HxAAsset *assets;
	uint asset_count;
	HxAAsset add_assets;
	uint resolution;
	uint depth_buffer;
	boolean create_active;
	float create_timer;
	char create_name[64];
	char path[HXA_PATH_SIZE_MAX];
	float objects_size;
	float scroll;
	void (*custom_render)(HXANode *node, void *fbo, float *center, float scale, uint resolution, void *user_pointer);
	void *user_pointer;
	char filter[64];
}HxAAssetBrowserSession;

HxAAssetBrowserSession *hxa_asset_browser_create(uint resolution, void (*custom_render)(HXANode *node, void *fbo, float *center, float scale, uint resolution, void *user_pointer), void *user_pointer)
{
	HxAAssetBrowserSession *session;
	session = malloc(sizeof *session);
	session->assets = NULL;
	session->asset_count = 0;
	session->resolution = resolution; 
	session->depth_buffer = ~0;
	session->create_active = FALSE;
	session->create_timer = 0;
	session->create_name[0] = 0;
	session->path[0] = '.';
	session->path[1] = '\n';
	session->objects_size = 0.1;
	session->scroll = 0;
	session->add_assets.x = -2;
	session->add_assets.y = 0;
	session->custom_render = custom_render;
	session->user_pointer = user_pointer;
	session->filter[0] = '\0';
	return session;
}

void hxa_asset_browser_destroy(HxAAssetBrowserSession *session)
{
	uint i;
	if(session->asset_count != 0)
	{
		for(i = 0; i < session->asset_count; i++)
		{
			hxa_util_free_file(session->assets[i].file);
			r_texture_free(session->assets[i].texture_id);
		}
		free(session->assets);
	}
	free(session);
}


float hxa_asset_browser_object_texture_wireframe_draw(HXANode *node)
{
	uint i, last, next, first;
	int32 *ref;
	float *v, scale = 1;	
	v = node->content.geometry.vertex_stack.layers->data.float_data;
	ref = node->content.geometry.corner_stack.layers->data.int32_data;

	last = first = ref[0];
	for(i = 1; i < node->content.geometry.edge_corner_count; i++)
	{
		if(ref[i] >= 0)
		{
			next = ref[i];
			r_primitive_line_3d(v[last * 3] * scale, v[last * 3 + 1] * scale, v[last * 3 + 2] * scale,
								v[next * 3] * scale, v[next * 3 + 1] * scale, v[next * 3 + 2] * scale, 1, 1, 1, 1);
			last = next;
		}
		else
		{
			next = -ref[i] - 1;
			r_primitive_line_3d(v[last * 3] * scale, v[last * 3 + 1] * scale, v[last * 3 + 2] * scale,
								v[next * 3] * scale, v[next * 3 + 1] * scale, v[next * 3 + 2] * scale, 1, 1, 1, 1);
			r_primitive_line_3d(v[first * 3] * scale, v[first * 3 + 1] * scale, v[first * 3 + 2] * scale,
								v[next * 3] * scale, v[next * 3 + 1] * scale, v[next * 3 + 2] * scale, 1, 1, 1, 1);
			while(i < node->content.geometry.edge_corner_count && ref[i] < 0)
				i++;
			if(i == node->content.geometry.edge_corner_count)
				return scale;
			last = first = ref[i];
		}
	}
	return scale;
}


void hxa_asset_browser_object_texture_draw(HxAAssetBrowserSession *session, HxAAsset *asset)
{
	RMatrix m, *save;
	void *fbo;
	uint i;
	float *v, min_max[4], scale, center[2];
	if(session->depth_buffer == ~0)
		session->depth_buffer = r_texture_allocate(R_IF_DEPTH32, session->resolution, session->resolution, 1, TRUE, FALSE, NULL);
	fbo = r_framebuffer_allocate(&asset->texture_id, 1, session->depth_buffer, RELINQUISH_TARGET_2D_TEXTURE); 
	if(fbo != NULL)
	{
		save = r_matrix_get();
		r_framebuffer_bind(fbo);
		r_framebuffer_clear(0, 0, 0, 0, TRUE, TRUE);
		r_matrix_set(&m);
		r_matrix_identity(&m);
		r_matrix_frustum(&m, -0.005, 0.005, -0.005, 0.005, 0.005, 100.0f);
		r_viewport(0, 0, session->resolution, session->resolution);

		if(asset->file->node_array->type == HXA_NT_GEOMETRY && asset->file->node_array->content.geometry.edge_corner_count != 0)
		{
			v = asset->file->node_array->content.geometry.vertex_stack.layers->data.float_data;
			asset->size = 0;
			min_max[0] = min_max[1] = 1000000000.0;
			min_max[2] = min_max[3] = -1000000000.0;
			for(i = 0; i < asset->file->node_array->content.geometry.vertex_count * 3; i += 3)
			{
				if(min_max[0] > v[i])
					min_max[0] = v[i];
				if(min_max[2] < v[i])
					min_max[2] = v[i];
				if(min_max[1] > v[i + 1])
					min_max[1] = v[i + 1];
				if(min_max[3] < v[i + 1])
					min_max[3] = v[i + 1];
				if(v[i] > asset->size)
					asset->size = v[i];
				if(v[i] < -asset->size)
					asset->size = -v[i];
				if(v[i + 1] > asset->size)
					asset->size = v[i + 1];
				if(v[i + 1] < -asset->size)
					asset->size = -v[i + 1];
			}
			if((min_max[2] - min_max[0]) > (min_max[3] - min_max[1]))
				asset->size = min_max[2] - min_max[0];
			else
				asset->size = min_max[3] - min_max[1];				
			scale = 1.8 / asset->size;

			center[0] = (min_max[2] + min_max[0]) * -0.5;
			center[1] = (min_max[3] + min_max[1]) * -0.5;

			r_matrix_scale(&m, scale, scale, 1.0);
			r_matrix_translate(&m, center[0], center[1], -1.0);

 			if(session->custom_render != NULL)
				session->custom_render(&asset->file->node_array[0], fbo, center, scale, session->resolution, session->user_pointer);
			else
				hxa_asset_browser_object_texture_wireframe_draw(&asset->file->node_array[0]);
			r_primitive_line_flush();
		}
		r_framebuffer_free(fbo);
		r_matrix_set(save);
	}
}

void hxa_asset_browser_add_asset(HxAAssetBrowserSession *session, HXAFile *file, char *path, char *name)
{
	uint i;
	if(session->asset_count % 16 == 0)
		session->assets = realloc(session->assets, (sizeof *session->assets) * (session->asset_count + 16));
	if(path != NULL)
	{
		for(i = 0; i < HXA_PATH_SIZE_MAX - 1 && path[i] != 0; i++)
			session->assets[session->asset_count].path[i] = path[i];
		session->assets[session->asset_count].path[i] = 0;
	}else
		session->assets[session->asset_count].path[0] = 0;
	if(name != NULL)
	{
		for(i = 0; i < HXA_NAME_SIZE_MAX - 1 && name[i] != 0; i++)
			session->assets[session->asset_count].name[i] = name[i];
		session->assets[session->asset_count].name[i] = 0;
	}else
	{
		session->assets[session->asset_count].name[0] = 0;
	}

	session->assets[session->asset_count].file = file;
	session->assets[session->asset_count].texture_id = ~0;
	session->assets[session->asset_count].x = -1.5 + 0.5 * f_randnf(session->asset_count);
	session->assets[session->asset_count].y = f_randnf(session->asset_count + 1);
	session->assets[session->asset_count].texture_id = r_texture_allocate(R_IF_RGBA_UINT8, session->resolution, session->resolution, 1, TRUE, FALSE, NULL);
	hxa_asset_browser_object_texture_draw(session, &session->assets[session->asset_count]);
	session->asset_count++;
}

boolean hxa_asset_browser_in_directory(HxAAssetBrowserSession *session, char *name)
{
	uint i, j;
	if(session == NULL)
		return FALSE;
	for(i = 0; i < session->asset_count; i++)
	{
		for(j = 0; name[j] != 0 && session->assets[i].name[j] == name[j]; j++);
		if(session->assets[i].name[j] == name[j])
			return TRUE;
	}
	return FALSE;
}


void hxa_asset_browser_add_directory(HxAAssetBrowserSession *session, char *path, HxAAssetBrowserSession *unless_in_session)
{
	HXAFile *file;
	char file_name_buffer[HXA_PATH_SIZE_MAX], name[HXA_NAME_SIZE_MAX];
	uint i, j, start;

	for(i = 0; i < HXA_PATH_SIZE_MAX - 1 && path[i] != '\0'; i++)
		session->path[i] = path[i];
	session->path[i] = '\0';
	for(start = 0; start < HXA_PATH_SIZE_MAX - HXA_NAME_SIZE_MAX && path[start] != '\0'; start++)
		file_name_buffer[start] = path[start];
	if(start == HXA_PATH_SIZE_MAX - HXA_NAME_SIZE_MAX)
	{
		printf("HXA Error: hxa_asset_browser_add_directory trying to add too long path\n");
		return;
	}

	for(i = 0; imagine_path_search("hxa", TRUE, path, TRUE, i, &file_name_buffer[start], HXA_NAME_SIZE_MAX); i++)
	{
		for(j = 0; j < HXA_NAME_SIZE_MAX - 1 && file_name_buffer[start + j] != '\0' && file_name_buffer[start + j] != '.'; j++)
		{
			if(file_name_buffer[start + j] == '_')
				name[j] = ' ';
			else
				name[j] = file_name_buffer[start + j];
		}
		name[j] = '\0';
		if(!hxa_asset_browser_in_directory(unless_in_session, name))
		{
			file = hxa_load(file_name_buffer, TRUE);
			if(file != NULL && file->node_count != 0)
			{
				hxa_util_meta_set(&file->node_array->meta_data, &file->node_array->meta_data_count, "AssetName", HXA_MDT_TEXT, name, 0, TRUE);
				hxa_asset_browser_add_asset(session, file, file_name_buffer, name);
			}
		}
	}
}

boolean hxa_asset_browser_asset_return(HxAAssetBrowserSession *session, HXAFile *file, boolean save)
{
	uint asset, i, j, k, start;
	char *name, new_path[HXA_PATH_SIZE_MAX], new_name[HXA_NAME_SIZE_MAX], number[16];
	for(asset = 0; asset < session->asset_count && session->assets[asset].file != file; asset++);
	if(asset == session->asset_count)
		return FALSE;

	if(save)
	{
		new_name[0] = '\0';
		name = session->path;
		for(i = 0; name[i] != '\0'; i++)
			new_path[i] = name[i];
		name = hxa_util_meta_data_get(file->node_array[0].meta_data, file->node_array[0].meta_data_count, "AssetName", HXA_MDT_TEXT, NULL, FALSE);
		if(name == NULL)
		{
			name = "Unnamed";
			for(j = 0; j < HXA_NAME_SIZE_MAX - 16 && name[j] != '\0'; j++)
				new_name[j] = name[j];
			new_name[j] = '\0';
		}else
		{
			for(j = k = 0; i < HXA_NAME_SIZE_MAX && name[j] != '\0'; j++)	
			{
				if((name[j] >= 'A' && name[j] <= 'Z') ||
				   (name[j] >= 'a' && name[j] <= 'z') ||
				   (name[j] >= '0' && name[j] <= '9') ||
				   name[j] == '_' || 
				   name[j] == ' ')
				{
					if(name[j] == ' ')
						new_name[k++] = '_';
					else
						new_name[k++] = name[j];
				}
			}
			new_name[k] = 0;
		}
		for(j = 0; i < HXA_PATH_SIZE_MAX - 20 && new_name[j] != '\0'; j++)	
			new_path[i++] = new_name[j];
		start = i;

		i = 1;
		do{
			if(i > 1)
			{
				sprintf(number, "%u", i);
				for(j = 0; new_name[j] != 0 && j < HXA_NAME_SIZE_MAX - 20; j++);
				for(k = 0; number[k] != 0; k++)
					new_name[j++] = number[k];
				new_name[j] = 0;
				j = start;
				for(k = 0; number[k] != 0; k++)
					new_path[j++] = number[k];
			}else
				j = start;
			new_path[j++] = '.';
			new_path[j++] = 'h';
			new_path[j++] = 'x';
			new_path[j++] = 'a';
			new_path[j] = '\0';
			for(j = 0; j < session->asset_count; j++)
			{
				if(j != asset)
				{
					for(k = 0; new_path[k] != 0 && new_path[k] == session->assets[asset].path[k]; k++);
					if(new_path[k] == session->assets[asset].path[k])
						break;
				}
			}
			if(j == session->asset_count)
			{
				imagine_path_remove(session->assets[asset].path);
				for(j = 0; new_path[j] != 0; j++)
					session->assets[asset].path[j] = new_path[j];
				session->assets[asset].path[j] = 0;
			}
			i++;
		} while(session->assets[asset].path[0] == 0);	

		hxa_util_meta_set(&file->node_array->meta_data, &file->node_array->meta_data_count, "AssetName", HXA_MDT_TEXT, new_name, 0, TRUE);
		hxa_save(session->assets[asset].path, session->assets[asset].file);
	}
	hxa_asset_browser_object_texture_draw(session, &session->assets[asset]);
	return TRUE;
}

boolean hxa_asset_browser_remove_asset(HxAAssetBrowserSession *session, HXAFile *file)
{
	uint i;
	for(i = 0; i < session->asset_count && session->assets[i].file != file; i++);
	if(i == session->asset_count)
		return FALSE;
	if(session->assets[i].texture_id != ~0)
		r_texture_free(session->assets[i].texture_id);
	for(session->asset_count--; i < session->asset_count; i++)
		session->assets[i] = session->assets[i + 1];
	return TRUE;
}

char *hxa_asset_browser_name_get(HxAAssetBrowserSession *session, HXAFile *file)
{
	uint i;
	for(i = 0; i < session->asset_count && session->assets[i].file != file; i++);
	if(i == session->asset_count)
		return NULL;
	return session->assets[i].name;
}

char *hxa_asset_browser_path_get(HxAAssetBrowserSession *session, HXAFile *file)
{
	uint i;
	for(i = 0; i < session->asset_count && session->assets[i].file != file; i++);
	if(i == session->asset_count)
		return NULL;
	return session->assets[i].path;
}

HXAFile	*hxa_asset_browser_get_by_name(HxAAssetBrowserSession *session, char *name)
{
	uint i, j;
	for(i = 0; i < session->asset_count; i++)
	{
		for(j = 0; j < name[j] != 0 && session->assets[i].name[j] == name[i]; j++);
		if(session->assets[i].name[j] == name[i])
			return session->assets[i].file;
	}
	return NULL;
}


void io_input_asset_browser_object_draw(BInputState *input, HxAAssetBrowserSession *session, HxAAsset *asset, float x, float y, float size, float scale)
{
	float text_size, brightness = 0.1;
	if(session->filter[0] == '\0' || f_text_filter(asset->name, session->filter))
		brightness = 1;
	r_primitive_image(x - size, y - size, 0, size * 2.0, size * 2.0, 0, 0, 1, 1, asset->texture_id, brightness, brightness, brightness, brightness); /*draws a textured and shaded rectangle.*/
	text_size = seduce_text_line_length(NULL, scale, SEDUCE_T_SPACE, asset->name, -1);
	if(text_size < size * 1.6)
		seduce_text_line_draw(NULL, x - text_size * 0.5, y - size + scale, scale, SEDUCE_T_SPACE, asset->name, 0.2, 0.6, 1, 1, -1);
	else
		seduce_text_line_draw(NULL, x - size * 0.8, y - size + scale, scale * (size * 1.6 / text_size), SEDUCE_T_SPACE, asset->name, 0.2, 0.6, 1, 1, -1);
	seduce_element_add_rectangle(input, asset, 0, x - size, y - size, size * 2.0, size * 2.0);
}
extern void hxa_util_primitive_empty(HXAFile *file);

HXAFile *hxa_asset_browser_create_panel(BInputState *input, HxAAssetBrowserSession *session)
{
	static float matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	SeducePanelElement elements[3];
	float  color[4] = {1, 1, 1, 1}, baground_color[4] = {0, 0, 0, 0.7};
	uint i, j;
	HXAFile *hxa;
	char path[HXA_PATH_SIZE_MAX];
	seduce_animate(input, &session->create_timer, session->create_active, 1.0);

	elements[0].text = "Asset Name";
	elements[0].description = NULL;
	elements[0].type = SEDUCE_PET_TEXT;
	memcpy(elements[0].param.text, session->create_name, 64);
	elements[1].text = "Create";
	elements[1].description = NULL;
	elements[1].type = SEDUCE_PET_OK_CANCEL;
	elements[1].param.ok_cancel = SEDUCE_PEOCS_UNDECIDED;
	elements[2].text = "";
	elements[2].description = NULL;
	elements[2].type = SEDUCE_PET_CLOSE_WINDOW;
	elements[2].param.close_window = FALSE;

	
	seduce_background_shape_matrix_interact(input, hxa_asset_browser_create_panel, matrix, TRUE, TRUE);
	if(input->mode == BAM_DRAW)
	{
		r_matrix_push(NULL);
		r_matrix_matrix_mult(NULL, matrix);
	}
	seduce_widget_list(input, hxa_asset_browser_create_panel, elements, 3, session->create_timer, "Create Asset", color, baground_color, SEDUCE_WLS_PANEL);
	if(input->mode == BAM_DRAW)
		r_matrix_pop(NULL);	
	memcpy(session->create_name, elements[0].param.text, 64);
	if(elements[2].param.close_window || 
	   elements[1].param.ok_cancel == SEDUCE_PEOCS_CANCEL)
		session->create_active = FALSE;
	if(elements[1].param.ok_cancel == SEDUCE_PEOCS_OK && session->create_name != '\0')
	{
		hxa = malloc(sizeof *hxa);
		hxa->node_array = NULL;
		hxa->node_count = 0;
		hxa->version = 1;
		hxa_util_primitive_empty(hxa);
		hxa_util_meta_set(&hxa->node_array->meta_data, &hxa->node_array->meta_data_count, "AssetName", HXA_MDT_TEXT, session->create_name, 0, TRUE);
		hxa_asset_browser_add_asset(session, hxa, NULL, session->create_name);
		session->create_active = FALSE;
		return hxa;
	}

	return NULL;
}
 

HXAFile *hxa_asset_browser_draw(BInputState *input, HxAAssetBrowserSession *session, float top, float bottom, float size, boolean create_new)
{
	float x, y, object_size, object_space, time = 1, d, pos_a[3] = {-1, 0, 0}, pos_b[3] = {1, 0, 0};
	STypeInState state;
	uint i, j, count;
	 /* Creates a linear slider between two points. The snaps option enables rulers for precise values. */
	object_size = 1.0 / session->objects_size;
	state = seduce_widget_slider_line(input, &session->objects_size, &object_size, -1 + size, top - size, 0, top - size, size, 1.0, 20.0, top - bottom, time, NULL, FALSE);
	if(S_TIS_IDLE != state)
		session->objects_size = 1.0 / object_size;
	top -= size * 2.0;
	bottom += size * 2.0;
	object_size = session->objects_size;
	count = (uint)((top - bottom) / object_size);
	if(count == 0)
	{
		object_size = top - bottom;
		count = 1;
	}
	object_space = ((top - bottom) / count);

	pos_a[1] = pos_b[1] = bottom - size; 
	seduce_manipulator_slider(input, &session->scroll, &session->scroll, pos_a, pos_b, object_space * (float)((session->asset_count + count - 1) / count), 2.0, size, 1, 1, 1);

	seduce_text_edit_line(input, session->filter, NULL, session->filter, 64, 0.05, top + size * 0.75, 1.0, size * 0.5, "Filter", TRUE, NULL, NULL, 0.2, 0.6, 1, 1, 0.2, 0.6, 1, 1);

	if(S_TIS_IDLE == state)
	{
		if(input->mode == BAM_MAIN)
			session->objects_size = session->objects_size * (1.0 - input->delta_time * 4) + ((top - bottom) / count) * input->delta_time * 4;
	}
	if(input->mode == BAM_MAIN)
	{
		d = input->delta_time * 5.0;
		if(d > 0.1)
			d = 0.1;
		top += object_space * 0.5;
		for(i = 0; i < session->asset_count; i++)
		{
			session->assets[i].y = session->assets[i].y * (1.0 - d) + (top - ((i % count) + 1) * object_space) * d;
			session->assets[i].x = session->assets[i].x * (1.0 - d) + (-1.0 + object_space * 0.5 + ((i / count)) * object_space) * d;
		}
		session->add_assets.y = session->add_assets.y * (1.0 - d) + (top - ((i % count) + 1) * object_space) * d;
		session->add_assets.x = session->add_assets.x * (1.0 - d) + (-1.0 + object_space * 0.5 + ((i / count)) * object_space) * d;
	}


	if(input->mode == BAM_DRAW)
	{
		for(i = 0; i < session->asset_count; i++)
			
				io_input_asset_browser_object_draw(input, session, &session->assets[i],  -session->scroll + session->assets[i].x, session->assets[i].y, object_size * 0.5, SEDUCE_T_SIZE * 2.0);

//			io_input_asset_browser_object_draw(input, session, &session->add_assets,  -session->scroll + session->add_assets.x, session->add_assets.y, object_size * 0.5, SEDUCE_T_SIZE * 2.0);
	}
	if(create_new)
	{
		if(seduce_widget_button_icon(input, &session->add_assets, SEDUCE_OBJECT_ADD, session->add_assets.x, session->add_assets.y, object_size * 0.5, 1, NULL))
		{
			session->create_name[0] = '\0';
			session->create_active = TRUE;
		}
	}
	

	for(i = 0; i < input->pointer_count; i++)
		if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
			for(j = 0; j < session->asset_count; j++)
				if(&session->assets[j] == seduce_element_pointer_id(input, i, NULL))
					return session->assets[j].file;
	return hxa_asset_browser_create_panel(input, session);;
}

void io_input_asset_browser_handler(BInputState *input, void *user_pointer)
{
	HxAAssetBrowserSession *session;
	float aspect = 1;
	session = user_pointer;
	if(input->mode == BAM_DRAW)
	{
		uint x, y;
		r_matrix_identity(NULL);
		aspect = betray_screen_mode_get(&x, &y, NULL);
		r_matrix_frustum(NULL, -0.005, 0.005, -0.005 * aspect, 0.005 * aspect, 0.005, 100.0f);
		r_viewport(0, 0, x, y);
		r_framebuffer_clear(1, 1, 1, 0, TRUE, TRUE);
	}
	seduce_element_endframe(input, FALSE);
}


uint hxa_asset_browser_asset_count_get(HxAAssetBrowserSession *session)
{
	return session->asset_count;
}

uint hxa_asset_browser_asset_lookup_by_name(HxAAssetBrowserSession *session, char *name)
{
	uint i, j;
	for(i = 0; i < session->asset_count; i++)
	{
		for(j = 0; (name[j] == session->assets[i].name[j] || (name[j] == '_' && session->assets[i].name[j] == ' ')) && name[j] != '\0'; j++);
		if(name[j] == session->assets[i].name[j])
			return i;
	}
	return ~0;
}

char *hxa_asset_browser_asset_name_get(HxAAssetBrowserSession *session, uint id)
{
	return session->assets[id].name;
}

uint hxa_asset_browser_asset_texture_id_get(HxAAssetBrowserSession *session, uint id)
{
	return session->assets[id].texture_id;
}

char *hxa_asset_browser_asset_path_get(HxAAssetBrowserSession *session, uint id)
{
	return session->assets[id].path;
}

float hxa_asset_browser_asset_size_get(HxAAssetBrowserSession *session, uint id)
{
	return session->assets[id].size;
}

HXAFile *hxa_asset_browser_asset_file_get(HxAAssetBrowserSession *session, uint id)
{
	return session->assets[id].file;
}