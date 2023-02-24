#include "betray.h"
#define RELINQUISH_INTERNAL
#define RShader void
#include "relinquish.h"
#undef RShader 

#define RELINQUISH_RESOURCE_DEBUGGER

#ifndef _WIN32
#define APIENTRY
#endif

#ifdef BETRAY_CONTEXT_OPENGLES
#define RELINQUISH_CONTEXT_OPENGLES
#endif
#ifdef BETRAY_CONTEXT_OPENGL
#define RELINQUISH_CONTEXT_OPENGL
#endif

#define R_INTERNAL

#define GL_TEXTURE0_ARB								0x84C0

#define GL_FRAMEBUFFER_EXT                     0x8D40
#define GL_RENDERBUFFER_EXT                    0x8D41

#define GL_STENCIL_INDEX1_EXT                  0x8D46
#define GL_STENCIL_INDEX4_EXT                  0x8D47
#define GL_STENCIL_INDEX8_EXT                  0x8D48
#define GL_STENCIL_INDEX16_EXT                 0x8D49

#define GL_RENDERBUFFER_WIDTH_EXT              0x8D42
#define GL_RENDERBUFFER_HEIGHT_EXT             0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT_EXT    0x8D44
#define GL_RENDERBUFFER_RED_SIZE_EXT           0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE_EXT         0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE_EXT          0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE_EXT         0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE_EXT         0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE_EXT       0x8D55

#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT            0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT            0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT          0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT  0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT     0x8CD4

#define GL_COLOR_ATTACHMENT0_EXT                0x8CE0
#define GL_DEPTH_ATTACHMENT_EXT                 0x8D00
#define GL_STENCIL_ATTACHMENT_EXT               0x8D20

#define GL_FRAMEBUFFER_COMPLETE_EXT                          0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT             0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT     0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT             0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT                0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT            0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT            0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT                       0x8CDD

#define GL_FRAMEBUFFER_BINDING_EXT             0x8CA6
#define GL_RENDERBUFFER_BINDING_EXT            0x8CA7
#define GL_MAX_COLOR_ATTACHMENTS_EXT           0x8CDF
#define GL_MAX_RENDERBUFFER_SIZE_EXT           0x84E8

#define GL_DEPTH_COMPONENT16				0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#define GL_STENCIL_INDEX8	             0x8D48
#define GL_CLAMP_TO_EDGE                  0x812F

#define GL_EXT_texture_cube_map             1 
#define GL_NORMAL_MAP_EXT                   0x8511 
#define GL_REFLECTION_MAP_EXT               0x8512 
#define GL_TEXTURE_CUBE_MAP_EXT             0x8513 
#define GL_TEXTURE_BINDING_CUBE_MAP_EXT     0x8514 
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT  0x8515 
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT  0x8516 
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT  0x8517 
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT  0x8518 
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT  0x8519 
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT  0x851A 
#define GL_PROXY_TEXTURE_CUBE_MAP_EXT       0x851B 
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT    0x851C 

#define GL_VERTEX_SHADER_ARB				0x8B31
#define GL_FRAGMENT_SHADER_ARB				0x8B30
#define GL_GEOMETRY_SHADER_ARB				0x8DD9
#define GL_COMPILE_STATUS					0x8B81
#define GL_LINK_STATUS						0x8B82

#define GL_SAMPLE_ALPHA_TO_COVERAGE			0x809E

#define GL_UNIFORM_BUFFER                   0x8A11
#define GL_SHADER_STORAGE_BUFFER			0x90D2
#define GL_ARRAY_BUFFER_ARB                 0x8892
#define GL_ELEMENT_ARRAY_BUFFER_ARB			0x8893
#define GL_COPY_READ_BUFFER					0x8F36
#define GL_DYNAMIC_DRAW_ARB					0x88E8


/* Defined in r_draw_uniform.c */

extern uint64 (APIENTRY *r_glGetTextureHandleARB)(uint texture);
extern uint64 (APIENTRY *r_glGetTextureSamplerHandleARB)(uint texture, uint sampler);
extern void (APIENTRY *r_glMakeTextureHandleResidentARB)(uint64 handle);
extern void (APIENTRY *r_glMakeTextureHandleNonResidentARB)(uint64 handle);


/* Defined in r_draw_buffers.c */

extern GLvoid (APIENTRY *r_glBindBufferARB)(uint target, uint buffer);
extern GLvoid (APIENTRY *r_glDeleteBuffersARB)(uint n, const uint *buffers);
extern GLvoid (APIENTRY *r_glGenBuffersARB)(uint n, uint *buffers);
extern GLvoid (APIENTRY *r_glBufferDataARB)(uint target, uint size, const void *data, uint usage);

#if defined(__APPLE__) || defined(__ANDROID__) || defined(_WIN32)
typedef unsigned int GLhandleARB;
#endif

typedef struct{
	uint depth_test;
	uint cull_face;
	uint blend_dest;
	uint blend_source;
	GLfloat offset_factor; 
	GLfloat offset_units;
	boolean alpha_to_coverage;
	boolean depth_mask;
	boolean color_mask[4];
	struct{
		uint32 function;
		uint32 reference;
		uint32 mask;
		uint32 stencil_fail_op;
		uint32 depth_fail_op;
		uint32 sucsess_op;
	}steincil[2];
}RState;






#define R_MAX_SHADER_STAGES 8

typedef enum{
	R_SIM_FLAT,
	R_SIM_UNIFORM_BLOCK,
	R_SIM_BUFFER_OBJECT,
	R_SIM_COUNT
}RShaderInputMode;

typedef enum{
	R_SIQ_UNIFORM,
	R_SIQ_TEXTURE,
	R_SIQ_ATTRIBUTE,
	R_SIQ_INOUT,
	R_SIQ_BUFFER,
	R_SIQ_BLOCK,
	R_SIQ_BLOCK_WITH_TEXTURE,
	R_SIQ_COUNT
}RShaderInputQualifyer;


extern char *r_qualifyer_names[R_SIQ_COUNT];

typedef struct{
	char name[32];
	RInputType type;
	RShaderInputQualifyer qualifyer;
	uint id;
	uint offset;
	uint size;
	uint array_length;
	uint block;
	uint object;
	boolean stages[R_MAX_SHADER_STAGES];
	boolean updated; 
}RShaderInput;

typedef struct{
	uint program;
	char name[32];
//	RShaderInput *textures;
//	uint texture_count;
	RShaderInput *attributes;
	uint attribute_count;
	RShaderInput *uniforms;
	uint uniform_count;
	RShaderInput *blocks;
	uint block_count;
	uint instance_block;
	uint8 *uniform_data;
	uint uniform_data_size;
	uint normal_matrix;
	uint model_view_matrix;
	uint projection_matrix;
	uint model_view_projection_matrix;
	RState state;
	RShaderInputMode mode;
	uint buffer_output_component_count;
	RFormats buffer_output_component_types[64];
	uint buffer_input_component_count;
	RFormats buffer_input_component_types[64];
}RShader;


#define R_TYPE_TEXTURE_START R_SIT_SAMPLER_1D

extern char *r_type_names[R_SIT_COUNT];
extern uint r_type_sizes[R_SIT_COUNT];
extern uint r_type_types[R_SIT_COUNT];
extern uint r_type_strides[R_SIT_COUNT];

extern uint r_sampler_names[];


extern RShader *r_current_shader; 
extern void *r_matrix_state;
extern uint r_parse_shaders(char **output, char **shader, uint *stages, uint shader_count, RShaderInput *input, RShaderInputMode mode, uint version, boolean embeded, char *instance_name, uint *instance_id);
extern void r_shader_matrix_parse(RShader	*shader);
extern void r_shader_parse_input_print(RShaderInput *input, uint count);

extern void r_shader_unifrom_data_set_block(RShader *s, uint8 *data, uint block);
extern void r_shader_unifrom_data_set_all(RShader *s, uint8 *data, uint exception);
extern void r_shader_uniform_matrix_set(RShader *shader, uint8 *data, uint block_id, RMatrix *matrix);



