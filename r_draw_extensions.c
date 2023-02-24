#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "forge.h"
#include "r_include.h"


void *(*r_gl_GetProcAddress)(const char* proc) = NULL;
GLubyte *(APIENTRY *r_glGetString)(GLenum name) = NULL;
GLubyte *(APIENTRY *r_glGetStringi)(GLenum name, GLuint index) = NULL;

void r_extension_init(void *(*glGetProcAddr)(const char* proc))
{
    r_gl_GetProcAddress = glGetProcAddr;
    r_glGetString = r_gl_GetProcAddress("glGetString");
    r_glGetStringi = r_gl_GetProcAddress("glGetStringi");
}

void *r_extension_get_address(const char* proc)
{
	if(r_gl_GetProcAddress == NULL)
		return NULL;
	else
		return r_gl_GetProcAddress(proc);
}

#define GL_NUM_EXTENSIONS 0x821d

boolean r_extension_test(const char *string)
{
	const char *extension, *a;
	uint i, j, count = 0;
    if(r_glGetStringi != NULL)
	{
		glGetIntegerv(GL_NUM_EXTENSIONS, &count);
		for(i = 0; i < count; i++)
		{
			extension = r_glGetStringi(GL_EXTENSIONS, i);
        //  printf("extension[%u] = %s\n", i, extension);
			for(j = 0; extension[j] == string[j] && string[j] != 0; j++);
			if(extension[j] == string[j])
				return TRUE;
		}
	}else if(r_glGetString != NULL)
	{
	    extension = r_glGetString(GL_EXTENSIONS);
		if(extension != NULL)
		{
		    for(a = extension; a[0] != 0; a++)
		    {
				for(i = 0; string[i] != 0 && a[i] != 0 && string[i] == a[i]; i++);
			    if(string[i] == 0)
				    return TRUE;
			}
        }
    }
	return FALSE;
}

#define GL_MAX_GEOMETRY_OUTPUT_VERTICES                0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS         0x8DE1

uint r_extension_query(uint query)
{
	uint value = 0;
	switch(query)
	{
		case R_Q_GEOMETRY_OUTPUT_VERTICES_MAX :
			glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &value);
		return value;
		case R_Q_GEOMETRY_OUTPUT_COMPONENTS_MAX :
			glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, &value);
		return value;
	}
	return 0;
}
extern void seduce_primitive_line_init();


extern uint r_framebuffer_debug_fbo_allocated;
extern uint r_framebuffer_debug_image_allocated;
extern uint r_array_debug_allocated;

boolean r_resource_debug()
{
	printf("Relinquish Resorce state:\n");
	printf("FBOs allocated %u\n", r_framebuffer_debug_fbo_allocated);
	printf("Images allocated %u\n", r_framebuffer_debug_image_allocated);
	printf("Arrays allocated %u\n", r_array_debug_allocated);
}


extern void		r_primitive_init(void);
extern boolean	r_shader_init(void);
extern void		r_array_init(void);
extern boolean	r_framebuffer_init(void);
extern void		r_uniform_init();
extern void		r_array_deactivate();
extern boolean	r_shader_set(RShader *shader);
extern void		r_matrix_identity(RMatrix *matrix);
extern void		r_matrix_set(RMatrix *matrix);
extern void		r_matrix_frustum(RMatrix *matrix, float left, float right, float bottom, float top, float znear, float zfar);

void r_enable()
{
	glEnable(GL_STENCIL_TEST);
}

void r_disable()
{
	r_shader_set(NULL);
	r_array_deactivate();
	r_framebuffer_bind(NULL);
	glDisable(GL_STENCIL_TEST);
}

boolean r_init(void *(*glGetProcAddr)(const char* proc))
{
	GLboolean (APIENTRY *SwapInterval)(int interval) = NULL;
	float default_aspect = 9.0 / 16.0;
	uint i;    
	glEnable(GL_STENCIL_TEST);
	r_matrix_set(NULL);
	r_matrix_identity(NULL);
	r_matrix_frustum(NULL, -0.05, 0.05, -0.05 * default_aspect, 0.05 * default_aspect, 0.05, 100.0f);
	r_extension_init(glGetProcAddr);
	
	SwapInterval = r_extension_get_address("wglSwapIntervalEXT");
	if(SwapInterval == NULL)
		SwapInterval = r_extension_get_address("glXSwapIntervalEXT");
//	if(SwapInterval != NULL)
//		SwapInterval(1); 
	if(!r_shader_init())
		return FALSE;
	if(!r_framebuffer_init())
		return FALSE;
    r_uniform_init();
	r_array_init();
	r_primitive_init();
	return TRUE;

}
