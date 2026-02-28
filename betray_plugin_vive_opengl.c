
#include <windows.h>
#include <GL/glu.h>
#include <stdio.h>
#include <math.h>
#include "forge.h"
#include "betray_plugin_api.h"

#define GL_RENDERBUFFER_EXT                    0x8D41
#define GL_DEPTH24_STENCIL8_EXT                0x88F0
#define GL_FRAMEBUFFER_EXT                     0x8D40
#define GL_COLOR_ATTACHMENT0_EXT               0x8CE0
#define GL_DEPTH_ATTACHMENT_EXT                0x8D00
#define GL_FRAMEBUFFER_COMPLETE_EXT            0x8CD5

typedef unsigned int GLhandleARB;

void *(*betray_plugin_open_vr_gl_GetProcAddress)(const char* proc) = NULL;

void (APIENTRY *plug_glFramebufferRenderbufferEXT)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) = NULL;
void (APIENTRY *plug_glGenRenderbuffersEXT)(GLsizei n, GLuint *renderbuffers) = NULL;
void (APIENTRY *plug_glBindRenderbufferEXT)(GLenum target, GLuint renderbuffer) = NULL;
void (APIENTRY *plug_glRenderbufferStorageEXT)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) = NULL;
void (APIENTRY *plug_glGenFramebuffersEXT)(GLsizei n, GLuint *framebuffers) = NULL;
void (APIENTRY *plug_glBindFramebufferEXT)(GLenum target, GLuint framebuffer) = NULL;
void (APIENTRY *plug_glFramebufferTexture2DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) = NULL;
GLenum(APIENTRY *plug_glCheckFramebufferStatusEXT)(GLenum target) = NULL;
GLhandleARB (APIENTRY *plug_glCreateShaderObjectARB)(GLenum  shaderType);
GLhandleARB (APIENTRY *plug_glCreateProgramObjectARB)(GLvoid);
GLvoid      (APIENTRY *plug_glAttachObjectARB)(GLhandleARB containerObj, GLhandleARB obj);
GLvoid      (APIENTRY *plug_glShaderSourceARB)(GLhandleARB shaderObj, GLsizei count, const char **string, const GLint *length);
GLvoid      (APIENTRY *plug_glCompileShaderARB)(GLhandleARB shaderObj);
GLvoid      (APIENTRY *plug_glLinkProgramARB)(GLhandleARB programObj);
GLvoid      (APIENTRY *plug_glUseProgramObjectARB)(GLhandleARB programObj);
GLint       (APIENTRY *plug_glGetUniformLocationARB)(GLhandleARB programObj, const char *name);
GLvoid      (APIENTRY *plug_glUniform1fARB)(GLint location, GLfloat v0);
GLvoid      (APIENTRY *plug_glUniform1iARB)(GLint location, GLint i);
GLvoid		(APIENTRY *plug_glUniformMatrix4fvARB)(GLint location, GLsizei count, GLboolean transpose, GLfloat *value);
GLvoid		(APIENTRY *plug_glGetShaderivARB)(GLuint shader,  GLenum pname,  GLint *params);

GLvoid		(APIENTRY *plug_glEnableVertexAttribArrayARB)(GLuint  index);
GLvoid		(APIENTRY *plug_glDisableVertexAttribArrayARB)(GLuint  index);
GLvoid		(APIENTRY *plug_glVertexAttribPointerARB)(GLuint  index,  GLint  size,  GLenum  type,  GLboolean  normalized,  GLsizei  stride,  const GLvoid *  pointer);


uint plug_texture_ids[2];
uint plug_framebuffer_ids[2];
uint plug_texture_resolution[2];
uint plug_program;

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
	if (plug_glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
		return -1;
	return fbo;
}

char *vertex_shader = 
"attribute vec4 vertex;"
"uniform mat4 ModelViewProjectionMatrix;"
"varying vec2 mapping;"
"void main()"
"{"
"	mapping = vertex.xy * vec2(0.5) + vec2(0.5);"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex.xyz, 1.0);"
"}";

char *fragment_shader = 
"uniform sampler2D image;"
"varying vec2 mapping;"
"void main()"
"{"
"	gl_FragColor = texture2D(image, mapping).rgba;"
"}";


#define GL_VERTEX_SHADER_ARB				0x8B31
#define GL_FRAGMENT_SHADER_ARB				0x8B30

void betray_plugin_open_vr_opengl_init()
{
	uint fragment, vertex;
	betray_plugin_open_vr_gl_GetProcAddress = (void*(*)(const char *))betray_plugin_gl_proc_address_get();
	plug_glFramebufferRenderbufferEXT = (void(_stdcall*)(GLenum, GLenum, GLenum, GLuint))betray_plugin_open_vr_gl_GetProcAddress("glFramebufferRenderbufferEXT");
	plug_glGenRenderbuffersEXT = (void(_stdcall*)(GLsizei, GLuint *))betray_plugin_open_vr_gl_GetProcAddress("glGenRenderbuffersEXT");
	plug_glBindRenderbufferEXT = (void(_stdcall*)(GLenum, GLuint))betray_plugin_open_vr_gl_GetProcAddress("glBindRenderbufferEXT");
	plug_glRenderbufferStorageEXT = (void(_stdcall*)(GLenum, GLenum, GLsizei, GLsizei))betray_plugin_open_vr_gl_GetProcAddress("glRenderbufferStorageEXT");
	plug_glGenFramebuffersEXT = (void(_stdcall*)(GLsizei, GLuint*))betray_plugin_open_vr_gl_GetProcAddress("glGenFramebuffersEXT");
	plug_glBindFramebufferEXT = (void(_stdcall*)(GLenum, GLuint))betray_plugin_open_vr_gl_GetProcAddress("glBindFramebufferEXT");
	plug_glFramebufferTexture2DEXT = (void(_stdcall*)(GLenum, GLenum, GLenum, GLuint, GLint))betray_plugin_open_vr_gl_GetProcAddress("glFramebufferTexture2DEXT");
	plug_glCheckFramebufferStatusEXT = (GLenum(_stdcall*)(GLenum))betray_plugin_open_vr_gl_GetProcAddress("glCheckFramebufferStatusEXT");

	plug_glCreateShaderObjectARB =	(GLhandleARB(_stdcall*)(GLenum  shaderType))betray_plugin_open_vr_gl_GetProcAddress("glCreateShaderObjectARB");	
	plug_glCreateProgramObjectARB =	(GLhandleARB(_stdcall*)(GLvoid))betray_plugin_open_vr_gl_GetProcAddress("glCreateProgramObjectARB");
	plug_glAttachObjectARB = (GLvoid(_stdcall*)(GLhandleARB containerObj, GLhandleARB obj))betray_plugin_open_vr_gl_GetProcAddress("glAttachObjectARB");
	plug_glShaderSourceARB = (GLvoid(_stdcall*)(GLhandleARB shaderObj, GLsizei count, const char **string, const GLint *length))betray_plugin_open_vr_gl_GetProcAddress("glShaderSourceARB");
	plug_glCompileShaderARB = (GLvoid(_stdcall*)(GLhandleARB shaderObj))betray_plugin_open_vr_gl_GetProcAddress("glCompileShaderARB");
	plug_glLinkProgramARB =	(GLvoid(_stdcall*)(GLhandleARB programObj))betray_plugin_open_vr_gl_GetProcAddress("glLinkProgramARB");
	plug_glUseProgramObjectARB =(GLvoid(_stdcall*)(GLhandleARB programObj))betray_plugin_open_vr_gl_GetProcAddress("glUseProgramObjectARB");
	plug_glGetUniformLocationARB = (GLint(_stdcall*)(GLhandleARB programObj, const char *name))betray_plugin_open_vr_gl_GetProcAddress("glGetUniformLocationARB");
	plug_glUniform1fARB =  (GLvoid(_stdcall*)(GLint location, GLfloat v0))betray_plugin_open_vr_gl_GetProcAddress("glUniform1fARB");
	plug_glUniform1iARB = (GLvoid(_stdcall*)(GLint location, GLint i))betray_plugin_open_vr_gl_GetProcAddress("glUniform1iARB");
	plug_glUniformMatrix4fvARB = (GLvoid(_stdcall*)(GLint location, GLsizei count, GLboolean transpose, GLfloat *value))betray_plugin_open_vr_gl_GetProcAddress("glUniformMatrix4fvARB");
	plug_glGetShaderivARB =	(GLvoid(_stdcall*)(GLuint shader,  GLenum pname,  GLint *params))betray_plugin_open_vr_gl_GetProcAddress("glGetShaderiv");


	plug_glEnableVertexAttribArrayARB = (GLvoid (_stdcall*)(GLuint  index))betray_plugin_open_vr_gl_GetProcAddress("glEnableVertexAttribArrayARB");
	plug_glDisableVertexAttribArrayARB = (GLvoid (_stdcall*)(GLuint  index))betray_plugin_open_vr_gl_GetProcAddress("glDisableVertexAttribArrayARB");
	plug_glVertexAttribPointerARB = (GLvoid (_stdcall*)(GLuint  index,  GLint  size,  GLenum  type,  GLboolean  normalized,  GLsizei  stride,  const GLvoid *  pointer))betray_plugin_open_vr_gl_GetProcAddress("glVertexAttribPointerARB");

	fragment = plug_glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	plug_glShaderSourceARB(fragment, 1, &fragment_shader, NULL);
	plug_glCompileShaderARB(fragment);
	vertex = plug_glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	plug_glShaderSourceARB(vertex, 1, &vertex_shader, NULL);
	plug_glCompileShaderARB(vertex);
	plug_program = plug_glCreateProgramObjectARB();
	plug_glAttachObjectARB(plug_program, fragment);
	plug_glAttachObjectARB(plug_program, vertex);
	plug_glLinkProgramARB(plug_program);
}


void betray_plugin_open_vr_opengl_draw(uint texture_id)
{
	float matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -1.0, 1};
	float array[] = {1, 1, 0, 1,
					-1, 1, 0, 1,
					-1, -1, 0, 1,
					1, 1, 0, 1,
					-1, -1, 0, 1,
					1, -1, 0, 1};
	uint x, y;
	betray_plugin_screen_mode_get(&x, &y, NULL);
	glViewport(0, 0, x, y);
	plug_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	plug_glUseProgramObjectARB(plug_program);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	plug_glUniformMatrix4fvARB(plug_glGetUniformLocationARB(plug_program, "ModelViewProjectionMatrix"), 1, FALSE, matrix);
	plug_glEnableVertexAttribArrayARB(0);
	plug_glVertexAttribPointerARB(0,  4,  GL_FLOAT, FALSE, 0, array);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	plug_glDisableVertexAttribArrayARB(0);
	plug_glUseProgramObjectARB(0);


}


