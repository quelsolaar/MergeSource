#include "betray_plugin_api.h"
#include "relinquish.h"
#define OVR_OS_WIN32
#include "OVR_CAPI_GL.h"
#include <stdio.h>
#include <math.h>

ovrEyeRenderDesc eye_render_desc[2];
ovrHmd current_hmd;
uint render_size[2] = {1024 , 1024};
uint render_fbos[2];
ovrGLTexture render_textures[2];
ovrVector3f eye_vectors[2];

boolean betray_plugin_image_warp(BInputState *input)
{
	ovrPosef outEyePoses[2];
	ovrTrackingState outHmdTrackingState;
	ovrFrameTiming time;
	ovrVector3f hmdToEyeViewOffset[2];
	float matrix[16];
	float view[3] = {0, 0, 1};
	uint i;
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	time = ovrHmd_BeginFrame(current_hmd, 0);
	ovrHmd_GetEyePoses(current_hmd, 0, eye_vectors,
                                   outEyePoses, &outHmdTrackingState);
	
	for(i = 0; i < 2; i++)
	{
		f_quaternion_to_matrixf(matrix, outEyePoses[i].Orientation.x, outEyePoses[i].Orientation.y, outEyePoses[i].Orientation.z, outEyePoses[i].Orientation.w);
		matrix[12] = outEyePoses[i].Position.x;
		matrix[13] = outEyePoses[i].Position.y;
		matrix[14] = outEyePoses[i].Position.z;
		betray_plugin_application_draw(render_fbos[i], render_size[0], render_size[1], view, TRUE, matrix);
	}
	ovrHmd_EndFrame(current_hmd, outEyePoses, (ovrTexture *)render_textures);
	return TRUE;
}


void betray_plugin_init(void)
{
	uint x, y, i;
	RFormats vertex_format_types[3] = {R_FLOAT, R_FLOAT};
	uint vertex_format_size[3] = {3, 2};
	ovr_InitializeRenderingShim();

	if(ovr_Initialize())
	{
		union{	
		    ovrRenderAPIConfig Config;
		    ovrGLConfigData    OGL;
		}cfg;
		ovrFovPort eye_fov[2];
		uint distortion_caps, result, depth_buffer, x, y;
		ovrSizei size;
		current_hmd = ovrHmd_Create(0);
		if(current_hmd != NULL)
		{
			betray_plugin_callback_set_image_warp(betray_plugin_image_warp, "Occulus Rift");
			ovrHmd_AttachToWindow(current_hmd, betray_plugin_windows_window_handle_get(), NULL, NULL);

			eye_fov[0] = current_hmd->DefaultEyeFov[0];
			eye_fov[1] = current_hmd->DefaultEyeFov[1];
			memset(&cfg, 0, sizeof cfg);
			cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
			betray_plugin_screen_mode_get(&x, &y, NULL);
		/*	cfg.OGL.Header.BackBufferSize.w = 1920;
			cfg.OGL.Header.BackBufferSize.h = 1080;*/
			cfg.OGL.Header.RTSize.w = 1920;
			cfg.OGL.Header.RTSize.h = 1920;
			cfg.OGL.Header.Multisample = 1;
			distortion_caps = ovrDistortionCap_Chromatic | ovrDistortionCap_TimeWarp |  ovrDistortionCap_Vignette | ovrDistortionCap_NoRestore |  ovrDistortionCap_Overdrive | ovrDistortionCap_HqDistortion;
			distortion_caps = 0;
			result = ovrHmd_ConfigureRendering(current_hmd, &cfg.Config, current_hmd->DistortionCaps, current_hmd->MaxEyeFov, eye_render_desc);
		
			size = ovrHmd_GetFovTextureSize(current_hmd, ovrEye_Left, current_hmd->DefaultEyeFov[0], 1);
			render_size[0] = size.h;
			render_size[1] = size.w;

			r_init(betray_plugin_gl_proc_address_get());
			depth_buffer = r_texture_allocate(R_IF_DEPTH24_STENCIL8, render_size[0], render_size[1], 1, TRUE, TRUE, NULL);

			for(i = 0; i < 2; i++)
			{
				eye_vectors[i] = eye_render_desc[i].HmdToEyeViewOffset;
				render_textures[i].OGL.Header.API = ovrRenderAPI_OpenGL;
				render_textures[i].OGL.Header.RenderViewport.Pos.x = 0;
				render_textures[i].OGL.Header.RenderViewport.Pos.y = 0;
				render_textures[i].OGL.Header.RenderViewport.Size.w = render_size[0];
				render_textures[i].OGL.Header.RenderViewport.Size.h = render_size[1];
				render_textures[i].OGL.Header.TextureSize.w = render_size[0];
				render_textures[i].OGL.Header.TextureSize.h = render_size[1];
				render_textures[i].OGL.TexId = r_texture_allocate(R_IF_RGBA_UINT8, render_size[0], render_size[1], 1, TRUE, TRUE, NULL);
				render_fbos[i] = r_framebuffer_allocate(&render_textures[i].OGL.TexId, 1, depth_buffer, RELINQUISH_TARGET_2D_TEXTURE);
			}
		}
	}
}
