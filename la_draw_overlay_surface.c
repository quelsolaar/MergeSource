#include "la_includes.h"

#include "la_geometry_undo.h"
#include "la_projection.h"
#include "la_tool.h"
#include "forge.h"
#include "la_particle_fx.h"
#include "la_draw_overlay.h"





extern uint lo_pointer_polygon;

char *la_surface_front_shader_vertex = 
"attribute vec3 vertex;"
"attribute vec3 normal;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform mat4 NormalMatrix;"
"uniform vec3 camera;"
"varying vec3 n;"
"varying vec3 pos;"
"varying vec3 n2;"
"varying vec3 camera_vec;"
"void main()"
"{"
"	camera_vec = camera - vertex;"
"	n = normalize((NormalMatrix * vec4(normal, 0.0)).xyz);"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex, 1.0);"
"	pos = vertex;"
"	n2 = -normal;"
"}";

char *la_surface_front_shader_fragment = 
"uniform sampler2D image;"
"varying vec3 n;"
"varying vec3 pos;"
"varying vec3 n2;"
"varying vec3 camera_vec;"
"uniform float scale;"
"uniform vec3 light;"
"uniform vec3 manipulator;"
"uniform vec4 manipulator_color;"
"void main()"
"{"
"	vec3 uv, v, ref;"
"	float dist, f, freznel;"
"	v = pos - light;"
"	dist = length(v) * scale;"
"	v = normalize(v);"
"	f = dot(v, normalize(n2));"
"	v = pos - manipulator;"
"	uv = normalize(n);"
"	freznel = dot(uv.xy, uv.xy);"
"	uv = vec3(0.5) + uv * vec3(0.5);"
"	gl_FragColor = texture2D(image, uv.xy) + vec4(vec3(max(f / dist, 0.0)), 1.0) * vec4(0.6, 0.42, 0.18, 1.0) + vec4(0.4 / length(v) / scale) * manipulator_color;"
"}";


char *la_surface_back_shader_vertex = 
"attribute vec3 vertex;"
"attribute vec3 normal;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform mat4 NormalMatrix;"
"uniform vec3 camera;"
"varying vec3 n;"
"varying vec3 pos;"
"varying vec3 n2;"
"varying vec3 camera_vec;"
"void main()"
"{"
"	camera_vec = camera - vertex;"
"	n = normalize((NormalMatrix * vec4(normal, 0.0)).xyz);"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex, 1.0);"
"	pos = vertex;"
"	n2 = -normalize(normal);"
"}";

char *la_surface_back_shader_fragment = 
"uniform sampler2D image;"
"varying vec3 n;"
"varying vec3 pos;"
"varying vec3 n2;"
"varying vec3 camera_vec;"
"uniform float scale;"
"uniform vec3 light;"
"uniform vec3 manipulator;"
"uniform vec4 manipulator_color;"
"void main()"
"{"
"	vec3 uv, v, ref;"
"	float dist, freznel;"
"	v = pos - manipulator;"
"	uv = normalize(n);"
"	freznel = dot(uv.xy, uv.xy);"
"	gl_FragColor = mix(vec4(0.1, 0.2, 0.8, 1), vec4(0.7, 0.5, 0.4, 1), n.y * -0.5 + 0.5) + vec4(1.0 / length(v) / scale) * manipulator_color;"		
"	gl_FragColor += vec4(0.1, 0.2, 0.15, 1) * vec4(max(freznel, 0.0));"
"}";

typedef enum{
	LO_OSJT_STRUCTURE,
	LO_OSJT_LOD,
	LO_OSJT_RESHAPE,
	LO_OSJT_SIMPLE,
	LO_OSJT_COUNT
}LoOverlaySurfaceJobType;

typedef struct{
	LoOverlaySurfaceJobType job;
	uint32 node_id;
	float eye[3];
	float lod;
	uint min_tess;
	uint max_tess;
	float *vertex_array;
	uint vertex_count;
	uint *ref;
	uint ref_count;
	uint *crease;
	void *mutex;
	PPolyStore *store;
	PMesh *mesh;
	float *output_vertex_array;
	boolean center;
	float min[3];
	float max[3];
	uint preview_id;
}LoOverlaySufaceGen;

#define LA_PREVIEW_COUNT 16

typedef enum{
	LA_OPS_EMPTY,
	LA_OPS_QUED,
	LA_OPS_COMPLETED
}LAOverlayPreviewState;

LoOverlaySufaceGen	lo_overlay_surface;
LoOverlaySufaceGen	lo_preview_surface[LA_PREVIEW_COUNT];
LAOverlayPreviewState lo_preview_working[LA_PREVIEW_COUNT];
RShader				*lo_overlay_front_shader;
RShader				*lo_overlay_back_shader;
void				*lo_preview_pool = NULL;
void				*lo_preview_vertex_section = NULL;
void				*lo_preview_ref_section = NULL;
uint				lo_preview_ref_length;
float				lo_preview_center[3];
float				lo_preview_scale;


void lo_overlay_surface_bounding_box(LoOverlaySufaceGen *job)
{
	uint i, j;
	for(j = 0; j < 3; j++)
	{
		job->max[j] = -10000000.0;
		job->min[j] = 10000000.0;
	}
	for(i = 0; i < job->vertex_count; i++)
	{
		if(job->vertex_array[i * 3] != PERSUADE_ILLEGAL_VERTEX)
		{
			for(j = 0; j < 3; j++)
			{
				if(job->vertex_array[i * 3 + j] > job->max[j])
					job->max[j] = job->vertex_array[i * 3 + j];
				if(job->vertex_array[i * 3 + j] < job->min[j])
					job->min[j] = job->vertex_array[i * 3 + j];
			}
		}
	}
}

void lo_overlay_surface_func(void *data)
{
	LoOverlaySufaceGen *job;
	uint vertex_count, i, j;
	float normal[3];
	job = data;
	imagine_mutex_lock(job->mutex);
	vertex_count = 0;
	switch(job->job)
	{
		case LO_OSJT_STRUCTURE :
			lo_overlay_surface_bounding_box(job);
			if(job->mesh != NULL)
				persuade_lod_destroy(job->mesh);
			job->mesh = NULL;
			if(job->store != NULL)
				persuade_mesh_destroy(job->store);
			job->store = persuade_mesh_create(job->ref, job->ref_count, job->vertex_array, job->vertex_count, job->crease, 1, job->max_tess);
		case LO_OSJT_LOD :
			if(job->store == NULL)
				break;
			if(job->mesh != NULL)
				persuade_lod_destroy(job->mesh);
			job->mesh = persuade2_lod_create(job->store, job->ref, job->vertex_array, job->lod, job->min_tess, job->eye);	
			vertex_count = persuade2_lod_vertex_length_get(job->mesh);
			if(job->output_vertex_array != NULL)
				free(job->output_vertex_array);
			job->output_vertex_array = malloc((sizeof *job->output_vertex_array) * vertex_count * 6);
		case LO_OSJT_RESHAPE :		
			if(job->mesh != NULL)
			{
				persuade2_lod_vertex_shape(job->mesh, job->output_vertex_array, (sizeof *job->output_vertex_array) * 6, job->vertex_array);	
				persuade2_lod_normal_shape(job->mesh, &job->output_vertex_array[3], (sizeof *job->output_vertex_array) * 6, job->output_vertex_array, (sizeof *job->output_vertex_array) * 6);
			}
		break;
		case LO_OSJT_SIMPLE :
		{		
			lo_overlay_surface_bounding_box(job);
			for(i = j = 0; i < job->ref_count * 4; i += 4)
			{
				if(job->ref[i] < job->vertex_count && 
					job->ref[i + 1] < job->vertex_count &&
					job->ref[i + 2] < job->vertex_count)
				{
					j++;
					if(job->ref[i + 3] < job->vertex_count)
						j++;
				}
			}
			if(job->output_vertex_array != NULL)
				free(job->output_vertex_array);
			job->output_vertex_array = NULL;
			if(j != 0)
			{
				job->output_vertex_array = malloc((sizeof *job->output_vertex_array) * j * 3 * 6);
				for(i = j = 0; i < job->ref_count * 4; i += 4)
				{
					if(job->ref[i] < job->vertex_count && 
						job->ref[i + 1] < job->vertex_count &&
						job->ref[i + 2] < job->vertex_count)
					{
						f_normal3f(normal, &job->vertex_array[job->ref[i] * 3], &job->vertex_array[job->ref[i + 1] * 3], &job->vertex_array[job->ref[i + 2] * 3]);
						job->output_vertex_array[j++] = job->vertex_array[job->ref[i] * 3];
						job->output_vertex_array[j++] = job->vertex_array[job->ref[i] * 3 + 1];
						job->output_vertex_array[j++] = job->vertex_array[job->ref[i] * 3 + 2];
						job->output_vertex_array[j++] = normal[0];
						job->output_vertex_array[j++] = normal[1];
						job->output_vertex_array[j++] = normal[2];
						job->output_vertex_array[j++] = job->vertex_array[job->ref[i + 1] * 3];
						job->output_vertex_array[j++] = job->vertex_array[job->ref[i + 1] * 3 + 1];
						job->output_vertex_array[j++] = job->vertex_array[job->ref[i + 1] * 3 + 2];
						job->output_vertex_array[j++] = normal[0];
						job->output_vertex_array[j++] = normal[1];
						job->output_vertex_array[j++] = normal[2];
						job->output_vertex_array[j++] = job->vertex_array[job->ref[i + 2] * 3];
						job->output_vertex_array[j++] = job->vertex_array[job->ref[i + 2] * 3 + 1];
						job->output_vertex_array[j++] = job->vertex_array[job->ref[i + 2] * 3 + 2];
						job->output_vertex_array[j++] = normal[0];
						job->output_vertex_array[j++] = normal[1];
						job->output_vertex_array[j++] = normal[2];
						if(job->ref[i + 3] < job->vertex_count)
						{
							job->output_vertex_array[j++] = job->vertex_array[job->ref[i] * 3];
							job->output_vertex_array[j++] = job->vertex_array[job->ref[i] * 3 + 1];
							job->output_vertex_array[j++] = job->vertex_array[job->ref[i] * 3 + 2];
							job->output_vertex_array[j++] = normal[0];
							job->output_vertex_array[j++] = normal[1];
							job->output_vertex_array[j++] = normal[2];
							job->output_vertex_array[j++] = job->vertex_array[job->ref[i + 2] * 3];
							job->output_vertex_array[j++] = job->vertex_array[job->ref[i + 2] * 3 + 1];
							job->output_vertex_array[j++] = job->vertex_array[job->ref[i + 2] * 3 + 2];
							job->output_vertex_array[j++] = normal[0];
							job->output_vertex_array[j++] = normal[1];
							job->output_vertex_array[j++] = normal[2];
							job->output_vertex_array[j++] = job->vertex_array[job->ref[i + 3] * 3];
							job->output_vertex_array[j++] = job->vertex_array[job->ref[i + 3] * 3 + 1];
							job->output_vertex_array[j++] = job->vertex_array[job->ref[i + 3] * 3 + 2];
							job->output_vertex_array[j++] = normal[0];
							job->output_vertex_array[j++] = normal[1];
							job->output_vertex_array[j++] = normal[2];
						}
					}
				}
			}
			job->vertex_count = j / 6;
		}
		break;
	}
	if(job >= lo_preview_surface  && job < lo_preview_surface + LA_PREVIEW_COUNT)
		lo_preview_working[job - lo_preview_surface] = LA_OPS_COMPLETED;
	imagine_mutex_unlock(job->mutex);
}

boolean draw_persuade_surface_ready_preview(uint *node_id)
{
	static float light_pos[3] = {0, 0, 0}, light_distance = 1, brightness = 1;
	float goal[3];
	uint v, i;
	for(i = 0; i < LA_PREVIEW_COUNT; i++)
	{
		if(imagine_mutex_lock_try(lo_preview_surface[i].mutex))
		{
		
			if(lo_preview_working[i] == LA_OPS_COMPLETED)
			{

				RFormats types[2] = {R_FLOAT, R_FLOAT};
				uint  vertex_size[2] = {3, 3}, size, ref_size, vertex_count;
				size = r_array_vertex_size(types, vertex_size, 2);
				if(lo_preview_surface[i].job == LO_OSJT_SIMPLE)
				{
					if(lo_preview_surface[i].output_vertex_array != NULL)
					{
						lo_preview_pool = r_array_allocate(lo_preview_surface[i].vertex_count, types, vertex_size, 2, 0);
						r_array_load_vertex(lo_preview_pool, NULL, lo_preview_surface[i].output_vertex_array, 0, lo_preview_surface[i].vertex_count);	
						if(lo_preview_surface[i].output_vertex_array != NULL)
							free(lo_preview_surface[i].output_vertex_array);
						lo_preview_surface[i].output_vertex_array = NULL;
						lo_preview_vertex_section = NULL;
						lo_preview_ref_section = NULL;
						lo_preview_ref_length = lo_preview_surface[i].vertex_count;
					}
				}else if(lo_preview_surface[i].mesh != NULL)
				{
					lo_preview_pool = r_array_allocate(persuade2_lod_vertex_length_get(lo_preview_surface[i].mesh), types, vertex_size, 2, p_rm_get_ref_length(lo_preview_surface[i].mesh));
					lo_preview_vertex_section = r_array_section_allocate_vertex(lo_preview_pool, persuade2_lod_vertex_length_get(lo_preview_surface[i].mesh));
					lo_preview_ref_section = r_array_section_allocate_reference(lo_preview_pool, p_rm_get_ref_length(lo_preview_surface[i].mesh));
					lo_preview_ref_length = p_rm_get_ref_length(lo_preview_surface[i].mesh);
					if(lo_preview_ref_length == 0)
						lo_preview_ref_length = 0;
					r_array_load_vertex(lo_preview_pool, lo_preview_vertex_section, lo_preview_surface[i].output_vertex_array, 0, persuade2_lod_vertex_length_get(lo_preview_surface[i].mesh));
					r_array_load_reference(lo_preview_pool, lo_preview_ref_section, lo_preview_vertex_section, p_rm_get_reference(lo_preview_surface[i].mesh), p_rm_get_ref_length(lo_preview_surface[i].mesh));
					persuade_lod_destroy(lo_preview_surface[i].mesh);
					lo_preview_surface[i].mesh = NULL;
					persuade_mesh_destroy(lo_preview_surface[i].store);
					lo_preview_surface[i].store = NULL;
				}else
					lo_preview_ref_length = 0;
				lo_preview_scale = lo_preview_surface[i].max[0] - lo_preview_surface[i].min[0];
				if(lo_preview_scale < lo_preview_surface[i].max[1] - lo_preview_surface[i].min[1])
					lo_preview_scale = lo_preview_surface[i].max[1] - lo_preview_surface[i].min[1];
				if(lo_preview_scale < lo_preview_surface[i].max[2] - lo_preview_surface[i].min[2])
					lo_preview_scale = lo_preview_surface[i].max[2] - lo_preview_surface[i].min[2];
				lo_preview_scale = 1.25 / lo_preview_scale;

				lo_preview_center[0] = (lo_preview_surface[i].min[0] + lo_preview_surface[i].max[0]) * -0.5;
				lo_preview_center[1] = (lo_preview_surface[i].min[1] + lo_preview_surface[i].max[1]) * -0.5;
				lo_preview_center[2] = (lo_preview_surface[i].min[2] + lo_preview_surface[i].max[2]) * -0.5;
				imagine_mutex_unlock(lo_preview_surface[i].mutex);
				lo_preview_working[i] = LA_OPS_EMPTY;
				*node_id = lo_preview_surface[i].node_id;
				return TRUE;
			}else
				imagine_mutex_unlock(lo_preview_surface[i].mutex);
		}
	}
	return FALSE;
}

boolean draw_persuade_surface_draw_preview()
{
	RShader *shader[2];
	double manipulator[3];
	float camera[3] = {1, 1, 1}, light_pos[3] = {1, 1, 1}, size, scale;
	uint i, face[2] = {GL_FRONT, GL_BACK};

	if(lo_preview_pool == NULL || lo_preview_ref_length == 0)
		return FALSE;
	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_CULL_FACE);
	shader[0] = lo_overlay_front_shader;
	shader[1] = lo_overlay_back_shader;
	r_matrix_push(NULL);
	r_matrix_rotate(NULL, 30, 1, 0, 0);
	r_matrix_rotate(NULL, 30, 0, 1, 0);
	r_matrix_scale(NULL, lo_preview_scale, lo_preview_scale, lo_preview_scale);
	r_matrix_translate(NULL, lo_preview_center[0], lo_preview_center[1], lo_preview_center[2]);
	size = 1.0 / lo_preview_scale / 1.25;
	for(i = 0; i < 2; i++)
	{
		r_shader_set(shader[i]);
		glCullFace(face[i]);
		r_shader_vec3_set(shader[i], r_shader_uniform_location(shader[i], "light"), size, size, size);
		r_shader_vec3_set(shader[i], r_shader_uniform_location(shader[i], "manipulator"), -size, size, size);
		r_shader_vec4_set(NULL, r_shader_uniform_location(shader[i], "manipulator_color"), 0.3, 0, 0.1, 1);
		r_shader_vec3_set(shader[i], r_shader_uniform_location(shader[i], "camera"), -1, 1, 1);
		r_shader_float_set(shader[i], r_shader_uniform_location(shader[i], "scale"), lo_preview_scale);

		r_array_draw(lo_preview_pool, lo_preview_ref_section, GL_TRIANGLES, 0, lo_preview_ref_length, NULL, NULL, 1);
	//	if(lo_preview_ref_section != 0)
	//		r_array_reference_draw(lo_preview_pool, lo_preview_ref_section, GL_TRIANGLES, 0, lo_preview_ref_length);
	}
	r_matrix_pop(NULL);
//	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	r_array_free(lo_preview_pool);
	lo_preview_pool = NULL;
	return TRUE;
}

boolean draw_persuade_surface_queue_preview(ENode *node)
{
	uint j;
	if(node == NULL)
		return FALSE;
	for(j = 0; j < LA_PREVIEW_COUNT; j++)
	{
		if(lo_preview_working[j] == LA_OPS_EMPTY)
		{
			EGeoLayer *layer;
			uint32 *ref, *crease, i;
			double *vertex;
			lo_preview_surface[j].node_id = e_ns_get_node_id(node);
			lo_preview_surface[j].lod = 100.0;
			lo_preview_surface[j].min_tess = 1;
			lo_preview_surface[j].max_tess = 1;
			lo_preview_surface[j].center = TRUE;
			lo_preview_surface[j].ref_count = i = e_nsg_get_polygon_length(node);
			if(lo_preview_surface[j].ref_count > 4000)
				lo_preview_surface[j].job = LO_OSJT_SIMPLE;
			else
				lo_preview_surface[j].job = LO_OSJT_STRUCTURE;
			lo_preview_surface[j].vertex_count = e_nsg_get_vertex_length(node);
			ref = e_nsg_get_layer_data(node, e_nsg_get_layer_by_id(node, 1));
			vertex = e_nsg_get_layer_data(node, e_nsg_get_layer_by_id(node, 0));
			if(ref == NULL || vertex == NULL/* || lo_preview_surface.ref_count < 1*/)
				return FALSE;

			for(i = lo_preview_surface[j].ref_count; i < 1000; i += 4)
				lo_preview_surface[j].max_tess++;


			layer = e_nsg_get_layer_crease_edge_layer(node);
			if(layer != NULL)
				crease = e_nsg_get_layer_data(node, layer);
			else
				crease = NULL;
			if(lo_preview_surface[j].ref != NULL)
				free(lo_preview_surface[j].ref);
			lo_preview_surface[j].ref = malloc((sizeof *lo_preview_surface[j].ref) * lo_preview_surface[j].ref_count * 4);
			memcpy(lo_preview_surface[j].ref, ref, (sizeof *lo_preview_surface[j].ref) * lo_preview_surface[j].ref_count * 4);

			if(lo_preview_surface[j].crease != NULL)
				free(lo_preview_surface[j].crease);
			lo_preview_surface[j].crease = malloc((sizeof *lo_preview_surface[j].crease) * lo_preview_surface[j].ref_count * 4);
			if(crease != NULL)
				memcpy(lo_preview_surface[j].crease, crease, (sizeof *lo_preview_surface[j].crease) * lo_preview_surface[j].ref_count * 4);
			else
				for(i = 0; i < lo_preview_surface[j].ref_count * 4; i++)
					lo_preview_surface[j].crease[i] = 0;

			if(lo_preview_surface[j].vertex_array != NULL)
				free(lo_preview_surface[j].vertex_array);
			lo_preview_surface[j].vertex_array = malloc((sizeof *lo_preview_surface[j].vertex_array) * lo_preview_surface[j].vertex_count * 3);
			for(i = 0; i < lo_preview_surface[j].vertex_count * 3; i++)
			{
				if(vertex[i] == V_REAL64_MAX)
					lo_preview_surface[j].vertex_array[i] = PERSUADE_ILLEGAL_VERTEX;
				else
					lo_preview_surface[j].vertex_array[i] = (float)vertex[i];
			}
			lo_preview_working[j] = LA_OPS_QUED;
			imagine_thread(lo_overlay_surface_func, &lo_preview_surface[j], "Loq Airou surface overlay");	

			return TRUE;
		}
	}
	return FALSE;
}
void draw_persuade_surface_init()
{
	char buffer[2048];
	uint i;
	lo_overlay_surface.vertex_array = NULL;
	lo_overlay_surface.vertex_count = 0;
	lo_overlay_surface.ref = NULL;
	lo_overlay_surface.ref_count = 0;
	lo_overlay_surface.crease = NULL;
	lo_overlay_surface.mutex = imagine_mutex_create();
	lo_overlay_surface.store = NULL;
	lo_overlay_surface.mesh = NULL;
	lo_overlay_surface.output_vertex_array = NULL;
	lo_overlay_front_shader = r_shader_create_simple(buffer, 2048, la_surface_front_shader_vertex, la_surface_front_shader_fragment, "SDS Surface");
	r_shader_texture_set(lo_overlay_front_shader, 0, la_pfx_surface_material());
	r_shader_state_set_cull_face(lo_overlay_front_shader, GL_FRONT);
	lo_overlay_back_shader = r_shader_create_simple(buffer, 2048, la_surface_back_shader_vertex, la_surface_back_shader_fragment, "SDS Surface");
	r_shader_texture_set(lo_overlay_back_shader, 0, la_pfx_surface_material());
	r_shader_state_set_cull_face(lo_overlay_back_shader, GL_BACK);
	for(i = 0; i < LA_PREVIEW_COUNT; i++)
	{
		lo_preview_surface[i].node_id = 0;
		lo_preview_surface[i].vertex_array = NULL;
		lo_preview_surface[i].vertex_count = 0;
		lo_preview_surface[i].ref = NULL;
		lo_preview_surface[i].ref_count = 0;
		lo_preview_surface[i].crease = NULL;
		lo_preview_surface[i].mutex = imagine_mutex_create();
		lo_preview_surface[i].store = NULL;
		lo_preview_surface[i].mesh = NULL;
		lo_preview_surface[i].output_vertex_array = NULL;
		lo_preview_working[i] = LA_OPS_EMPTY;
	}
}
	
void draw_persuade_surface(BInputState *input)
{
	static boolean init = FALSE, working = FALSE;
	static uint structure_version = 0, shape_version = 0, ref_length = 0;
	static void *pool = NULL, *vertex_section = NULL, *ref_section = NULL;
	static float light_pos[3] = {0, 0, 0}, light_distance = 1, brightness = 1, lod_timer = 0;
	float goal[3];
	uint v, i;
	if(!imagine_setting_boolean_get("RENDER_AS_SDS", TRUE, "Render smooth surface"))
		return;
	lod_timer += input->delta_time;
	if(working)
	{
		if(imagine_mutex_lock_try(lo_overlay_surface.mutex))
		{
			if(lo_overlay_surface.mesh != NULL)
			{
				RFormats types[2] = {R_FLOAT, R_FLOAT};
				uint  vertex_size[2] = {3, 3}, ref_size, vertex_count;
				if(pool != NULL)
					r_array_free(pool);
				pool = r_array_allocate(persuade2_lod_vertex_length_get(lo_overlay_surface.mesh), types, vertex_size, 2, p_rm_get_ref_length(lo_overlay_surface.mesh));
				vertex_section = r_array_section_allocate_vertex(pool, persuade2_lod_vertex_length_get(lo_overlay_surface.mesh));
				ref_section = r_array_section_allocate_reference(pool, p_rm_get_ref_length(lo_overlay_surface.mesh));
				ref_length = p_rm_get_ref_length(lo_overlay_surface.mesh);
				r_array_load_vertex(pool, vertex_section, lo_overlay_surface.output_vertex_array, 0, persuade2_lod_vertex_length_get(lo_overlay_surface.mesh));
				r_array_load_reference(pool, ref_section, vertex_section, p_rm_get_reference(lo_overlay_surface.mesh), p_rm_get_ref_length(lo_overlay_surface.mesh));
			}else
			{
				if(pool != NULL)
					r_array_free(pool);
				pool = NULL;
			}
			imagine_mutex_unlock(lo_overlay_surface.mutex);
			working = FALSE;
		}
		lod_timer = 0;
	}

	if(!working)
	{
		v =	udg_get_version(TRUE, FALSE, FALSE, FALSE, FALSE);
		seduce_view_camera_getf(NULL, lo_overlay_surface.eye);
		
		lo_overlay_surface.lod = imagine_setting_double_get("GEOMETRY_COMPLEXITY", 1, NULL) * 200.0;
		lo_overlay_surface.max_tess = imagine_setting_integer_get("MAX_TESS_LEVEL", 3, NULL);
		if(v != structure_version)
		{	
			uint32 *ref, *crease, limit, poly_count;
			double *vertex;
			lo_overlay_surface.job = LO_OSJT_STRUCTURE;
			lo_overlay_surface.center = FALSE;
			udg_get_geometry(&lo_overlay_surface.vertex_count, &lo_overlay_surface.ref_count, &vertex, &ref, &crease);
			limit = imagine_setting_integer_get("SDS_LIMIT", 10000, "Maximum number of prolygons drawn befor SDS is turned off");
			if(lo_overlay_surface.ref_count < limit)
			{
				poly_count = lo_overlay_surface.ref_count;
				i = lo_overlay_surface.max_tess;
				for(lo_overlay_surface.max_tess = 1; i > lo_overlay_surface.max_tess && poly_count < limit; lo_overlay_surface.max_tess++)
					poly_count *= 4;
				
				if(lo_overlay_surface.ref != NULL)	
					free(lo_overlay_surface.ref);
				lo_overlay_surface.ref = malloc((sizeof *lo_overlay_surface.ref) * lo_overlay_surface.ref_count * 4);
				memcpy(lo_overlay_surface.ref, ref, (sizeof *lo_overlay_surface.ref) * lo_overlay_surface.ref_count * 4);

				if(lo_overlay_surface.crease != NULL)	
					free(lo_overlay_surface.crease);
				lo_overlay_surface.crease = malloc((sizeof *lo_overlay_surface.crease) * lo_overlay_surface.ref_count * 4);

				memcpy(lo_overlay_surface.crease, crease, (sizeof *lo_overlay_surface.crease) * lo_overlay_surface.ref_count * 4);
				if(lo_overlay_surface.vertex_array != NULL)	
					free(lo_overlay_surface.vertex_array);
				lo_overlay_surface.vertex_array = malloc((sizeof *lo_overlay_surface.vertex_array) * lo_overlay_surface.vertex_count * 3);
				for(i = 0; i < lo_overlay_surface.vertex_count * 3; i++)
				{
					if(vertex[i] == V_REAL64_MAX)
						lo_overlay_surface.vertex_array[i] = PERSUADE_ILLEGAL_VERTEX;
					else
						lo_overlay_surface.vertex_array[i] = (float)vertex[i];
				}
				imagine_thread(lo_overlay_surface_func, &lo_overlay_surface, "Loq Airou surface overlay");	
				working = TRUE;
				structure_version = v;
				shape_version = udg_get_version(TRUE, TRUE, FALSE, FALSE, FALSE);
			}else if(pool != NULL)
			{
				r_array_free(pool);
				pool = NULL;
			}
		}else
		{
			v =	udg_get_version(TRUE, TRUE, FALSE, FALSE, FALSE);
			if(v != shape_version)
			{
				double *vertex;
				udg_get_geometry(NULL, NULL, &vertex, NULL, NULL);
				for(i = 0; i < lo_overlay_surface.vertex_count * 3; i++)
				{
					if(vertex[i] == V_REAL64_MAX)
						lo_overlay_surface.vertex_array[i] = PERSUADE_ILLEGAL_VERTEX;
					else
						lo_overlay_surface.vertex_array[i] = (float)vertex[i];
				}
				lo_overlay_surface.job = LO_OSJT_RESHAPE;
				imagine_thread(lo_overlay_surface_func, &lo_overlay_surface, "Loq Airou surface overlay");	
				working = TRUE;
				shape_version = v;
			}else if(lo_overlay_surface.store != NULL && lod_timer > 1.0)
			{
				lo_overlay_surface.job = LO_OSJT_LOD;
				imagine_thread(lo_overlay_surface_func, &lo_overlay_surface, "Loq Airou surface overlay");	
				working = TRUE;
			}
		}
	}

	if(pool != NULL)
	{
		RShader *shader[2];
		double manipulator[3];
		float camera[3];

		shader[0] = lo_overlay_front_shader;
		shader[1] = lo_overlay_back_shader;
		if(!la_t_tm_hiden())
		{
			la_t_tm_get_pos(manipulator);
			brightness = brightness * (1.0 - input->delta_time * 5.0) + input->delta_time * 5.0;
		}else
			brightness = brightness * (1.0 - input->delta_time * 5.0);

		for(i = 0; i < 2; i++)
		{
			r_shader_set(shader[i]);
			r_shader_vec3_set(shader[i], r_shader_uniform_location(shader[i], "light"), light_pos[0], light_pos[1], light_pos[2]);
			if(!la_t_tm_hiden())
				r_shader_vec3_set(shader[i], r_shader_uniform_location(shader[i], "manipulator"), manipulator[0], manipulator[1], manipulator[2]);
			r_shader_vec4_set(shader[i], r_shader_uniform_location(shader[i], "manipulator_color"), brightness, brightness * 0.1, brightness * 0.4, 1);
			seduce_view_camera_getf(NULL, camera);
			r_shader_vec3_set(shader[i], r_shader_uniform_location(shader[i], "camera"), camera[0], camera[1], camera[2]);
			r_shader_float_set(shader[i], r_shader_uniform_location(shader[i], "scale"), 10.0 / seduce_view_distance_camera_get(NULL));

			r_array_reference_draw(pool, ref_section, GL_TRIANGLES, 0, ref_length);
		}
	}
	if(lo_pointer_polygon == -1)
	{
		r_matrix_projection_worldf(&la_world_matrix, goal, input->pointers[0].pointer_x, input->pointers[0].pointer_y, -light_distance);
	}else
	{
		float pos[3], x[3], y[3], normal[3], aim[3], camera[3], distance;
		uint32 *ref, vertex_count, ref_count;
		double *vertex;
		udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, NULL);
		if(lo_pointer_polygon < ref_count && ref[lo_pointer_polygon * 4] < vertex_count)
		{
			pos[0] = vertex[ref[lo_pointer_polygon * 4] * 3 + 0];
			pos[1] = vertex[ref[lo_pointer_polygon * 4] * 3 + 1];
			pos[2] = vertex[ref[lo_pointer_polygon * 4] * 3 + 2];
			x[0] = vertex[ref[lo_pointer_polygon * 4 + 2] * 3 + 0] - pos[0];
			x[1] = vertex[ref[lo_pointer_polygon * 4 + 2] * 3 + 1] - pos[1];
			x[2] = vertex[ref[lo_pointer_polygon * 4 + 2] * 3 + 2] - pos[2];
			y[0] = vertex[ref[lo_pointer_polygon * 4 + 1] * 3 + 0] - pos[0];
			y[1] = vertex[ref[lo_pointer_polygon * 4 + 1] * 3 + 1] - pos[1];
			y[2] = vertex[ref[lo_pointer_polygon * 4 + 1] * 3 + 2] - pos[2];
			f_cross3f(normal, x, y);
			f_normalize3f(normal);
			r_matrix_projection_vertexf(&la_world_matrix, aim, light_pos, input->pointers[0].pointer_x, input->pointers[0].pointer_y);
			seduce_view_camera_getf(NULL, camera);
			aim[0] -= camera[0];
			aim[1] -= camera[1];
			aim[2] -= camera[2];
			light_distance = f_length3f(aim);
			distance = seduce_view_distance_camera_get(NULL);
			f_project3f(goal, pos, normal, camera, aim);
			goal[0] += 0.1 * distance * normal[0];
			goal[1] += 0.1 * distance * normal[1];
			goal[2] += 0.1 * distance * normal[2];
		}else
		{
			goal[0] = light_pos[0];
			goal[1] = light_pos[1];
			goal[2] = light_pos[2];
		}
	}
	light_pos[0] = light_pos[0] * (1.0 - input->delta_time * 5.0) + goal[0] * input->delta_time * 5.0;
	light_pos[1] = light_pos[1] * (1.0 - input->delta_time * 5.0) + goal[1] * input->delta_time * 5.0;
	light_pos[2] = light_pos[2] * (1.0 - input->delta_time * 5.0) + goal[2] * input->delta_time * 5.0;
}