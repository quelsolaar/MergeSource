#include "betray_plugin_api.h"
#include "relinquish.h"
//oculus headers
//#define OVR_OS_WIN32
//#include "OVR_CAPI_GL.h"
#include "openvr.h"
#include <GL/glu.h>
#include <stdio.h>
#include <math.h>

vr::IVRSystem *m_pHMD;


boolean betray_plugin_image_warp(BInputState *input)
{


	
}






void betray_plugin_init(void)
{

	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
	
	betray_plugin_callback_set_image_warp(betray_plugin_image_warp, "HTC Vive");
			
	betray_plugin_screen_mode_get(&x, &y, NULL);
			
	render_size[0] = size.h;
	render_size[1] = size.w;

	r_init(betray_plugin_gl_proc_address_get());
	depth_buffer = r_texture_allocate(R_IF_DEPTH24_STENCIL8, render_size[0], render_size[1], 1, TRUE, TRUE, NULL);

			for (i = 0; i < 2; i++)
			{
				
			}
}
