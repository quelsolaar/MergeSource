#define PERSUADE_INTERNAL
#include <stdlib.h>
#include "persuade2.h"
#include "p2_sds_table.h"

void blend_one_vertex(PTessTableElement *t, float *vertex, uint corners, uint id)
{
	float pos[8] = {-0.3, 0.3, 0.3, 0.3, 0.3, -0.3, -0.3, -0.3};
//	float pos[8] = {-0.3, 0.3, 0.3, 0.3, 0, -0.2, -0.3, -0.3};

	uint i;
	vertex[0] = 0;
	vertex[1] = 0;
	for(i = 0; i < corners; i++)
	{
		vertex[0] += t->vertex_influence[id * corners + i] * pos[i * 2];
		vertex[1] += t->vertex_influence[id * corners + i] * pos[i * 2 + 1];
	}
}



void draw_table(PTessTableElement *t, uint corners)
{
//	static uint time = 0;
	float *vertex;

	uint i, j, k;
	vertex = malloc((sizeof *vertex) * t->element_count * 2);

	for(i = 0; i < t->element_count * 2; i++)
		vertex[i] = 0; 

	for(i = 0; i < t->element_count; i++)
	{
		blend_one_vertex(t, &vertex[i * 2], corners, t->index[i]);
	/*	if(t->index[i] < (time / 100) % t->edges[4])
		{
			vertex[i * 2] += 0.1;
			vertex[i * 2 + 1] += 0.1;
		}*/
	}
//	time++;

//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//	r_gl(GL_QUADS, pos, 4, 2, 1, 0, 1);	
//	r_gl(GL_TRIANGLES, vertex, t->element_count, 2, 1, 1, 1, 1);	

	free(vertex);
}


void draw_table_normals(PTessTableElement *t, uint corners)
{
	float vertex[6], a[2], b[2];
	uint i, j, k;
	for(i = 0; i < t->vertex_count; i++)
	{
		blend_one_vertex(t, vertex, corners, i);
		blend_one_vertex(t, a, corners, t->normals[i * 4]);
		blend_one_vertex(t, b, corners, t->normals[i * 4 + 1]);
		vertex[2] = (vertex[0] * 0.7 + a[0] * 0.3);
		vertex[3] = (vertex[1] * 0.7 + a[1] * 0.3);
		vertex[4] = (vertex[0] * 0.7 + b[0] * 0.3);
		vertex[5] = (vertex[1] * 0.7 + b[1] * 0.3);
	//	r_gl(GL_TRIANGLES, vertex, 3, 2, 0, 1, 0, 1);

		blend_one_vertex(t, vertex, corners, i);
		blend_one_vertex(t, a, corners, t->normals[i * 4 + 2]);
		blend_one_vertex(t, b, corners, t->normals[i * 4 + 3]);
		vertex[2] = (vertex[0] * 0.7 + a[0] * 0.3);
		vertex[3] = (vertex[1] * 0.7 + a[1] * 0.3);
		vertex[4] = (vertex[0] * 0.7 + b[0] * 0.3);
		vertex[5] = (vertex[1] * 0.7 + b[1] * 0.3);
	//	r_gl(GL_TRIANGLES, vertex, 3, 2, 1, 0, 0, 1);
	}
}

void draw_table_debugging(void)
{
	static uint splits[4] = {3, 2, 3, 4}, counter = 6400 * 2;
	PTessTableElement *t;

	splits[0] = (counter / 100)% 4;
	splits[1] = (counter / 400) % 4;
	splits[2] = (counter / 1600)% 4;
	splits[3] = (counter / 6400)% 4;
	counter++;

//	glPushMatrix();
//	glTranslatef(0, 0, -1);
//	get_dynamic_table_quad(p_get_max_tess_level(), splits);
	t = get_dynamic_table_quad(4, splits);
//	t = get_dynamic_table_tri(4, splits);
/*	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	draw_table(t, 4);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	draw_table_normals(t, 4);
	glTranslatef(0, 0.7, 0);
	draw_table(t, 4);
	glPopMatrix();*/
}

