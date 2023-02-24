#include <math.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include "forge.h"

typedef struct{
	void *id;
	uint part;
	uint shader_id;
	float a[3];
	float b[3];
	float c[3];
}SeduceBackgroundTriangle;

typedef struct{
	uint texture_id;
	float array[12];
	float scale[2];
	float origo[2];
}SeduceBackgroundImage;

typedef struct{
	uint surface_length;
	uint surface_used;
	float *surface_buffer;
	void *surface_pool;
	uint shadow_length;
	float *shadow_buffer;
	void *shadow_pool;
	SeduceBackgroundImage *images;
	uint image_length;
	uint image_used;
	SeduceBackgroundTriangle *triangles;
	uint triangle_length;
	uint triangle_used;
	uint shader_id_max;
}SeduceBackgroundObject;

#define SEDUCE_INTERNAL_BACKGROUND
#include "seduce.h"


#define SEDUCE_SURFACE_CLICK_BUFFER_SIZE 5 

RShader *s_background_shader_circles = NULL;
RShader *s_background_shader_surface2 = NULL;
RShader *s_background_shader_image = NULL;
void	*s_background_pool_image = NULL;
RShader *s_background_shader_shadow = NULL;
uint s_background_shader_surface_point_locations[SEDUCE_SURFACE_CLICK_BUFFER_SIZE][4];


typedef struct{
	float pos[2];
	float color[3];
	float size;
	float timer;
	void *id;
	uint part;
}SeduceSurfaceClickBuffer;

SeduceSurfaceClickBuffer seduce_surface_click_buffer[SEDUCE_SURFACE_CLICK_BUFFER_SIZE];
int seduce_surface_click_buffer_next = 0;

 void seduce_surface_click_buffer_update(BInputState *input)
 {
	uint i;
	for(i = 0; i < SEDUCE_SURFACE_CLICK_BUFFER_SIZE; i++)
		seduce_surface_click_buffer[i].timer += input->delta_time;
 }

void seduce_surface_circle_spawn(BInputState *input, float x, float y, float size, void *id, uint part, float red, float green, float blue)
{
	if(seduce_surface_click_buffer[seduce_surface_click_buffer_next].timer > 1.0 / (float)(SEDUCE_SURFACE_CLICK_BUFFER_SIZE + 1))
	{				
		seduce_surface_click_buffer_next = (seduce_surface_click_buffer_next + 1) % SEDUCE_SURFACE_CLICK_BUFFER_SIZE;
		seduce_surface_click_buffer[seduce_surface_click_buffer_next].color[0] = red;
		seduce_surface_click_buffer[seduce_surface_click_buffer_next].color[1] = green;
		seduce_surface_click_buffer[seduce_surface_click_buffer_next].color[2] = blue;
		seduce_surface_click_buffer[seduce_surface_click_buffer_next].pos[0] = x;
		seduce_surface_click_buffer[seduce_surface_click_buffer_next].pos[1] = y;
		seduce_surface_click_buffer[seduce_surface_click_buffer_next].size = size;
		seduce_surface_click_buffer[seduce_surface_click_buffer_next].timer = 0;
		seduce_surface_click_buffer[seduce_surface_click_buffer_next].id = id;
		seduce_surface_click_buffer[seduce_surface_click_buffer_next].part = part;
	}
 }


char *s_background_shader_surface_vertex2 =
"uniform mat4 ModelViewProjectionMatrix;"
"attribute vec4 vertex;"
"attribute vec4 color;"
"varying vec2 vector1;"
"varying vec2 vector2;"
"varying vec2 vector3;"
"varying vec2 vector4;"
"varying vec2 vector5;"
"varying vec2 vector6;"
"varying vec2 red;"
"varying vec2 green;"
"varying vec2 blue;"
"varying vec2 center;"
"varying vec2 shade;"
"varying float distance;"
"varying vec4 c;"
"varying vec2 timep;"
"varying vec2 p0p;"
"varying vec2 p1p;"
"varying vec2 p2p;"
"varying vec2 p3p;"
"varying vec2 p4p;"
"uniform vec2 pointer;"
"uniform vec2 point_0_pos;"
"uniform float point_0_id;"
"uniform vec2 point_1_pos;"
"uniform float point_1_id;"
"uniform vec2 point_2_pos;"
"uniform float point_2_id;"
"uniform vec2 point_3_pos;"
"uniform float point_3_id;"
"uniform vec2 point_4_pos;"
"uniform float point_4_id;"
"void main()"
"{"
"	vec3 v;"
"	distance = 0.5;"//length(pointer.xy);"
"	vector1 = (pointer.xy * -vec2(-1.5 + distance * 3.0) - vertex.xy) * vec2(2.0) * (2.0 + distance * -1.2);"
"	vector2 = (pointer.xy * vec2(1.3) - vertex.xy) * vec2(2.60) * (1.2 + distance * -0.7);"
"	vector3 = (pointer.xy * vec2(0.9) - vertex.xy) * vec2(1.50) * (1.0 + distance * 0.7);"
"	vector4 = (pointer.xy * -vec2(1.8) - vertex.xy) * vec2(1.70) * (0.6 + distance * -0.4);"
"	vector5 = (pointer.xy * -vec2(0.9) - vertex.xy) * vec2(5.0) * (0.5 + distance * 0.4);"
"	vector6 = (pointer.xy * -vec2(0.9) - vertex.xy) * vec2(3.0) * (0.5 + distance * 0.4);"

/*"	vector1 = (pointer.xy * -vec2(-1.5 + distance * 3.0) - vertex.xy) * vec2(10.0) * (2.0 + distance * -1.2);"
"	vector2 = (pointer.xy * vec2(1.3) - vertex.xy) * vec2(13.0) * (1.2 + distance * -0.7);"
"	vector3 = (pointer.xy * vec2(0.9) - vertex.xy) * vec2(5.0) * (1.0 + distance * 0.7);"
"	vector4 = (pointer.xy * -vec2(1.8) - vertex.xy) * vec2(7.0) * (0.6 + distance * -0.4);"
"	vector5 = (pointer.xy * -vec2(0.9) - vertex.xy) * vec2(25.0) * (0.5 + distance * 0.4);"
"	vector6 = (pointer.xy * -vec2(0.9) - vertex.xy) * vec2(15.0) * (0.5 + distance * 0.4);"*/

"	shade = (pointer.xy * -vec2(0.3) - vertex.xy) * vec2(2.2) * vec2(0.7 + distance * 0.4);"
"	red = (pointer.xy * vec2(0.6) - vertex.xy) * vec2(3.0);"
"	green = (pointer.xy * vec2(0.8) - vertex.xy) * vec2(3.5);"
"	blue = (pointer.xy * vec2(1.0) - vertex.xy) * vec2(4.0);"
"	center = vertex.xy * vec2(0.4);"
"	c = color;"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex.xyz, 1.0);"
"	timep = vertex.xy;"
"	if(point_0_id < 0.0 || (point_0_id > vertex.a && point_0_id < vertex.a + 1.0))"	
"		p0p = vertex.xy - point_0_pos.xy;"
"	else"
"		p0p = vec2(10.0);"
"	if(point_1_id < 0.0 || (point_1_id > vertex.a && point_1_id < vertex.a + 1.0))"	
"		p1p = vertex.xy - point_1_pos.xy;"
"	else"
"		p1p = vec2(10.0);"
"	if(point_2_id < 0.0 || (point_2_id > vertex.a && point_2_id < vertex.a + 1.0))"	
"		p2p = vertex.xy - point_2_pos.xy;"
"	else"
"		p2p = vec2(10.0);"
"	if(point_3_id < 0.0 || (point_3_id > vertex.a && point_3_id < vertex.a + 1.0))"	
"		p3p = vertex.xy - point_3_pos.xy;"
"	else"
"		p3p = vec2(10.0);"
"	if(point_4_id < 0.0 || (point_4_id > vertex.a && point_4_id < vertex.a + 1.0))"	
"		p4p = vertex.xy - point_4_pos.xy;"
"	else"
"		p4p = vec2(10.0);"
"}";
char *s_background_shader_surface_fragment2_old = 
"varying vec2 vector1;"
"varying vec2 vector2;"
"varying vec2 vector3;"
"varying vec2 vector4;"
"varying vec2 vector5;"
"varying vec2 vector6;"
"varying vec2 red;"
"varying vec2 green;"
"varying vec2 blue;"
"varying vec2 center;"
"varying vec2 shade;"
"varying float distance;"
"varying vec4 c;"
"varying vec2 timep;"
"varying vec2 p0p;"
"uniform float time_size;"
"uniform float resolution;"
"uniform vec4 point_0_color;"
"uniform float point_0_size;"
"varying vec2 p1p;"
"uniform vec4 point_1_color;"
"uniform float point_1_size;"
"varying vec2 p2p;"
"uniform vec4 point_2_color;"
"uniform float point_2_size;"
"varying vec2 p3p;"
"uniform vec4 point_3_color;"
"uniform float point_3_size;"
"varying vec2 p4p;"
"uniform vec4 point_4_color;"
"uniform float point_4_size;"
"void main()"
"{"
"	vec4 color;"
"	float f;"
"	f = 1.0 / (1.0 + dot(vector1, vector1) * 0.1);"
"	color += mix(vec4(0.15, 0.1, 0.15, 0.0), vec4(0.3, -0.15, 0.2, 0.0), length(vector1) * 0.06) * vec4(f);"
"	f = 1.0 / (1.0 + dot(vector2, vector2));"
"	color += mix(vec4(0.25, 0.3, 0.35, 0.0), vec4(0.1, 0.05, 0.15, 0.0), length(vector2) * 0.3) * vec4(f);"
"	f = 1.0 / (1.0 + dot(vector3, vector3));"
"	color += mix(vec4(0.2, 0.2, 0.3, 0.0), vec4(0.0, -0.05, -0.1, 0.0), length(vector3) * 0.2) * vec4(f);"
"	f = 1.0 / (1.0 + dot(vector4, vector4) * 0.1);"
"	color += mix(vec4(0.085, 0.1, 0.07, 0.0), vec4(0.1, 0.1, 0.1, 0.0), length(vector4) * 0.01) * vec4(f);"
"	f = 1.0 / (1.0 + dot(vector5, vector5));"
"	color += mix(vec4(0.1, 0.11, 0.12, 0.0), vec4(-0.07, 0.10, -0.03, 0.0), length(vector5) * 0.05) * vec4(f);"
"	f = 1.0 / (1.0 + dot(vector6, vector6) * 0.1);"
"	color += mix(vec4(-0.04, -0.02, -0.02, 0.0), vec4(-0.2, -0.175, -0.15, 0.0), length(vector6) * 0.2) * vec4(f);"
"	f = 1.0 / (1.0 + dot(red, red));"
"	color += vec4(0.3, 0.0, 0.0, 0.0) * vec4(f);"
"	f = 1.1 / (1.0 + dot(green, green));"
"	color += vec4(0.0, 0.3, 0.0, 0.0) * vec4(f);"
"	f = 1.2 / (1.0 + dot(blue, blue));"
"	color += vec4(0.0, 0.0, 0.3, 0.0) * vec4(f);"
"	gl_FragColor = c;"
"	f = min(max((point_0_size - length(p0p)), 0) * resolution, 1.0);"
"	gl_FragColor = mix(gl_FragColor, point_0_color, point_0_color.a * f);"
"	f = min(max((point_1_size - length(p1p)), 0) * resolution, 1.0);"
"	gl_FragColor = mix(gl_FragColor, point_1_color, point_1_color.a * f);"
"	f = min(max((point_2_size - length(p2p)), 0) * resolution, 1.0);"
"	gl_FragColor = mix(gl_FragColor, point_2_color, point_2_color.a * f);"
"	f = min(max((point_3_size - length(p3p)), 0) * resolution, 1.0);"
"	gl_FragColor = mix(gl_FragColor, point_3_color, point_3_color.a * f);"
"	f = min(max((point_4_size - length(p4p)), 0) * resolution, 1.0);"
"	gl_FragColor = mix(gl_FragColor, point_4_color, point_4_color.a * f);"
"	gl_FragColor += color * vec4(distance * 0.5);"
"	gl_FragColor += vec4(dot(center, center) / (1.0 + dot(shade, shade))) * vec4(1, 0.8, 0.6, 0) - vec4(0.05);"
"	gl_FragColor.a = c.a;"
"	f = min(max((time_size - length(timep)), 0) * resolution, 1.0);"
"	gl_FragColor *= vec4(f);" 
"}";


char *s_background_shader_surface_fragment2lookdev = 
"varying vec2 vector1;"
"varying vec2 vector2;"
"varying vec2 vector3;"
"varying vec2 vector4;"
"varying vec2 vector5;"
"varying vec2 vector6;"
"varying vec2 red;"
"varying vec2 green;"
"varying vec2 blue;"
"varying vec2 center;"
"varying vec2 shade;"
"varying float distance;"
"varying vec4 c;"
"varying vec2 timep;"
"varying vec2 p0p;"
"uniform float time_size;"
"uniform float resolution;"
"uniform vec4 point_0_color;"
"uniform float point_0_size;"
"varying vec2 p1p;"
"uniform vec4 point_1_color;"
"uniform float point_1_size;"
"varying vec2 p2p;"
"uniform vec4 point_2_color;"
"uniform float point_2_size;"
"varying vec2 p3p;"
"uniform vec4 point_3_color;"
"uniform float point_3_size;"
"varying vec2 p4p;"
"uniform vec4 point_4_color;"
"uniform float point_4_size;"
"void main()"
"{"
"	vec4 color;"
"	float f;"
"	if(time_size < mod(dot(timep * vec2(2.0), timep * vec2(2.0)) * 10.0, 1.0) + length(timep) * 2.0)"
"		discard;"
"	f = point_0_size / (1.0 + dot(p0p, p0p) * 1000.0);"
"	color = vec4(f) * point_0_color;"
"	f = point_1_size / (1.0 + dot(p1p, p1p) * 1000.0);"
"	color += vec4(f) * point_1_color;"
"	f = point_2_size / (1.0 + dot(p2p, p2p) * 1000.0);"
"	color += vec4(f) * point_2_color;"
"	f = point_3_size / (1.0 + dot(p3p, p3p) * 1000.0);"
"	color += vec4(f) * point_3_color;"
"	f = point_4_size / (1.0 + dot(p4p, p4p) * 1000.0);"
"	color += vec4(f) * point_4_color;"
"   if(color.a > 0.2)"
"		gl_FragColor = vec4(c.rgb - vec3(0.1) + color.rgb, c.a);"
"	else"
"		gl_FragColor = c + vec4(color.rgb, 0.0);"
"}";



char *s_background_shader_surface_fragment2 = 
"varying vec2 vector1;"
"varying vec2 vector2;"
"varying vec2 vector3;"
"varying vec2 vector4;"
"varying vec2 vector5;"
"varying vec2 vector6;"
"varying vec2 red;"
"varying vec2 green;"
"varying vec2 blue;"
"varying vec2 center;"
"varying vec2 shade;"
"varying float distance;"
"varying vec4 c;"
"varying vec2 timep;"
"varying vec2 p0p;"
"uniform float time_size;"
"uniform float resolution;"
"uniform vec4 point_0_color;"
"uniform float point_0_size;"
"varying vec2 p1p;"
"uniform vec4 point_1_color;"
"uniform float point_1_size;"
"varying vec2 p2p;"
"uniform vec4 point_2_color;"
"uniform float point_2_size;"
"varying vec2 p3p;"
"uniform vec4 point_3_color;"
"uniform float point_3_size;"
"varying vec2 p4p;"
"uniform vec4 point_4_color;"
"uniform float point_4_size;"
"void main()"
"{"
"	vec4 color;"
"	float f;"
"	if(time_size < mod(dot(timep * vec2(2.0), timep * vec2(2.0)) * 10.0, 1.0) + length(timep) * 2.0)"
"		discard;"
"	f = 1.0 / (1.0 + dot(vector1, vector1) * 0.1);"
"	color = mix(vec4(0.15, 0.1, 0.15, 0.0), vec4(0.3, -0.15, 0.2, 0.0), length(vector1) * 0.06) * vec4(f);"
"	f = 1.0 / (1.0 + dot(vector2, vector2));"
"	color += mix(vec4(0.25, 0.3, 0.35, 0.0), vec4(0.1, 0.05, 0.15, 0.0), length(vector2) * 0.3) * vec4(f);"
"	f = 1.0 / (1.0 + dot(vector3, vector3));"
"	color += mix(vec4(0.2, 0.2, 0.3, 0.0), vec4(0.0, -0.05, -0.1, 0.0), length(vector3) * 0.2) * vec4(f);"
"	f = 1.0 / (1.0 + dot(vector4, vector4) * 0.1);"
"	color += mix(vec4(0.085, 0.1, 0.07, 0.0), vec4(0.1, 0.1, 0.1, 0.0), length(vector4) * 0.01) * vec4(f);"
"	f = 1.0 / (1.0 + dot(vector5, vector5));"
"	color += mix(vec4(0.1, 0.11, 0.12, 0.0), vec4(-0.07, 0.10, -0.03, 0.0), length(vector5) * 0.05) * vec4(f);"
"	f = 1.0 / (1.0 + dot(vector6, vector6) * 0.1);"
"	color += mix(vec4(-0.04, -0.02, -0.02, 0.0), vec4(-0.2, -0.175, -0.15, 0.0), length(vector6) * 0.2) * vec4(f);"
"	f = 1.0 / (1.0 + dot(red, red));"
"	color += vec4(0.3, 0.0, 0.0, 0.0) * vec4(f);"
"	f = 1.1 / (1.0 + dot(green, green));"
"	color += vec4(0.0, 0.3, 0.0, 0.0) * vec4(f);"
"	f = 1.2 / (1.0 + dot(blue, blue));"
"	color += vec4(0.0, 0.0, 0.3, 0.0) * vec4(f);"
"	f = min(max((point_0_size - length(p0p)), 0) * resolution, 1.0);"
"	gl_FragColor = point_0_color * vec4(point_0_color.a * f);"
"	f = min(max((point_1_size - length(p1p)), 0) * resolution, 1.0);"
"	gl_FragColor += point_1_color * vec4(point_1_color.a * f);"
"	f = min(max((point_2_size - length(p2p)), 0) * resolution, 1.0);"
"	gl_FragColor += point_2_color * vec4(point_2_color.a * f);"
"	f = min(max((point_3_size - length(p3p)), 0) * resolution, 1.0);"
"	gl_FragColor += point_3_color * vec4(point_3_color.a * f);"
"	f = min(max((point_4_size - length(p4p)), 0) * resolution, 1.0);"
"	gl_FragColor += point_4_color * vec4(point_4_color.a * f);"
"	gl_FragColor += c;"
"	gl_FragColor += color * vec4(distance * 0.5);"
"	gl_FragColor += vec4(dot(center, center) / (1.0 + dot(shade, shade))) * vec4(1, 0.8, 0.6, 0) - vec4(0.05);"
"	gl_FragColor.a = c.a;"
"}";


char *s_background_shader_circles_vertex = 
"attribute vec3 vertex;"
"attribute vec4 surface_color;"
"uniform mat4 ModelViewProjectionMatrix;"

"varying vec3 pos;\n"
"varying vec2 projected;\n"
"varying vec4 surface;\n"
"void main()"
"{"
"	pos = vertex.xyz;"
"	gl_Position = (ModelViewProjectionMatrix * vec4(vertex.xyz, 1.0));"
"	projected = (gl_Position.xy) * vec2(0.5) + vec2(0.5);" 
"	surface = surface_color;"
"}";

/*
char *s_background_shader_surface_fragment2 = 
"uniform sampler2D rings;"
"uniform sampler2D light;"
"uniform float time;"
"uniform float scroll;"
"uniform float inner;"
"uniform float outer;"
"uniform float scale;"
"uniform float fill;"
"uniform mat3 color_matrix;"
"varying vec4 surface;\n"
"varying vec3 pos;\n"
"varying vec2 projected;\n"
"void main()\n"
"{\n"
"	vec4 c, c2, gray, alpha;"
"	float f;"
"	f = length(pos);\n"
"	if(f > outer || f < inner)\n"
"		c = vec4(0.0);"
"	else"
"	{"
"		c = texture2D(rings, vec2(f * scale + scroll, time)) * surface.aaaa;\n"
"		c2 = vec4(color_matrix * c.rgb, 0.0) * (texture2D(light, projected));\n"
"		alpha = texture2D(rings, vec2(f * scale * 10.7 + scroll, fill * 0.235));\n"
"		gray = vec4((surface.rrr + surface.ggg + surface.bbb) * vec3(0.25), surface.a);"
"		c = mix(c2, mix(surface, gray, c.r * 0.2) + c2, min(alpha.rrrr, vec4(1.0)));\n"
"	}"
"	gl_FragColor = c;\n"
"}";
*/


char *s_background_shader_image_vertex = 
"attribute vec2 vertex;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform vec2 origo;"
"uniform vec2 stretch;"
"varying vec3 pos;\n"
"varying vec2 projected;\n"
"varying vec4 surface;\n"
"varying vec2 uv;\n"
"void main()"
"{"
"	pos = vec3(vertex.xy, 0.0);"
"	uv = (vertex.xy - origo) * stretch;"
"	gl_Position = (ModelViewProjectionMatrix * vec4(vertex.xy, 0.0, 1.0));"
"	projected = (gl_Position.xy/* / gl_Position.zz*/) * vec2(0.5) + vec2(0.5);" 
"	surface = vec4(1.0);"
"}";


char *s_background_shader_image_fragment = 
"uniform sampler2D rings;"
"uniform sampler2D image;"
"uniform float time;"
"uniform float scroll;"
"uniform float inner;"
"uniform float outer;"
"uniform float scale;"
"uniform float fill;"
"uniform mat3 color_matrix;"
"varying vec4 surface;\n"
"varying vec3 pos;\n"
"varying vec2 projected;\n"
"varying vec2 uv;\n"
"void main()\n"
"{\n"
"	vec4 c, c2, gray, alpha;"
"	float f;"
"	f = length(pos);\n"
"	if(f > outer || f < inner)\n"
"		c = vec4(0.0);"
"	else"
"	{"
"		c = texture2D(rings, vec2(f * scale + scroll, time));\n"
//"		c = surface * (vec4(fill) + texture2D(light, projected) * vec4(0.4)) + vec4(color_matrix * c.rgb, 0.0) * (texture2D(light, projected)) - c.rrrr * vec4(0.2);\n"
//"		c = vec4(color_matrix * c.rgb, 0.0) * (texture2D(light, projected)) - c.rrrr * vec4(0.2);\n"
"		c2 = vec4(color_matrix * c.rgb, 0.0) * (texture2D(rings, projected));\n"
"		alpha = texture2D(rings, vec2(f * scale * 1.7 + scroll, fill * 0.235));\n"
"		gray = vec4((surface.rrr + surface.ggg + surface.bbb) * vec3(0.25), surface.a);"
"		c = mix(c2, mix(surface, gray, c.r * 0.2) + c2, min(alpha.rrrr, vec4(1.0)));\n"
//"		c = vec4(alpha.rrr, 1.0);\n"
"	}"
"	c = texture2D(image, uv);\n"
"	gl_FragColor = c;\n"
"}";


char *s_background_shader_circles_fragment = 
"uniform sampler2D rings;"
"uniform float time;"
"uniform float scroll;"
"uniform float inner;"
"uniform float outer;"
"uniform float scale;"
"uniform float fill;"
"uniform vec4 flare_color;"
"uniform mat3 color_matrix;"
"varying vec4 surface;\n"
"varying vec3 pos;\n"
"varying vec2 projected;\n"
"void main()\n"
"{\n"
"	vec4 c;"
"	float f, flare;"
"	f = length(pos);\n"
"	flare = 1.0 / (1.0 + f * f * 100.0) - 0.05;"
"	if(f > outer || f < inner)\n"
"		c = vec4(0.0);"
"	else"
"	{"
"		c = texture2D(rings, vec2(f * scale + scroll, time));\n"
"		c = vec4(color_matrix * c.rgb, 0.0);\n"
"	}"
//"		c = surface * vec4(fill) + vec4(color_matrix * texture2D(rings, vec2(f * scale + scroll, time)).rgb, 0.0) * (texture2D(light, projected));\n"
"	gl_FragColor = c + vec4(flare) * flare_color;\n"
"}";

char *s_background_shader_shadow_vertex = 
"attribute vec4 vertex;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform float shadow_size;"
"varying float alpha;\n"
"void main()"
"{"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex.xy + vertex.ba * vec2(shadow_size), 0, 1.0);"
"	if(dot(vertex.ba, vertex.ba) < 0.000001)"
"		alpha = 1.0;"
"	else"
"		alpha = 0.0;"
"}";

char *s_background_shader_shadow_fragment = 
"uniform float shadow_size;"
"varying float alpha;\n"
"uniform float color;\n"
"void main()\n"
"{\n"
"	vec2 v;"
"	gl_FragColor = vec4(0, 0, 0, color * alpha * alpha * alpha);\n"
"}";





SeduceBackgroundObject *seduce_global_background_buffer; 

void *s_background_circles_pool = NULL;
void *s_background_circles_array = NULL;
uint s_background_circles_array_length = 0;

SeduceBackgroundObject;


SeduceBackgroundObject *seduce_background_object_allocate()
{
	SeduceBackgroundObject *obj;
	obj = malloc(sizeof *obj);
	obj->surface_length = 64;
	obj->surface_used = 0;
	obj->surface_buffer = malloc((sizeof *obj->surface_buffer) * obj->surface_length * 8 * 3);
	obj->surface_pool = NULL;
	obj->shadow_length = 0;
	obj->shadow_buffer = NULL;
	obj->shadow_pool = NULL;
	obj->images = NULL;
	obj->image_length = 0;
	obj->image_used = 0;

	obj->triangles = NULL;
	obj->triangle_length = 0;
	obj->triangle_used = 0;
	obj->shader_id_max = 0.0;
	return obj;
}

void seduce_primitive_background_object_free(SeduceBackgroundObject *object)
{
	if(object->surface_buffer != NULL)
		free(object->surface_buffer);
	if(object->surface_pool != NULL)
		r_array_free(object->surface_pool);
	if(object->shadow_buffer != NULL)
		free(object->shadow_buffer);
	if(object->shadow_pool != NULL)
		r_array_free(object->shadow_pool);
	if(object->triangles != NULL)
		free(object->triangles);
	free(object);
}

void seduce_background_shadow_add(SeduceBackgroundObject *object, float *list, uint count, boolean closed, float size)
{
	uint i, ii, run;
	float pos_a[2], pos_b[2], temp[2], last[2], r;
	float *array;
	run = count;
	if(!closed)
		run--;
	object->shadow_length += run * 2 * 3;
	object->shadow_buffer = realloc(object->shadow_buffer, (sizeof *object->shadow_buffer) * object->shadow_length * 4 * 2 * 3);
	array = &object->shadow_buffer[(object->shadow_length - run * 2 * 3) * 4];
	if(closed)
	{
		pos_a[1] = -(list[count * 2 - 2] - list[0]);
		pos_a[0] = (list[count * 2 - 1] - list[1]);
		f_normalize2f(pos_a);
		pos_b[1] = -(list[0] - list[2]);
		pos_b[0] = (list[1] - list[3]);
		f_normalize2f(pos_b);
		r = (pos_a[0] + pos_b[0]) * pos_b[0] + (pos_a[1] + pos_b[1]) * pos_b[1];
		last[0] = (pos_a[0] + pos_b[0]) / r;
		last[1] = (pos_a[1] + pos_b[1]) / r;
		pos_a[0] = pos_b[0];
		pos_a[1] = pos_b[1];
	}else
	{
		pos_a[1] = -(list[0] - list[2]);
		pos_a[0] = (list[1] - list[3]);
		f_normalize2f(pos_a);
		last[0] = pos_b[0] = pos_a[0];
		last[1] = pos_b[1] = pos_a[1];
	}
	for(i = 0; i < run; i++)
	{
		if(closed || i + 1 < run)
		{
			pos_b[1] = -(list[((i + 1) % count) * 2] - list[((i + 2) % count) * 2]);
			pos_b[0] = (list[((i + 1) % count) * 2 + 1] - list[((i + 2) % count) * 2 + 1]);
			f_normalize2f(pos_b);
		}
		r = (pos_a[0] + pos_b[0]) * pos_b[0] + (pos_a[1] + pos_b[1]) * pos_b[1];
		temp[0] = (pos_a[0] + pos_b[0]) / r;
		temp[1] = (pos_a[1] + pos_b[1]) / r;
		
		ii = (i + 1) % count;
		
		array[i * 2 * 3 * 4 + 0] = list[i * 2];
		array[i * 2 * 3 * 4 + 1] = list[i * 2 + 1];
		array[i * 2 * 3 * 4 + 2] = 0;
		array[i * 2 * 3 * 4 + 3] = 0;

		array[i * 2 * 3 * 4 + 4] = list[i * 2];
		array[i * 2 * 3 * 4 + 5] = list[i * 2 + 1];
		array[i * 2 * 3 * 4 + 6] = last[0] * size;
		array[i * 2 * 3 * 4 + 7] = last[1] * size;

		array[i * 2 * 3 * 4 + 8] = list[ii * 2];
		array[i * 2 * 3 * 4 + 9] = list[ii * 2 + 1];
		array[i * 2 * 3 * 4 + 10] = 0;
		array[i * 2 * 3 * 4 + 11] = 0;

		array[i * 2 * 3 * 4 + 12] = list[i * 2];
		array[i * 2 * 3 * 4 + 13] = list[i * 2 + 1];
		array[i * 2 * 3 * 4 + 14] = last[0] * size;
		array[i * 2 * 3 * 4 + 15] = last[1] * size;

		array[i * 2 * 3 * 4 + 16] = list[ii * 2];
		array[i * 2 * 3 * 4 + 17] = list[ii * 2 + 1];
		array[i * 2 * 3 * 4 + 18] = temp[0] * size;
		array[i * 2 * 3 * 4 + 19] = temp[1] * size;
		
		array[i * 2 * 3 * 4 + 20] = list[ii * 2];
		array[i * 2 * 3 * 4 + 21] = list[ii * 2 + 1];
		array[i * 2 * 3 * 4 + 22] = 0;
		array[i * 2 * 3 * 4 + 23] = 0;
		
		last[0] = temp[0];
		last[1] = temp[1];
		pos_a[0] = pos_b[0];
		pos_a[1] = pos_b[1];
	}
}

void seduce_background_shadow_square_add(SeduceBackgroundObject *object, float pos_x, float pos_y, float size_x, float size_y, float size)
{
	float list[8];
	list[0] = pos_x;
	list[1] = pos_y;
	list[2] = pos_x;
	list[3] = pos_y + size_y;
	list[4] = pos_x + size_x;
	list[5] = pos_y + size_y;
	list[6] = pos_x + size_x;
	list[7] = pos_y;
	seduce_background_shadow_add(object, list, 4, TRUE, size);
}

void seduce_background_shape_add(SeduceBackgroundObject *object, void *id, uint part, float *list, uint count, float surface_r, float surface_g, float surface_b, float surface_a)
{
	float *array, *copy, normals[6], vec[2], f, dist, best;
	uint i, j, jj, found;
	float f_id = -1.0;
	float center[2];
	if(object->surface_length < object->surface_used + count - 2)
	{
		object->surface_length = object->surface_used + count - 2;
		object->surface_buffer = realloc(object->surface_buffer, (sizeof *object->surface_buffer) * object->surface_length * 8 * 3);
	}
	if(id != NULL)
	{
		for(i = 0; i < object->triangle_length && (object->triangles[i].id != id || object->triangles[i].part != part); i++);
		if(i < object->triangle_used)
			f_id = object->triangles[i].shader_id;
		else
			f_id = ++object->shader_id_max;

		if(object->triangle_length < object->triangle_used + count - 2)
		{
			object->triangle_length += 16 + count - 2;
			object->triangles = realloc(object->triangles, (sizeof *object->triangles) * object->triangle_length);
		}
	}
	array = &object->surface_buffer[object->surface_used * 8 * 3];

	copy = malloc((sizeof *copy) * count * 2);
	memcpy(copy, list, (sizeof *copy) * count * 2);
	object->surface_used += 3 * (count - 2);
	while(count > 2)
	{
		best = 0;
		for(i = 0; i < count; i++)
		{
			if((copy[((i + 1) % count) * 2 + 0] - copy[i * 2 + 0]) * (copy[i * 2 + 1] - copy[((i + 2) % count) * 2 + 1]) +
				(copy[((i + 1) % count) * 2 + 1] - copy[i * 2 + 1]) * (copy[((i + 2) % count) * 2 + 0] - copy[i * 2 + 0]) > 0)
			{
				
				normals[0] = copy[((i + 1) % count) * 2 + 0] - copy[i * 2 + 0];
				normals[1] = copy[((i + 1) % count) * 2 + 1] - copy[i * 2 + 1];
				normals[2] = copy[((i + 2) % count) * 2 + 0] - copy[((i + 1) % count) * 2 + 0];
				normals[3] = copy[((i + 2) % count) * 2 + 1] - copy[((i + 1) % count) * 2 + 1];
				normals[4] = copy[i * 2 + 0] - copy[((i + 2) % count) * 2 + 0];
				normals[5] = copy[i * 2 + 1] - copy[((i + 2) % count) * 2 + 1];
				for(j = 3; j < count; j++)
				{
					jj = (j + i) % count;					
					vec[0] = copy[jj * 2 + 0] - copy[i * 2 + 0];
					vec[1] = copy[jj * 2 + 1] - copy[i * 2 + 1];
					if(vec[0] * normals[1] - 
						vec[1] * normals[0] > 0.0)
					{
						vec[0] = copy[jj * 2 + 0] - copy[((i + 1) % count) * 2 + 0];
						vec[1] = copy[jj * 2 + 1] - copy[((i + 1) % count) * 2 + 1];
						if(vec[0] * normals[3] - 
							vec[1] * normals[2] > 0.0)
						{
							vec[0] = copy[jj * 2 + 0] - copy[((i + 2) % count) * 2 + 0];
							vec[1] = copy[jj * 2 + 1] - copy[((i + 2) % count) * 2 + 1];
							if(vec[0] * normals[5] - 
								vec[1] * normals[4] > 0.0)
							{
								break;
							}
						}
					}
				}
				if(j == count)
				{
					f_normalize2f(&normals[0]);
					f_normalize2f(&normals[2]);
					f_normalize2f(&normals[4]);
					dist = normals[1] * (copy[((i + 2) % count) * 2 + 0] - copy[i * 2 + 0]) -
						normals[0] * (copy[((i + 2) % count) * 2 + 1] - copy[i * 2 + 1]);

					f = normals[3] * (copy[((i + 0) % count) * 2 + 0] - copy[((i + 1) % count) * 2 + 0]) -
						normals[2] * (copy[((i + 0) % count) * 2 + 1] - copy[((i + 1) % count) * 2 + 1]);
					if(f > dist)
						dist = f;
					f = normals[5] * (copy[((i + 1) % count) * 2 + 0] - copy[((i + 2) % count) * 2 + 0]) -
						normals[4] * (copy[((i + 1) % count) * 2 + 1] - copy[((i + 2) % count) * 2 + 1]);
					if(f > dist)
						dist = f;
					if(best < dist)
					{
						best = dist;
						found = i;
					}
				}
			}
		}			

		array[0] = copy[found * 2 + 0];
		array[1] = copy[found * 2 + 1];
		array[2] = 0;
		array[3] = f_id;
		array[4] = surface_r;
		array[5] = surface_g;
		array[6] = surface_b;
		array[7] = surface_a;

		array[8] = copy[((found + 1) % count) * 2 + 0];
		array[9] = copy[((found + 1) % count) * 2 + 1];
		array[10] = 0;
		array[11] = f_id;
		array[12] = surface_r;
		array[13] = surface_g;
		array[14] = surface_b;
		array[15] = surface_a;

		array[16] = copy[((found + 2) % count) * 2 + 0];
		array[17] = copy[((found + 2) % count) * 2 + 1];
		array[18] = 0;
		array[19] = f_id;
		array[20] = surface_r;
		array[21] = surface_g;
		array[22] = surface_b;
		array[23] = surface_a;
		array += 24;
		if(id != NULL)
		{
			object->triangles[object->triangle_used].id = id;
			object->triangles[object->triangle_used].part = part;
			object->triangles[object->triangle_used].a[0] = copy[found * 2 + 0];
			object->triangles[object->triangle_used].a[1] = copy[found * 2 + 1];
			object->triangles[object->triangle_used].a[2] = 0;
			object->triangles[object->triangle_used].b[0] = copy[((found + 1) % count) * 2 + 0];
			object->triangles[object->triangle_used].b[1] = copy[((found + 1) % count) * 2 + 1];
			object->triangles[object->triangle_used].b[2] = 0;
			object->triangles[object->triangle_used].c[0] = copy[((found + 2) % count) * 2 + 0];
			object->triangles[object->triangle_used].c[1] = copy[((found + 2) % count) * 2 + 1];
			object->triangles[object->triangle_used].c[2] = 0;
			object->triangle_used++;	
		}			
		for(i = ((found + 1) % count); i < count; i++)
		{
			copy[i * 2 + 0] = copy[i * 2 + 2];
			copy[i * 2 + 1] = copy[i * 2 + 3];
		}
		count--;
	}
	free(copy);
}


void seduce_background_quad_add(SeduceBackgroundObject *object, void *id, uint part,
									float a_x, float a_y, float a_z, 
									float b_x, float b_y, float b_z, 
									float c_x, float c_y, float c_z, 
									float d_x, float d_y, float d_z, 
									float surface_r, float surface_g, float surface_b, float surface_a)
{
	uint i;
	float *array;
	float f_id = -1.0;
	if(object->surface_length < object->surface_used + 2)
	{
		object->surface_length += 16;
		object->surface_buffer = realloc(object->surface_buffer, (sizeof *object->surface_buffer) * object->surface_length * 8 * 3);
	}
	if(id != NULL)
	{
		for(i = 0; i < object->triangle_length && (object->triangles[i].id != id || object->triangles[i].part != part); i++);
		if(i < object->triangle_used)
			f_id = object->triangles[i].shader_id;
		else
			f_id = ++object->shader_id_max;
	}
	array = &object->surface_buffer[object->surface_used * 8 * 3];
	array[0] = a_x;
	array[1] = a_y;
	array[2] = a_z;
	array[3] = f_id;
	array[4] = surface_r;
	array[5] = surface_g;
	array[6] = surface_b;
	array[7] = surface_a;
	array[8] = b_x;
	array[9] = b_y;
	array[10] = b_z;
	array[11] = f_id;
	array[12] = surface_r;
	array[13] = surface_g;
	array[14] = surface_b;
	array[15] = surface_a;
	array[16] = c_x;
	array[17] = c_y;
	array[18] = c_z;
	array[19] = f_id;
	array[20] = surface_r;
	array[21] = surface_g;
	array[22] = surface_b;
	array[23] = surface_a;
	array[24] = a_x;
	array[25] = a_y;
	array[26] = a_z;
	array[27] = f_id;
	array[28] = surface_r;
	array[29] = surface_g;
	array[30] = surface_b;
	array[31] = surface_a;
	array[32] = c_x;
	array[33] = c_y;
	array[34] = c_z;
	array[35] = f_id;
	array[36] = surface_r;
	array[37] = surface_g;
	array[38] = surface_b;
	array[39] = surface_a;
	array[40] = d_x;
	array[41] = d_y;
	array[42] = d_z;
	array[43] = f_id;
	array[44] = surface_r;
	array[45] = surface_g;
	array[46] = surface_b;
	array[47] = surface_a;
	object->surface_used += 2;
	if(id == NULL)
		return;
	if(object->triangle_length < object->triangle_used + 2)
	{
		object->triangle_length += 16;
		object->triangles = realloc(object->triangles, (sizeof *object->triangles) * object->triangle_length);
	}
	object->triangles[object->triangle_used].id = id;
	object->triangles[object->triangle_used].part = part;
	object->triangles[object->triangle_used].shader_id = f_id;
	object->triangles[object->triangle_used].a[0] = a_x;
	object->triangles[object->triangle_used].a[1] = a_y;
	object->triangles[object->triangle_used].a[2] = a_z;
	object->triangles[object->triangle_used].b[0] = b_x;
	object->triangles[object->triangle_used].b[1] = b_y;
	object->triangles[object->triangle_used].b[2] = b_z;
	object->triangles[object->triangle_used].c[0] = c_x;
	object->triangles[object->triangle_used].c[1] = c_y;
	object->triangles[object->triangle_used].c[2] = c_z;
	object->triangle_used++;
	object->triangles[object->triangle_used].id = id;
	object->triangles[object->triangle_used].part = part;
	object->triangles[object->triangle_used].shader_id = f_id;
	object->triangles[object->triangle_used].a[0] = a_x;
	object->triangles[object->triangle_used].a[1] = a_y;
	object->triangles[object->triangle_used].a[2] = a_z;
	object->triangles[object->triangle_used].b[0] = c_x;
	object->triangles[object->triangle_used].b[1] = c_y;
	object->triangles[object->triangle_used].b[2] = c_z;
	object->triangles[object->triangle_used].c[0] = d_x;
	object->triangles[object->triangle_used].c[1] = d_y;
	object->triangles[object->triangle_used].c[2] = d_z;
	object->triangle_used++;
}



void seduce_background_square_add(SeduceBackgroundObject *object, void *id, uint part,
									float pos_x, float pos_y, float pos_z, 
									float size_x, float size_y,
									float surface_r, float surface_g, float surface_b, float surface_a)
{
	seduce_background_quad_add(object, id, part,
										pos_x, pos_y, pos_z, 
										pos_x, pos_y + size_y, pos_z, 
										pos_x + size_x, pos_y + size_y, pos_z, 
										pos_x + size_x, pos_y, pos_z, 
										surface_r, surface_g, surface_b, surface_a);
}

void seduce_background_tri_add(SeduceBackgroundObject *object, void *id, uint part,
									float a_x, float a_y, float a_z, 
									float b_x, float b_y, float b_z, 
									float c_x, float c_y, float c_z, 
									float surface_r, float surface_g, float surface_b, float surface_a)
{
	float *array;
	float f_id = -1.0;
	uint i;
	if(id != NULL)
	{
		for(i = 0; i < object->triangle_length && (object->triangles[i].id != id || object->triangles[i].part != part); i++);
		if(i < object->triangle_used)
			f_id = object->triangles[i].shader_id;
		else
			f_id = ++object->shader_id_max;
	}
	if(object->surface_length <= object->surface_used)
	{
		object->surface_length += 16;
		object->surface_buffer = realloc(object->surface_buffer, (sizeof *object->surface_buffer) * object->surface_length * 8 * 3);
	}
	array = &object->surface_buffer[object->surface_used * 8 * 3];
	array[0] = a_x;
	array[1] = a_y;
	array[2] = a_z;
	array[3] = f_id;
	array[4] = surface_r;
	array[5] = surface_g;
	array[6] = surface_b;
	array[7] = surface_a;
	array[8] = b_x;
	array[9] = b_y;
	array[10] = b_z;
	array[11] = f_id;
	array[12] = surface_r;
	array[13] = surface_g;
	array[14] = surface_b;
	array[15] = surface_a;
	array[16] = c_x;
	array[17] = c_y;
	array[18] = c_z;
	array[19] = f_id;
	array[20] = surface_r;
	array[21] = surface_g;
	array[22] = surface_b;
	array[23] = surface_a;
	object->surface_used += 1;
	if(id == NULL)
		return;

	if(object->triangle_length <= object->surface_used)
	{
		object->triangle_length += 16;
		object->triangles = realloc(object->triangles, (sizeof *object->triangles) * object->triangle_length);
	}
	object->triangles[object->triangle_used].id = id;
	object->triangles[object->triangle_used].part = part;
	object->triangles[object->triangle_used].a[0] = a_x;
	object->triangles[object->triangle_used].a[1] = a_y;
	object->triangles[object->triangle_used].a[2] = a_z;
	object->triangles[object->triangle_used].b[0] = b_x;
	object->triangles[object->triangle_used].b[1] = b_y;
	object->triangles[object->triangle_used].b[2] = b_z;
	object->triangles[object->triangle_used].c[0] = c_x;
	object->triangles[object->triangle_used].c[1] = c_y;
	object->triangles[object->triangle_used].c[2] = c_z;
	object->triangle_used++;
}


void seduce_background_tri_fade_add(SeduceBackgroundObject *object, void *id, uint part,
									float a_x, float a_y, float a_z, 
									float b_x, float b_y, float b_z, 
									float c_x, float c_y, float c_z, 
									float a_surface_r, float a_surface_g, float a_surface_b, float a_surface_a,
									float b_surface_r, float b_surface_g, float b_surface_b, float b_surface_a,
									float c_surface_r, float c_surface_g, float c_surface_b, float c_surface_a)
{
	float *array;
	float f_id = -1.0;
	uint i;
	if(id != NULL)
	{
		for(i = 0; i < object->triangle_length && (object->triangles[i].id != id || object->triangles[i].part != part); i++);
		if(i < object->triangle_used)
			f_id = object->triangles[i].shader_id;
		else
			f_id = ++object->shader_id_max;
	}
	if(object->surface_length <= object->surface_used)
	{
		object->surface_length += 16;
		object->surface_buffer = realloc(object->surface_buffer, (sizeof *object->surface_buffer) * object->surface_length * 8 * 3);
	}
	array = &object->surface_buffer[object->surface_used * 8 * 3];
	array[0] = a_x;
	array[1] = a_y;
	array[2] = a_z;
	array[3] = f_id;
	array[4] = a_surface_r;
	array[5] = a_surface_g;
	array[6] = a_surface_b;
	array[7] = a_surface_a;
	array[8] = b_x;
	array[9] = b_y;
	array[10] = b_z;
	array[11] = f_id;
	array[12] = b_surface_r;
	array[13] = b_surface_g;
	array[14] = b_surface_b;
	array[15] = b_surface_a;
	array[16] = c_x;
	array[17] = c_y;
	array[18] = c_z;
	array[19] = f_id;
	array[20] = c_surface_r;
	array[21] = c_surface_g;
	array[22] = c_surface_b;
	array[23] = c_surface_a;
	object->surface_used += 1;
	if(id == NULL)
		return;

	if(object->triangle_length <= object->surface_used)
	{
		object->triangle_length += 16;
		object->triangles = realloc(object->triangles, (sizeof *object->triangles) * object->triangle_length);
	}
	object->triangles[object->triangle_used].id = id;
	object->triangles[object->triangle_used].part = part;
	object->triangles[object->triangle_used].a[0] = a_x;
	object->triangles[object->triangle_used].a[1] = a_y;
	object->triangles[object->triangle_used].a[2] = a_z;
	object->triangles[object->triangle_used].b[0] = b_x;
	object->triangles[object->triangle_used].b[1] = b_y;
	object->triangles[object->triangle_used].b[2] = b_z;
	object->triangles[object->triangle_used].c[0] = c_x;
	object->triangles[object->triangle_used].c[1] = c_y;
	object->triangles[object->triangle_used].c[2] = c_z;
	object->triangle_used++;
}


void seduce_background_image_add(SeduceBackgroundObject *object, void *id, uint part, uint texture_id, 
									float a_x, float a_y, 
									float b_x, float b_y, 
									float c_x, float c_y, 
									float d_x, float d_y, 
									float origo_x, float origo_y, float scale_x, float scale_y)
{
	SeduceBackgroundImage *image;
	if(object->image_length <= object->image_used)
	{
		object->image_length += 16;
		object->images = realloc(object->images, (sizeof *object->images) * object->image_length);
	}
	image = &object->images[object->image_used++];	
	image->texture_id = texture_id;
	image->array[0] = a_x;
	image->array[1] = a_y;
	image->array[2] = b_x;
	image->array[3] = b_y;
	image->array[4] = c_x;
	image->array[5] = c_y;
	image->array[6] = a_x;
	image->array[7] = a_y;
	image->array[8] = c_x;
	image->array[9] = c_y;
	image->array[10] = d_x;
	image->array[11] = d_y;
	image->origo[0] = origo_x;
	image->origo[1] = origo_y;
	image->scale[0] = scale_x;
	image->scale[1] = scale_y;
	if(id == NULL)
		return;

	if(object->triangle_length < object->triangle_used + 2)
	{
		object->triangle_length += 16;
		object->triangles = realloc(object->triangles, (sizeof *object->triangles) * object->triangle_length);
	}
	object->triangles[object->triangle_used].id = id;
	object->triangles[object->triangle_used].part = part;
	object->triangles[object->triangle_used].a[0] = a_x;
	object->triangles[object->triangle_used].a[1] = a_y;
	object->triangles[object->triangle_used].a[2] = 0;
	object->triangles[object->triangle_used].b[0] = b_x;
	object->triangles[object->triangle_used].b[1] = b_y;
	object->triangles[object->triangle_used].b[2] = 0;
	object->triangles[object->triangle_used].c[0] = c_x;
	object->triangles[object->triangle_used].c[1] = c_y;
	object->triangles[object->triangle_used].c[2] = 0;
	object->triangle_used++;
	object->triangles[object->triangle_used].id = id;
	object->triangles[object->triangle_used].part = part;
	object->triangles[object->triangle_used].a[0] = a_x;
	object->triangles[object->triangle_used].a[1] = a_y;
	object->triangles[object->triangle_used].a[2] = 0;
	object->triangles[object->triangle_used].b[0] = c_x;
	object->triangles[object->triangle_used].b[1] = c_y;
	object->triangles[object->triangle_used].b[2] = 0;
	object->triangles[object->triangle_used].c[0] = d_x;
	object->triangles[object->triangle_used].c[1] = d_y;
	object->triangles[object->triangle_used].c[2] = 0;
	object->triangle_used++;
}

extern uint seduce_line_texture_id;

void seduce_background_circles_init()
{
	char buffer[2048];
	float *array;
	uint size[3] = {3, 4}, i;
	RFormats types[3] = {R_FLOAT, R_FLOAT};
	s_background_circles_pool = r_array_allocate(6, types, size, 2, 0);

	array = malloc((sizeof *array) * 6 * 10);

	array[0] = -1;
	array[1] = -1;
	array[2] = 0;
	array[3] = 0.6;
	array[4] = 0.6;
	array[5] = 0.6;
	array[6] = 0.8;

	array[7] = 1;
	array[8] = -1;
	array[9] = 0;
	array[10] = 0.6;
	array[11] = 0.6;
	array[12] = 0.6;
	array[13] = 0.8;

	array[14] = 1;
	array[15] = 1;
	array[16] = 0;
	array[17] = 0.2;
	array[18] = 0.6;
	array[19] = 1.0;
	array[20] = 0.8;

	array[21] = -1;
	array[22] = -1;
	array[23] = 0;
	array[24] = 0.6;
	array[25] = 0.6;
	array[26] = 0.6;
	array[27] = 1;

	array[28] = 1;
	array[29] = 1;
	array[30] = 0;
	array[31] = 0.2;
	array[32] = 0.6;
	array[33] = 1.0;
	array[34] = 1;

	array[35] = -1;
	array[36] = 1;
	array[37] = 0;
	array[38] = 0.2;
	array[39] = 0.6;
	array[40] = 1.0;
	array[41] = 1;

	r_array_load_vertex(s_background_circles_pool, NULL, array, 0, 6);
	s_background_shader_circles = r_shader_create_simple(buffer, 2048, s_background_shader_circles_vertex, s_background_shader_circles_fragment, "background circles");
	r_shader_texture_set(s_background_shader_circles, 0, seduce_line_texture_id);
	r_shader_state_set_blend_mode(s_background_shader_circles, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	r_shader_state_set_depth_test(s_background_shader_circles, GL_ALWAYS);
	r_shader_state_set_mask(s_background_shader_circles, TRUE, TRUE, TRUE, TRUE, FALSE);
	s_background_shader_surface2 = r_shader_create_simple(buffer, 2048, s_background_shader_surface_vertex2, s_background_shader_surface_fragment2, "background surface");
	


	for(i = 0; i < SEDUCE_SURFACE_CLICK_BUFFER_SIZE; i++)
	{
		seduce_surface_click_buffer[i].timer = 100;
		sprintf(buffer, "point_%u_pos", i);
		s_background_shader_surface_point_locations[i][0] = r_shader_uniform_location(s_background_shader_surface2, buffer);
		sprintf(buffer, "point_%u_size", i);
		s_background_shader_surface_point_locations[i][1] = r_shader_uniform_location(s_background_shader_surface2, buffer);
		sprintf(buffer, "point_%u_color", i);
		s_background_shader_surface_point_locations[i][2] = r_shader_uniform_location(s_background_shader_surface2, buffer);
		sprintf(buffer, "point_%u_id", i);
		s_background_shader_surface_point_locations[i][3] = r_shader_uniform_location(s_background_shader_surface2, buffer);
	}



	r_shader_state_set_blend_mode(s_background_shader_surface2, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	r_shader_state_set_depth_test(s_background_shader_surface2, GL_ALWAYS);

	
	s_background_shader_image = r_shader_create_simple(buffer, 2048, s_background_shader_image_vertex, s_background_shader_image_fragment, "background image");
	r_shader_texture_set(s_background_shader_image, 0, seduce_line_texture_id);
	r_shader_state_set_blend_mode(s_background_shader_image, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	r_shader_state_set_depth_test(s_background_shader_image, GL_ALWAYS);

	s_background_shader_shadow = r_shader_create_simple(buffer, 2048, s_background_shader_shadow_vertex, s_background_shader_shadow_fragment, "background surface");
	r_shader_state_set_blend_mode(s_background_shader_shadow, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	r_shader_state_set_mask(s_background_shader_shadow, TRUE, TRUE, TRUE, TRUE, FALSE);
	r_shader_state_set_depth_test(s_background_shader_shadow, GL_ALWAYS);
}



void seduce_background_polygon_flush(BInputState *input, float *center, float time);
void seduce_particle_render(BInputState *input);

extern uint particle_debug_texture_id;


void seduce_primitive_shadow_draw(SeduceBackgroundObject *object, float time)
{
	if(time < 0.6)
		return;
	time = (time - 0.6) / 0.4;
	if(object->shadow_pool == NULL)
	{
		uint size = 4, i;
		RFormats types = R_FLOAT;
		if(object->shadow_length == 0)
			return;
		object->shadow_pool = r_array_allocate(object->shadow_length, &types, &size, 1, 0);
		r_array_load_vertex(object->shadow_pool, NULL, object->shadow_buffer, 0, object->shadow_length);
		free(object->shadow_buffer);
		object->shadow_buffer = NULL;
	}
	r_shader_set(s_background_shader_shadow);
	r_shader_float_set(s_background_shader_shadow, r_shader_uniform_location(s_background_shader_shadow, "shadow_size"), 3.0 - 2.0 * time);
	r_shader_float_set(s_background_shader_shadow, r_shader_uniform_location(s_background_shader_shadow, "color"), time * 0.1);
	r_array_draw(object->shadow_pool, NULL, R_PRIMITIVE_TRIANGLES, 0, -1, NULL, NULL, 1); 
}



void seduce_primitive_surface_draw(BInputState *input, SeduceBackgroundObject *object, float time)
{
	static boolean init = FALSE;
	float t, brightness, size = 0.5, flare_colors[] = {0.2, 0.6, 1.0, 0.0, 0.6, 0.2, 1.0, 0.0, 1.0, 0.6, 0.2, 0.0}, matrix[9], gray[9], f;
	uint i, ii, j;
	if(time < 0.001 || input->mode != BAM_DRAW)
		return;
		
	if(object->surface_pool == NULL)
	{
		uint size[3] = {4, 4}, i;
		RFormats types[3] = {R_FLOAT, R_FLOAT};
		if(object->surface_used != 0)
		{
			object->surface_pool = r_array_allocate(object->surface_used * 3, types, size, 2, 0);
			r_array_load_vertex(object->surface_pool, NULL, object->surface_buffer, 0, object->surface_used * 3);
			object->surface_used = 0;
			object->surface_length = 0;
			free(object->surface_buffer);
			object->surface_buffer = NULL;
		}
	}
	if(object->surface_pool != NULL)
	{
		r_shader_set(s_background_shader_surface2);

		t = time;
	
		if(t < 0)
			t = 0;
		if(t > 1.0)
			t = 1.0;
		time = t;
		t /= 4;

		matrix[0] = 0.2;
		matrix[1] = 0.6;
		matrix[2] = 1.0;
		matrix[3] = 0.6;
		matrix[4] = 0.2;
		matrix[5] = 1.0;
		matrix[6] = 1.0;
		matrix[7] = 0.6;
		matrix[8] = 0.2;

		matrix[0] = 0.4;
		matrix[1] = 0.0;
		matrix[2] = 0.0;
		matrix[3] = 0.2;
		matrix[4] = 0.6;
		matrix[5] = 1.0;
		matrix[6] = 0.0;
		matrix[7] = 0.0;
		matrix[8] = 1.0;
	
		if(time < 0.3)
		{	
			r_shader_float_set(s_background_shader_surface2, r_shader_uniform_location(s_background_shader_surface2, "fill"), 0.005);
			r_shader_float_set(s_background_shader_surface2, r_shader_uniform_location(s_background_shader_surface2, "scroll"), t * -0.1);
			r_shader_float_set(s_background_shader_surface2, r_shader_uniform_location(s_background_shader_surface2, "scale"), 1.0 / (1 + t * 4));
			r_shader_float_set(s_background_shader_surface2, r_shader_uniform_location(s_background_shader_surface2, "time"), t);
			r_shader_mat3v_set(s_background_shader_surface2, r_shader_uniform_location(s_background_shader_surface2, "color_matrix"), matrix);	
			r_shader_float_set(s_background_shader_surface2, r_shader_uniform_location(s_background_shader_surface2, "outer"), t * t * 8.0);
		}else
		{
			brightness = (time - 0.3) / 0.7;
			r_shader_float_set(s_background_shader_surface2, r_shader_uniform_location(s_background_shader_surface2, "fill"), brightness);
			r_shader_float_set(s_background_shader_surface2, r_shader_uniform_location(s_background_shader_surface2, "scroll"), t * -0.1 + sin(input->minute_time * PI * 2) * 0.2 * brightness);
			r_shader_float_set(s_background_shader_surface2, r_shader_uniform_location(s_background_shader_surface2, "scale"), 1.0 / (1 + t * 4) * (1.0 - brightness) + 0.7 * brightness);
			r_shader_float_set(s_background_shader_surface2, r_shader_uniform_location(s_background_shader_surface2, "time"), t * (1.0 - brightness) + 0.25 + input->minute_time * 0.75 * brightness);
			r_shader_mat3v_set(s_background_shader_surface2, r_shader_uniform_location(s_background_shader_surface2, "color_matrix"), matrix);	
			r_shader_float_set(s_background_shader_surface2, r_shader_uniform_location(s_background_shader_surface2, "outer"), t * t * 8.0 + brightness * 100.0);
		}
		r_shader_float_set(s_background_shader_surface2, r_shader_uniform_location(s_background_shader_surface2, "inner"), 0);

		if(time < 0.99)
			r_shader_float_set(s_background_shader_surface2, r_shader_uniform_location(s_background_shader_surface2, "time_size"), time * time * 2.0);
		else
			r_shader_float_set(s_background_shader_surface2, r_shader_uniform_location(s_background_shader_surface2, "time_size"), 1000000.0);
		
		f = betray_screen_mode_get(NULL, NULL, NULL);
		if(f < 1.0)
			f = 1.0;
		f *= 0.6 + 0.2 * sin(input->minute_time * PI * 11.0);
		r_shader_vec2_set(s_background_shader_surface2, r_shader_uniform_location(s_background_shader_surface2, "pointer"), f * sin(input->minute_time * PI * 4.0), f * cos(input->minute_time * PI * 4.0));
		betray_screen_mode_get(&i, NULL, NULL);
		r_shader_float_set(s_background_shader_surface2, r_shader_uniform_location(s_background_shader_surface2, "resolution"), i / 2);
	

		for(i = 0; i < SEDUCE_SURFACE_CLICK_BUFFER_SIZE; i++)
		{
			ii = (seduce_surface_click_buffer_next + i) % SEDUCE_SURFACE_CLICK_BUFFER_SIZE;

			r_shader_vec2_set(s_background_shader_surface2, s_background_shader_surface_point_locations[i][0], 
															seduce_surface_click_buffer[ii].pos[0],
															seduce_surface_click_buffer[ii].pos[1]);
			if(seduce_surface_click_buffer[ii].timer > 1.0)
				r_shader_float_set(s_background_shader_surface2, s_background_shader_surface_point_locations[i][1], 0);
			else
			{
				f = 1.0 - seduce_surface_click_buffer[ii].timer;
				r_shader_float_set(s_background_shader_surface2, s_background_shader_surface_point_locations[i][1], (1.0 - f * f) * seduce_surface_click_buffer[ii].size);
			}
			
			if(seduce_surface_click_buffer[ii].timer > 0.6)
				r_shader_vec4_set(s_background_shader_surface2, s_background_shader_surface_point_locations[i][2], 
																seduce_surface_click_buffer[ii].color[0],
																seduce_surface_click_buffer[ii].color[1],
																seduce_surface_click_buffer[ii].color[2],
																1.0 - (seduce_surface_click_buffer[ii].timer - 0.6) / 0.4);
			else
				r_shader_vec4_set(s_background_shader_surface2, s_background_shader_surface_point_locations[i][2], 
																seduce_surface_click_buffer[ii].color[0],
																seduce_surface_click_buffer[ii].color[1],
																seduce_surface_click_buffer[ii].color[2],
																1.0);
			for(j = 0; j < object->triangle_used && (object->triangles[j].id != seduce_surface_click_buffer[ii].id || object->triangles[j].part != seduce_surface_click_buffer[ii].part); j++);
			if(j < object->triangle_used)
				r_shader_float_set(s_background_shader_surface2, s_background_shader_surface_point_locations[i][3], (float)object->triangles[j].shader_id + 0.5);
			else	
				r_shader_float_set(s_background_shader_surface2, s_background_shader_surface_point_locations[i][3], -1);										
		}
		r_array_draw(object->surface_pool, NULL, R_PRIMITIVE_TRIANGLES, 0, -1, NULL, NULL, 1);
	}
	if(object->images != 0)
	{
		if(s_background_pool_image == NULL)
		{
			uint size = 2;
			RFormats types = R_FLOAT;
			s_background_pool_image = r_array_allocate(6, &types, &size, 1, 0);
		}
		r_shader_set(s_background_shader_image);
		if(time < 0.7)
		{	
			r_shader_float_set(s_background_shader_image, r_shader_uniform_location(s_background_shader_image, "fill"), 0.005);
			r_shader_float_set(s_background_shader_image, r_shader_uniform_location(s_background_shader_image, "scroll"), t * -1);
			r_shader_float_set(s_background_shader_image, r_shader_uniform_location(s_background_shader_image, "scale"), 1.0 / (1 + t * 4));
			r_shader_float_set(s_background_shader_image, r_shader_uniform_location(s_background_shader_image, "time"), t);
			r_shader_mat3v_set(s_background_shader_image, r_shader_uniform_location(s_background_shader_image, "color_matrix"), matrix);	
		}else
		{
			brightness = (time - 0.7) / 0.3;
			r_shader_float_set(s_background_shader_image, r_shader_uniform_location(s_background_shader_image, "fill"), brightness);
			r_shader_float_set(s_background_shader_image, r_shader_uniform_location(s_background_shader_image, "scroll"), t * -1 + sin(input->minute_time * PI * 2) * 0.2 * brightness);
			r_shader_float_set(s_background_shader_image, r_shader_uniform_location(s_background_shader_image, "scale"), 1.0 / (1 + t * 4) * (1.0 - brightness) + 0.7 * brightness);
			r_shader_float_set(s_background_shader_image, r_shader_uniform_location(s_background_shader_image, "time"), t * (1.0 - brightness) + 0.25 + input->minute_time * 0.75 * brightness);
			r_shader_mat3v_set(s_background_shader_image, r_shader_uniform_location(s_background_shader_image, "color_matrix"), matrix);	

		}
		r_shader_float_set(s_background_shader_image, r_shader_uniform_location(s_background_shader_image, "inner"), 0);
		r_shader_float_set(s_background_shader_image, r_shader_uniform_location(s_background_shader_image, "outer"), t * 4 + t * t * 100.0);

		for(i = 0; i < object->image_used; i++)
		{
			uint j;
			r_array_load_vertex(s_background_pool_image, NULL, object->images[i].array, 0, 6);
			for(j = 0; j < 8; j++)
				printf("draw[%u] = %f\n", j, object->images[i].array[j]);
			r_shader_texture_set(s_background_shader_image, 2, object->images[i].texture_id);
			r_shader_vec2_set(s_background_shader_image, r_shader_uniform_location(s_background_shader_image, "origo"), object->images[i].origo[0], object->images[i].origo[1]);	
			r_shader_vec2_set(s_background_shader_image, r_shader_uniform_location(s_background_shader_image, "stretch"), 1.0 / object->images[i].scale[0], 1.0 / object->images[i].scale[1]);	
			r_array_draw(s_background_pool_image, NULL, R_PRIMITIVE_TRIANGLES, 0, 6, NULL, NULL, 1);
		}
	}


	if(time > 0.7)
		for(i = 0; i < object->triangle_used; i++)
			seduce_element_add_triangle(input, object->triangles[i].id, object->triangles[i].part, object->triangles[i].a, object->triangles[i].b, object->triangles[i].c);

	seduce_primitive_shadow_draw(object, time);
}


void seduce_primitive_background_flare_draw(float x, float y, float z, float time, float size)
{
	float t, brightness, matrix[9], flare_colors[] = {0.2, 0.6, 1.0, 0.0, 0.6, 0.2, 1.0, 0.0, 1.0, 0.6, 0.2, 0.0};
	BInputState *input;
	uint i;
	if(time > 0.999 || time < 0.001)
		return;
	input = betray_get_input_state();
	matrix[0] = 0.4;
	matrix[1] = 0.0;
	matrix[2] = 0.0;
	matrix[3] = 0.6;
	matrix[4] = 0.8;
	matrix[5] = 0.6;
	matrix[6] = 1.0;
	matrix[7] = 1.0;
	matrix[8] = 1.0;

	r_matrix_push(NULL);
	r_matrix_translate(NULL, x, y, z);
	r_matrix_scale(NULL, size, size, size);
	t = time;
	t /= 4;	
	r_shader_set(s_background_shader_circles);
	r_shader_mat3v_set(s_background_shader_circles, r_shader_uniform_location(s_background_shader_circles, "color_matrix"), matrix);
	for(i = 0 ; i < 3; i++)
	{
		r_matrix_translate(NULL, 0, 0, 0.05 + 0.05 * t);
		brightness = time * (1 - time) * (1 - time) * 10.0;
		r_shader_vec4_set(s_background_shader_circles, r_shader_uniform_location(s_background_shader_circles, "flare_color"), flare_colors[i * 4 + 0] * brightness, flare_colors[i * 4 + 1] * brightness, flare_colors[i * 4 + 2] * brightness, flare_colors[i * 4 + 3] * brightness);
		r_shader_float_set(s_background_shader_circles, r_shader_uniform_location(s_background_shader_circles, "fill"), 0.001);
		r_shader_float_set(s_background_shader_circles, r_shader_uniform_location(s_background_shader_circles, "scroll"), size * -time + (float)i / 4.0 + sin(input->minute_time * PI * 2 + (float)i * PI * 1.5) * 0.2);
		r_shader_float_set(s_background_shader_circles, r_shader_uniform_location(s_background_shader_circles, "time"), 0.25 + t + 0.25 * (float)i);
		r_shader_float_set(s_background_shader_circles, r_shader_uniform_location(s_background_shader_circles, "inner"), time * time);
		r_shader_float_set(s_background_shader_circles, r_shader_uniform_location(s_background_shader_circles, "outer"), time);
		r_shader_float_set(s_background_shader_circles, r_shader_uniform_location(s_background_shader_circles, "scale"), 1.0 / (1 + t * 4));
		r_array_draw(s_background_circles_pool, NULL, R_PRIMITIVE_TRIANGLES, 0, 6, NULL, NULL, 1); 
	}
	r_matrix_pop(NULL);
}
/*
typedef enum{
	S_PUT_TOP,
	S_PUT_BOTTOM,
	S_PUT_ANGLE,
	S_PUT_BUTTON,
	S_PUT_IMAGE,
}SPopUpType;

typedef struct{
	SPopUpType	type;
	char		*text;
	union{
		float button_pos[2];
		float angle[2];
		struct{
			float pos[2];
			float size[2];
			uint texture_id;
		}image;
		struct{
			float angle[2];
			float *value;
		}slider_angle;
	}data;
}SUIPUElement;
*/
void seduce_background_popup_element(SeduceBackgroundObject *obj, void *id, uint part, float y, float x, float gap, float height, float width, float z, float *color)
{
	float list[8];
	height = (height - gap) / 2;
	gap *= 0.5;
	list[0] = (x - width) * (y + height) / y + gap;
	list[1] = y + height; 
	list[2] = (x + width) * (y + height) / y - gap;
	list[3] = y + height; 
	list[4] = (x + width) * (y - height) / y - gap; 
	list[5] = y - height; 
	list[6] = (x - width) * (y - height) / y + gap; 
	list[7] = y - height;
	seduce_background_quad_add(obj, id, part,
									list[0], list[1], z, 
									list[2], list[3], z, 
									list[4], list[5], z, 
									list[6], list[7], z, 
									color[0], color[1], color[2], color[3]);
	seduce_background_shadow_add(obj, list, 4, TRUE, gap);
}

#define SEDUCE_POPUP_HORIZONTAL_MIN_SIZE 0.06

uint seduce_popup(BInputState *input, void *id, SUIPUElement *elements, uint count, float time)
{
	SeduceBackgroundObject *obj;
	uint i, j, k, length, type[2] = {S_PUT_TOP, S_PUT_BOTTOM}, type_count[S_PUT_COUNT], element, image_start, output = -1, part;
	float pos, shadow[4] = {0.08, 0, -0.08, 0}, f, size, gap = 0, square_size, square_dist, vec[12], rnd, color[4] = {0.0, 0.0, 0.0, 0.9}, selected[4] = {0.2, 0.6, 1.0, 0.9}, *c, center[3] = {1.0, 1.0, 1.0}, text[3] = {1.0, 1.0, 1.0}, list[12];
	obj = seduce_background_object_allocate();
	gap = 0.002 + (1.0 - time) * 0.02;
	gap = 0.002 + (1.0 - time) * 0.02;

	if(input->mode == BAM_DRAW)
	{
		for(i = 0; i < S_PUT_COUNT; i++)
			type_count[i] = 0;

		for(i = 0; i < count; i++)
			type_count[elements[i].type]++;
	
		for(i = 0; i < count; i++)
		{
			if(elements[i].type == S_PUT_ANGLE)
			{
				vec[0] = sin(elements[i].data.angle[0] / 180.0 * PI);
				vec[1] = cos(elements[i].data.angle[0] / 180.0 * PI);
				vec[2] = 0;
				vec[3] = sin((elements[i].data.angle[0] + elements[i].data.angle[1]) / 360.0 * PI);
				vec[4] = cos((elements[i].data.angle[0] + elements[i].data.angle[1]) / 360.0 * PI);
				vec[5] = 0;
				vec[6] = sin(elements[i].data.angle[1] / 180.0 * PI);
				vec[7] = cos(elements[i].data.angle[1] / 180.0 * PI);
				vec[8] = 0;
				if(elements[i].data.angle[1] - elements[i].data.angle[0] > 360.0 / 6.5)
					f = 0.09;
				else
					f = 0.06;
				rnd = f_randnf(i) * (1.0 - time) * 0.2;
				if(seduce_element_active(input, id, &part) && i == part)
					c = selected;
				else
					c = color; 
				list[0] = vec[0] * 0.07 + vec[1] * gap * 0.5; 
				list[1] = vec[1] * 0.07 - vec[0] * gap * 0.5;
				list[2] = vec[0] * 0.19 + vec[1] * gap * 0.5; 
				list[3] = vec[1] * 0.19 - vec[0] * gap * 0.5;
				list[4] = vec[3] * 0.25;
				list[5] = vec[4] * 0.25;
				list[6] = vec[6] * 0.19 - vec[7] * gap * 0.5;
				list[7] = vec[7] * 0.19 + vec[6] * gap * 0.5;
				list[8] = vec[6] * 0.07 - vec[7] * gap * 0.5;
				list[9] = vec[7] * 0.07 + vec[6] * gap * 0.5;
				list[10] = vec[3] * f; 
				list[11] = vec[4] * f;
				seduce_background_shadow_add(obj, list, 6, TRUE, gap * 4);

				seduce_background_quad_add(obj, NULL, 0,
										vec[0] * 0.07 + vec[1] * gap * 0.5, vec[1] * 0.07 - vec[0] * gap * 0.5, rnd, 
										vec[0] * 0.19 + vec[1] * gap * 0.5, vec[1] * 0.19 - vec[0] * gap * 0.5, rnd, 
										vec[3] * 0.25, vec[4] * 0.25, rnd, 
										vec[3] * f, vec[4] * f, rnd, 
										c[0], c[1], c[2], c[3]);
				seduce_background_quad_add(obj, NULL, 0,
										vec[3] * f, vec[4] * f, rnd, 
										vec[3] * 0.25, vec[4] * 0.25, rnd, 
										vec[6] * 0.19 - vec[7] * gap * 0.5, vec[7] * 0.19 + vec[6] * gap * 0.5, rnd, 
										vec[6] * 0.07 - vec[7] * gap * 0.5, vec[7] * 0.07 + vec[6] * gap * 0.5, rnd, 
										c[0], c[1], c[2], c[3]);			
				vec[3] = vec[0] * 0.06;
				vec[4] = vec[1] * 0.06;
				vec[5] = vec[2] * 0.06;
				vec[9] = vec[6] * 0.06;
				vec[10] = vec[7] * 0.06;
				vec[11] = vec[8] * 0.06;
				seduce_element_add_quad(input, id, i, &vec[0], &vec[3], &vec[9], &vec[6]);
			}
		}

		for(image_start = 2; ; image_start++)
		{
			k = image_start;
			for(i = 0; ; k++)
			{
				if(i + k >= type_count[S_PUT_IMAGE])
					break;
				i += k;
			}
			if((type_count[S_PUT_IMAGE] - i) % 2 == k % 2)
				break;
		}
		if(type_count[S_PUT_BOTTOM] == 0)
		{
			square_dist = 0.02 + 0.15 * (1.0 / (float)(type_count[S_PUT_IMAGE] + 6));
			square_size = square_dist - gap * 0.7;
		}else
		{
			square_dist = -0.02 - 0.15 * (1.0 / (float)(type_count[S_PUT_IMAGE] + 6));
			square_size = square_dist + gap * 0.7;
		}
		j = image_start;
		for(i = element = 0; i < type_count[S_PUT_IMAGE];)
		{
			if(type_count[S_PUT_IMAGE] - i < j)
			{
				k = (j - (type_count[S_PUT_IMAGE] - i)) / 2;
			}else
				k = 0;
			for(; k < j && i < type_count[S_PUT_IMAGE]; k++)
			{
				vec[0] = square_dist * (float)j - (float)k * square_dist * 2.0 - square_dist;
				vec[1] = -square_dist * (float)j;
				while(elements[element].type != S_PUT_IMAGE)
					element++;
				
				rnd = f_randnf(i) * (1.0 - time) * 0.2;
				if(seduce_element_active(input, id, &part) && element == part)
					c = selected;
				else
					c = color; 
				seduce_background_quad_add(obj, id, element,
										vec[0] + square_size, vec[1], rnd, 
										vec[0], vec[1] + square_size, rnd, 
										vec[0] - square_size, vec[1], rnd, 
										vec[0], vec[1] - square_size, rnd, 
										c[0], c[1], c[2], c[3]);
				element++;
				i++;
			}
			j++;
		}

		pos = 0.07;
		for(j = element = 0; j < type_count[S_PUT_TOP]; j += length)
		{
			f = pos * 4 / (1 + 0.03 * (float)type_count[S_PUT_TOP]);
			if(f > 1)
				size = 0.05;
			else
			{
				size = 0.007 * (float)type_count[S_PUT_TOP];
				if(size > 0.2)
					size = 0.2;
				size = f_splinef(f, 0.1, size + 0.1, 0.04 + 0.003 * (float)type_count[S_PUT_TOP], SEDUCE_POPUP_HORIZONTAL_MIN_SIZE);
			}
			if(size > pos)
				size = pos;
			length = (uint)(size / SEDUCE_POPUP_HORIZONTAL_MIN_SIZE); 
			if(length == 0)
				length = 1;
			for(k = 0; k < length && j + k < type_count[S_PUT_TOP]; k++)
			{
				while(elements[element].type != S_PUT_TOP)
					element++;
				if(seduce_element_active(input, id, &part) && element == part)
					c = selected;
				else
					c = color; 
				seduce_background_popup_element(obj, id, element, pos, size * (((float)k + 0.5) * 2.0 / (length)) - size, gap, 0.03, size / (float)length, f_randnf(i) * (1.0 - time) * 0.2, c);
				element++;
			}
			shadow[1] = shadow[3] = pos;
			pos += 0.03; 
		}
		pos = -0.07;
		for(j = element = 0; j < type_count[S_PUT_BOTTOM]; j += length)
		{
			f = pos * -4 / (1 + 0.03 * (float)type_count[S_PUT_BOTTOM]);
			if(f > 1)
				size = 0.05;
			else
			{
				size = 0.007 * (float)type_count[S_PUT_BOTTOM];
				if(size > 0.2)
					size = 0.2;
				size = f_splinef(f, 0.1, size + 0.1, 0.04 + 0.003 * (float)type_count[S_PUT_BOTTOM], SEDUCE_POPUP_HORIZONTAL_MIN_SIZE);
			}
			if(size > -pos)
				size = -pos;
			length = (uint)(size / SEDUCE_POPUP_HORIZONTAL_MIN_SIZE); 
			if(length == 0)
				length = 1;
			for(k = 0; k < length && j + k < type_count[S_PUT_BOTTOM]; k++)
			{
				while(elements[element].type != S_PUT_BOTTOM)
					element++;
				if(seduce_element_active(input, id, &part) && element == part)
					c = selected;
				else
					c = color; 
				seduce_background_popup_element(obj, id, element, pos, size * (((float)k + 0.5) * 2.0 / (length)) - size, gap, 0.03, size / (float)length, f_randnf(i) * (1.0 - time) * 0.2, c);
				element++;
			}
			shadow[1] = shadow[3] = pos;
			pos -= 0.03; 
		}	
		seduce_primitive_surface_draw(input, obj, time);	
	//	seduce_primitive_background_flare_draw(0, 0, 0, time * 0.7, 0.4);
		seduce_primitive_background_object_free(obj);


		for(i = 0; i < count; i++)
		{
			if(elements[i].type == S_PUT_ANGLE)
			{
				f = (elements[i].data.angle[0] + elements[i].data.angle[1]) * 0.5;
				if(f < 180.0)
				{
					r_matrix_push(NULL);
					r_matrix_rotate(NULL, 90 - f,  0, 0, 1);
					seduce_text_line_draw(NULL, 0.20 - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, elements[i].text, -1), SEDUCE_T_SIZE * -0.75, SEDUCE_T_SIZE, SEDUCE_T_SPACE, elements[i].text, 1, 1, 1, 1, -1);
					r_matrix_pop(NULL);
				}else 
				{
					r_matrix_push(NULL);
					r_matrix_rotate(NULL, 270 - f,  0, 0, 1);
					seduce_text_line_draw(NULL, -0.20, SEDUCE_T_SIZE * -0.75, SEDUCE_T_SIZE, SEDUCE_T_SPACE, elements[i].text, 1, 1, 1, 1, -1);
					r_matrix_pop(NULL);
				}
			}
		}
	
		pos = 0.07;
		for(j = element = 0; j < type_count[S_PUT_TOP]; j += length)
		{
			f = pos * 4 / (1 + 0.03 * (float)type_count[S_PUT_TOP]);
			if(f > 1)
				size = 0.05;
			else
			{
				size = 0.007 * (float)type_count[S_PUT_TOP];
				if(size > 0.2)
					size = 0.2;
				size = f_splinef(f, 0.1, size + 0.1, 0.04 + 0.003 * (float)type_count[S_PUT_TOP], SEDUCE_POPUP_HORIZONTAL_MIN_SIZE);
			}
			if(size > pos)
				size = pos;
			length = (uint)(size / SEDUCE_POPUP_HORIZONTAL_MIN_SIZE); 
			if(length == 0)
				length = 1;
			for(k = 0; k < length && j + k < type_count[S_PUT_TOP]; k++)
			{
				while(elements[element].type != S_PUT_TOP)
					element++;
				seduce_text_line_draw(NULL, size * (((float)k + 0.5) * 2.0 / (length)) - size - seduce_text_line_length(NULL, 0.5 * SEDUCE_T_SIZE, SEDUCE_T_SPACE, elements[element].text, -1), pos - SEDUCE_T_SIZE * 0.75, SEDUCE_T_SIZE, SEDUCE_T_SPACE, elements[element].text, text[0], text[1], text[2], 1, -1);
				element++;
			}
			shadow[1] = shadow[3] = pos;
			pos += 0.03; 
		}
		pos = -0.07;
		for(j = element = 0; j < type_count[S_PUT_BOTTOM]; j += length)
		{
			f = pos * -4 / (1 + 0.03 * (float)type_count[S_PUT_BOTTOM]);
			if(f > 1)
				size = 0.05;
			else
			{
				size = 0.007 * (float)type_count[S_PUT_BOTTOM];
				if(size > 0.2)
					size = 0.2;
				size = f_splinef(f, 0.1, size + 0.1, 0.04 + 0.003 * (float)type_count[S_PUT_BOTTOM], SEDUCE_POPUP_HORIZONTAL_MIN_SIZE);
			}
			if(size > -pos)
				size = -pos;
			length = (uint)(size / SEDUCE_POPUP_HORIZONTAL_MIN_SIZE); 
			if(length == 0)
				length = 1;
			for(k = 0; k < length && j + k < type_count[S_PUT_BOTTOM]; k++)
			{
				while(elements[element].type != S_PUT_BOTTOM)
					element++;
				seduce_text_line_draw(NULL, size * (((float)k + 0.5) * 2.0 / (length)) - size - seduce_text_line_length(NULL, 0.5 * SEDUCE_T_SIZE, SEDUCE_T_SPACE, elements[element].text, -1), pos - SEDUCE_T_SIZE * 0.75, SEDUCE_T_SIZE, SEDUCE_T_SPACE, elements[element].text, text[0], text[1], text[2], 1, -1);
				element++;
			}
			shadow[1] = shadow[3] = pos;
			pos -= 0.03; 
		}	


		j = image_start;
		for(i = element = 0; i < type_count[S_PUT_IMAGE];)
		{
			if(type_count[S_PUT_IMAGE] - i < j)
			{
				k = (j - (type_count[S_PUT_IMAGE] - i)) / 2; 
			}else
				k = 0;
			for(; k < j && i < type_count[S_PUT_IMAGE]; k++)
			{
				while(elements[element].type != S_PUT_IMAGE)
					element++;				
			//	if(seduce_text_button(input, &elements[element], square_dist * (float)j - (float)k * square_dist * 2.0 - square_dist, -square_dist * (float)j - SEDUCE_T_SIZE * 0.75, 0.5, SEDUCE_T_SIZE, SEDUCE_T_SPACE, elements[element].text, 0, 0, 0, 1, 1, 1, 1, 1))
			//		return element;

				seduce_text_line_draw(NULL, square_dist * (float)j - (float)k * square_dist * 2.0 - square_dist - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, elements[i].text, -1) * 0.5, -square_dist * (float)j - SEDUCE_T_SIZE * 0.75, SEDUCE_T_SIZE, SEDUCE_T_SPACE, elements[i].text, text[0], text[1], text[2], 1, -1);
				element++;
				i++;
			}
			j++;
		}
	}
	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
			{
				output = SEDUCE_POP_UP_DEACTIVATE;
				if(id == seduce_element_pointer_id(input, i, &j))
				{
					seduce_surface_circle_spawn(input, input->pointers[i].pointer_x, input->pointers[i].pointer_y, 1, NULL, 0, 1, 1, 1);
					return j;
				}
			}				
		}
		for(i = 0; i < input->button_event_count; i++)
		{
			if(input->button_event[i].state && (input->button_event[i].button == BETRAY_BUTTON_FACE_A || input->button_event[i].button == BETRAY_BUTTON_YES))
			{
				uint axis;
				axis = seduce_element_primary_axis(input, input->button_event[i].user_id);
				if(axis != -1)
				{
					r_matrix_projection_screenf(NULL, center, 0, 0, 0);
					center[0] += input->axis[axis].axis[0];
					center[1] += input->axis[axis].axis[1];
					center[2] = 0;
					if(id == seduce_element_colission_test(input->axis[axis].axis, &j, input->axis[axis].user_id))
						return j;
				}
			}				
		}
	}
	return output;
}

void seduce_background_circles_test()
{
	static boolean init = FALSE;
	SeduceBackgroundObject *obj;
	BInputState *input;
	static float time = 1.0;
	float array[16 * 2], f;
	uint i, count;
	input = betray_get_input_state();
	seduce_animate(input, &time, input->pointers[0].button[0], 1.0);

/*	if(time > 0.9)
		time = 0.9;*/
		
//	return;
		
	if(input->mode != BAM_DRAW)
		return;
	{
		static SUIPUElement elements[60];
		count = (uint)(30.0 * (1.0 + input->pointers[0].pointer_x));
		for(i = 0; i < count / 2; i++)
		{
			elements[i].text = "Option";
			elements[i].type = S_PUT_ANGLE;
			elements[i].data.angle[0] = 45.0 + (float)((i / 2) * 90) * 4 / (float)count;
			elements[i].data.angle[1] = 45.0 + (float)((i / 2) * 90 + 90) * 4 / (float)count;
			i++;
			elements[i].text = "Option";
			elements[i].type = S_PUT_ANGLE;
			elements[i].data.angle[0] = 180 + 45.0 + (float)((i / 2) * 90) * 4 / (float)count;
			elements[i].data.angle[1] = 180 + 45.0 + (float)((i / 2) * 90 + 90) * 4 / (float)count;
		}
		for(; i < count * 3 / 4; i++)
		{
			elements[i].text = "Option";
			elements[i].type = S_PUT_BOTTOM;
		}
		for(; i < count; i++)
		{
			elements[i].text = "Option";
			elements[i].type = S_PUT_IMAGE;
		}
	//	seduce_background_popup(input, elements, elements, count, time);
	}
	return;
	if(!init)
	{
		init = TRUE;
		seduce_background_circles_init();
	}
	r_matrix_push(NULL);
//	r_matrix_rotate(NULL, input->minute_time * 720, 0, 1, 0);

	for(i = 0; i < 16; i++)
	{
		f = 0.1 + 0.04 * f_randf(i + 4);
		array[i * 2 + 0] = sin((float)i / 16.0 * 2.0 * PI) * f;
		array[i * 2 + 1] = cos((float)i / 16.0 * 2.0 * PI) * f;
	}

	obj = seduce_background_object_allocate();
	
//	seduce_background_shadow_add(obj, array, 16, FALSE);
	seduce_background_quad_add(obj, obj, 0,
									0.15, 0.4, 0, 
									-0.15, 0.4, 0, 
									-0.15, -0.4, 0, 
									0.15, -0.4, 0, 
									 0.8, 0.8, 0.8, 0.8);
//	seduce_background_shape_add(obj, obj, 0, array, 16, 0.7, 0.7, 0.7, 1);



	seduce_primitive_surface_draw(input, obj, time);	
	seduce_primitive_background_flare_draw(0, 0, 0, time, 0.25);
//	seduce_primitive_shadow_draw(obj, time);

	r_matrix_translate(NULL, 0.3, 0.3, 0);

	seduce_primitive_surface_draw(input, obj, time);	
	seduce_primitive_background_flare_draw(0, 0, 0, time, 0.25);

	seduce_primitive_background_object_free(obj);
	r_matrix_pop(NULL);
}