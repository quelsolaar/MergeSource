#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "forge.h"
#include "r_include.h"

char *r_qualifyer_names[R_SIQ_COUNT] = {"R_SIQ_UNIFORM",
										"R_SIQ_TEXTURE",
										"R_SIQ_ATTRIBUTE",
										"R_SIQ_INOUT",
										"R_SIQ_BUFFER",
										"R_SIQ_BLOCK",
										"R_SIQ_BLOCK_WITH_TEXTURE"};

void r_shader_parse_input_print(RShaderInput *input, uint count)
{
	uint i, j;
	for(i = 0; i < count; i++)
	{
		printf("------------------------ input %u ---------\n", i);
		printf("\tname = %s\n", input[i].name);
		if(input[i].type < R_SIT_COUNT && input[i].type >= 0)
			printf("\ttype = %s\n", r_type_names[input[i].type]);
		else
			printf("\ttype = %u\n", input[i].type);
		printf("\tqualifyer = %s\n", r_qualifyer_names[input[i].qualifyer]);
		printf("\tid = %u\n", input[i].id);
		printf("\toffset = %u\n", input[i].offset);
		printf("\tsize = %u\n", input[i].size);
		printf("\tarray_length = %u\n", input[i].array_length);
		printf("\tblock = %u\n", input[i].block);
		printf("\tobject = %u\n", input[i].object);
		printf("\tstages = {%u", (uint)input[i].stages[0]);
		for(j = 1; j < R_MAX_SHADER_STAGES; j++)
			printf(", %u", (uint)input[i].stages[j]);
		printf("}\n\tupdated = %u\n", (uint)input[i].updated);
	}
}

uint r_shader_parse_type(RShaderInput *input, char *source, uint start, uint end)
{
	uint i = 0, j, k;
	for(j = start; j < end; j++)
	{
		for(k = 0; source[i + k] != 0 && source[i + k] == r_type_names[j][k]; k++);
		if(r_type_names[j][k] == 0)
		{
			i += k;
			for(++i; source[i] <= 32 && source[i] != 0 && source[i] != ';'; i++);
			input->type = j; 
			for(j = 0; source[i + j] > 32 && source[i + j] != 0 && source[i + j] != ';' && source[i + j] != '[' && j < 32 ; j++)
				input->name[j] = source[i + j];
			input->name[j] = 0;
			input->array_length = 0;
			if(source[i + j] == '[')
			{
				j++;
				if(source[i + j] >= 48 &&  source[i + j] < 58)
				{
					input->array_length = source[i + j] - 48;
					for(j++;source[i + j] >= 48 &&  source[i + j] < 58; j++)
					{
						input->array_length *= 10;
						input->array_length += source[i + j] - 48;
					}
				}else
					input->array_length = -1;
			}	
			return  i + j;
		}
	}
	return 0;
}

boolean r_shader_parse_add_test(RShaderInput *input, uint count, uint stage)
{
	uint i, j;
	for(i = 0; i < count; i++)
	{
		if(input[i].qualifyer == input[count].qualifyer &&
			input[i].type == input[count].type)
		{
			for(j = 0; input[i].name[j] == input[count].name[j] && input[i].name[j] != 0; j++);
			if(input[i].name[j] == input[count].name[j])
			{
				input[i].stages[stage] = TRUE;
				return FALSE;
			}
		}
	}
	return TRUE;
}


void r_shader_parse(RShaderInput *input, uint *count, char *source, char *key, uint start, uint end, RShaderInputQualifyer qualifyer, uint stage)
{
	uint i, j, k, found, block = 0, brakets = 0;
	boolean begin = TRUE;
	for(i = 0; source[i] != 0; i++)
	{
		if(source[i] == '(' || source[i] == '{')
			brakets++;
		if(source[i] == ')' || source[i] == '}')
			brakets--;

		if(begin && brakets == 0)
		{
			for(j = 0; source[i + j] != 0 && source[i + j] == key[j]; j++);
			if(key[j] == 0 && source[i + j] <= 32)
			{
				i += j;
				while(source[++i] <= 32);
			
				for(j = i; source[j] != 0 && source[j] != '{' && source[j] != ';' ; j++);
				if(source[j] == '{')
				{
					for(k = 0; source[i + k] > 32 && source[i + k] != '{'; k++)
						input[*count].name[k] = source[i + k];
					input[*count].name[k] = 0;
					input[*count].block = block;
					input[*count].qualifyer = R_SIQ_BLOCK;
					input[*count].array_length = 1;
					if(r_shader_parse_add_test(input, *count, stage))
					{
						for(k = 0; k < R_MAX_SHADER_STAGES; k++)
							input[*count].stages[k] = FALSE;
						input[*count].stages[stage] = TRUE;
						(*count)++;
					}
					j++;
					while(source[j] != '}')
					{
						found = r_shader_parse_type(&input[*count], &source[j], start, end);
						if(found != 0)
						{
							input[*count].block = block;
							input[*count].qualifyer = qualifyer;	
							j += found;
							if(r_shader_parse_add_test(input, *count, stage))
							{
								for(k = 0; k < R_MAX_SHADER_STAGES; k++)
									input[*count].stages[k] = FALSE;
								input[*count].stages[stage] = TRUE;
								(*count)++;
							}
						}else
							j++;
					}
					block++;
					i = j;
				}


				found = r_shader_parse_type(&input[*count], &source[i], start, end);
				if(found != 0)
				{
					input[*count].block = -1;
					input[*count].qualifyer = qualifyer;
					i += found;
					if(r_shader_parse_add_test(input, *count, stage))
					{
						for(k = 0; k < R_MAX_SHADER_STAGES; k++)
							input[*count].stages[k] = FALSE;
						input[*count].stages[stage] = TRUE;
						(*count)++;
					}
				}
			}
		}
		begin = source[i] != '_' && (source[i] < '0' || source[i] > 'z' || (source[i] > 'Z' && source[i] < 'a') || (source[i] > '9' && source[i] < 'A'));
	}
}

uint r_shader_parse_test(char *shader, char *keyword)
{
	uint i;
	int brackets = 0;
	for(i = 0; shader[i] == keyword[i] && keyword[i] != 0; i++);
	if(keyword[i] == 0)
	{
		for(; shader[i] != 0 && (shader[i] != ';' || brackets != 0); i++)
		{
			if(shader[i] == '{')
				brackets++;
			if(shader[i] == '}')
				brackets--;
		}
		if(shader[i] == ';')
			i++;
		if(shader[i] == '\n')
			i++;
		return i;
	}
	return 0;
}

//gl_DrawID
//gl_InstanceID
void r_shader_parse_clean(char *shader, char *output, RShaderInput *input, uint input_count,  uint instance_id, boolean fragment_add, boolean pass_to_fragment)
{
	uint i, j = 0, k, l, out = 0;
	char *add = "InstanceData.b[gl_InstanceID + gl_BaseInstanceARB].";  
	if(fragment_add && instance_id != -1)
	{
		char *in = "in flat int relinquish_instance_id;\n\n";
		for(l = 0; in[l] != 0; l++)
			output[j++] = in[l];
		add = "InstanceData.b[relinquish_instance_id].";
	}
	if(pass_to_fragment && instance_id != -1)
	{
		char *in = "out flat int relinquish_instance_id;\n\n";
		for(l = 0; in[l] != 0; l++)
			output[j++] = in[l];
	}
	for(i = 0; shader[i] != 0;)
	{
		if(i == 0 || (shader[i - 1] != '_' && (shader[i - 1] < '0' || shader[i - 1] > 'z' || (shader[i - 1] > 'Z' && shader[i - 1] < 'a') || (shader[i - 1] > '9' && shader[i - 1] < 'A'))))
		{
			out = r_shader_parse_test(&shader[i], "uniform");
			if(out == 0)
				out = r_shader_parse_test(&shader[i], "varying");
			if(out == 0)
				out = r_shader_parse_test(&shader[i], "attribute");
			if(out == 0 && pass_to_fragment && instance_id != -1)
			{
				char *main = "main", *move = "\n\trelinquish_instance_id = gl_InstanceID + gl_BaseInstanceARB;\n";
				for(k = 0; main[k] == shader[i + k] && main[k] != 0; k++);
				if(main[k] == 0)
				{
					while(shader[i] != 0 && shader[i] != '{')
						output[j++] = shader[i++];
					if(shader[i] == '{')
					{
						output[j++] = shader[i++];
						for(l = 0; move[l] != 0; l++)
							output[j++] = move[l];
					}
				}
			}

			if(out == 0)
			{
				for(k = l = 0; k < input_count; k++)
				{
					if(input[k].block == instance_id && input[k].qualifyer == R_SIQ_UNIFORM)
					{
						for(l = 0; input[k].name[l] == shader[i + l] && input[k].name[l] != 0; l++);
						if(input[k].name[l] == 0 && shader[i + l] != '_' && (shader[i + l] < '0' || shader[i + l] > 'z' || (shader[i + l] > 'Z' && shader[i + l] < 'a') || (shader[i + l] > '9' && shader[i + l] < 'A')))
						{
							for(l = 0; add[l] != 0; l++)
								output[j++] = add[l];
							for(l = 0; input[k].name[l] != 0; l++)
							{
								output[j++] = input[k].name[l];
								i++;
							}
							break;
						}else
							l = 0;
					}
				}
				if(l == 0)
					output[j++] = shader[i++];
			}else
				i += out;
		}else
			output[j++] = shader[i++];

	}
	output[j] = 0;
}



void r_shader_matrix_parse(RShader	*shader)
{
	char *name, *types[] = {"NormalMatrix", "ModelViewMatrix", "ProjectionMatrix", "ModelViewProjectionMatrix"};
	uint i, j;
	shader->normal_matrix = -1;
	shader->model_view_matrix = -1;
	shader->projection_matrix = -1;
	shader->model_view_projection_matrix = -1;
	for(i = 0; i < shader->uniform_count; i++)
	{
		if(shader->uniforms[i].type == R_SIT_MAT4)
		{
			name = shader->uniforms[i].name;
			for(j = 0; name[j] == types[0][j] && name[j] != 0; j++);
			if(name[j] == types[0][j])
				shader->normal_matrix = i;
			for(j = 0; name[j] == types[1][j] && name[j] != 0; j++);
			if(name[j] == types[1][j])
				shader->model_view_matrix = i;
			for(j = 0; name[j] == types[2][j] && name[j] != 0; j++);
			if(name[j] == types[2][j])
				shader->projection_matrix = i;
			for(j = 0; name[j] == types[3][j] && name[j] != 0; j++);
			if(name[j] == types[3][j])
				shader->model_view_projection_matrix = i;
		}
	}
}


/*
structure
---------

Differences:

Add instance keyword!
-Auto version.

varying -> in / out;
uniforms -> blocks;
Blocks -> uniforms;

Blocks -> buffers;


Old -> New;

Turn all uniforms in to blocks;
if(buffers)
	Turn all blocks on to buffer

New -> Old;
Turn all blocks in to uniforms

New -> New;
if(buffers)
	Turn all blocks on to buffer

Old -> Old;
do nothing
*/


char *r_shader_parser_test_vertex = 
"uniform vec2 size[342];\n"
"uniform MatrixBlock\n"
"{\n"
"  mat4 projection;\n"
"  mat4 modelview;\n"
"} matrices[3];\n"
"attribute vec4 vertex;\n"
"attribute vec4 alpha;\n"
"uniform OtherBlock\n"
"{\n"
"uniform vec4 color;\n"
"uniform vec3 pos;\n"
"};\n"
"uniform vec2 pixel;\n"
"uniform mat4 NormalMatrix;\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"varying vec4 col;\n"
"varying vec3 normal;\n"
"varying vec4 v;\n"
"void main()\n"
"{\n"
"	vec3 expand;\n"
"	vec3 vec;\n"
"	col = color * alpha;\n"
"	normal = normalize((NormalMatrix * vec4(0.0, 0.0, -1.0, 0.0)).xyz);\n"
"	vec = pos + vec3(vertex.xy * size, 0.0);\n"
"	expand = vec3((ModelViewProjectionMatrix * vec4(vec, 1.0)).zz * vertex.ba * pixel, 0);\n"
"	gl_Position = v = ModelViewProjectionMatrix * vec4(vec + expand, 1.0) + vec4(expand, 0);\n"
"}\n";

uint shader_parse_input_print(char *output, uint output_size, RShaderInput *input, uint input_count, RShaderInputMode mode, uint index_block, uint stage, uint version, uint unit)
{
	char *qualifyer_names[R_SIQ_COUNT] = {"uniform", "uniform", "attribute", "varying", "buffer"}, *q;
	char buffer[256];
	uint i, j, k, block_count = 0, pos = 0;
	boolean textures;
	output_size--;
	for(i = 0; i < input_count; i++)
		if(input[i].block > block_count && input[i].qualifyer == R_SIQ_UNIFORM)
			block_count = input[i].block;
	block_count++;
	switch(mode)
	{
		case R_SIM_FLAT :
			for(i = 0; i < input_count; i++)
			{
				if(input[i].qualifyer == R_SIQ_UNIFORM && input[i].stages[stage])
				{
					if(input[i].array_length == 0)
						sprintf(buffer, "uniform %s %s;\n",  r_type_names[input[i].type], input[i].name);
					else
						sprintf(buffer, "uniform %s %s[%u];\n",  r_type_names[input[i].type], input[i].name, input[i].array_length);
					for(j = 0; buffer[j] != 0 && pos < output_size; j++)
						output[pos++] = buffer[j];
				}
			}
		break;
		case R_SIM_UNIFORM_BLOCK :
		case R_SIM_BUFFER_OBJECT :
			for(k = 0; k < block_count; k++)
			{
				textures = FALSE;
				if(r_glGetTextureHandleARB == NULL)
					for(i = 0; i < input_count; i++)
						if(input[i].qualifyer == R_SIQ_UNIFORM && input[i].block == k && input[i].type >= R_TYPE_TEXTURE_START)
							textures = TRUE;
				if(textures)
				{
					for(i = 0; i < input_count; i++)
					{
						if(input[i].qualifyer == R_SIQ_UNIFORM && input[i].block == k && input[i].stages[stage])
						{
							if(input[i].array_length == 0)
								sprintf(buffer, "uniform %s %s %u;\n",  r_type_names[input[i].type], input[i].name, input[i].block);
							else
								sprintf(buffer, "uniform %s %s[%u];\n",  r_type_names[input[i].type], input[i].name, input[i].array_length);
							for(j = 0; buffer[j] != 0 && pos < output_size; j++)
								output[pos++] = buffer[j];
						}
					}

				}else
				{
					for(i = 0; i < input_count; i++)
						if(input[i].qualifyer == R_SIQ_BLOCK && input[i].block == k)
							break;
					if(input[i].stages[stage])
					{
						if(index_block != k)
						{
							for(j = 0; (input[j].qualifyer != R_SIQ_BLOCK || input[j].block != k ) && j < input_count; j++);
							if(j < input_count)
								sprintf(buffer, "layout (std140) uniform %s{\n", input[j].name);
							else
								sprintf(buffer, "layout (std140) uniform block%u{\n", k);
							for(j = 0; buffer[j] != 0 && pos < output_size; j++)
								output[pos++] = buffer[j];	
						}else
						{
							sprintf(buffer, "struct InstanceStruct{\n");
							for(j = 0; buffer[j] != 0 && pos < output_size; j++)
								output[pos++] = buffer[j];	
						}
						for(i = 0; i < input_count; i++)
						{
							if(input[i].block == k && input[i].qualifyer == R_SIQ_UNIFORM)
							{
								if(input[i].array_length == 0)
									sprintf(buffer, "\t%s %s '%u;\n",  r_type_names[input[i].type], input[i].name, input[i].block);
								else
									sprintf(buffer, "\t%s %s[%u];\n",  r_type_names[input[i].type], input[i].name, input[i].array_length);
								for(j = 0; buffer[j] != 0 && pos < output_size; j++)
									output[pos++] = buffer[j];
							}
						}
						if(index_block != k)
						{
							sprintf(buffer, "};\n\n");
							for(j = 0; buffer[j] != 0 && pos < output_size; j++)
								output[pos++] = buffer[j];	
						}else
						{
							if(mode ==  R_SIM_UNIFORM_BLOCK)
							{
								char uniform_footer[128];
								for(i = 0; i < input_count; i++)
									if(input[i].qualifyer == R_SIQ_BLOCK && input[i].block == k)
										break;
								sprintf(uniform_footer,	"};\n\n"
														"layout (std140) uniform %s {\n"
														"\tInstanceStruct b[%u];\n"
														"}InstanceData;\n\n", input[i].name, input[i].array_length);
								for(j = 0; uniform_footer[j] != 0 && pos < output_size; j++)
									output[pos++] = uniform_footer[j];	
							}else
							{
								char *buffer_footer =  "};\n\n"
														"layout (std140) buffer InstanceBlock {\n"
														"\tInstanceStruct b[];\n"
														"}InstanceData;\n\n";
								for(j = 0; buffer_footer[j] != 0 && pos < output_size; j++)
									output[pos++] = buffer_footer[j];	
							}
						}
					}
				}
			}
		break;
	}
    if(version >= 150)
        qualifyer_names[R_SIQ_ATTRIBUTE] = "in";
        
	for(k = R_SIQ_ATTRIBUTE; k < R_SIQ_BLOCK; k++)
	{
		for(i = 0; i < input_count; i++)
		{
			if(input[i].qualifyer == k && input[i].stages[stage])
			{
				char *in = "in";
				if(input[i].qualifyer == R_SIQ_INOUT)
				{
                    if(version <= 120)
						q = "varying";
                    else if(stage != 0 && input[i].stages[stage - 1] && input[i].stages[stage + 1])
						q = "in out";
                    else if(stage == 0 || input[i].stages[stage + 1])
						q = "out";
					else
						q = in;
				}else
					q = qualifyer_names[k];
					
				if(unit == GL_GEOMETRY_SHADER_ARB && q == in)
				{
					if(input[i].array_length == 0)
						sprintf(buffer, "%s %s %s[];\n", q, r_type_names[input[i].type], input[i].name);
					else if(input[i].array_length == -1)
						sprintf(buffer, "%s %s %s[][];\n", q, r_type_names[input[i].type], input[i].name);
					else
						sprintf(buffer, "%s %s %s[][%u];\n", q, r_type_names[input[i].type], input[i].name, input[i].array_length);
				}else
				{
					if(input[i].array_length == 0)
						sprintf(buffer, "%s %s %s;\n", q, r_type_names[input[i].type], input[i].name);
					else if(input[i].array_length == -1)
						sprintf(buffer, "%s %s %s[];\n", q, r_type_names[input[i].type], input[i].name);
					else
						sprintf(buffer, "%s %s %s[%u];\n", q, r_type_names[input[i].type], input[i].name, input[i].array_length);
				}
				for(j = 0; buffer[j] != 0 && pos < output_size; j++)
					output[pos++] = buffer[j];
			}
		}
	}
	output[pos] = 0;
	return pos;
}

uint shader_parse_instance_id_get(RShaderInput *input, uint input_count, RShaderInputMode mode, char *instance_name)
{
	uint i, j;
	if(mode == R_SIM_FLAT || instance_name == 0)
		return 0;
	for(i = 0; i < input_count; i++)
	{
		if(input[i].qualifyer == R_SIQ_BLOCK)
		{
			for(j = 0; instance_name[j] == input[i].name[j] && instance_name[j] != 0; j++);
			if(instance_name[j] == input[i].name[j])
				return input[i].block;
		}
	}
	return 0;
}

uint r_parse_shaders(char **output, char **shader, uint *stages, uint shader_count, RShaderInput *input, RShaderInputMode mode, uint version, boolean embeded, char *instance_name, uint *instance_id)
{
	uint i, j, k, pos = 0, count = 0, sum = 0, max_uniform = 512 * 4, offset, instance_array;
	char *instrance_name = NULL;
	for(i = 0; i < shader_count; i++)
	{	
		r_shader_parse(input, &count, shader[i], "attribute", 0, R_SIT_VDOUBLE4 + 1, R_SIQ_ATTRIBUTE, i);
		r_shader_parse(input, &count, shader[i], "uniform", 0, R_SIT_COUNT, R_SIQ_UNIFORM, i);
		r_shader_parse(input, &count, shader[i], "varying", 0, R_SIT_DMAT4 + 1, R_SIQ_INOUT, i);
		r_shader_parse(input, &count, shader[i], "in", 0, R_SIT_DMAT4 + 1, R_SIQ_INOUT, i);
		r_shader_parse(input, &count, shader[i], "out", 0, R_SIT_DMAT4 + 1, R_SIQ_INOUT, i);
	}
	

	for(i = 0; i < count; i++)
	{
		if(input[i].qualifyer != R_SIQ_BLOCK)
		{
			if(input[i].array_length == 0)
				input[i].size = r_type_strides[input[i].type];
			else
				input[i].size = r_type_strides[input[i].type] * input[i].array_length;
		}
	}

	/* if there are uniform outside a block create one*/
	for(i = 0; i < count && (input[i].qualifyer != R_SIQ_UNIFORM || input[i].block != -1); i++);
	if(i < count)
	{
		for(i = 0; i < count && input[i].qualifyer != R_SIQ_BLOCK; i++);
		if(i == count)
		{
			char *InstanceData = "InstanceBlock";
			for(i = 0; InstanceData[i] != 0 && i < 31; i++)
				input[count].name[i] = InstanceData[i];
		}else
		{
			char *stray_block = "strayblock";
			for(i = 0; stray_block[i] != 0 && i < 31; i++)
				input[count].name[i] = stray_block[i];		
		}
		input[count].name[i] = 0;
		input[count].type = 0;
		input[count].qualifyer = R_SIQ_BLOCK;
		input[count].id = 0;
		input[count].offset = 0;
		input[count].array_length = 1;
		for(i = j = 0; i < count; i++)
			if(input[i].qualifyer == R_SIQ_BLOCK)
				j++;
		input[count].block = j;
		for(i = 0; i < count; i++)
			if(input[i].qualifyer == R_SIQ_UNIFORM && input[i].block == -1)
				input[i].block = input[count].block;
		count++;
	}
	for(i = 0; i < count; i++)
	{
		if(input[i].qualifyer == R_SIQ_BLOCK)
		{
			input[i].size = 0;
			for(j = 0; j < R_MAX_SHADER_STAGES; j++)
				input[i].stages[j] = FALSE;

			for(j = 0; j < count; j++)
			{
				if(input[i].block == input[j].block && input[j].qualifyer == R_SIQ_UNIFORM && i != j)
				{
					for(k = 0; k < R_MAX_SHADER_STAGES; k++)
						if(input[j].stages[k])
							input[i].stages[k] = TRUE;
					while(input[i].size % input[j].size != 0)
						input[i].size++;
					input[j].offset = input[i].size;
					input[i].size += input[j].size;
				}
			}
			while(input[i].size % 16 != 0)
				input[i].size++;
			sum += input[i].size;
		}
	}

	*instance_id = shader_parse_instance_id_get(input, count,  mode, instance_name);
	offset = 0;
	for(i = 0; i < count; i++)
	{
		if(input[i].qualifyer == R_SIQ_BLOCK)
		{
			input[i].offset = offset;
			if(input[i].array_length == 0)
				offset += input[i].size;
			else
				offset += input[i].array_length * input[i].size;

			if(input[i].block == *instance_id && mode != R_SIM_FLAT)
			{
				if(mode == R_SIM_BUFFER_OBJECT)
				{
					input[i].array_length = 1024 * 1024 / input[i].size;
					if(input[i].array_length < 1)
						input[i].array_length = 1;
				}else
					input[i].array_length = (max_uniform - sum + input[i].size) / input[i].size;
			}
		/*	k = 0;
			for(j = 0; j < count; j++)
			{
				if(input[i].block == input[j].block && input[j].qualifyer == R_SIQ_UNIFORM && i != j)
				{
					input[j].offset = k;
					k += input[j].size;
				}
			}*/
		}
	}


	for(i = 0; i < count; i++)
		if(input[i].qualifyer == R_SIQ_BLOCK && input[i].block == *instance_id)
			instrance_name = input[i].name;

	if(mode == R_SIM_FLAT)
		instance_array = -1;
	else
	{
		instance_array = *instance_id;
		if(r_glGetTextureHandleARB == NULL)
			for(i = 0; i < count; i++)
				if(input[i].block == instance_array && input[i].qualifyer == R_SIQ_UNIFORM && input[i].type >= R_TYPE_TEXTURE_START)
					instance_array = -1;
	}
	for(i = 0; i < shader_count; i++)
	{	
		output[i] = malloc(1024 * 1024);
		if(version != 0)
		{
			if(r_glGetTextureHandleARB == NULL)
				sprintf(output[i], "#version %u\n", version);
			else
				sprintf(output[i], "#version %u\n#extension GL_ARB_bindless_texture : enable\n", version);
			if(mode != R_SIM_FLAT)
				sprintf(output[i], "#version %u\n#extension GL_ARB_shader_draw_parameters : enable\n", version);
			for(pos = 0; output[i][pos] != 0; pos++);
            if(version > 150)
            {
                if(stages[i] == GL_FRAGMENT_SHADER_ARB)
                    sprintf(&output[i][pos], "#define texture2D texture\n#define gl_FragColor betray_FragColor\nlayout(location = 0) out vec4 betray_FragColor;\n");
                else
                    sprintf(&output[i][pos], "#define texture2D texture\n");
                for(; output[i][pos] != 0; pos++);
            }
		}
		pos += shader_parse_input_print(&output[i][pos], 1024 * 1024, input, count, mode, *instance_id, i, version, stages[i]);
		r_shader_parse_clean(shader[i], &output[i][pos], input, count, instance_array, stages[i] == GL_FRAGMENT_SHADER_ARB, i + 1 < shader_count && stages[i + 1] == GL_FRAGMENT_SHADER_ARB);
	}


	return count;
}


char *test_shader_vertex_color_vertex = 
"attribute vec3 vertex;\n"
"attribute vec4 color;\n"
"varying vec4 col;\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"void main()\n"
"{\n"
"	col = color;\n"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex, 1.0);\n"
"}\n";

char *test_shader_vertex_color_fragment = 
"varying vec4 col;\n"
"void main()\n"
"{\n"
"	gl_FragColor = col;\n"
"}\n";

