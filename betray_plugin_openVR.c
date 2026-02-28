#include <stdio.h>
#include <math.h>
#include "betray_plugin_api.h"

#pragma comment(lib, "OpenGL32.lib")

#define GL_RENDERBUFFER_EXT                    0x8D41
#define GL_DEPTH24_STENCIL8_EXT                0x88F0
#define GL_FRAMEBUFFER_EXT                     0x8D40
#define GL_COLOR_ATTACHMENT0_EXT               0x8CE0
#define GL_DEPTH_ATTACHMENT_EXT                0x8D00
#define GL_FRAMEBUFFER_COMPLETE_EXT            0x8CD5

void *(*betray_plugin_open_vr_gl_GetProcAddress)(const char* proc) = NULL;

void (APIENTRY *plug_glFramebufferRenderbufferEXT)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) = NULL;
void (APIENTRY *plug_glGenRenderbuffersEXT)(GLsizei n, GLuint *renderbuffers) = NULL;
void (APIENTRY *plug_glBindRenderbufferEXT)(GLenum target, GLuint renderbuffer) = NULL;
void (APIENTRY *plug_glRenderbufferStorageEXT)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) = NULL;
void (APIENTRY *plug_glGenFramebuffersEXT)(GLsizei n, GLuint *framebuffers) = NULL;
void (APIENTRY *plug_glBindFramebufferEXT)(GLenum target, GLuint framebuffer) = NULL;
void (APIENTRY *plug_glFramebufferTexture2DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) = NULL;
GLenum (APIENTRY *plug_glCheckFramebufferStatusEXT)(GLenum target) = NULL;

uint plug_texture_ids[2];
uint plug_framebuffer_ids[2];
uint plug_texture_resolution[2];

uint betray_plugin_open_vr_texture_allocate(uint x, uint y)
{
	uint texture_id;
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	return texture_id;
}

uint betray_plugin_open_vr_depth_allocate(uint x, uint y)
{
	uint texture_id;
	plug_glGenRenderbuffersEXT(1, &texture_id);		
	plug_glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, texture_id);
	plug_glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT, x, y);
	return texture_id;
}

uint betray_plugin_open_vr_framebuffer_allocate(uint texture_id, uint depth_id)
{
	uint fbo;
	plug_glGenFramebuffersEXT(1, &fbo);
	plug_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo); 
	plug_glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture_id, 0);
	plug_glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_id);
	if(plug_glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
		return -1;
	return fbo;
}

boolean betray_plugin_image_warp(BInputState *input)
{
	float view[3] = {0, 0, 1};

	/* get head tracking */

	/* set camera for one eye */
	betray_plugin_application_draw(plug_framebuffer_ids[0], plug_texture_resolution[0], plug_texture_resolution[1], view, TRUE, NULL);
	/* set camera for other eye */
	betray_plugin_application_draw(plug_framebuffer_ids[1], plug_texture_resolution[0], plug_texture_resolution[1], view, TRUE, NULL);
	/* submit */

	return TRUE;
}


void betray_plugin_init(void)
{
	uint x, y;
	uint i, depth;

	/* Check if HMD is connected, if not return*/

	betray_plugin_callback_set_image_warp(betray_plugin_image_warp, "Anaglyph 3D");
	betray_plugin_open_vr_gl_GetProcAddress = betray_plugin_gl_proc_address_get();
	plug_glFramebufferRenderbufferEXT = betray_plugin_open_vr_gl_GetProcAddress("glFramebufferRenderbufferEXT");
	plug_glGenRenderbuffersEXT = betray_plugin_open_vr_gl_GetProcAddress("glFramebufferRenderbufferEXT");
	plug_glBindRenderbufferEXT = betray_plugin_open_vr_gl_GetProcAddress("glFramebufferRenderbufferEXT");
	plug_glRenderbufferStorageEXT = betray_plugin_open_vr_gl_GetProcAddress("glFramebufferRenderbufferEXT");
	plug_glGenFramebuffersEXT = betray_plugin_open_vr_gl_GetProcAddress("glFramebufferRenderbufferEXT");
	plug_glBindFramebufferEXT = betray_plugin_open_vr_gl_GetProcAddress("glFramebufferRenderbufferEXT");
	plug_glFramebufferTexture2DEXT = betray_plugin_open_vr_gl_GetProcAddress("glFramebufferRenderbufferEXT");
	plug_glCheckFramebufferStatusEXT = betray_plugin_open_vr_gl_GetProcAddress("glFramebufferRenderbufferEXT");

	/* query recomended resolution */
	plug_texture_resolution[0] = 1024;
	plug_texture_resolution[1] = 1024;

	depth = betray_plugin_open_vr_depth_allocate(plug_texture_resolution[0], plug_texture_resolution[1]);

	for(i = 0; i < 2; i++)
	{
		plug_texture_ids[i] = betray_plugin_open_vr_texture_allocate(plug_texture_resolution[0], plug_texture_resolution[1]);
		plug_framebuffer_ids[i] = betray_plugin_open_vr_framebuffer_allocate(plug_texture_ids[i], depth);
	}
}
