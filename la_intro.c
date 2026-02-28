#include "la_includes.h"

#include "la_geometry_undo.h"
#include "forge.h"
#include "la_particle_fx.h"
#include "la_projection.h"

struct{
	void *pool;
	void *circle;
	void *blade;
	void *shader;
}GlobalIntro;


char *r_intro_shader_vertex = 
"attribute vec2 vertex;"
"uniform mat4 ModelViewProjectionMatrix;"
"void main()"
"{"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex, 0.0, 1.0);"
"}";

char *r_intro_shader_fragment = 
"uniform vec4 color;"
"void main()"
"{"
"	gl_FragColor = color;"
"}";



#define CIRCLE_SEGMENTS 48
#define PI  3.141592653

/* allocate a big array of vertex */

void la_intro_init(void)
{
	RFormats types = R_FLOAT;
	float *blade;
	float *ring;
	uint i, size = 2;
	blade = malloc((sizeof *blade) * 42 * 2);
	ring = malloc((sizeof *ring) * CIRCLE_SEGMENTS * 6 * 2);

	for(i = 0; i < CIRCLE_SEGMENTS; i++)
	{
		ring[i * 12 + 0] = 0.09 * sin((float)i * PI * 2 / CIRCLE_SEGMENTS);
		ring[i * 12 + 1] = 0.09 * cos((float)i * PI * 2 / CIRCLE_SEGMENTS);
		ring[i * 12 + 2] = 0.11 * sin((float)i * PI * 2 / CIRCLE_SEGMENTS);
		ring[i * 12 + 3] = 0.11 * cos((float)i * PI * 2 / CIRCLE_SEGMENTS);
		ring[i * 12 + 4] = 0.11 * sin((float)(i + 1) * PI * 2 / CIRCLE_SEGMENTS);
		ring[i * 12 + 5] = 0.11 * cos((float)(i + 1) * PI * 2 / CIRCLE_SEGMENTS);
		ring[i * 12 + 6] = 0.09 * sin((float)i * PI * 2 / CIRCLE_SEGMENTS);
		ring[i * 12 + 7] = 0.09 * cos((float)i * PI * 2 / CIRCLE_SEGMENTS);
		ring[i * 12 + 8] = 0.11 * sin((float)(i + 1) * PI * 2 / CIRCLE_SEGMENTS);
		ring[i * 12 + 9] = 0.11 * cos((float)(i + 1) * PI * 2 / CIRCLE_SEGMENTS);
		ring[i * 12 + 10] = 0.09 * sin((float)(i + 1) * PI * 2 / CIRCLE_SEGMENTS);
		ring[i * 12 + 11] = 0.09 * cos((float)(i + 1) * PI * 2 / CIRCLE_SEGMENTS);
	}
	blade[0] = 0.01;
	blade[1] = 0.4175;
	blade[2] = -0.01;
	blade[3] = 0.4175;
	blade[4] = -0.02;
	blade[5] = 0.415;
	blade[6] = 0.01;
	blade[7] = 0.4175;
	blade[8] = -0.02;
	blade[9] = 0.415;
	blade[10] = 0.02;
	blade[11] = 0.415;

	blade[12] = 0.02; 
	blade[13] = 0.415;
	blade[14] = -0.02; 
	blade[15] = 0.415;
	blade[16] = -0.03;
	blade[17] = 0.41;
	blade[18] = 0.02; 
	blade[19] = 0.415;
	blade[20] = -0.03;
	blade[21] = 0.41;
	blade[22] = 0.03; 
	blade[23] = 0.41;

	blade[24] = 0.03; 
	blade[25] = 0.41;
	blade[26] = -0.03;
	blade[27] = 0.41;
	blade[28] = -0.06;
	blade[29] = 0.03;
	blade[30] = 0.03; 
	blade[31] = 0.41;
	blade[32] = -0.06;
	blade[33] = 0.03;
	blade[34] = 0.06;
	blade[35] = 0.03;

	blade[36] = 0.06;
	blade[37] = 0.03;
	blade[38] = -0.06;
	blade[39] = 0.03;
	blade[40] = -0.057;
	blade[41] = 0.025;
	blade[42] = 0.06;
	blade[43] = 0.03;
	blade[44] = -0.057;
	blade[45] = 0.025;
	blade[46] = 0.057;
	blade[47] = 0.025;

	blade[48] = 0.057;
	blade[49] = 0.025;
	blade[50] = -0.057;
	blade[51] = 0.025;
	blade[52] = -0.05;
	blade[53] = 0.02;
	blade[54] = 0.057;
	blade[55] = 0.025;
	blade[56] = -0.05;
	blade[57] = 0.02;
	blade[58] = 0.05;
	blade[59] = 0.02;

	blade[60] = -0.05;
	blade[61] = 0.02;
	blade[62] = -0.04;
	blade[63] = 0.016;
	blade[64] = -0.035;
	blade[65] = 0.014;
	blade[66] = -0.05;
	blade[67] = 0.02;
	blade[68] = -0.035;
	blade[69] = 0.014;
	blade[70] = -0.01;
	blade[71] = 0.02;

	blade[72] = 0.035;
	blade[73] = 0.014;
	blade[74] = 0.04;
	blade[75] = 0.016;
	blade[76] = 0.05;
	blade[77] = 0.02;
	blade[78] = 0.035;
	blade[79] = 0.014;
	blade[80] = 0.05;
	blade[81] = 0.02;
	blade[82] = 0.01;
	blade[83] = 0.02;

	GlobalIntro.pool = r_array_allocate(CIRCLE_SEGMENTS * 6 + 42, &types, &size, 1, 0);
	GlobalIntro.circle = r_array_section_allocate_vertex(GlobalIntro.pool, CIRCLE_SEGMENTS * 6);
	GlobalIntro.blade = r_array_section_allocate_vertex(GlobalIntro.pool, 42);
	r_array_load_vertex(GlobalIntro.pool, GlobalIntro.circle, ring, 0, CIRCLE_SEGMENTS * 6);
	r_array_load_vertex(GlobalIntro.pool, GlobalIntro.blade, blade, 0, 42);
	free(ring);
	free(blade);

	GlobalIntro.shader = r_shader_create_simple(NULL, 0, r_intro_shader_vertex, r_intro_shader_fragment, "color font");
	r_shader_set(GlobalIntro.shader);
	r_shader_vec4_set(NULL, r_shader_uniform_location(GlobalIntro.shader, "color"), 0, 0, 0, 1);
}


void draw_la_star(double open)
{
	uint i;
	r_matrix_push(NULL);
	r_matrix_scale(NULL, 0.09, 0.09, 0.09);
	seduce_lens_draw(0.3, 0.5, 1.0, 0.6, 0.2, 1.0, 0.1, 0.7, 1.0);
	r_matrix_pop(NULL);
	r_shader_set(GlobalIntro.shader);
	r_array_section_draw(GlobalIntro.pool, GlobalIntro.circle, GL_TRIANGLES, 0, CIRCLE_SEGMENTS * 6);
	for(i = 0; i < 4; i++)
	{
		r_matrix_push(NULL);
		r_matrix_rotate(NULL, 90 * (double)i, 0, 0, 1);
		r_matrix_translate(NULL, 0, 0.1, 0);
		r_matrix_rotate(NULL, open * 100, 1, 0, 0);
		r_array_section_draw(GlobalIntro.pool, GlobalIntro.blade, GL_TRIANGLES, 0, 42);
		r_matrix_pop(NULL);
	}
}

void draw_la_star_drop(double x, double y, double z, double timer, boolean spark)
{
	double t2, pos[3];
	r_matrix_push(NULL);
	t2 = timer;
	if(timer > 1)
		timer = 1;
	timer = 1 - timer;
	pos[0] = x;
	pos[1] = y; 
	pos[2] = z - timer * timer * timer * 1;
	r_matrix_translate(NULL, pos[0], pos[1], pos[2]);
/*	if(spark)
		la_pfx_create_intro_spark(pos);*/
	r_matrix_rotate(NULL, 20 * t2, 0, 0, 1);
	r_matrix_rotate(NULL, timer * sin(timer * 20), 0, 1, 0);
	r_matrix_rotate(NULL, timer * cos(timer * 20), 1, 0, 0);
	draw_la_star(1 - (1 / (1 + timer * timer * timer * timer)));
	r_matrix_pop(NULL);
}


void la_intro_draw(void *user)
{
	uint i;
	static double t = -100, rot = 1;
	static uint spark = 0;
	double r;
	t += betray_time_delta_get() * 20.0;
	spark++;
//	sw_draw_background_square();
	r = t - 400;
	if(r > 0)
		r = 0;
//	r_text("LOQ", SUITM_SPACED, NULL, NULL, 0, -0.28 + r * 0.002, 0, 0.5, sui_get_material(SUIM_BLACK_FONT));
//	r_text(-0.44 + r * 0.002, 0, SEDUCE_T_SIZE * 4, SEDUCE_T_SPACE, "LOQ", 0.5, 0.5, 0.5);
//	r_text("AIROU", SUITM_SPACED, NULL, NULL, 0, 0.2 - r * 0.002, 0, 0.5, sui_get_material(SUIM_BLACK_FONT));
//	r_text(0.3 - r * 0.002, 0, SEDUCE_T_SIZE * 4, SEDUCE_T_SPACE, "AIROU", 0.5, 0.5, 0.5);
	r_matrix_push(NULL);
	r_matrix_scale(NULL, 0.2, 0.2, 0.2);
	if(t > 100)
		rot *= 0.992;
	r_matrix_translate(NULL, 0, 0, 2 - (2 * rot));
	r_matrix_rotate(NULL, rot * 30 + sin(t * 0.01) * 10.0, 0, 0, 1);
	r_matrix_rotate(NULL, rot * -90 + cos(t * 0.01) * 10.0, 0, 1, 0);

	for(i = 1; i < 40; i++)
		draw_la_star_drop(sin(i * 4) * 1.5, cos(i * 343.6) * 1.5, 1 + cos(i * 656.3) * 4.6 + t * 0.01 - 1, t * 0.02 + ((cos(i * 7) * 2) - 3), spark % 10 == i % 10);
	draw_la_star_drop(0, 0, 1.5, t * 0.01, spark % 4 == 1);
/*	la_pfx_draw(TRUE);*/
	r_matrix_pop(NULL);
}

