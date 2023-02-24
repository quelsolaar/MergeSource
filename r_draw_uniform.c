#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "forge.h"
#include "r_include.h"

char *r_type_names[R_SIT_COUNT] = {"bool",
									"vbool2",
									"vbool3",
									"vbool4",
									"int",
									"vint2",
									"vint3",
									"vint4",
									"uint",
									"vuint2",
									"vuint3",
									"vuint4",
									"float",
									"vec2",
									"vec3",
									"vec4",
									"double",
									"vdouble2",
									"vdouble3",
									"vdouble4",
									"mat2",
									"mat2x3",
									"mat2x4",
									"mat3x2",
									"mat3",
									"mat3x4",
									"mat4x2",
									"mat4x3",
									"mat4",
									"dmat2",
									"dmat2x3",
									"dmat2x4",
									"dmat3x2",
									"dmat3",
									"dmat3x4",
									"dmat4x2",
									"dmat4x3",
									"dmat4",
									"sampler1D",
									"sampler2D",
									"sampler3D",
									"samplerCube",
									"sampler2DRect",
									"sampler1DArray",
									"sampler2DArray",
									"samplerCubeArray",
									"samplerBuffer",
									"sampler2DMS",
									"sampler2DMSArray"};

uint r_type_sizes[R_SIT_COUNT] = {sizeof(uint8),
									sizeof(uint8) * 2,
									sizeof(uint8) * 3,
									sizeof(uint8) * 4,
									sizeof(int),
									sizeof(int) * 2,
									sizeof(int) * 3,
									sizeof(int) * 4,
									sizeof(uint),
									sizeof(uint) * 2,
									sizeof(uint) * 3,
									sizeof(uint) * 4,
									sizeof(float),
									sizeof(float) * 2,
									sizeof(float) * 3,
									sizeof(float) * 4,
									sizeof(double),
									sizeof(double) * 2,
									sizeof(double) * 3,
									sizeof(double) * 4,
									sizeof(float) * 2 * 2,
									sizeof(float) * 2 * 3,
									sizeof(float) * 2 * 4,
									sizeof(float) * 3 * 2,
									sizeof(float) * 3 * 3,
									sizeof(float) * 3 * 4,
									sizeof(float) * 4 * 2,
									sizeof(float) * 4 * 3,
									sizeof(float) * 4 * 4,
									sizeof(double) * 2 * 2,
									sizeof(double) * 2 * 3,
									sizeof(double) * 2 * 4,
									sizeof(double) * 3 * 2,
									sizeof(double) * 3 * 3,
									sizeof(double) * 3 * 4,
									sizeof(double) * 4 * 2,
									sizeof(double) * 4 * 3,
									sizeof(double) * 4 * 4,
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64)};

uint r_type_strides[R_SIT_COUNT] = {sizeof(uint8),
									sizeof(uint8) * 2,
									sizeof(uint8) * 4,
									sizeof(uint8) * 4,
									sizeof(int),
									sizeof(int) * 2,
									sizeof(int) * 4,
									sizeof(int) * 4,
									sizeof(uint),
									sizeof(uint) * 2,
									sizeof(uint) * 4,
									sizeof(uint) * 4,
									sizeof(float),
									sizeof(float) * 2,
									sizeof(float) * 4,
									sizeof(float) * 4,
									sizeof(double),
									sizeof(double) * 2,
									sizeof(double) * 4,
									sizeof(double) * 4,
									sizeof(float) * 2 * 2,
									sizeof(float) * 2 * 3,
									sizeof(float) * 2 * 4,
									sizeof(float) * 3 * 2,
									sizeof(float) * 3 * 3,
									sizeof(float) * 3 * 4,
									sizeof(float) * 4 * 2,
									sizeof(float) * 4 * 3,
									sizeof(float) * 4 * 4,
									sizeof(double) * 2 * 2,
									sizeof(double) * 2 * 3,
									sizeof(double) * 2 * 4,
									sizeof(double) * 3 * 2,
									sizeof(double) * 3 * 3,
									sizeof(double) * 3 * 4,
									sizeof(double) * 4 * 2,
									sizeof(double) * 4 * 3,
									sizeof(double) * 4 * 4,
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64),
									sizeof(uint64)};

 
void (APIENTRY *r_glUniform1fv)(GLint location, GLsizei count, const GLfloat *value);
void (APIENTRY *r_glUniform2fv)(GLint location, GLsizei count, const GLfloat *value);
void (APIENTRY *r_glUniform3fv)(GLint location, GLsizei count, const GLfloat *value);
void (APIENTRY *r_glUniform4fv)(GLint location, GLsizei count, const GLfloat *value);
void (APIENTRY *r_glUniform1iv)(GLint location, GLsizei count, const GLint *value);
void (APIENTRY *r_glUniform2iv)(GLint location, GLsizei count, const GLint *value);
void (APIENTRY *r_glUniform3iv)(GLint location, GLsizei count, const GLint *value);
void (APIENTRY *r_glUniform4iv)(GLint location, GLsizei count, const GLint *value);
void (APIENTRY *r_glUniform1uiv)(GLint location, GLsizei count, const GLuint *value);
void (APIENTRY *r_glUniform2uiv)(GLint location, GLsizei count, const GLuint *value);
void (APIENTRY *r_glUniform3uiv)(GLint location, GLsizei count, const GLuint *value);
void (APIENTRY *r_glUniform4uiv)(GLint location, GLsizei count, const GLuint *value);
void (APIENTRY *r_glUniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void (APIENTRY *r_glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void (APIENTRY *r_glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void (APIENTRY *r_glUniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void (APIENTRY *r_glUniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void (APIENTRY *r_glUniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void (APIENTRY *r_glUniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void (APIENTRY *r_glUniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void (APIENTRY *r_glUniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);


extern GLvoid (APIENTRY *r_glActiveTextureARB)(GLenum texture);
uint64 (APIENTRY *r_glGetTextureHandleARB)(uint texture);
uint64 (APIENTRY *r_glGetTextureSamplerHandleARB)(uint texture, uint sampler);
void (APIENTRY *r_glMakeTextureHandleResidentARB)(uint64 handle);
void (APIENTRY *r_glMakeTextureHandleNonResidentARB)(uint64 handle);

extern GLint (APIENTRY *r_glGetUniformLocationARB)(GLhandleARB programObj, const char *name);
extern void	(APIENTRY *r_glBindBufferBase)(GLenum target, GLuint index, GLuint buffer);

void (APIENTRY *r_glUniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
void (APIENTRY *r_glGetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
extern uint r_shader_glsl_version;
extern uint64	r_shader_uniform_texture_pointer_get(uint texture_id);


void r_uniform_init()
{
	if(r_shader_glsl_version > 110 || r_extension_test("GL_ARB_shading_language_100"))
	{
		r_glUniform1fv =			r_extension_get_address("glUniform1fv");
		r_glUniform2fv =			r_extension_get_address("glUniform2fv");
		r_glUniform3fv =			r_extension_get_address("glUniform3fv");
		r_glUniform4fv =			r_extension_get_address("glUniform4fv");
		r_glUniform1iv =			r_extension_get_address("glUniform1iv");
		r_glUniform2iv =			r_extension_get_address("glUniform2iv");
		r_glUniform3iv =			r_extension_get_address("glUniform3iv");
		r_glUniform4iv =			r_extension_get_address("glUniform4iv");
		r_glUniform1uiv =			r_extension_get_address("glUniform1uiv");
		r_glUniform2uiv =			r_extension_get_address("glUniform2uiv");
		r_glUniform3uiv =			r_extension_get_address("glUniform3uiv");
		r_glUniform4uiv =			r_extension_get_address("glUniform4uiv");
		r_glUniformMatrix2fv =		r_extension_get_address("glUniformMatrix2fv");
		r_glUniformMatrix3fv =		r_extension_get_address("glUniformMatrix3fv");
		r_glUniformMatrix4fv =		r_extension_get_address("glUniformMatrix4fv");
		r_glUniformMatrix2x3fv =	r_extension_get_address("glUniformMatrix2x3fv");
		r_glUniformMatrix3x2fv =	r_extension_get_address("glUniformMatrix3x2fv");
		r_glUniformMatrix2x4fv =	r_extension_get_address("glUniformMatrix2x4fv");
		r_glUniformMatrix4x2fv =	r_extension_get_address("glUniformMatrix4x2fv");
		r_glUniformMatrix3x4fv =	r_extension_get_address("glUniformMatrix3x4fv");
		r_glUniformMatrix4x3fv =	r_extension_get_address("glUniformMatrix4x3fv");
	}
	r_glBindBufferBase = NULL;
	if(r_extension_test("GL_ARB_uniform_buffer_object") && r_extension_test("GL_ARB_shader_draw_parameters"))
	{
		r_glBindBufferBase = r_extension_get_address("glBindBufferBase");
		r_glUniformBlockBinding = r_extension_get_address("glUniformBlockBinding");
		r_glGetActiveUniformBlockiv = r_extension_get_address("glGetActiveUniformBlockiv");
	}
	r_glGetTextureHandleARB = NULL;
	if(r_extension_test("GL_ARB_bindless_texture"))
	{
/*		r_glGetTextureHandleARB =				r_extension_get_address("glGetTextureHandleARB");
		r_glGetTextureSamplerHandleARB =		r_extension_get_address("glGetTextureSamplerHandleARB");
		r_glMakeTextureHandleResidentARB =		r_extension_get_address("glMakeTextureHandleResidentARB");
		r_glMakeTextureHandleNonResidentARB =	r_extension_get_address("glMakeTextureHandleNonResidentARB");*/
	}
}

#define R_UNIFORM_LOCATION_DIVIDER 1024


uint r_shader_uniform_block_count(RShader *shader)
{
	return shader->block_count;
}

uint r_shader_uniform_block_member_count(RShader *shader, uint block)
{
	uint i, count;
	for(i = count = 0; i < shader->uniform_count; i++)
		if(shader->uniforms[i].block == block)
			count++;
	return count;
}

char *r_shader_uniform_block_member_name(RShader *shader, uint block, uint member)
{
	uint i, count;
	for(i = count = 0; count <= member; i++)
		if(shader->uniforms[i].block == block)
			count++;
	return shader->uniforms[i].name;
}

RInputType r_shader_uniform_block_member_type(RShader *shader, uint block, uint member)
{
	uint i, count;
	for(i = count = 0; count <= member; i++)
		if(shader->uniforms[i].block == block)
			count++;
	return shader->uniforms[i].type;
}

int r_shader_uniform_block_member_offset(RShader *shader, uint block, uint member)
{
	uint i, count;
	for(i = count = 0; count <= member; i++)
		if(shader->uniforms[i].block == block)
			count++;
	return shader->uniforms[i].offset;
}

uint r_shader_uniform_block_size(RShader *shader, uint block)
{
	return shader->blocks[block].size;
}

void r_shader_uniform_data_set(RShader *shader, void *data, uint block_id); 

uint r_shader_uniform_location(RShader	*s, char *name)
{
	uint i, j, sum;
/*	for(i = 0; i < s->texture_count; i++)
	{
		for(j = 0; name[j] != 0 && name[j] == s->textures[i].name[j]; j++);
		if(name[j] == s->textures[i].name[j])
			return i;
	}*/
	for(i = 0; i < s->uniform_count; i++)
	{
		for(j = 0; name[j] != 0 && name[j] == s->uniforms[i].name[j]; j++);
		if(name[j] == s->uniforms[i].name[j])
		{
			sum = s->uniforms[i].offset;
			for(j = 0; j < s->uniforms[i].block; j++)
				sum += s->blocks[j].size;
			return sum * R_UNIFORM_LOCATION_DIVIDER + i;
		}
	}
	return -1;
}

uint r_shader_uniform_block_location(RShader *s, char *name)
{
	uint i, j;
/*	for(i = 0; i < s->texture_count; i++)
	{
		for(j = 0; name[j] != 0 && name[j] == s->textures[i].name[j]; j++);
		if(name[j] == s->textures[i].name[j])
			return i;
	}*/
	for(i = 0; i < s->uniform_count; i++)
	{
		for(j = 0; name[j] != 0 && name[j] == s->uniforms[i].name[j]; j++);
		if(name[j] == s->uniforms[i].name[j])
			return s->uniforms[i].offset;
	}
	return -1;
}

extern char *r_type_names[R_SIT_COUNT];
extern uint r_type_sizes[R_SIT_COUNT];

void r_shader_mat4v_set(RShader *shader, uint location, float *matrix)
{
	if(location == -1)
		return;
	if(shader == NULL)
		shader = r_current_shader;
	shader->uniforms[location % R_UNIFORM_LOCATION_DIVIDER].updated = FALSE;
	shader->blocks[shader->uniforms[location % R_UNIFORM_LOCATION_DIVIDER].block].updated = FALSE;
	location /= R_UNIFORM_LOCATION_DIVIDER;
	memcpy(&shader->uniform_data[location], matrix, r_type_sizes[R_SIT_MAT4]);
}

void r_shader_mat3v_set(RShader *shader, uint location, float *matrix)
{
	if(location == -1)
		return;
	if(shader == NULL)
		shader = r_current_shader;
	shader->uniforms[location % R_UNIFORM_LOCATION_DIVIDER].updated = FALSE;
	shader->blocks[shader->uniforms[location % R_UNIFORM_LOCATION_DIVIDER].block].updated = FALSE;
	location /= R_UNIFORM_LOCATION_DIVIDER;
	memcpy(&shader->uniform_data[location], matrix, r_type_sizes[R_SIT_MAT3]);
}

void r_shader_mat2v_set(RShader *shader, uint location, float *matrix)
{
	if(location == -1)
		return;
	if(shader == NULL)
		shader = r_current_shader;
	shader->uniforms[location % R_UNIFORM_LOCATION_DIVIDER].updated = FALSE;
	shader->blocks[shader->uniforms[location % R_UNIFORM_LOCATION_DIVIDER].block].updated = FALSE;
	location /= R_UNIFORM_LOCATION_DIVIDER;
	memcpy(&shader->uniform_data[location], matrix, r_type_sizes[R_SIT_MAT2]);
}

void r_shader_vec4_set(RShader *shader, uint location, float v0, float v1, float v2, float v3)
{
	float *array;
	if(location == -1)
		return;
	if(shader == NULL)
		shader = r_current_shader;
	shader->uniforms[location % R_UNIFORM_LOCATION_DIVIDER].updated = FALSE;
	shader->blocks[shader->uniforms[location % R_UNIFORM_LOCATION_DIVIDER].block].updated = FALSE;
	location /= R_UNIFORM_LOCATION_DIVIDER;
	array = (float *)&shader->uniform_data[location];
	array[0] = v0;
	array[1] = v1;
	array[2] = v2;
	array[3] = v3;
}

void r_shader_vec3_set(RShader *shader, uint location, float v0, float v1, float v2)
{
	float *array;
	if(location == -1)
		return;
	if(shader == NULL)
		shader = r_current_shader;
	shader->uniforms[location % R_UNIFORM_LOCATION_DIVIDER].updated = FALSE;
	shader->blocks[shader->uniforms[location % R_UNIFORM_LOCATION_DIVIDER].block].updated = FALSE;
	location /= R_UNIFORM_LOCATION_DIVIDER;
	array = (float *)&shader->uniform_data[location];
	array[0] = v0;
	array[1] = v1;
	array[2] = v2;
}

void r_shader_vec2_set(RShader *shader, uint location, float v0, float v1)
{
	float *array;
	if(location == -1)
		return;
	if(shader == NULL)
		shader = r_current_shader;
	shader->uniforms[location % R_UNIFORM_LOCATION_DIVIDER].updated = FALSE;
	shader->blocks[shader->uniforms[location % R_UNIFORM_LOCATION_DIVIDER].block].updated = FALSE;
	location /= R_UNIFORM_LOCATION_DIVIDER;
	array = (float *)&shader->uniform_data[location];
	array[0] = v0;
	array[1] = v1;
}

void r_shader_float_set(RShader *shader, uint location, float f)
{
	float *array;	
	if(location == -1)
		return;
	if(shader == NULL)
		shader = r_current_shader;

	shader->uniforms[location % R_UNIFORM_LOCATION_DIVIDER].updated = FALSE;
	shader->blocks[shader->uniforms[location % R_UNIFORM_LOCATION_DIVIDER].block].updated = FALSE;
	location /= R_UNIFORM_LOCATION_DIVIDER;
	array = (float *)&shader->uniform_data[location];
	array[0] = f;
}

void r_shader_uniform_texture_set(RShader *shader, uint location, uint64 texture_id)
{
	uint64 *array;	
	shader->uniforms[location % R_UNIFORM_LOCATION_DIVIDER].updated = FALSE;
	shader->blocks[shader->uniforms[location % R_UNIFORM_LOCATION_DIVIDER].block].updated = FALSE;
	location /= R_UNIFORM_LOCATION_DIVIDER;
	array = (uint64 *)&shader->uniform_data[location];
	array[0] = texture_id;
}

/*
void r_shader_int_set(uint location, int i)
{
	r_glUniform1fARB(location, i);
}

void r_shader_int2_set(uint location, int i0, int i1)
{
	r_glUniform2fARB(location, i0, i1);
}


void r_shader_int3_set(uint location, int i0, int i1, int i2)
{
	r_glUniform3fARB(location, i0, i1, i2);
}


void r_shader_int4_set(uint location, int i0, int i1, int i2, int i3)
{
	r_glUniform4fARB(location, i0, i1, i2, i3);
}*/


void r_shader_uniform_matrix_set(RShader *shader, uint8 *data, uint block_id, RMatrix *matrix)
{ 
	if(shader == NULL)
		shader = r_current_shader;
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;

	if(shader->normal_matrix != -1)
	{
		float m[16], f, *m2;
		m2 = matrix->matrix[matrix->current];
		f = sqrt(m2[0] * m2[0] + m2[1] * m2[1] + m2[2] * m2[2]);
		m[0] = m2[0] / f;
		m[1] = m2[1] / f;
		m[2] = m2[2] / f;
		m[3] = 0.0f;
		f = sqrt(m2[4] * m2[4] + m2[5] * m2[5] + m2[6] * m2[6]);
		m[4] = m2[4] / f;
		m[5] = m2[5] / f;
		m[6] = m2[6] / f;
		m[7] = 0.0f;
		f = sqrt(m2[8] * m2[8] + m2[9] * m2[9] + m2[10] * m2[10]);
		m[8] = m2[8] / f;
		m[9] = m2[9] / f;
		m[10] = m2[10] / f;
		m[11] = 0.0f;
		m[12] = 0.0f;
		m[13] = 0.0f;
		m[14] = 0.0f;
		m[15] = 1.0f;
		shader->blocks[block_id].updated = FALSE;
		shader->uniforms[shader->normal_matrix].updated = FALSE;
		memcpy(&data[shader->uniforms[shader->normal_matrix].offset + shader->blocks[block_id].offset], m, r_type_sizes[R_SIT_MAT4]);
	}
	if(shader->model_view_matrix != -1 && shader->uniforms[shader->model_view_matrix].block == block_id)
	{
		shader->blocks[block_id].updated = FALSE;
		shader->uniforms[shader->model_view_matrix].updated = FALSE;
		memcpy(&data[shader->uniforms[shader->model_view_matrix].offset + shader->blocks[block_id].offset], matrix->matrix[matrix->current], r_type_sizes[R_SIT_MAT4]);
	}
	if(shader->projection_matrix != -1 && shader->uniforms[shader->projection_matrix].block == block_id)
	{
		shader->blocks[block_id].updated = FALSE;
		shader->uniforms[shader->projection_matrix].updated = FALSE;
		memcpy(&data[shader->uniforms[shader->projection_matrix].offset + shader->blocks[block_id].offset], matrix->projection, r_type_sizes[R_SIT_MAT4]);
	}	
	if(shader->model_view_projection_matrix != -1  && shader->uniforms[shader->model_view_projection_matrix].block == block_id)
	{
		if(!matrix->multiplied)
		{
			f_matrix_multiplyf(matrix->modelviewprojection, matrix->projection, matrix->matrix[matrix->current]);
			matrix->multiplied = TRUE;
		}
		shader->blocks[block_id].updated = FALSE;
		shader->uniforms[shader->model_view_projection_matrix].updated = FALSE;
		memcpy(&data[shader->uniforms[shader->model_view_projection_matrix].offset + shader->blocks[block_id].offset], matrix->modelviewprojection, r_type_sizes[R_SIT_MAT4]);
	}
}



void r_shader_unifrom_data_set_block(RShader *s, uint8 *data, uint block)
{
	uint i;
	for(i = 0; i < s->uniform_count; i++)
	{
		if(s->uniforms[i].block == block && (!s->blocks[block].updated || !s->uniforms[i].updated))
		{
			s->uniforms[i].updated = TRUE;
			switch(s->uniforms[i].type)
			{
				case R_SIT_BOOL : 	
				break;
				case R_SIT_VBOOL2 : 
				break;
				case R_SIT_VBOOL3 : 
				break;
				case R_SIT_VBOOL4 : 
				break;
				case R_SIT_INT : 
				break;
				case R_SIT_VINT2 : 
				break;
				case R_SIT_VINT3 : 
				break;
				case R_SIT_VINT4 : 
				break;
				case R_SIT_UINT :
				break;
				case R_SIT_VUINT2 : 
				break;
				case R_SIT_VUINT3 : 
				break;
				case R_SIT_VUINT4 : 
				break;
				case R_SIT_FLOAT : 
					r_glUniform1fv(s->uniforms[i].id, s->uniforms[i].array_length, (float *)&data[s->uniforms[i].offset]);
				break;
				case R_SIT_VEC2 : 
					r_glUniform2fv(s->uniforms[i].id, s->uniforms[i].array_length, (float *)&data[s->uniforms[i].offset]);
				break;
				case R_SIT_VEC3 : 
					r_glUniform3fv(s->uniforms[i].id, s->uniforms[i].array_length, (float *)&data[s->uniforms[i].offset]);
				break;
				case R_SIT_VEC4 : 
					r_glUniform4fv(s->uniforms[i].id, s->uniforms[i].array_length, (float *)&data[s->uniforms[i].offset]);
				break;
				case R_SIT_DOUBLE : 
				break;
				case R_SIT_VDOUBLE2 : 
				break;
				case R_SIT_VDOUBLE3 : 
				break;
				case R_SIT_VDOUBLE4 : 
				break;
				case R_SIT_MAT2 : 
					r_glUniformMatrix2fv(s->uniforms[i].id, s->uniforms[i].array_length, FALSE, (float *)&data[s->uniforms[i].offset]);
				break;
				case R_SIT_MAT2X3 : 
				break;
				case R_SIT_MAT2X4 : 
				break;
				case R_SIT_MAT3X2 : 
				break;
				case R_SIT_MAT3 : 
					r_glUniformMatrix3fv(s->uniforms[i].id, s->uniforms[i].array_length, FALSE, (float *)&data[s->uniforms[i].offset]);
				break;
				case R_SIT_MAT3X4 : 
				break;
				case R_SIT_MAT4X2 : 
				break;
				case R_SIT_MAT4X3 : 
				break;
				case R_SIT_MAT4 : 
					r_glUniformMatrix4fv(s->uniforms[i].id, s->uniforms[i].array_length, FALSE, (float *)&data[s->uniforms[i].offset]);
				break;
				case R_SIT_DMAT2 : 
				break;
				case R_SIT_DMAT2X3 : 
				break;
				case R_SIT_DMAT2X4 : 
				break;
				case R_SIT_DMAT3X2 : 
				break;
				case R_SIT_DMAT3 : 
				break;
				case R_SIT_DMAT3X4 : 
				break;
				case R_SIT_DMAT4X2 : 
				break;
				case R_SIT_DMAT4X3 : 
				break;
				case R_SIT_DMAT4 : 
				break;
				case R_SIT_SAMPLER_1D : 
				case R_SIT_SAMPLER_2D :
				case R_SIT_SAMPLER_3D : 
				case R_SIT_SAMPLER_CUBE :
				case R_SIT_SAMPLER_RECTANGLE : 
				case R_SIT_SAMPLER_1D_ARRAY :
				case R_SIT_SAMPLER_2D_ARRAY : 
				case R_SIT_SAMPLER_CUBE_MAP_ARRAY :
				case R_SIT_SAMPLER_BUFFER :
				case R_SIT_SAMPLER_2D_MULTISAMPLE : 
				case R_SIT_SAMPLER_2D_MULTISAMPLE_ARRAY :
				{
					uint64 *texture;
					texture = (uint64 *)(&data[s->uniforms[i].offset]);
					r_glActiveTextureARB(GL_TEXTURE0_ARB + s->uniforms[i].id);
					glBindTexture(r_sampler_names[s->uniforms[i].type - R_TYPE_TEXTURE_START], (uint)*texture);
				}
				break;
			}
		}
	}
	s->blocks[block].updated = TRUE;
}

void r_shader_unifrom_data_set_all(RShader *s, uint8 *data, uint exception)
{
	uint i, offset, bind_point, array_length; 
	for(i = offset = 0; i < r_current_shader->block_count; i++)
	{
		if(exception != i)
		{
			if(r_current_shader->blocks[i].object == -1)
			{
				if(!r_current_shader->blocks[i].updated)
					r_shader_unifrom_data_set_block(s, &r_current_shader->uniform_data[r_current_shader->blocks[i].offset], i);
			}else
			{
				bind_point = GL_UNIFORM_BUFFER;
				if(s->instance_block == i)
				{
					array_length = 1;
					if(r_current_shader->mode == R_SIM_BUFFER_OBJECT)
						bind_point = GL_SHADER_STORAGE_BUFFER;
				}else
					array_length = r_current_shader->blocks[i].array_length;
				if(!r_current_shader->blocks[i].updated)
				{
					r_current_shader->blocks[i].updated = TRUE;
					r_glBindBufferARB(bind_point, r_current_shader->blocks[i].object);
					r_glBufferDataARB(bind_point, 
										array_length * r_current_shader->blocks[i].size,
										&r_current_shader->uniform_data[r_current_shader->blocks[i].offset], 
										GL_DYNAMIC_DRAW_ARB);
				}
				r_glBindBufferBase(bind_point, r_current_shader->blocks[i].id, r_current_shader->blocks[i].object);
			}
		}
		offset += s->blocks[i].size * s->blocks[i].array_length;
	}
}


void r_shader_uniform_data_set(RShader *shader, void *data, uint block_id)
{
	uint size;
	size = shader->blocks[block_id].size;
	if(shader->instance_block != block_id)
		size *= shader->blocks[block_id].array_length;
	if(shader->blocks[block_id].object == -1)
	{
		memcpy(&shader->uniform_data[r_current_shader->blocks[block_id].offset], data, size);
	}else
	{
		r_current_shader->blocks[block_id].updated = TRUE;
		r_glBindBufferARB(GL_UNIFORM_BUFFER, r_current_shader->blocks[block_id].object);
		r_glBufferDataARB(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW_ARB);
	}
}



void r_shader_texture_set(RShader *s, uint slot, uint texture_id)
{
	uint i, j, sum, count;
	if(s == NULL)
	{
		printf("RELINQUISH Error: r_shader_active_texture shader is NULL.\n");
		return;
	}

	for(i = count = 0; i < s->uniform_count; i++)
	{
		if(s->uniforms[i].type >= R_TYPE_TEXTURE_START)
		{
			if(slot == count)
			{
				sum = s->uniforms[i].offset;
				for(j = 0; j < s->uniforms[i].block; j++)
					sum += s->blocks[j].size;
				r_shader_uniform_texture_set(s, sum * R_UNIFORM_LOCATION_DIVIDER + i, r_shader_uniform_texture_pointer_get(texture_id));
				return;
			}
			count++;
		}
	}
	printf("RELINQUISH Error: r_shader_active_texture trying to set slot %u but %s only has %u slots.\n", slot, s->name, s->uniform_count);
}
