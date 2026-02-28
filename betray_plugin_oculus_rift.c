#include "betray_plugin_api.h"
#include "relinquish.h"
#include <stdio.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif
extern void betray_plugin_oculus_sensor_init(void);
extern void betray_plugin_oculus_sensor_read(float *quaternion);
#ifdef __cplusplus
} // extern "C"
#endif

char *betray_plugin_oculus_vertex_shader =
				"uniform mat4 ModelViewProjectionMatrix;\n"
				"attribute vec4 vertex;\n"
				"attribute vec2 uv_attrib;\n"
				"varying vec2 uv;\n"
                "void main() {\n"
                "   uv = uv_attrib;\n"
				"   gl_Position = ModelViewProjectionMatrix * vec4(vertex.xyz, 1.0);\n"
                "}";
       
char *betray_plugin_oculus_fragment_shader_single_sample =
                "uniform sampler2D tex;\n"
				"varying vec2 uv;\n"
                "uniform vec2 LensCenter;\n"
                "uniform vec2 ScreenCenter;\n"
                "uniform vec2 Scale;\n"
                "uniform vec2 ScaleIn;\n"
                "uniform vec4 HmdWarpParam;\n"
                "\n"
                "vec2 HmdWarp(vec2 texIn)\n"
                "{\n"
                "   vec2 theta = (texIn - LensCenter) * ScaleIn;\n"
                "   float  rSq= theta.x * theta.x + theta.y * theta.y;\n"
                "   vec2 theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq + "
                "           HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);\n"
                "   return LensCenter + Scale * theta1;\n"
                "}\n"
                "\n"
                "\n"
                "\n"
                "void main()\n"
                "{\n"
                "   vec2 tc = HmdWarp(uv);\n"
                "   if(any(notEqual(clamp(tc, ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25, 0.5)) - tc, vec2(0.0, 0.0))))\n"
                "       gl_FragColor = vec4(0.0, 0.7, 0.0, 1.0);\n"
                "   else\n"
                "       gl_FragColor = texture2D(tex, tc);\n"
                "}";

  
char *betray_plugin_oculus_fragment_shader =
                "uniform sampler2D tex;\n"
				"varying vec2 uv;\n"
                "uniform vec2 LensCenter;\n"
                "uniform vec2 ScreenCenter;\n"
                "uniform vec2 Scale;\n"
                "uniform vec2 ScaleIn;\n"
                "uniform vec4 HmdWarpParam;\n"
                "\n"
                "vec2 HmdWarp(vec2 texIn)\n"
                "{\n"
                "   vec2 theta = (texIn - LensCenter) * ScaleIn;\n"
                "   float  rSq= theta.x * theta.x + theta.y * theta.y;\n"
                "   vec2 theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq + "
                "           HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);\n"
                "   return LensCenter + Scale * theta1;\n"
                "}\n"
                "\n"
                "\n"
                "\n"
                "void main()\n"
                "{\n"
				"   gl_FragColor = texture2D(tex, HmdWarp(uv + vec2(0.000, 0.000)));\n"
                "   gl_FragColor += texture2D(tex, HmdWarp(uv + vec2(0.000, 0.0005)));\n"
                "   gl_FragColor += texture2D(tex, HmdWarp(uv + vec2(0.000, -0.0005)));\n"
                "   gl_FragColor += texture2D(tex, HmdWarp(uv + vec2(0.0005, 0.000)));\n"
                "   gl_FragColor += texture2D(tex, HmdWarp(uv + vec2(-0.0005, 0.000)));\n"
                "   gl_FragColor += texture2D(tex, HmdWarp(uv + vec2(-0.0005, 0.0005)));\n"
                "   gl_FragColor += texture2D(tex, HmdWarp(uv + vec2(-0.0005, -0.0005)));\n"
                "   gl_FragColor += texture2D(tex, HmdWarp(uv + vec2(0.0005, -0.0005)));\n"
                "   gl_FragColor += texture2D(tex, HmdWarp(uv + vec2(0.0005, 0.0005)));\n"
                "   gl_FragColor /= vec4(9.0);\n"
                "}";



char *betray_plugin_oculus_fragment_shader_SAVE =
                "uniform sampler2D tex;\n"
				"varying vec2 uv;\n"
                "\n"
                "void main()\n"
                "{\n"
				"       gl_FragColor = texture2D(tex, uv);\n"
                "}";


RShader *warp_shader_multi;
RShader *warp_shader_single;

void *warp_array;

uint settings_id[3];
uint texture_id = 0;
uint depth_id = 0;
uint fbo_id = 0;
uint screen_size[2] = {0, 0};

void betray_plugin_primitive_image(RShader *shader, uint texture_id, float side, BInputState *input)
{
	RMatrix *m;
	float x, scale = 1.0, offset = 0.25;
	r_shader_set(shader);
	offset *= side;
	x = 0.25 + 0.25 * side;
    r_shader_vec2_set(r_shader_uniform_location(shader, "LensCenter"), 0.5 + 0.085 * side,  0.5);
    r_shader_vec2_set(r_shader_uniform_location(shader, "ScreenCenter"), 0.5, 0.5);
	r_shader_vec2_set(r_shader_uniform_location(shader, "Scale"), 0.25 * scale, (1.0 / 2.0) * scale * 0.5);
    r_shader_vec2_set(r_shader_uniform_location(shader, "ScaleIn"), 2.0 / 0.5 * 0.5 * 0.8, 2.0 / 0.5 * 3.0 / 4.0); 
	r_shader_vec4_set(NULL, r_shader_uniform_location(shader, "HmdWarpParam"), 1.0, 0.22, 0.24, 0.0);
	r_shader_texture_set(shader, 0, texture_id);
	r_array_section_draw(warp_array, NULL, GL_TRIANGLES, 0, 6);
}

boolean betray_plugin_image_warp(BInputState *input)
{
	RMatrix matrix;
	RShader *shader;
	float f, view[3] = {0, 0, 1.0}, color[3], eye_distance, m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	uint image_x, image_y, screen_x, screen_y, scalar;
	float surface[5 * 6 * 2] = {-0.5, -1, 0, 0, 0,
							0.5, -1, 0, 1, 0,
							0.5, 1, 0, 1, 1,
							-0.5, -1, 0, 0, 0,
							0.5, 1, 0, 1, 1,
							-0.5, 1, 0, 0, 1};
	float quaternion[4];


	betray_plugin_oculus_sensor_read(quaternion);
	f_quaternion_to_matrixf(m, quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
	glDisable(GL_DEPTH_TEST);
	f = betray_plugin_screen_mode_get(&screen_x, &screen_y, NULL);
	image_x = screen_x / 2;
	image_y = screen_y;

	scalar = (256.0 + betray_settings_slider_get(settings_id[2]) * 768.1);
	image_x = (image_x * scalar) / 256;
	image_y = (image_y * scalar) / 256;
	
	if(betray_settings_toggle_get(settings_id[1]))	
		shader = warp_shader_multi;
	else
		shader = warp_shader_single;

	r_array_load_vertex(warp_array, NULL, surface, 0, 6);
	if(screen_size[0] != image_x || screen_size[1] != image_y)
	{
		screen_size[0] = image_x;
		screen_size[1] = image_y;
		if(texture_id != 0)
			r_texture_free(texture_id);
		if(depth_id != 0)
			r_texture_free(depth_id);
		if(fbo_id != 0)	
			r_framebuffer_free(fbo_id);
		texture_id = r_texture_allocate(R_IF_RGB_UINT8, image_x, image_y, 1, TRUE, TRUE, NULL);
		depth_id = r_texture_allocate(R_IF_DEPTH32, image_x, image_y , 1, TRUE, TRUE, NULL);
		fbo_id = r_framebuffer_allocate(&texture_id, 1, depth_id, RELINQUISH_TARGET_2D_TEXTURE);
	}
	eye_distance = betray_settings_number_float_get(settings_id[0]);
	eye_distance *= eye_distance;
	m[12] = eye_distance;
	betray_plugin_application_draw(fbo_id, image_x, image_y, view, FALSE, m);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glViewport(0, 0, screen_x, screen_y);
	r_matrix_set(&matrix);
	r_matrix_identity(&matrix);
	r_matrix_frustum(&matrix, -0.01, 0.01, -0.01 * f, 0.01 * f, 0.01, 100.0);
	r_matrix_translate(&matrix, 0, 0, -1.0);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	r_matrix_translate(&matrix, -0.5, 0.0, 0.0);
	betray_plugin_primitive_image(shader, texture_id, -1.0, input);
//	r_primitive_image(-1, -f, 0.0, 1, 2 * f, 0, 0, 1, 1, texture_id, 1, 0, 0, 1);
	m[12] = -eye_distance;
	betray_plugin_application_draw(fbo_id, image_x, image_y, view, FALSE, m);
	r_matrix_translate(&matrix, 1.0, 0.0, 0.0);
	betray_plugin_primitive_image(shader, texture_id, 1.0, input);
//	r_primitive_image(0, -f, 0.0, 1, 2 * f, 0, 0, 1, 1, texture_id, 0, 1, 1, 1);
	return TRUE;
}


void betray_plugin_init(void)
{
	uint x, y, i;
	RFormats vertex_format_types[3] = {R_FLOAT, R_FLOAT};
	uint vertex_format_size[3] = {3, 2};
	betray_plugin_callback_set_image_warp(betray_plugin_image_warp, "Occulus Rift");
	glClearColor(1.0, 0.0, 1.0, 1.0);
	r_init(betray_plugin_gl_proc_address_get());
	warp_shader_multi = r_shader_create_simple(NULL, 0, betray_plugin_oculus_vertex_shader, betray_plugin_oculus_fragment_shader, "warp shader");
	warp_shader_single = r_shader_create_simple(NULL, 0, betray_plugin_oculus_vertex_shader, betray_plugin_oculus_fragment_shader_single_sample, "warp shader");
	warp_array = r_array_allocate(6, vertex_format_types, vertex_format_size, 2, 0);
	settings_id[0] = betray_settings_create(BETRAY_ST_SLIDER, "Eye distance", 0, NULL);
	betray_settings_number_float_set(settings_id[0], 0.15);
	settings_id[1] = betray_settings_create(BETRAY_ST_TOGGLE, "Supersample", 0, NULL);
	betray_settings_toggle_set(settings_id[1], FALSE);
	settings_id[2] = betray_settings_create(BETRAY_ST_SLIDER, "Resolution", 0, NULL);
	betray_settings_slider_set(settings_id[2], 0.333333);
//	betray_plugin_oculus_sensor_init();
}
