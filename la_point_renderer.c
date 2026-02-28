

#include <stdio.h>
#include <math.h>
#include "forge.h"
#include "betray.h"
#include "relinquish.h"
#include "enough.h"
#include "la_geometry_undo.h"

float f_triangle_area2df(float *a, float *b, float *c)
{
	float vec[2], normal[2], f, f2; 
	normal[0] = a[0] - b[0];
	normal[1] = a[1] - b[1];
	f = sqrt(normal[0] * normal[0] + normal[1] * normal[1]);
	normal[0] /= f;
	normal[1] /= f;
	vec[0] = c[0] - b[0];
	vec[1] = c[1] - b[1];
	f2 = normal[0] * vec[0] + normal[1] * vec[1];
	vec[0] -= normal[0] * f2;
	vec[1] -= normal[1] * f2;
	return f * sqrt(vec[0] * vec[0] + vec[1] * vec[1]) / 2.0;
}

double f_triangle_area2dd(double *a, double *b, double *c)
{
	double vec[2], normal[2], f, f2; 
	normal[0] = a[0] - b[0];
	normal[1] = a[1] - b[1];
	f = sqrt(normal[0] * normal[0] + normal[1] * normal[1]);
	normal[0] /= f;
	normal[1] /= f;
	vec[0] = c[0] - b[0];
	vec[1] = c[1] - b[1];
	f2 = normal[0] * vec[0] + normal[1] * vec[1];
	vec[0] -= normal[0] * f2;
	vec[1] -= normal[1] * f2;
	return f * sqrt(vec[0] * vec[0] + vec[1] * vec[1]) / 2.0;
}

float f_triangle_area3df(float *a, float *b, float *c)
{
	float vec[3], normal[3], f, f2; 
	normal[0] = a[0] - b[0];
	normal[1] = a[1] - b[1];
	normal[2] = a[2] - b[2];
	f = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	normal[0] /= f;
	normal[1] /= f;
	normal[2] /= f;
	vec[0] = c[0] - b[0];
	vec[1] = c[1] - b[1];
	vec[2] = c[2] - b[2];
	f2 = normal[0] * vec[0] + normal[1] * vec[1] + normal[2] * vec[2];
	vec[0] -= normal[0] * f2;
	vec[1] -= normal[1] * f2;
	vec[2] -= normal[2] * f2;
	f2 = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
	f = f * f2 / 2.0;
	if(f != f)
		f = 1;
	return f;
}

double f_triangle_area3dd(double *a, double *b, double *c)
{
	double vec[3], normal[3], f, f2; 
	normal[0] = a[0] - b[0];
	normal[1] = a[1] - b[1];
	normal[2] = a[2] - b[2];
	f = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	normal[0] /= f;
	normal[1] /= f;
	normal[2] /= f;
	vec[0] = c[0] - b[0];
	vec[1] = c[1] - b[1];
	vec[2] = c[2] - b[2];
	f2 = normal[0] * vec[0] + normal[1] * vec[1] + normal[2] * vec[2];
	vec[0] -= normal[0] * f2;
	vec[1] -= normal[1] * f2;
	vec[2] -= normal[2] * f2;
	return f * sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]) / 2.0;
}


float *compute_point_cloud(uint *triangles, uint triangle_count, float *vertex, uint stride, uint point_count, float *surface)
{
	float *distance_array, *output, sum = 0, f, *a, *b, *c, weight_a, weight_b, weight_c, *point_cloud, r, r2;
	uint i, j, min, max, test;
	distance_array = malloc((triangle_count + 1) * (sizeof *distance_array));
	point_cloud = malloc(point_count * (stride + 3) * (sizeof *point_cloud));
	triangle_count *= 3;
	distance_array[0] = 0;
	for(i = 0; i < triangle_count; )
	{
		f = f_triangle_area3df(&vertex[triangles[i] * stride], &vertex[triangles[i + 1] * stride], &vertex[triangles[i + 2] * stride]);
		i += 3;
		sum += f;
		distance_array[i / 3] = sum;
	}
	distance_array[i / 3] = sum;
	*surface = sqrt(sum);
	triangle_count /= 3;
	for(i = 0; i < point_count; i++)
	{
		min = 0;
		max = triangle_count;
		f = f_randf(i) * sum;
		while(min + 1 < max)
		{
			test = min + (uint)((f - distance_array[min]) / (distance_array[max] - distance_array[min]) * (float)(max - min));
			if(test <= min + 1)
				test = min + 1;
			if(test >= max - 1)
				test = max - 1;
			if(distance_array[test] < f)
				min = test;
			else
				max = test;
		}
		min *= 3; 
		a = &vertex[triangles[min] * stride];
		b = &vertex[triangles[min + 1] * stride];
		c = &vertex[triangles[min + 2] * stride];

		r2 = f_randf(i + 1);
		r = sqrt(f_randf(i + 2));
		weight_a = 1 - r;
		weight_b = r * (1 - r2);
		weight_c = r * r2;


		for(j = 0; j < stride; j++)
			point_cloud[i * (stride + 3) + j] = a[j] * weight_a + b[j] * weight_b + c[j] * weight_c;

		f_normal3f(&point_cloud[i * (stride + 3) + stride], a, b, c);
		
	//	for(j = 0; j < 3; j++)
	//		point_cloud[i * stride + j] += 0.01 * f_randnf(i + 3 + j);
	}
	free(distance_array);
	return point_cloud;
}

uint *compute_triangels(uint *polygons, uint poly_count, uint vertex_count, uint *output_count, float *vertex_array)
{
	uint i, j, count = 0, *triangles;
	float normal[3];
	poly_count *= 4;
	for(i = 0; i < poly_count; i += 4)
	{	
		if(polygons[i] < vertex_count && polygons[i + 1] < vertex_count && polygons[i + 2] < vertex_count)
		{
			count++;
			if(polygons[i + 3] < vertex_count)
				count++;	
		}
	}
	*output_count = count;
	if(count == 0)
		return NULL;
	triangles = malloc((sizeof *triangles) * count * 3);
	for(i = count = 0; i < poly_count; i += 4)
	{	
		if(polygons[i] < vertex_count && polygons[i + 1] < vertex_count && polygons[i + 2] < vertex_count)
		{
			triangles[count++] = polygons[i];
			triangles[count++] = polygons[i + 1];
			triangles[count++] = polygons[i + 2];
			f_normal3f(normal, &vertex_array[polygons[i] * 6], &vertex_array[polygons[i + 1] * 6], &vertex_array[polygons[i + 2] * 6]);
			
			vertex_array[polygons[i] * 6 + 3] += normal[0];
			vertex_array[polygons[i] * 6 + 4] += normal[1];
			vertex_array[polygons[i] * 6 + 5] += normal[2];
			vertex_array[polygons[i + 1] * 6 + 3] += normal[0];
			vertex_array[polygons[i + 1] * 6 + 4] += normal[1];
			vertex_array[polygons[i + 1] * 6 + 5] += normal[2];
			vertex_array[polygons[i + 2] * 6 + 3] += normal[0];
			vertex_array[polygons[i + 2] * 6 + 4] += normal[1];
			vertex_array[polygons[i + 2] * 6 + 5] += normal[2];

			if(polygons[i + 3] < vertex_count)
			{
				triangles[count++] = polygons[i];
				triangles[count++] = polygons[i + 2];
				triangles[count++] = polygons[i + 3];
				f_normal3f(normal, &vertex_array[polygons[i] * 6], &vertex_array[polygons[i + 2] * 6], &vertex_array[polygons[i + 3] * 6]);			
				vertex_array[polygons[i] * 6 + 3] += normal[0];
				vertex_array[polygons[i] * 6 + 4] += normal[1];
				vertex_array[polygons[i] * 6 + 5] += normal[2];
				vertex_array[polygons[i + 2] * 6 + 3] += normal[0];
				vertex_array[polygons[i + 2] * 6 + 4] += normal[1];
				vertex_array[polygons[i + 2] * 6 + 5] += normal[2];
				vertex_array[polygons[i + 3] * 6 + 3] += normal[0];
				vertex_array[polygons[i + 3] * 6 + 4] += normal[1];
				vertex_array[polygons[i + 3] * 6 + 5] += normal[2];
			}	
		}
	}
	return triangles;
}

#define POINT_CLOAD_SIZE 100000

void *point_cloud_test_convert(float *surface)
{
	uint32 vertex_count, polygon_count, *ref;
	double *vertexd;
	float *vertexf, *points;
	RFormats vertex_format_types[3] = {R_FLOAT, R_FLOAT, R_FLOAT};
	uint i, vertex_format_size[3] = {3, 3, 3};
	void *pool = NULL;
	udg_get_geometry(&vertex_count,&polygon_count, &vertexd, &ref, NULL);
	vertexf = malloc((sizeof *vertexf) * 6 * vertex_count);
	for(i = 0; i < vertex_count; i++)
	{
		vertexf[i * 6 + 0] = (float)vertexd[i * 3 + 0] + 2;
		vertexf[i * 6 + 1] = (float)vertexd[i * 3 + 1];
		vertexf[i * 6 + 2] = (float)vertexd[i * 3 + 2];
		vertexf[i * 6 + 3] = 0;
		vertexf[i * 6 + 4] = 0;
		vertexf[i * 6 + 5] = 0;
	}

	ref = compute_triangels(ref, polygon_count, vertex_count, &polygon_count, vertexf);
	for(i = 0; i < vertex_count; i++)
	{
		f_normalize3f(&vertexf[i * 6 + 3]);
	}
	if(ref == NULL)
	{
		free(vertexf);
		return NULL;
	}
	points = compute_point_cloud(ref, polygon_count, vertexf, 6, POINT_CLOAD_SIZE, surface);
	free(ref);
	pool = r_array_allocate(POINT_CLOAD_SIZE, vertex_format_types, vertex_format_size, 3, 0);
	r_array_load_vertex(pool, NULL, points, 0, POINT_CLOAD_SIZE);
	free(points);
	return pool;
}


/*
void point_cloud_test_sanity()
{
	uint32 vertex_count, polygon_count, *ref;
	double *vertexd;
	uint i;
	udg_get_geometry(&vertex_count,&polygon_count, &vertexd, &ref, NULL);
	ref = compute_triangels(ref, polygon_count, vertex_count, &polygon_count);

	
	for(i = 0; i < polygon_count; i++)
	{
		r_primitive_line_3d((float)vertexd[ref[i * 3 + 0] * 3 + 0] + 0.3,
							(float)vertexd[ref[i * 3 + 0] * 3 + 1] + 0.3,
							(float)vertexd[ref[i * 3 + 0] * 3 + 2] + 0.3,
							(float)vertexd[ref[i * 3 + 1] * 3 + 0] + 0.3,
							(float)vertexd[ref[i * 3 + 1] * 3 + 1] + 0.3,
							(float)vertexd[ref[i * 3 + 1] * 3 + 2] + 0.3, 1, 1, 1, 1);
		r_primitive_line_3d((float)vertexd[ref[i * 3 + 1] * 3 + 0] + 0.3,
							(float)vertexd[ref[i * 3 + 1] * 3 + 1] + 0.3,
							(float)vertexd[ref[i * 3 + 1] * 3 + 2] + 0.3,
							(float)vertexd[ref[i * 3 + 2] * 3 + 0] + 0.3,
							(float)vertexd[ref[i * 3 + 2] * 3 + 1] + 0.3,
							(float)vertexd[ref[i * 3 + 2] * 3 + 2] + 0.3, 1, 1, 1, 1);
		r_primitive_line_3d((float)vertexd[ref[i * 3 + 0] * 3 + 0] + 0.3,
							(float)vertexd[ref[i * 3 + 0] * 3 + 1] + 0.3,
							(float)vertexd[ref[i * 3 + 0] * 3 + 2] + 0.3,
							(float)vertexd[ref[i * 3 + 2] * 3 + 0] + 0.3,
							(float)vertexd[ref[i * 3 + 2] * 3 + 1] + 0.3,
							(float)vertexd[ref[i * 3 + 2] * 3 + 2] + 0.3, 1, 1, 1, 1);
	}
	r_primitive_line_flush();
	free(ref);
}
*/

char *point_cloud_vertex = 
"attribute vec3 vertex;\n"
"attribute vec3 soft;\n"
"attribute vec3 normal;\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"uniform mat4 ModelViewMatrix;\n"
"uniform mat4 NormalMatrix;\n"
"uniform vec3 light;\n"
"uniform float point_size;\n"
"varying vec4 color;\n"
"void main()\n"
"{\n"
"	vec4 n, v;\n"
"	float size;\n"
//"	color = vec4(normal, 1.0);\n"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex, 1.0);\n"
"	n = NormalMatrix * vec4(soft, 1.0);\n"
"	v = ModelViewMatrix * vec4(vertex, 1.0);\n"
//"	color = vec4(vec3(max(dot(n.rgb, light), 0.0)), 1.0);\n"
"	color = vec4(vec3(0.3, 0.5, 0.7) + vec3(0.7, 0.5, 0.4) * vec3(dot(n.rgb, light)), 1.0);\n"
"	n = NormalMatrix * vec4(normal, 1.0)\n;"
"	size = 0.1 + dot(n.xyz, normalize(v.xyz));\n"
"	gl_PointSize = point_size / gl_Position.z * size;\n"
"}\n";

char *point_cloud_fragment = 
"varying vec4 color;\n"
"void main()\n"
"{\n"
"	gl_FragColor = color;\n"
"}\n";

#define GL_PROGRAM_POINT_SIZE 0x8642

void point_cloud_test(BInputState *input)
{
	static void *pool = NULL;
	static uint version;
	static float surface = 1;
	uint i;
	return;
	i = udg_get_version(TRUE, TRUE, FALSE, FALSE, FALSE);
	if(i != version)
	{
		version = i;
		if(pool != NULL)
			r_array_free(pool);
		pool = point_cloud_test_convert(&surface);
	}

	if(pool != NULL)
	{
		static RShader	*shader = NULL;
		float f, vec[3] = {2, 2, 0};
		if(shader == NULL)
		{
			shader = r_shader_create_simple(NULL, 0, point_cloud_vertex, point_cloud_fragment, "point shader");
		}
		glPointSize(5);
		glEnable(GL_PROGRAM_POINT_SIZE);
		r_shader_set(shader);
		f = sqrt(input->pointers[0].pointer_x * 0.5 + 0.5);
		r_shader_float_set(shader, r_shader_uniform_location(shader, "point_size"), surface * 10.0 / f); 
		vec[0] += sin(input->minute_time * 80 * PI) * 3.0;
		f_normalize3f(vec);
		r_shader_vec3_set(shader, r_shader_uniform_location(shader, "light"), vec[0], vec[1], vec[2]); 

		r_array_draw(pool, NULL, R_PRIMITIVE_POINTS, 0, (uint)((input->pointers[0].pointer_x * 0.5 + 0.5) * POINT_CLOAD_SIZE), NULL, NULL, 1);
	//	glDisable(GL_PROGRAM_POINT_SIZE);
	}
//	point_cloud_test_sanity();

}









