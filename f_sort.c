#include <stdlib.h>
#include <stdio.h>
#include "forge.h"

uint *f_sort_ids(uint length, uint *ids, boolean (*compare_func)(uint bigger, uint smaller, void *user), void *user)
{
	uint i, size = 1, *a, *b, *c, pos0, pos1, end0, end1, put;
	a = malloc((sizeof *a) * length);
	b = malloc((sizeof *b) * length);
	if(ids == NULL)
	{
		for(i = 0; i < length; i++)
			a[i] = i;
	}else
		for(i = 0; i < length; i++)
			a[i] = ids[i];

	while(size < length)
	{
		put = 0;
		for(i = 0; i < length; i += size * 2)
		{
			pos0 = i; 
			pos1 = i + size;
			end0 = pos1;
			end1 = pos1 + size;
			if(end0 > length)
				end0 = length;
			if(end1 > length)
				end1 = length;
			while(pos0 < end0 && pos1 < end1)
			{
				if(compare_func(a[pos0], a[pos1], user))
					b[put++] = a[pos1++];  
				else
					b[put++] = a[pos0++];  
			}
			while(pos0 < end0)
				b[put++] = a[pos0++];
			while(pos1 < end1)
				b[put++] = a[pos1++];
		}
		c = b;
		b = a;
		a = c;
		size *= 2;
	}
	free(b);
	return a;
}


void **f_sort_pointers(uint length, void **pointers, boolean (*compare_func)(void *biger, void *smaller, void *user), void *user)
{
	uint i, size = 1, pos0, pos1, end0, end1, put;
	void **a, **b, **c;
	a = malloc((sizeof *a) * length);
	b = malloc((sizeof *b) * length);
	
	for(i = 0; i < length; i++)
		a[i] = pointers[i];

	while(size < length)
	{
		put = 0;
		for(i = 0; i < length; i += size * 2)
		{
			pos0 = i; 
			pos1 = i + size;
			end0 = pos1;
			end1 = pos1 + size;
			if(end0 > length)
				end0 = length;
			if(end1 > length)
				end1 = length;
			while(pos0 < end0 && pos1 < end1)
			{
				if(compare_func(a[pos0], a[pos1], user))
					b[put++] = a[pos1++];  
				else
					b[put++] = a[pos0++];  
			}
			while(pos0 < end0)
				b[put++] = a[pos0++];
			while(pos1 < end1)
				b[put++] = a[pos1++];
		}
		c = b;
		b = a;
		a = c;
		size *= 2;
	}
	free(b);
	return a;
}


boolean f_sort_strings_is_character(char value)
{
	return (value >= 48 && value <= 58) ||/* number */
			(value >= 65 && value <= 90) || /* text */
			(value >= 97 && value <= 122); /* text */
}



FStringSortResult f_sort_strings(char *a, char *b)
{
	uint i, a_pos = 0, b_pos = 0, number_a, number_b;
	char char_a, char_b;
	while(TRUE)
	{
		while(!f_sort_strings_is_character(a[a_pos]) && a[a_pos] != 0)	
			a_pos++;
		while(!f_sort_strings_is_character(b[b_pos]) && b[b_pos] != 0)
			b_pos++;
		if(a[a_pos] == 0)
		{
			if(b[b_pos] == 0)
			{
				break;
			}
			return F_SSR_B;
		}
		if(b[b_pos] == 0)
			return F_SSR_A;


		if(a[a_pos] <= '9')
		{
			if(b[b_pos] <= '9')
			{
				while(a[a_pos] == '0')
					a_pos++;
				while(b[b_pos] == '0')
					b_pos++;
				for(number_a = 0; a[a_pos + number_a] >= '0' && a[a_pos + number_a] <= '9'; number_a++);
				for(number_b = 0; b[b_pos + number_b] >= '0' && b[b_pos + number_b] <= '9'; number_b++);
				if(number_a == number_b)
				{
					for(i = 0; i < number_a; i++)
					{
						if(a[a_pos + i] != b[b_pos + i])
						{
							if(a[a_pos + i] < b[b_pos + i])
								return F_SSR_A;
							else
								return F_SSR_B;
						}
					}
					a_pos += number_a;
					b_pos += number_b;
				}else
				{
					if(number_a < number_b)
						return F_SSR_A;
					else
						return F_SSR_B;
				}
			}else
				return F_SSR_A;
		}else
		{
			if(b[b_pos] <= '9')
				return F_SSR_B;

			char_a = a[a_pos];
			if(char_a <= "Z")
				char_a += 32;
			char_b = b[b_pos];
			if(char_b <= "Z")
				char_b += 32;

			if(char_a != char_b)
			{
				if(char_a < char_b)
					return F_SSR_A;
				else
					return F_SSR_B;
			}
		}
		a_pos++;
		b_pos++;
	}

	for(i = 0; a[i] != 0 && a[i] == b[i]; i++);
	if(a[i] == b[i])
		return F_SSR_EQUAL;
	return a[i] > b[i];
}


boolean compate_func(uint big, uint small, void *user)
{
	float *array;
	array = user;
	return array[big] > array[small];
}

void f_sort_test()
{
	float array[100];
	uint *sorted, i;
	for(i = 0; i < 100; i++)
		array[i] = f_randf(i);
	sorted = f_sort_ids(100, NULL, compate_func, array);

	for(i = 0; i < 100; i++)
		printf("sort[%u] %u = %f\n", i, sorted[i], array[sorted[i]]);
	exit(0);
}


uint *f_sort_quad_tri_neighbor(uint *ref, uint quad_length, uint tri_length, uint vertex_count)
{
	uint i, cor, clear = 0, *n, *v, a, b;
	uint counter = 0, laps = 0;
	n = malloc((sizeof *n) * (quad_length + tri_length));
	for(i = 0; i < (quad_length + tri_length); i++)
		n[i] = -1;
	v = malloc((sizeof *v) * vertex_count);
	for(i = 0; i < vertex_count; i++)
		v[i] = -1;
/*
	printf("count\n");
	for(i = 0; i < (quad_length + tri_length); i++)
	{
		if(ref[i] == 9)
			printf("9 %u\n", i);
		if(ref[i] == 12)
			printf("12 %u\n", i);
	}
	for(i = 0; i < quad_length; i += 4)
		printf("ref[%u] %u %u %u %u\n", i, ref[i + 0], ref[i + 1], ref[i + 2], ref[i + 3]);
	for(i = 0; i < quad_length + tri_length; i += 3)
		printf("ref[%u] %u %u %u\n", i, ref[i + 0], ref[i + 1], ref[i + 2]);*/

	while(clear < quad_length + tri_length)
	{
		for(i = 0; i < quad_length && clear < quad_length + tri_length; i++)
		{
			counter++;
			cor = v[ref[i]];
			if(cor != -1 && ref[cor] == 12)
				ref[cor] = 12;
			if(cor == -1)
			{
				if(n[i] == -1 || n[(i / 4) * 4 + (i + 3) % 4] == -1)
					v[ref[i]] = i;
		//		else
		//			printf("jump!");
			}
			else if(cor == i)
				v[ref[i]] = -1;
			else
			{
				if(cor >= quad_length)
				{	/* other poly is a tri */
					a = (i / 4) * 4;
					b = quad_length + ((cor - quad_length) / 3) * 3;
					if((n[cor] == -1 && n[a + (i + 3) % 4] == -1) && ref[a + (i + 3) % 4] == ref[b + (cor - b + 1) % 3])
					{
						n[a + (i + 3) % 4] = cor;
						n[cor] = a + (i + 3) % 4;
//						printf("i = %u clear = %u\n", i, clear); 
						clear = 0;
						if(n[b + (cor - b + 2) % 3] != -1)
						{
							if(n[i] == -1)
								v[ref[i]] = i;
							else
								v[ref[i]] = -1;
						}
					}
					if((n[i] == -1 && n[b + (cor - b + 2) % 3] == -1) && ref[a + (i + 1) % 4] == ref[b + (cor - b + 2) % 3])
					{
						n[i] = b + (cor - b + 2) % 3;						
						n[b + (cor - b + 2) % 3] = i;
//						printf("i = %u clear = %u\n", i, clear); 
						clear = 0;
						if(n[cor] != -1)
						{
							if(n[a + (i + 3) % 4] == -1)
								v[ref[i]] = i;
							else
								v[ref[i]] = -1;
						}
//						v[ref[i]] = -1;
					}
				}else
				{	
					/* other poly is a quad */
					a = (i / 4) * 4;
					b = (cor / 4) * 4;
					if((n[cor] == -1 && n[a + (i + 3) % 4] == -1) && ref[a + (i + 3) % 4] == ref[b + (cor + 1) % 4])
					{
						n[a + (i + 3) % 4] = cor;
						n[cor] = a + (i + 3) % 4;
						clear = 0;	

						if(n[b + (cor + 3) % 4] != -1)
						{
							if(n[i] == -1)
								v[ref[i]] = i;
							else
								v[ref[i]] = -1;
						}
					}
					if((n[i] == -1 && n[b + (cor + 3) % 4] == -1) && ref[a + (i + 1) % 4] == ref[b + (cor + 3) % 4])
					{

						n[i] = b + (cor + 3) % 4;
						n[b + (cor + 3) % 4] = i;
//							printf("i = %u clear = %u\n", i, clear); 
						clear = 0;	

						if(n[cor] != -1)
						{
							if(n[a + (i + 3) % 4] == -1)
								v[ref[i]] = i;
							else
								v[ref[i]] = -1;
						}
					}
				}						
			}
			clear++;
		}
		for(; i < quad_length + tri_length && clear < quad_length + tri_length; i++)
		{

			cor = v[ref[i]];
			if(cor != -1 && ref[cor] == 12)
				ref[cor] = 12;
			if(cor == -1)
			{
				if(n[i] == -1 || n[quad_length + ((i - quad_length) / 3) * 3 + (i + 2) % 3] == -1)
					v[ref[i]] = i;
			}
			else if(cor == i)
			{
				v[ref[i]] = -1;
			}
			else 
			{
				if(cor >= quad_length)
				{	/* other poly is a tri */
					a = quad_length + ((i - quad_length) / 3) * 3;
					b = quad_length + ((cor - quad_length) / 3) * 3;
					if((n[cor] == -1 && n[a + (i - a + 2) % 3] == -1) && ref[a + (i - a + 2) % 3] == ref[b + (cor - b + 1) % 3])
					{
						n[a + (i - a + 2) % 3] = cor;
						n[cor] = a + (i - a + 2) % 3;
	//					printf("i = %u clear = %u\n", i, clear); 
						clear = 0;
						if(n[b + (cor - b + 2) % 3] != -1)
						{
							if(n[i] == -1)
								v[ref[i]] = i;
							else
								v[ref[i]] = -1;
						}
//						v[ref[i]] = -1;
					}
					if((n[i] == -1 && n[b + (cor - b + 2) % 3] == -1) && ref[a + (i - a + 1) % 3] == ref[b + (cor - b + 2) % 3])
					{
						n[i] = b + (cor - b + 2) % 3;						
						n[b + (cor - b + 2) % 3] = i;
//							printf("i = %u clear = %u\n", i, clear); 
						clear = 0;

						if(n[cor] != -1)
						{
							if(n[a + (i - a + 2) % 3] == -1)
								v[ref[i]] = i;
							else
								v[ref[i]] = -1;
						}
//						v[ref[i]] = -1;
					}
				}else
				{
					/* other poly is a quad */
					a = quad_length + ((i - quad_length) / 3) * 3;
					b = (cor / 4) * 4;
					if((n[cor] == -1 && n[a + (i - a + 2) % 3] == -1) && ref[a + (i - a + 2) % 3] == ref[b + (cor + 1) % 4])
					{

						n[a + (i - a + 2) % 3] = cor;
						n[cor] = a + (i - a + 2) % 3;
//							printf("i = %u clear = %u\n", i, clear); 
						clear = 0;

						if(n[b + (cor + 3) % 4] != -1)
						{
							if(n[i] == -1)
								v[ref[i]] = i;
							else
								v[ref[i]] = -1;
						}
//						v[ref[i]] = -1;
					}
					if((n[i] == -1 && n[b + (cor + 3) % 4] == -1) && ref[a + (i - a + 1) % 3] == ref[b + (cor + 3) % 4])
					{
						n[i] = b + (cor + 3) % 4;
						n[b + (cor + 3) % 4] = i;				
//							printf("i = %u clear = %u\n", i, clear); 						
						clear = 0;
						if(n[cor] != -1)
						{
							if(n[a + (i - a + 2) % 3] == -1)
								v[ref[i]] = i;
							else
								v[ref[i]] = -1;
						}
//						v[ref[i]] = -1;
					}
				}						
			}
			counter++;
			clear++;
		}
		laps++;
		
	}
	counter = 0;
	free(v);

/*
	for(i = 0; i < quad_length; i += 4)
		printf("n[%u] %u %u %u %u\n", i, n[i + 0], n[i + 1], n[i + 2], n[i + 3]);
	for(i = 0; i < quad_length + tri_length; i += 3)
		printf("n[%u] %u %u %u\n", i, n[i + 0], n[i + 1], n[i + 2]);*/
	return n;
}



uint *f_sort_tri_neighbor(uint *ref, uint tri_length, uint vertex_count)
{
	uint i, cor, clear = 0, *n, *v, a, b;
	uint counter = 0, laps = 0;
	n = malloc((sizeof *n) * tri_length);
	for(i = 0; i < tri_length; i++)
		n[i] = -1;
	v = malloc((sizeof *v) * vertex_count);
	for(i = 0; i < vertex_count; i++)
		v[i] = -1;
	while(clear < tri_length)
	{
		for(i = 0; i < tri_length && clear < tri_length; i++)
		{
			counter++;
			clear++;
			cor = v[ref[i]];
			if(cor == -1)
			 {
				 if(n[i] == -1 || n[(i / 3) * 3 + (i + 2) % 3] == -1)
					v[ref[i]] = i;
			}else if(cor == i)
				v[ref[i]] = -1;
			else 
			{
				a = (i / 3) * 3;
				b = (cor / 3) * 3;
				if((n[cor] == -1 && n[a + (i + 2) % 3] == -1) && ref[a + (i + 2) % 3] == ref[b + (cor + 1) % 3])
				{
					n[a + (i + 2) % 3] = cor;
					n[cor] = a + (i + 2) % 3;
					clear = 0;
					if(n[b + (cor + 2) % 3] != -1)
					{
						if(n[i] == -1)
							v[ref[i]] = i;
						else
							v[ref[i]] = -1;
					}
				}
				if((n[i] == -1 && n[b + (cor + 2) % 3] == -1) && ref[a + (i + 1) % 3] == ref[b + (cor + 2) % 3])
				{
					n[i] = b + (cor + 2) % 3;						
					n[b + (cor + 2) % 3] = i;
					clear = 0;
					if(n[cor] != -1)
					{
						if(n[a + (i + 2) % 3] == -1)
							v[ref[i]] = i;
						else
							v[ref[i]] = -1;
					}
				}
			}
		}
		laps++;
	}
	counter = 0;
	free(v);
	return n;
}
