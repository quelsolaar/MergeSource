#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "forge.h"
#include "r_include.h"



void (APIENTRY *r_glBindFramebufferEXT)(GLenum target, GLuint framebuffer) = NULL;
void (APIENTRY *r_glDeleteFramebuffersEXT)(GLsizei n, const GLuint *framebuffers) = NULL;
void (APIENTRY *r_glGenFramebuffersEXT)(GLsizei n, GLuint *framebuffers) = NULL;

void (APIENTRY *r_glBindRenderbufferEXT)(GLenum target, GLuint renderbuffer) = NULL;
void (APIENTRY *r_glDeleteRenderbuffersEXT)(GLsizei n, const GLuint *renderbuffers) = NULL;
void (APIENTRY *r_glGenRenderbuffersEXT)(GLsizei n, GLuint *renderbuffers) = NULL;
GLenum (APIENTRY *r_glCheckFramebufferStatusEXT)(GLenum target) = NULL;
void (APIENTRY *r_glRenderbufferStorageEXT)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) = NULL;

void (APIENTRY *r_glFramebufferTexture2DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) = NULL;
void (APIENTRY *r_glFramebufferRenderbufferEXT)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) = NULL;
void (APIENTRY *r_glDrawBuffersARB)(GLuint, GLenum *buffers) = NULL;

void (APIENTRY *r_glTexImage3D)(GLenum target, int level, GLenum internalformat, uint width, uint height, uint depth, int border, GLenum format, GLenum type, const void* pixels) = NULL;

uint r_framebuffer_debug_fbo_allocated = 0;
uint r_framebuffer_debug_image_allocated = 0;

typedef struct{
	uint fbo_id;
	uint layer_count;
}RFrameBufferObject;


#define       GL_R8                      0x8229
#define       GL_RG8                     0x822B
#define       GL_R16F                    0x822D
#define       GL_R32F                    0x822E
#define       GL_RG16F                   0x822F
#define       GL_RG32F                   0x8230
#define       GL_RED                     0x1903
#define       GL_RG                      0x8227

#define       GL_RGB16F_ARB					0x881B
#define       GL_RGBA16F_ARB				0x881A
#define       GL_RGB32F_ARB					0x8815
#define       GL_RGBA32F_ARB				0x8814

#define       GL_TEXTURE_WRAP_R				0x8072
#define       GL_TEXTURE_3D					0x806F
#define       GL_DEPTH_COMPONENT16			0x81A5

#define       GL_DEPTH24_STENCIL8_EXT		0x88F0
#define       GL_DEPTH_STENCIL_ATTACHMENT	0x821A

boolean r_framebuffer_status(void)
{
    GLenum status;

    if (!r_glBindFramebufferEXT)
        return FALSE;
    status = r_glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    switch(status)
	{
        case GL_FRAMEBUFFER_COMPLETE_EXT:
			return TRUE;
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            fprintf(stderr, "Unsupported framebuffer format\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            fprintf(stderr, "Framebuffer incomplete, missing attachment\n");
            break;
/*		case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
            printf("Framebuffer incomplete, duplicate attachment\n");
            break;*/
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            fprintf(stderr, "Framebuffer incomplete, attached images must have same dimensions\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            fprintf(stderr, "Framebuffer incomplete, attached images must have same format\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            fprintf(stderr, "Framebuffer incomplete, missing draw buffer\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            fprintf(stderr, "Framebuffer incomplete, missing read buffer\n");
            break;
		default:
            fprintf(stderr, "Error id %x\n", status);
            break;
    }
	return FALSE;
}


#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT  0x8515 
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT  0x8516 
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT  0x8517 
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT  0x8518 
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT  0x8519 
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT  0x851A 

void *r_framebuffer_allocate(uint *buffers, uint buffer_count, uint depth_buffer, uint target)
{
	RFrameBufferObject *obj;
	uint i, fbo = 0;
	r_glGenFramebuffersEXT(1, &fbo);
	r_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	if(target >= 6)
		target = GL_TEXTURE_2D;
	else
		target = GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT + target;
	for(i = 0; i < buffer_count; i++)
		r_glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, target, buffers[i], 0);
	if(depth_buffer != -1)
		r_glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_buffer);
	if(!r_framebuffer_status())
	{			
		r_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		return NULL;
	}
	r_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	obj = malloc(sizeof *obj);
	obj->fbo_id = fbo;
	obj->layer_count  = buffer_count; 
	r_framebuffer_debug_fbo_allocated++;
	return obj;
}

void r_framebuffer_bind(void *fbo)
{
	if(fbo == NULL)
	{
		r_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);	
#ifndef BETRAY_CONTEXT_OPENGLES
        glDrawBuffer(GL_BACK);
#endif
	}else
	{
		GLenum attachements[8] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT0_EXT + 1, GL_COLOR_ATTACHMENT0_EXT + 2, GL_COLOR_ATTACHMENT0_EXT + 3, GL_COLOR_ATTACHMENT0_EXT + 4, GL_COLOR_ATTACHMENT0_EXT + 5, GL_COLOR_ATTACHMENT0_EXT + 6, GL_COLOR_ATTACHMENT0_EXT + 7};
		r_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ((RFrameBufferObject *)fbo)->fbo_id);
        if(r_glDrawBuffersARB != NULL)
            r_glDrawBuffersARB(((RFrameBufferObject *)fbo)->layer_count, attachements);
	}
}

uint r_framebuffer_id(void *fbo)
{
	return ((RFrameBufferObject *)fbo)->fbo_id;
}

void r_framebuffer_free(void *fbo)
{
	r_glDeleteFramebuffersEXT(1, &((RFrameBufferObject *)fbo)->fbo_id);
	r_framebuffer_debug_fbo_allocated--;
	free(fbo);
}

void r_framebuffer_clear(float red, float green, float blue, float alpha, boolean color, boolean depth_stencil)
{
	uint32 bits = 0;
/*  static uint rnd = 0;
    glClearColor(f_randf(rnd++), green, blue,  alpha);
*/  glClearColor(red, green, blue, alpha);
	if(color)
	{
		glDepthMask(1);
		bits = GL_COLOR_BUFFER_BIT;
	}
	if(depth_stencil)
	{
		bits = bits | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
		glColorMask(1, 1, 1, 1);
	}
	glClear(bits);
}

boolean r_framebuffer_init(void)
{
    r_glTexImage3D = r_extension_get_address("glTexImage3D");

    r_glDrawBuffersARB = r_extension_get_address("glDrawBuffersARB");
    if( r_glDrawBuffersARB == NULL )
    {
        r_glDrawBuffersARB = r_extension_get_address("glDrawBuffers");
    }
    
#ifndef BETRAY_CONTEXT_OPENGLES
#ifndef __APPLE__
    //FK: Macos OpenGL Extenion list is unreliable :-/
    if(r_extension_test("GL_ARB_framebuffer_object"))
#endif
    {
        r_glBindFramebufferEXT = r_extension_get_address("glBindFramebuffer");
        r_glDeleteFramebuffersEXT = r_extension_get_address("glDeleteFramebuffers");
        r_glGenFramebuffersEXT = r_extension_get_address("glGenFramebuffers");
        r_glBindRenderbufferEXT = r_extension_get_address("glBindRenderbuffer");
        r_glDeleteRenderbuffersEXT = r_extension_get_address("glDeleteRenderbuffers");
        r_glGenRenderbuffersEXT = r_extension_get_address("glGenRenderbuffers");
        r_glCheckFramebufferStatusEXT = r_extension_get_address("glCheckFramebufferStatus");
        r_glRenderbufferStorageEXT = r_extension_get_address("glRenderbufferStorage");
        r_glFramebufferTexture2DEXT = r_extension_get_address("glFramebufferTexture2D");
        r_glFramebufferRenderbufferEXT = r_extension_get_address("glFramebufferRenderbuffer");
        return TRUE;
    }
    return FALSE;
#else
    //FK: Part of OpenGL ES since ES 2.0 - No need to check for extension since there won't be an extension.
    r_glBindFramebufferEXT = glBindFramebuffer;
    r_glDeleteFramebuffersEXT = glDeleteFramebuffers;
    r_glGenFramebuffersEXT = glGenFramebuffers;
    r_glBindRenderbufferEXT = glBindRenderbuffer;
    r_glDeleteRenderbuffersEXT = glDeleteRenderbuffers;
    r_glGenRenderbuffersEXT = glGenRenderbuffers;
    r_glCheckFramebufferStatusEXT = glCheckFramebufferStatus;
    r_glRenderbufferStorageEXT = glRenderbufferStorage;
    r_glFramebufferTexture2DEXT = glFramebufferTexture2D;
    r_glFramebufferRenderbufferEXT = glFramebufferRenderbuffer;
    return TRUE;
#endif
}

uint r_texture_allocate(PImageFormat format, uint x, uint y, uint z, boolean filter, boolean tile, void *data)
{
	uint i, texture_id, type, dimensions = GL_TEXTURE_2D, internal_format, input_format, data_type, stride;
	boolean free_data;

	if(x == 0)
		x = 1;	
	if(y == 0)
		y = 1;	
	if(z == 0)
		z = 1;	

	if(r_glTexImage3D == NULL && z != 1 && z != R_IB_IMAGE_CUBE)
	{
		printf("RELINQUISH Error: Graphics driver does not support 3D textures\n");
		return -1;
	}
	r_framebuffer_debug_image_allocated++;
	if(format >= R_IF_DEPTH16)
	{
		r_glGenRenderbuffersEXT(1, &texture_id);		
		r_glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, texture_id);
		if(format == R_IF_DEPTH16)
			r_glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT16, x, y);
		else if(format == R_IF_DEPTH24)
			r_glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, x, y);
		else if(format == R_IF_DEPTH32)
			r_glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT32, x, y);
		else
			r_glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT, x, y);
		return texture_id;
	}else
	{
		if(z == R_IB_IMAGE_CUBE)
			dimensions = GL_TEXTURE_CUBE_MAP_EXT;
		else if(z != 1)
			dimensions = GL_TEXTURE_3D;

		glEnable(dimensions);
		glGenTextures(1, &texture_id);
		glBindTexture(dimensions, texture_id);

		switch(format)
		{
			case R_IF_R_UINT8 :
				internal_format = GL_R8;
				input_format = GL_RED;
				data_type = GL_UNSIGNED_BYTE;
			break;
			case R_IF_RG_UINT8 :
				internal_format = GL_RG8;
				input_format = GL_RG;
				data_type = GL_UNSIGNED_BYTE;
			break;
			case R_IF_RGB_UINT8 :
				internal_format = GL_RGB;
				input_format = GL_RGB;
				data_type = GL_UNSIGNED_BYTE;
			break;
			case R_IF_RGBA_UINT8 :
				internal_format = GL_RGBA;
				input_format = GL_RGBA;
				data_type = GL_UNSIGNED_BYTE;
			break;
			case R_IF_R_FLOAT16 :
				internal_format = GL_R16F;
				input_format = GL_RED;
				data_type = GL_FLOAT;
			break;
			case R_IF_RG_FLOAT16 :
				internal_format = GL_RG16F;
				input_format = GL_RG;
				data_type = GL_FLOAT;
			break;
			case R_IF_RGB_FLOAT16 :
				internal_format = GL_RGB16F_ARB;
				input_format = GL_RGB;
				data_type = GL_FLOAT;
			break;
			case R_IF_RGBA_FLOAT16 :
				internal_format = GL_RGBA16F_ARB;
				input_format = GL_RGBA;
				data_type = GL_FLOAT;
			break;
			case R_IF_R_FLOAT32 :
				internal_format = GL_R32F;
				input_format = GL_RED;
				data_type = GL_FLOAT;
			break;
			case R_IF_RG_FLOAT32 :
				internal_format = GL_RG32F;
				input_format = GL_RG;
				data_type = GL_FLOAT;
			break;
			case R_IF_RGB_FLOAT32 :
				internal_format = GL_RGB32F_ARB;
				input_format = GL_RGB;
				data_type = GL_FLOAT;
			break;
			case R_IF_RGBA_FLOAT32 :
				internal_format = GL_RGBA32F_ARB;
				input_format = GL_RGBA;
				data_type = GL_FLOAT;
			break;
			default :
				printf("Reliquish Error: Calling r_texture_allocate with ilegal format : %u\n", format);
				exit(0);
			break;
		}
		stride = 3 + format % 2;
		free_data = data == NULL;
		if(free_data)
		{
			float *buf = NULL;
			data_type = GL_FLOAT;
			if(dimensions == GL_TEXTURE_2D)
			{
				
				buf = malloc((sizeof *buf) * x * y * stride);
				for(i = 0; i < x * y; i++)
				{
					buf[i * stride + 0] = (float)(i % x) / (float)x;
					buf[i * stride + 1] = (float)(i / x) / (float)y;
					buf[i * stride + 2] = 1.0;

					if(((i % x) / 16 + (i / x) / 16) % 2 == 0)
					{
						buf[i * stride + 0] = 1.0f - buf[i * stride + 0];
						buf[i * stride + 1] = 1.0f - buf[i * stride + 1];
						buf[i * stride + 2] = 1.0f - buf[i * stride + 2];
					}
					if(stride == 4)
						buf[i * stride + 3] = (float)(((i % x) / 16 + (i / x) / 16) % 2);
				}
			}
			if(dimensions == GL_TEXTURE_3D)
			{
				buf = malloc((sizeof *buf) * x * y * z * stride);
				for(i = 0; i < x * y * z; i++)
				{
					buf[i * stride + 0] = (float)(i % x) / (float)x;
					buf[i * stride + 1] = (float)((i / x) % y) / (float)y;
					buf[i * stride + 2] = (float)(i / (x * y)) / (float)z;

					if(((i % x) / 8 + ((i / x) % y) / 8 + (i / (x * y)) / 8) % 2 == 0)
					{
						buf[i * stride + 0] = 1.0f - buf[i * stride + 0];
						buf[i * stride + 1] = 1.0f - buf[i * stride + 1];
						buf[i * stride + 2] = 1.0f - buf[i * stride + 2];
					}
					if(stride == 4)
						buf[i * stride + 3] = (float)(((i % x) / 8 + ((i / x) % y) / 8 + (i / (x * y)) / 8) % 2);
				}
			}
			if(dimensions == GL_TEXTURE_CUBE_MAP_EXT)
			{
				buf = malloc((sizeof *buf) * x * y * 6 * stride);
				for(i = 0; i < x * y; i++)
				{
					buf[i * stride + 0] = (float)(i % x) / (float)x;
					buf[i * stride + 1] = (float)((i / x) % y) / (float)y;
					buf[i * stride + 2] = (float)(i % x) / (float)x;

					if(((i % x) / 16 + ((i / x) % (x * y)) / 16) % 2 == 0)
					{
						buf[i * stride + 0] = 1.0f - buf[i * stride + 0];
						buf[i * stride + 1] = 1.0f - buf[i * stride + 1];
						buf[i * stride + 2] = 1.0f - buf[i * stride + 2];
					}
					if(stride == 4)
						buf[i * stride + 3] = (float)(((i % x) / 16 + ((i / x) % (x * y)) / 16) % 2);
				}
				for(i = x * y * stride * 0 + 0; i < x * y * stride * 1; i += stride)
					buf[i] = 1.0;
				for(i = x * y * stride * 1 + 1; i < x * y * stride * 2; i += stride)
					buf[i] = 1.0;
				for(i = x * y * stride * 2 + 3; i < x * y * stride * 3; i += stride)
					buf[i] = 1.0;
				for(i = x * y * stride * 3 + 0; i < x * y * stride * 4; i += stride)
					buf[i] = 0.0;
				for(i = x * y * stride * 4 + 1; i < x * y * stride * 5; i += stride)
					buf[i] = 0.0;
				for(i = x * y * stride * 5 + 2; i < x * y * stride * 6; i += stride)
					buf[i] = 0.0;
			}
			data = buf;

		}


		if(dimensions == GL_TEXTURE_CUBE_MAP_EXT)
		{
			if(data == NULL)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT, 0, internal_format, x, y, 0, input_format, data_type, NULL);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT, 0, internal_format, x, y, 0, input_format, data_type, NULL);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT, 0, internal_format, x, y, 0, input_format, data_type, NULL);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT, 0, internal_format, x, y, 0, input_format, data_type, NULL);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT, 0, internal_format, x, y, 0, input_format, data_type, NULL);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT, 0, internal_format, x, y, 0, input_format, data_type, NULL);
			}else
			{
				if(data_type == GL_FLOAT)
				{
					float *d;
					d = data;
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT, 0, internal_format, x, y, 0, input_format, data_type, d);
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT, 0, internal_format, x, y, 0, input_format, data_type, &d[stride * x * y * 1]);
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT, 0, internal_format, x, y, 0, input_format, data_type, &d[stride * x * y * 2]);
					glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT, 0, internal_format, x, y, 0, input_format, data_type, &d[stride * x * y * 3]);
					glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT, 0, internal_format, x, y, 0, input_format, data_type, &d[stride * x * y * 4]);
					glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT, 0, internal_format, x, y, 0, input_format, data_type, &d[stride * x * y * 5]);
				}else
				{
					unsigned char *d;
					d = data;
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT, 0, internal_format, x, y, 0, input_format, data_type, d);
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT, 0, internal_format, x, y, 0, input_format, data_type, &d[stride * x * y * 1]);
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT, 0, internal_format, x, y, 0, input_format, data_type, &d[stride * x * y * 2]);
					glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT, 0, internal_format, x, y, 0, input_format, data_type, &d[stride * x * y * 3]);
					glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT, 0, internal_format, x, y, 0, input_format, data_type, &d[stride * x * y * 4]);
					glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT, 0, internal_format, x, y, 0, input_format, data_type, &d[stride * x * y * 5]);
				}
			}
		}else if(dimensions == GL_TEXTURE_3D)
			r_glTexImage3D(dimensions, 0, internal_format, x, y, z, 0, input_format, data_type, data);
		else
			glTexImage2D(dimensions, 0, internal_format, x, y, 0, input_format, data_type, data);
	
		if(free_data && data != NULL)
			free(data);

		if(tile)
			type = GL_REPEAT;
		else
			type = GL_CLAMP;
		
		glTexParameteri(dimensions, GL_TEXTURE_WRAP_S, type);
		glTexParameteri(dimensions, GL_TEXTURE_WRAP_T, type);
		if(dimensions == GL_TEXTURE_3D)
			glTexParameteri(dimensions, GL_TEXTURE_WRAP_R, type);

		if(filter)
			type = GL_LINEAR;
		else
			type = GL_NEAREST;

		glTexParameteri(dimensions, GL_TEXTURE_MAG_FILTER, type);
		glTexParameteri(dimensions, GL_TEXTURE_MIN_FILTER, type);
	}
	return texture_id;
}

void r_texture_update(uint texture_id, PImageFormat format, uint x_offset, uint y_offset, uint z_offset, uint x_size, uint y_size, uint z_size, void *data)
{	
	uint input_format, data_type;
	glBindTexture(GL_TEXTURE_2D, texture_id);

	switch(format)
	{
		case R_IF_R_UINT8 :
			input_format = GL_RED;
			data_type = GL_UNSIGNED_BYTE;
		break;
		case R_IF_RG_UINT8 :
			input_format = GL_RG;
			data_type = GL_UNSIGNED_BYTE;
		break;
		case R_IF_RGB_UINT8 :
			input_format = GL_RGB;
			data_type = GL_UNSIGNED_BYTE;
		break;
		case R_IF_RGBA_UINT8 :
			input_format = GL_RGBA;
			data_type = GL_UNSIGNED_BYTE;
		break;
		case R_IF_R_FLOAT16 :
			input_format = GL_RED;
			data_type = GL_FLOAT;
		break;
		case R_IF_RG_FLOAT16 :
			input_format = GL_RG;
			data_type = GL_FLOAT;
		break;
		case R_IF_RGB_FLOAT16 :
			input_format = GL_RGB;
			data_type = GL_FLOAT;
		break;
		case R_IF_RGBA_FLOAT16 :
			input_format = GL_RGBA;
			data_type = GL_FLOAT;
		break;
		case R_IF_R_FLOAT32 :
			input_format = GL_RED;
			data_type = GL_FLOAT;
		break;
		case R_IF_RG_FLOAT32 :
			input_format = GL_RG;
			data_type = GL_FLOAT;
		break;
		case R_IF_RGB_FLOAT32 :
			input_format = GL_RGB;
			data_type = GL_FLOAT;
		break;
		case R_IF_RGBA_FLOAT32 :
			input_format = GL_RGBA;
			data_type = GL_FLOAT;
		break;
		default :
			printf("Reliquish Error: Calling r_texture_update with ilegal format : %u\n", format);
			exit(0);
		break;
	}
	glTexSubImage2D(GL_TEXTURE_2D, 0, x_offset, y_offset, x_size, y_size, input_format, data_type, data);
}


void r_texture_free(uint texture_id)
{	
	r_framebuffer_debug_image_allocated--;
	glDeleteTextures(1, &texture_id);
}

uint64 r_shader_uniform_texture_pointer_get(uint texture_id)
{
	if(r_glGetTextureHandleARB == NULL)
		return (uint64)texture_id;
	else
	{
		uint64 handle;
		handle = r_glGetTextureHandleARB(texture_id); 
		r_glMakeTextureHandleResidentARB(handle);
		return handle;
	}
}
