#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "forge.h"
#include "r_include.h"


#define GL_SHADING_LANGUAGE_VERSION 0x8B8C

uint r_sampler_names[] = {0x0DE0, /*GL_TEXTURE_1D*/
										GL_TEXTURE_2D,
										0x806F/*GL_TEXTURE_3D*/,
										0x8513/*GL_TEXTURE_CUBE_MAP*/,
										0x84F5 /*GL_TEXTURE_RECTANGLE*/,
										-1/*GL_TEXTURE_1D_ARRAY*/,
										-1/*GL_TEXTURE_2D_ARRAY*/,
										0x9009 /*GL_TEXTURE_CUBE_MAP_ARRAY*/,
										0x8C2A /*GL_TEXTURE_BUFFER*/,
										0x9100 /*GL_TEXTURE_2D_MULTISAMPLE*/,
										0x9102 /*GL_TEXTURE_2D_MULTISAMPLE_ARRAY*/};

GLhandleARB (APIENTRY *r_glCreateShaderObjectARB)(GLenum  shaderType);
GLvoid      (APIENTRY *r_glDeleteObjectARB)(GLhandleARB obj);
GLhandleARB (APIENTRY *r_glCreateProgramObjectARB)(GLvoid);
GLvoid      (APIENTRY *r_glAttachObjectARB)(GLhandleARB containerObj, GLhandleARB obj);
GLvoid      (APIENTRY *r_glShaderSourceARB)(GLhandleARB shaderObj, GLsizei count, const char **string, const GLint *length);
GLvoid      (APIENTRY *r_glCompileShaderARB)(GLhandleARB shaderObj);
GLvoid      (APIENTRY *r_glLinkProgramARB)(GLhandleARB programObj);
GLvoid      (APIENTRY *r_glUseProgramObjectARB)(GLhandleARB programObj);
GLint       (APIENTRY *r_glGetUniformLocationARB)(GLhandleARB programObj, const char *name);
GLvoid      (APIENTRY *r_glUniform4fARB)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GLvoid      (APIENTRY *r_glUniform3fARB)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GLvoid      (APIENTRY *r_glUniform2fARB)(GLint location, GLfloat v0, GLfloat v1);
GLvoid      (APIENTRY *r_glUniform1fARB)(GLint location, GLfloat v0);
GLvoid      (APIENTRY *r_glUniform1iARB)(GLint location, GLint i);
GLvoid      (APIENTRY *r_glUniform2iARB)(GLint location, GLint i0, GLint i1);
GLvoid      (APIENTRY *r_glUniform3iARB)(GLint location, GLint i0, GLint i1, GLint i2);
GLvoid      (APIENTRY *r_glUniform4iARB)(GLint location, GLint i0, GLint i1, GLint i2, GLint i3);
GLvoid		(APIENTRY *r_glUniformMatrix4fvARB)(GLint location, GLsizei count, GLboolean transpose, GLfloat *value);
GLvoid		(APIENTRY *r_glUniformMatrix3fvARB)(GLint location, GLsizei count, GLboolean transpose, GLfloat *value);
GLvoid		(APIENTRY *r_glUniformMatrix2fvARB)(GLint location, GLsizei count, GLboolean transpose, GLfloat *value);
GLvoid		(APIENTRY *r_glGetShaderivARB)(GLuint shader,  GLenum pname,  GLint *params);
GLvoid		(APIENTRY *r_glGetProgramivARB)(GLuint shader,  GLenum pname,  GLint *params);
GLint		(APIENTRY *r_glGetProgramiARB)(GLuint shader,  GLenum pname);

GLint       (APIENTRY *r_glGetAttribLocationARB)(GLhandleARB programObj, const char *name);
GLvoid		(APIENTRY *r_glVertexAttrib4fARB)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GLvoid		(APIENTRY *r_glActiveTextureARB)(GLenum texture);

GLvoid		(APIENTRY *r_glBindAttribLocationARB)(GLhandleARB programObj, GLuint index, const char *name);

GLvoid		(APIENTRY *r_glGetShaderInfoLog)(GLhandleARB object, GLsizei maxLenght, GLsizei *length, char *infoLog);
GLvoid		(APIENTRY *r_glGetProgramInfoLog)(GLhandleARB object, GLsizei maxLenght, GLsizei *length, char *infoLog);

GLvoid		(APIENTRY *r_glStencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask);
GLvoid		(APIENTRY *r_glStencilOpSeparate)(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);

/* From buffer */

extern GLvoid (APIENTRY *r_glGenBuffersARB)(uint n, uint *buffers);
extern GLvoid (APIENTRY *r_glBindBufferARB)(uint target, uint buffer);
extern GLvoid (APIENTRY *r_glBufferDataARB)(uint target, uint size, const void *data, uint usage);

GLvoid		(APIENTRY *r_glBindTextures)(GLuint first, GLsizei count, const GLuint *textures);

GLvoid		(APIENTRY *r_glTransformFeedbackVaryings)(GLuint program, GLsizei count, const char **varyings, GLenum bufferMode);
#define GL_INTERLEAVED_ATTRIBS 0x8C8C


/* From experimental */

GLuint (APIENTRY *r_glGetUniformBlockIndex)(GLuint program, const char *uniformBlockName);
extern void (APIENTRY *r_glUniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
void (APIENTRY *r_glBindBufferBase)(GLenum target, GLuint index, GLuint buffer);
extern void (APIENTRY *r_glGetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
GLuint (APIENTRY *r_glGetUniformIndices)(GLuint program, GLsizei uniformCount, const char **uniformNames, GLuint *uniformIndices);
void (APIENTRY *r_glGetActiveUniformsiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
void (APIENTRY *r_glDrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
void  (APIENTRY *r_gluniform3fvARB)(GLint location, uint count, GLfloat *v);

GLuint (APIENTRY *r_glGetProgramResourceIndex)(GLuint program,GLenum programInterface, const char * name);
void (APIENTRY *r_glShaderStorageBlockBinding)(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding);


RShader *r_current_shader = NULL;
uint p_current_textures[64] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
								-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
								-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
								-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};


RState r_current_state;
uint r_shader_glsl_version;

uint r_shader_experimental_program_get(RShader *shader)
{
	return shader->program;
}

void r_shader_state_set_default(RState *state)
{
	uint i;
	state->depth_test = GL_LEQUAL;
//	state->depth_test = GL_GEQUAL;
	state->cull_face = 0;
	state->blend_dest = GL_ONE_MINUS_SRC_ALPHA;
	state->blend_source = GL_SRC_ALPHA;
	state->offset_factor = 0; 
	state->offset_units = 0;
	state->alpha_to_coverage = FALSE;
	state->depth_mask = TRUE;
	state->color_mask[0] = TRUE;
	state->color_mask[1] = TRUE;
	state->color_mask[2] = TRUE;
	state->color_mask[3] = TRUE;
	for(i = 0; i < 2; i++)
	{
		state->steincil[i].function = GL_ALWAYS;
		state->steincil[i].reference = 0;
		state->steincil[i].mask = 0;
		state->steincil[i].stencil_fail_op = GL_KEEP;	
		state->steincil[i].depth_fail_op = GL_KEEP;	
		state->steincil[i].sucsess_op = GL_KEEP;
	}
}

void r_shader_state_set(RState *state)
{

//	if(r_current_state.depth_test != state->depth_test)
	{
	/*	if(state->depth_test == 0)
			glDisable(GL_DEPTH_TEST);
		else
		{
		//	if(r_current_state.depth_test == 0)*/
				glEnable(GL_DEPTH_TEST);
			glDepthFunc(state->depth_test);
	//	}
	}
//	if(r_current_state.cull_face != state->cull_face)
	{
		if(state->cull_face == 0)
			glDisable(GL_CULL_FACE);
		else
		{
			if(r_current_state.cull_face == 0)
				glEnable(GL_CULL_FACE);
			glCullFace(state->cull_face);
		}
	}
		
//	if(r_current_state.blend_dest != state->blend_dest ||
//		r_current_state.blend_source != state->blend_source)

	glEnable(GL_BLEND);
	{
		glBlendFunc(state->blend_source, state->blend_dest);

	}
//		glBlendFunc(1, 1);

//	if(r_current_state.offset_factor != state->offset_factor ||
//		r_current_state.offset_units != state->offset_units)
//		glPolygonOffset(state->offset_units, state->offset_units);
	
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(state->offset_units, state->offset_units);
		
//	if(r_current_state.alpha_to_coverage != state->alpha_to_coverage)
	{
		if(state->alpha_to_coverage)
			glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		else
			glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	}

//	if(r_current_state.depth_mask != state->depth_mask)
	glDepthMask(state->depth_mask);
//	glDepthMask(FALSE);

//	if(r_current_state.color_mask[0] != state->color_mask[0] ||
//		r_current_state.color_mask[1] != state->color_mask[1] ||
//		r_current_state.color_mask[2] != state->color_mask[2] ||
//		r_current_state.color_mask[3] != state->color_mask[3])
	glColorMask(state->color_mask[0], state->color_mask[1], state->color_mask[2], state->color_mask[3]);

	if(r_glStencilFuncSeparate != NULL)
	{
		r_glStencilFuncSeparate(GL_FRONT, state->steincil[0].function, state->steincil[0].reference, state->steincil[0].mask);
		r_glStencilFuncSeparate(GL_BACK, state->steincil[1].function, state->steincil[1].reference, state->steincil[1].mask);
	}else
		glStencilFunc(state->steincil[0].function, state->steincil[0].reference, state->steincil[0].mask);
	if(r_glStencilOpSeparate != NULL)
	{
		r_glStencilOpSeparate(GL_FRONT, state->steincil[0].stencil_fail_op, state->steincil[0].depth_fail_op, state->steincil[0].sucsess_op);
		r_glStencilOpSeparate(GL_BACK, state->steincil[1].stencil_fail_op, state->steincil[1].depth_fail_op, state->steincil[1].sucsess_op);
	}else
		glStencilOp(state->steincil[0].stencil_fail_op, state->steincil[0].depth_fail_op, state->steincil[0].sucsess_op);
	
	r_current_state = *state;
}


void r_shader_state_set_depth_test(RShader *shader, uint depth_test) 
{
	uint i;
	if(r_shader_presets_get(P_SP_COLOR_UNIFORM) == shader)
		i = 0;

	if(r_current_shader == shader)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(depth_test);
	}
	shader->state.depth_test = depth_test;
}

void r_shader_state_set_cull_face(RShader *shader, uint cull_face) 
{

	if(r_current_shader == shader)
	{
		if(cull_face == 0)
			glDisable(GL_CULL_FACE);
		else
		{
			if(r_current_state.cull_face == 0)
				glEnable(GL_CULL_FACE);
			glCullFace(cull_face);
		}
	}
	shader->state.cull_face = cull_face;
}

void r_shader_state_set_blend_mode(RShader *shader, uint blend_source, uint blend_destination) 
{
	if(r_current_shader == shader)
		glBlendFunc(blend_source, blend_destination);
	shader->state.blend_dest = blend_destination;
	shader->state.blend_source = blend_source;
}

void r_shader_state_set_offset(RShader *shader, float offset_factor, float offset_units) 
{
	if(r_current_shader == shader)
		glPolygonOffset(offset_units, offset_units);
	shader->state.offset_factor = offset_factor; 
	shader->state.offset_units = offset_units;
}

void r_shader_state_set_alpha_to_coverage(RShader *shader, boolean alpha_to_coverage) 
{
	if(r_current_shader == shader)
	{
		if(alpha_to_coverage)
			glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		else
			glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	}
	shader->state.alpha_to_coverage = alpha_to_coverage;
}

void r_shader_state_set_mask(RShader *shader, boolean red, boolean green, boolean blue, boolean alpha, boolean depth) 
{
	uint i;
	if(r_shader_presets_get(P_SP_COLOR_UNIFORM) == shader)
		i = 0;
	if(r_current_shader == shader)
	{
		glDepthMask(depth);
		glColorMask(red, green, blue, alpha);
	}

	shader->state.depth_mask = depth;
	shader->state.color_mask[0] = red;
	shader->state.color_mask[1] = green;
	shader->state.color_mask[2] = blue;
	shader->state.color_mask[3] = alpha;
}

typedef enum{
	R_SF_NEVER,
	R_SF_ALWAYS,
	R_SF_LESS,
	R_SF_LESS_OR_EQUAL,
	R_SF_GREATER,
	R_SF_GREATER_OR_EQUAL,
	R_SF_EQUAL,
	R_SF_NOT_EQUAL,
	R_SF_COUNT
}RStencilFunc;

typedef enum{
	R_SO_KEEP,
	R_SO_INVERT,
	R_SO_ZERO,
	R_SO_REPLACE,
	R_SO_INCR,
	R_SO_INCR_WRAP,
	R_SO_DECR,
	R_SO_DECR_WRAP
}RStencilOp;

void r_shader_state_set_stencil(RShader *shader, boolean front, RStencilFunc function, uint32 reference, uint32 mask, RStencilOp stencil_fail_op, RStencilOp depth_fail_op, RStencilOp sucsess_op)
{
	uint side = 0, gl_enum;
	uint32 func_convert[] = {GL_NEVER,
							GL_ALWAYS,
							GL_LESS,
							GL_LEQUAL,
							GL_GREATER,
							GL_GEQUAL,
							GL_EQUAL,
							GL_NOTEQUAL};
	uint32 op_convert[] = {GL_KEEP,
							GL_INVERT,
							GL_ZERO,
							GL_REPLACE,
							GL_INCR,
							0x8507,
							GL_DECR,
							0x8508};
	if(!front)
		side = 1;
	shader->state.steincil[side].function = func_convert[function];
	shader->state.steincil[side].reference = reference;
	shader->state.steincil[side].mask = mask;
	shader->state.steincil[side].stencil_fail_op = op_convert[stencil_fail_op];
	shader->state.steincil[side].depth_fail_op = op_convert[depth_fail_op];
	shader->state.steincil[side].sucsess_op = op_convert[sucsess_op];
	if(r_current_shader == shader)
	{
		if(front)
			gl_enum = GL_FRONT;
		else
			gl_enum = GL_BACK;
		if(r_glStencilFuncSeparate == NULL)
			r_glStencilFuncSeparate(gl_enum, shader->state.steincil[side].function, shader->state.steincil[side].reference, shader->state.steincil[side].mask);
		else
			glStencilFunc(shader->state.steincil[side].function, shader->state.steincil[side].reference, shader->state.steincil[side].mask);
		if(r_glStencilOpSeparate)
			r_glStencilOpSeparate(gl_enum, shader->state.steincil[side].stencil_fail_op, shader->state.steincil[side].depth_fail_op, shader->state.steincil[side].sucsess_op);
		else
			glStencilOp(shader->state.steincil[side].stencil_fail_op, shader->state.steincil[side].depth_fail_op, shader->state.steincil[side].sucsess_op);
	}
}





boolean r_shader_init(void)
{
    char *version;
	RState state;
	uint8 *data;
	uint i, j, major = 0;
    r_shader_glsl_version = 110;
    version = glGetString(GL_SHADING_LANGUAGE_VERSION);
    for(i = 0; version[i] != 0 && (version[i] < '0' || version[i] > '9'); i++);
    j = i;
    r_shader_glsl_version = 0;
    for(; version[i] >= '0' && version[i] <= '9'; i++)
    {
        r_shader_glsl_version *= 10;
        r_shader_glsl_version += version[i] - '0';
    }
    if(version[i] == '.')
    {
        i++;
        for(j = 0; j < 2 && version[i] >= '0' && version[i] <= '9'; i++)
        {
            r_shader_glsl_version *= 10;
            r_shader_glsl_version += version[i] - '0';
            j++;
        }
        for(; j < 2; j++)
            r_shader_glsl_version *= 10;
    }
    if(r_shader_glsl_version < 110)
    {
        printf("Relinquish Error: Failed to parse GLSL version string \"%s\". Revetring to GLSL version 110. ", version);
        r_shader_glsl_version = 110;
    }
#ifdef RELINQUISH_CONTEXT_OPENGLES
	r_glCreateShaderObjectARB =			glCreateShader;
	r_glDeleteObjectARB =				glDeleteShader;
	r_glCreateProgramObjectARB =		glCreateProgram;
	r_glAttachObjectARB =				glAttachShader;
	r_glShaderSourceARB =				glShaderSource;
	r_glCompileShaderARB =				glCompileShader;
	r_glLinkProgramARB =				glLinkProgram;
	r_glUseProgramObjectARB =			glUseProgram;
	r_glGetUniformLocationARB =			glGetUniformLocation;
	r_glUniform4fARB =					glUniform4f;
	r_glUniform3fARB =					glUniform3f;
	r_glUniform2fARB =					glUniform2f;
	r_glUniform1fARB =					glUniform1f;
	r_glUniform1iARB =					glUniform1i;
	r_glUniform2iARB =					glUniform2i;
	r_glUniform3iARB =					glUniform3i;
	r_glUniform4iARB =					glUniform4i;
	r_glUniformMatrix4fvARB =			glUniformMatrix4fv;
	r_glUniformMatrix3fvARB =			glUniformMatrix3fv;
	r_glUniformMatrix2fvARB =			glUniformMatrix2fv;
	r_glGetShaderivARB =				glGetShaderiv;
	r_glGetProgramivARB =				glGetProgramiv;
	r_glBindAttribLocationARB =			glBindAttribLocation;
	r_glGetAttribLocationARB =			glGetAttribLocation;
	r_glVertexAttrib4fARB =				glVertexAttrib4f;
	r_glActiveTextureARB =				glActiveTexture;
	r_glGetShaderInfoLog =					glGetShaderInfoLog;
	r_glGetProgramInfoLog =					glGetProgramInfoLog;

#endif
#ifdef RELINQUISH_CONTEXT_OPENGL
	if(r_shader_glsl_version > 110 || r_extension_test("GL_ARB_shading_language_100"))
	{
		r_glCreateShaderObjectARB =			r_extension_get_address("glCreateShaderObjectARB");
		r_glDeleteObjectARB =				r_extension_get_address("glDeleteObjectARB");
		r_glCreateProgramObjectARB =		r_extension_get_address("glCreateProgramObjectARB");
		r_glAttachObjectARB =				r_extension_get_address("glAttachObjectARB");
		r_glShaderSourceARB =				r_extension_get_address("glShaderSource");
		r_glCompileShaderARB =				r_extension_get_address("glCompileShaderARB");
		r_glLinkProgramARB =				r_extension_get_address("glLinkProgramARB");
		r_glUseProgramObjectARB =			r_extension_get_address("glUseProgramObjectARB");
		r_glGetUniformLocationARB =			r_extension_get_address("glGetUniformLocationARB");
		r_glUniform4fARB =					r_extension_get_address("glUniform4fARB");
		r_glUniform3fARB =					r_extension_get_address("glUniform3fARB");
		r_glUniform2fARB =					r_extension_get_address("glUniform2fARB");
		r_glUniform1fARB =					r_extension_get_address("glUniform1fARB");
		r_glUniform1iARB =					r_extension_get_address("glUniform1iARB");
		r_glUniform2iARB =					r_extension_get_address("glUniform2iARB");
		r_glUniform3iARB =					r_extension_get_address("glUniform3iARB");
		r_glUniform4iARB =					r_extension_get_address("glUniform4iARB");
		r_glUniformMatrix4fvARB =			r_extension_get_address("glUniformMatrix4fvARB");
		r_glUniformMatrix3fvARB =			r_extension_get_address("glUniformMatrix3fvARB");
		r_glUniformMatrix2fvARB =			r_extension_get_address("glUniformMatrix2fvARB");
		r_glGetShaderivARB =				r_extension_get_address("glGetShaderiv");
		r_glGetProgramivARB =				r_extension_get_address("glGetProgramivARB");
		r_glBindAttribLocationARB =			r_extension_get_address("glBindAttribLocationARB");
		r_glGetAttribLocationARB =			r_extension_get_address("glGetAttribLocationARB");
		r_glVertexAttrib4fARB =				r_extension_get_address("glVertexAttrib4fARB");
		r_glActiveTextureARB =				r_extension_get_address("glActiveTextureARB");
		r_glGetShaderInfoLog =				r_extension_get_address("glGetShaderInfoLog");
        r_glGetProgramInfoLog =				r_extension_get_address("glGetProgramInfoLog");
        if(r_glGetShaderInfoLog == NULL)
            r_glGetShaderInfoLog =			r_extension_get_address("glGetInfoLogARB");
        if(r_glGetProgramInfoLog == NULL)
            r_glGetProgramInfoLog =			r_extension_get_address("glGetInfoLogARB");
		r_glTransformFeedbackVaryings =		r_extension_get_address("glTransformFeedbackVaryings");
		
		r_glStencilFuncSeparate =			r_extension_get_address("glStencilFuncSeparate");
		r_glStencilOpSeparate =				r_extension_get_address("glStencilOpSeparate");

	}else
		return FALSE;
#endif

	r_glBindTextures = r_extension_get_address("glBindTextures");

	r_current_shader = NULL;
	for(i = 0; i < 64; i++)
		p_current_textures[i] = -1;

	data = (uint8 *)&r_current_state;
	for(i = 0; i < (sizeof r_current_state); i++)
        data[i] = 255;

	r_shader_state_set_default(&state);
	r_shader_state_set(&state);
	return TRUE;
}



#define GL_COMPILE_STATUS  0x8B81
#define GL_LINK_STATUS    0x8B82

extern void b_debug_message();
/*
GL_VERTEX_SHADER_ARB
GL_FRAGMENT_SHADER_ARB
*/

extern char *experimental_shader_vertex_test2;
extern char *experimental_shader_fragment_test2;



void r_shader_debug_print_uniform_buffer(RShader *shader, uint8 *uniform_buffer, char *text, uint buffer_id)
{
	uint i, j, *ip, block_size = 0;
	float *fp;

	if(buffer_id == -1)
	{
		if(uniform_buffer == NULL)
			uniform_buffer = shader->uniform_data;
		buffer_id = 0;
		for(i = 0; i < shader->uniform_data_size; i += 4)
		{
			if(i == block_size)
			{
				printf("Block %u : %s\n", buffer_id, shader->blocks[buffer_id].name);
				block_size = shader->blocks[0].offset;
			}
			fp = (float *)&uniform_buffer[i];
			ip = (uint *)&uniform_buffer[i];
			printf("\tMem[%u] = u%u, f%f", i, ip[0], fp[0]);
			for(j = 0; j < shader->uniform_count; j++)
			{
				if(shader->uniforms[j].offset + shader->blocks[shader->uniforms[j].block].offset == i)
				{
					printf(" %s %s", shader->uniforms[j].name, r_type_names[shader->uniforms[j].type]);
					break;
				}
			}
			printf("\n");
		}
	}else
	{
		printf("Block %u : %s\n", buffer_id, shader->blocks[buffer_id].name);
		block_size = shader->blocks[buffer_id].size;
		if(uniform_buffer == NULL)
		{
			uniform_buffer = &shader->uniform_data[shader->blocks[buffer_id].offset];
			if(buffer_id != shader->instance_block)
				block_size *= shader->blocks[buffer_id].array_length;
		}
		for(i = 0; i < block_size; i += 4)
		{
			if(i % shader->blocks[buffer_id].size == 0 && i != 0)
				printf("Array[%u]\n", i / shader->blocks[buffer_id].size);

			fp = (float *)&uniform_buffer[i];
			ip = (uint *)&uniform_buffer[i];
			printf("\tMem[%u] = u%u, f%f", i, ip[0], fp[0]);
			for(j = 0; j < shader->uniform_count; j++)
			{
				if(shader->uniforms[j].block == buffer_id &&
					shader->uniforms[j].offset % shader->blocks[buffer_id].size == i)
				{
					printf(" %s", shader->uniforms[j].name);
					break;
				}
			}
			printf("\n");
		}

	}
}


void r_shader_debug_print_shader(RShader *shader)
{
	printf("Reliquish Shader: %s:\n", shader->name);
	switch(shader->mode)
	{
		case R_SIM_FLAT :
		printf("Mode: R_SIM_FLAT - Uniforms are not merged in to blocks.\n");
		break;
		case R_SIM_UNIFORM_BLOCK :
		printf("Mode: R_SIM_UNIFORM_BLOCK - Uniforms are merged in to blocks.\n");
		break;
		case R_SIM_BUFFER_OBJECT :
		printf("Mode: R_SIM_BUFFER_OBJECT - Uniforms are merged in to blocks, and instance block is a Buffer object binding.\n");
		break;
	}
	printf("Program ID: %u:\n", shader->program);
	printf("Attributes: \n");
	r_shader_parse_input_print(shader->attributes, shader->attribute_count);
	printf("Uniforms: \n");
	r_shader_parse_input_print(shader->uniforms, shader->uniform_count);
	printf("Blocks: \n");
	r_shader_parse_input_print(shader->blocks, shader->block_count);
	printf("instance_block: %u - %s\n", shader->instance_block, shader->blocks[shader->instance_block].name);


	if(shader->normal_matrix != -1 || shader->model_view_matrix != -1 || shader->projection_matrix != -1 || shader->model_view_projection_matrix != -1)
	{
		printf("Matrices: \n");
		if(shader->normal_matrix != -1)
			printf("\tnormal_matrix: %u\n", shader->normal_matrix);
		if(shader->model_view_matrix != -1)
			printf("\tmodel_view_matrix: %u\n", shader->model_view_matrix);
		if(shader->projection_matrix != -1)
			printf("\tprojection_matrix: %u\n", shader->projection_matrix);
		if(shader->model_view_projection_matrix != -1)
			printf("\tmodel_view_projection_matrix: %u\n", shader->model_view_projection_matrix);
	}else
		printf("No Built in Matrices\n");
	printf("Uniform data size %u \n", shader->uniform_data_size);
	r_shader_debug_print_uniform_buffer(shader, NULL, "Uniform buffer data", -1);

}

void r_test2()
{
	if(r_glCreateShaderObjectARB == NULL)
		exit(0);
}



char *r_shader_debug_override_shader = NULL;
uint r_shader_debug_override_stage = -1;

void r_shader_debug_override(char *shaders, uint stages)
{
	r_shader_debug_override_shader = shaders;
	r_shader_debug_override_stage = stages;
}

void *r_shader_create(char *debug_output, uint output_size, char **shaders, uint *stages, uint stage_count, char *name, char *instance_block)
{
	static RShaderInput *input = NULL;
	uint stage_obj[R_MAX_SHADER_STAGES], prog_obj, i, j, size = 0, add_size = 0, input_count, bind_point, texture_count;
	int status;
	RShader *shader;
	char *text[2], *output[R_MAX_SHADER_STAGES];
	if(input == NULL)
		input = malloc((sizeof *input) * 256);
	if(R_MAX_SHADER_STAGES < stage_count)
	{	
		printf("Relinquish ERROR: Shader %s has too many stages (%u). the maximum number of stages is %u.\n", name, stage_count, R_MAX_SHADER_STAGES);
		return NULL;
	}
	shader = malloc(sizeof *shader);
	shader->uniforms = NULL; 
	shader->mode = R_SIM_UNIFORM_BLOCK;
	if(r_glBindBufferBase != NULL) /* Test if uniffom bocks are available */
	{
		shader->mode = R_SIM_UNIFORM_BLOCK;
//		shader->mode = R_SIM_BUFFER_OBJECT;
	}else
		shader->mode = R_SIM_FLAT;
	shader->mode = R_SIM_FLAT;
//	shader->mode = 0;
	for(i = 0; i < 31 && name[i] != 0; i++)
		shader->name[i] = name[i];
    shader->name[i] = 0;
    input_count = r_parse_shaders(output, shaders, stages, stage_count, input, shader->mode, r_shader_glsl_version, FALSE, instance_block, &shader->instance_block);
/*	output[0] = experimental_shader_vertex_test2;
	output[1] = experimental_shader_fragment_test2;*/

/*	for(j = 0; j < stage_count; j++)
		printf(shaders[j]);*/
	if(r_shader_debug_override_stage < stage_count)
	{
		output[r_shader_debug_override_stage] = r_shader_debug_override_shader;
		r_shader_debug_override_stage = -1;
	}

	for(i = 0; i < stage_count; i++)
	{
        char *read;
		text[0] = output[i];
	//	text[0] = shaders[i];
        read = output[i];
		stage_obj[i] = r_glCreateShaderObjectARB(stages[i]);
		r_glShaderSourceARB(stage_obj[i], 1, text/*(const char **)&vertex*/, NULL);
		r_glCompileShaderARB(stage_obj[i]);
		r_glGetShaderivARB(stage_obj[i], GL_COMPILE_STATUS, &status);


	/*	{
			char log[2048];
			uint size = 2048;
			r_glGetShaderInfoLog(stage_obj[i], 2048, &size, log);
			printf("LOG: %s\n", log);
		}*/

		if(!status)
		{
			if(debug_output == NULL)
			{
				output_size = 1024;
				debug_output = malloc(output_size);
			}
			sprintf(debug_output, "RELINQUISH: Shader Debug: %s\n", name);
			
			for(j = 0; j < stage_count; j++)
				printf(output[j]);
	//			printf(shaders[j]);

			for(add_size = 0; debug_output[add_size] != 0; add_size++);
			output_size -= add_size;
			r_glGetShaderInfoLog(stage_obj[i], output_size, &size, &debug_output[add_size]);
			add_size += size;
			output_size -= size;
			for(j = 0; j < i + 1; j++)
				r_glDeleteObjectARB(stage_obj[j]);
			
			printf(debug_output);
			exit(0);
		//	free(debug_output);
			for(i = 0; i < stage_count; i++)
				if(output[i] != r_shader_debug_override_shader)
					free(output[i]);
			return NULL;
		}
	}
	shader->program = prog_obj = r_glCreateProgramObjectARB();
	for(i = 0; i < stage_count; i++)
		r_glAttachObjectARB(prog_obj, stage_obj[i]);

	for(shader->attribute_count = i = 0; i < input_count; i++)
		if(input[i].qualifyer == R_SIQ_ATTRIBUTE)
			shader->attribute_count++;
	shader->attributes = malloc((sizeof *shader->attributes) * shader->attribute_count);
	if(shader->attribute_count == 0)
		shader->attribute_count = 0;
	shader->attribute_count = 0;
	for(i = 0; i < input_count; i++)
	{
		if(input[i].qualifyer == R_SIQ_ATTRIBUTE)
		{
			shader->attributes[shader->attribute_count] = input[i];
			shader->attributes[shader->attribute_count].id = shader->attribute_count;
			r_glBindAttribLocationARB(prog_obj, shader->attribute_count, input[i].name);
			shader->attribute_count++;
		}
	}
	shader->buffer_output_component_count = 0;
	if(r_glTransformFeedbackVaryings != NULL)
	{	
		for(i = 1; i < stage_count && stages[i] != GL_FRAGMENT_SHADER_ARB; i++);
		if(i < stage_count)
		{
			char *names[64];
			uint output_count = 0;
			for(j = 0; j < input_count; j++)
			{
				if(input[j].qualifyer == R_SIQ_INOUT && input[j].stages[i] && input[j].stages[i - 1])
				{
					names[output_count++] = input[j].name;
					shader->buffer_input_component_types[shader->buffer_output_component_count++] = input[j].type;
				}
			}
			r_glTransformFeedbackVaryings(prog_obj, output_count, names, GL_INTERLEAVED_ATTRIBS);
		}
	}

	r_glLinkProgramARB(prog_obj);
	r_glGetProgramivARB(prog_obj, GL_LINK_STATUS, &status);
	if(!status)
	{
		if(debug_output == NULL)
		{
			output_size = 1024;
			debug_output = malloc(output_size);
		}
		sprintf(debug_output, "RELINQUISH: Link Program Debug: %s\n", name);
		for(add_size = 0; debug_output[add_size] != 0; add_size++);
		output_size -= add_size;
		r_glGetProgramInfoLog(prog_obj, output_size, &size, &debug_output[add_size]);
		add_size += size;
		output_size -= size;
		printf(debug_output);
		for(i = 0; i < stage_count; i++)
			r_glDeleteObjectARB(stage_obj[i]);
		printf(debug_output);
		exit(0);
		r_glDeleteObjectARB(prog_obj);
		free(shader);
		for(i = 0; i < stage_count; i++)
			if(output[i] != r_shader_debug_override_shader)
				free(output[i]);
		return NULL;
	}
	r_glUseProgramObjectARB(prog_obj);
//	r_experimental_print_uniforms(prog_obj);
	
	for(i = 0; i < input_count; i++)
		if(input[i].array_length == 0)
			input[i].array_length = 1;

	for(shader->uniform_count = i = 0; i < input_count; i++)
		if(input[i].qualifyer == R_SIQ_UNIFORM)
			shader->uniform_count++;
	shader->uniforms = malloc((sizeof *shader->uniforms) * shader->uniform_count);
	shader->uniform_count = 0;
	texture_count = 0;
	for(i = 0; i <  input_count; i++)
	{
		if(input[i].qualifyer == R_SIQ_UNIFORM)
		{
			shader->uniforms[shader->uniform_count] = input[i];
			shader->uniforms[shader->uniform_count].id = r_glGetUniformLocationARB(prog_obj, input[i].name);
			if(shader->uniforms[shader->uniform_count].type >= R_TYPE_TEXTURE_START)
			{
				r_glUniform1iARB(shader->uniforms[shader->uniform_count].id, texture_count);
				shader->uniforms[shader->uniform_count].id = texture_count++;
			}
			shader->uniform_count++;
		}
	}

	for(shader->block_count = shader->uniform_data_size = i = 0; i < input_count; i++)
	{
		if(input[i].qualifyer == R_SIQ_BLOCK)
		{
			if(shader->instance_block == shader->block_count)
				shader->uniform_data_size += input[i].size;
			else
				shader->uniform_data_size += input[i].size * input[i].array_length;
			shader->block_count++;
		}
	}
	shader->blocks = malloc((sizeof *shader->blocks) * shader->block_count);
	shader->block_count = 0;
	bind_point = 1;
	for(i = 0; i <  input_count; i++)
	{
		if(input[i].qualifyer == R_SIQ_BLOCK)
		{
			shader->blocks[shader->block_count] = input[i];

			shader->blocks[shader->block_count].object = -1;
			shader->blocks[shader->block_count].updated = FALSE;
			for(j = 0; j < input_count && (input[j].block != shader->block_count || input[j].type < R_TYPE_TEXTURE_START); j++);
			if(j < input_count)
				shader->blocks[shader->block_count].qualifyer = R_SIQ_BLOCK_WITH_TEXTURE;

			if(shader->mode != R_SIM_FLAT)
			{
				uint index;
				if(shader->instance_block == shader->block_count)
					j = 0;
				else
					j = bind_point++;
				index = r_glGetUniformBlockIndex(shader->program, shader->blocks[shader->block_count].name);
				if(index != -1)
					r_glUniformBlockBinding(shader->program, index, j);
				shader->blocks[shader->block_count].id = j;
				if(shader->blocks[shader->block_count].qualifyer == R_SIQ_BLOCK || r_glGetTextureHandleARB != NULL)
				{
					r_glGenBuffersARB(1, &shader->blocks[shader->block_count].object);
					r_glBindBufferARB(GL_UNIFORM_BUFFER, shader->blocks[shader->block_count].object);
					r_glBufferDataARB(GL_UNIFORM_BUFFER, shader->blocks[shader->block_count].size * shader->blocks[shader->block_count].array_length, NULL, GL_DYNAMIC_DRAW_ARB);
				}				
			}
			shader->block_count++;
		}
	}
	if(shader->mode != R_SIM_FLAT)
		r_glBindBufferARB(GL_UNIFORM_BUFFER, 0);
	if(shader->uniform_data_size != 0)
		shader->uniform_data = malloc(shader->uniform_data_size);	
	else 
		shader->uniform_data = NULL;

	if(debug_output != NULL)
	{
		r_glGetProgramInfoLog(prog_obj, output_size, &size, &debug_output[add_size]);
		add_size += size;
		output_size -= size;
	}
	r_shader_matrix_parse(shader);
	r_shader_state_set_default(&shader->state);
	for(i = 0; i < stage_count; i++)
		if(output[i] != r_shader_debug_override_shader)
			free(output[i]);
	return shader;
}

void *r_shader_create_simple(char *debug_output, uint output_size, char *vertex, char *fragment, char *name)
{
	char *shaders[2];
	uint stages[2];
	stages[0] = GL_VERTEX_SHADER_ARB;
	stages[1] = GL_FRAGMENT_SHADER_ARB;
	shaders[0] = vertex;
	shaders[1] = fragment;
	return r_shader_create(debug_output, output_size, shaders, stages, 2, name, NULL);
}


void *r_shader_create_from_file(char *debug_output, uint output_size, char *vertex_file, char *fragment_file, char *name)
{
	RShader	*shader;
	uint size;
	char *vertex, *fragment;
	FILE *f;
	f = fopen(vertex_file, "r");
	if(f == NULL)
		return 0;
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	rewind(f);
	vertex = malloc(size + 1);
	size = fread(vertex, 1, size, f);
	fclose(f);
	vertex[size] = 0;
	f = fopen(fragment_file, "r");
	if(f == NULL)
	{
		free(vertex);
		return 0;
	}
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	rewind(f);
	fragment = malloc(size + 1);
	size = fread(fragment, 1, size, f);
	fclose(f);
	fragment[size] = 0;
//	printf(fragment);
//	printf(vertex);
	shader = r_shader_create_simple(debug_output, output_size, vertex, fragment, name);
	free(fragment);
	free(vertex);
	return shader;
}

void r_shader_destroy(RShader	*s)
{
	if(s->attributes != NULL)
		free(s->attributes);
	if(s->uniforms != NULL)
		free(s->uniforms);
	if(s->blocks != NULL)
		free(s->blocks);
	r_glDeleteObjectARB(s->program);
	free(s);
}



boolean r_shader_set(RShader *s)
{
	if(s == NULL)
	{
		RState state;
		r_glUseProgramObjectARB(0);
		r_current_shader = NULL;
		r_shader_state_set_default(&state);
		r_shader_state_set(&state);
		return TRUE;
	}
	if(r_current_shader != s)
	{
		r_shader_state_set(&s->state);
		r_glUseProgramObjectARB(s->program);
		r_current_shader = s;
	}
	return TRUE;
}

void unbind_textures()
{
	uint i;
	for(i = 0; i < 64; i++)
	{
		r_glActiveTextureARB(GL_TEXTURE0_ARB + i);
		glBindTexture(R_SIT_SAMPLER_2D, -1);
	}
}

uint r_texture_gen(void *data, uint type, uint x_size, uint y_size, boolean alpha)
{
	uint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	if(alpha)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x_size, y_size, 0, GL_RGBA, type, data);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x_size, y_size, 0, GL_RGB, type, data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	return texture_id;
}

void r_shader_texture_unit_set(RShader	*s, char *name, uint id)
{
	r_glUniform1iARB(r_glGetUniformLocationARB(s->program, name), id);
}

RFormats *r_shader_input_fomat_components_get(RShader *s)
{
	return s->buffer_input_component_types;
}

uint r_shader_input_fomat_count_get(RShader *s)
{
	return s->buffer_input_component_count;
}

RFormats *r_shader_output_fomat_components_get(RShader *s)
{
	return s->buffer_output_component_types;
}

uint r_shader_output_fomat_count_get(RShader *s)
{
	return s->buffer_output_component_count;
}

void *r_array_allocate(uint vertex_count, RFormats *vertex_format_types, uint *vertex_format_size, uint vertex_format_count, uint reference_count);

void *r_array_allocate_from_shader(RShader *s, uint vertex_count, uint reference_count, boolean in)
{
	if(in)
		return r_array_allocate(vertex_count, s->buffer_input_component_types, NULL, s->buffer_input_component_count, reference_count);
	else
		return r_array_allocate(vertex_count, s->buffer_output_component_types, NULL, s->buffer_output_component_count, reference_count);
}
