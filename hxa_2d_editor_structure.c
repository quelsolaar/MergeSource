#pragma warning(error:6201)
#include <math.h>
#include <stdlib.h>
#include <stdio.h> 
#include "betray.h"
#include "imagine.h"
#include "seduce.h"
#include "hxa.h"
#include "hxa_utils.h"
#include "hxa_2d_editor_internal.h"


#pragma warning(error:6201)

void hxa_2d_editor_structure_add_loop(HxA2DEditorInstance *level, uint32 material, double x, double y, double size, uint allocate)
{
	HxA2DEditorLoop *l;
	if(level->loop_count == level->loop_allocated)
	{
		level->loop_allocated += 16;
		level->loops = realloc(level->loops, (sizeof *level->loops) * level->loop_allocated * 2);
	}
	level->loop_selected = level->loop_count;
	l = &level->loops[level->loop_count++];
	if(allocate < 4)
		allocate = 4;
	l->loop_allocated = allocate;
	l->loop = malloc((sizeof *l->loop) * l->loop_allocated * 2);
	l->selection = malloc((sizeof *l->selection) * l->loop_allocated);
	l->selection[0] = FALSE;
	l->selection[1] = FALSE;
	l->selection[2] = FALSE;
	l->selection[3] = FALSE;
	l->loop[0] = size + x;
	l->loop[1] = size + y;
	l->loop[2] = size + x;
	l->loop[3] = -size + y;
	l->loop[4] = -size + x;
	l->loop[5] = -size + y;
	l->loop[6] = -size + x;
	l->loop[7] = size + y;
	l->loop_size = 4;
	l->material = material;
	l->matrix[0] = 1.0;
	l->matrix[1] = 0.0;
	l->matrix[2] = 0.0;
	l->matrix[3] = 0.0;
	l->matrix[4] = 0.0;
	l->matrix[5] = 1.0;
	l->matrix[6] = 0.0;
	l->matrix[7] = 0.0;
	l->matrix[8] = 0.0;
	l->matrix[9] = 0.0;
	l->matrix[10] = 1.0;
	l->matrix[11] = 0.0;
	l->matrix[12] = 0.0;
	l->matrix[13] = 0.0;
	l->matrix[14] = 0.0;
	l->matrix[15] = 1.0;
	l->rotate = 0.5;
	l->scale = 0.5;
	l->triangle_array = NULL;
	l->pool = NULL;
}

void hxa_2d_editor_structure_entity_add(HxA2DEditorInstance *level, float x, float y, HXANode *node, char *node_name)
{
	uint i = 0;
	if(level->entity_count == level->entity_allocated)
	{
		level->entity_allocated += 16;
		level->entity = realloc(level->entity, (sizeof *level->entity) * level->entity_allocated);
	}
	level->entity[level->entity_count].active = TRUE;
	level->entity[level->entity_count].selected = FALSE;
	level->entity[level->entity_count].loop_id = 0;
	level->entity[level->entity_count].pos[0] = x;
	level->entity[level->entity_count].pos[1] = y;
	if(node_name != NULL)
		for(i = 0; node_name[i] != '\0' && i < 32 - 1; i++)
			level->entity[level->entity_count].name[i] = node_name[i];
	level->entity[level->entity_count].name[i] = '\0';	
	hxa_util_node_clone_content(&level->entity[level->entity_count++].node, node);
}


void hxa_2d_editor_material_replace(HxA2DEditorShape *edit, uint32 material, uint32 replacement)
{
	uint i, j; 
	edit->updated = TRUE;
	for(i = 0; i < edit->instance_count; i++)
		for(j = 0; j < edit->instances[i].loop_count; j++)
			if(edit->instances[i].loops[j].material == material)
				edit->instances[i].loops[j].material = replacement;
}


uint hxa_2d_editor_entity_count(HxA2DEditorShape *edit)
{
	return edit->instances[edit->instance_current].entity_count;
}

HXANode *hxa_2d_editor_entity_get(HxA2DEditorShape *edit, uint id, float *pos, boolean *selected)
{
	if(selected != NULL)
		*selected = edit->instances[edit->instance_current].entity[id].selected;
	if(pos != NULL)
	{
		pos[0] = edit->instances[edit->instance_current].entity[id].pos[0];
		pos[1] = edit->instances[edit->instance_current].entity[id].pos[1];
	}
	return &edit->instances[edit->instance_current].entity[id].node;
}


void hxa_2d_editor_structure_remove_loop(HxA2DEditorInstance *level, uint loop)
{
	uint i;
	free(level->loops[loop].loop);
	if(level->loops[loop].triangle_array != NULL)
		free(level->loops[loop].triangle_array);
	if(level->loops[loop].pool != NULL)
		r_array_free(level->loops[loop].pool);
/*	for(i = 0; i < level->entity_count; i++)
	{
		if(level->entity[i].loop_id == loop)
			level->entity[i--] = level->entity[--level->entity_count];
		else if(level->entity[i].loop_id > loop)
			level->entity[i].loop_id--;
	}*/
	for(level->loop_count--; loop < level->loop_count; loop++)
		level->loops[loop] = level->loops[loop + 1];
}

void hxa_2d_editor_entity_delete_selected(HxA2DEditorShape *edit)
{
	uint i, j;
	HxA2DEditorInstance *instance;
	instance = hxa_2d_editor_structure_instance_add(edit);

	for(i = 0; i < instance->entity_count; i++)
	{
		if(instance->entity[i].selected)
		{
			hxa_util_free_node_content(&instance->entity[i].node);
			instance->entity[i--] = instance->entity[--instance->entity_count];
		}
	}
	for(i = 0; i < instance->loop_count; i++)
	{
		for(j = 0; j < instance->loops[i].loop_size && instance->loops[i].selection[j]; j++);
		if(j == instance->loops[i].loop_size)
		{
			hxa_2d_editor_structure_remove_loop(instance, i--);
		}
	}
}

void hxa_2d_editor_structure_instance_free(HxA2DEditorInstance *instance)
{
	uint i, j;
	if(instance->entity != NULL)
	{
		for(i = 0; i < instance->entity_count; i++)
			hxa_util_free_node_content(&instance->entity[i].node);
		free(instance->entity);
	}
	for(j = 0; j < instance->loop_count; j++)
	{
		free(instance->loops[j].loop);
		free(instance->loops[j].selection);
	}
	free(instance->loops);
}

void hxa_2d_editor_structure_instance_clone(HxA2DEditorInstance *l_old, HxA2DEditorInstance *l_new)
{
	uint i, j;
	*l_new = *l_old;

	if(l_new->value_allocated != 0)
	{
		l_new->values = malloc((sizeof *l_new->values) * l_new->value_allocated);
		for(i = 0; i < l_new->value_count; i++)
			l_new->values[i] = l_old->values[i];
	}else
		l_new->values = NULL;

	if(l_new->entity_allocated != 0)
	{
		l_new->entity = malloc((sizeof *l_new->entity) * l_new->entity_allocated);
		for(i = 0; i < l_new->entity_count; i++)
		{
			l_new->entity[i] = l_old->entity[i];
			hxa_util_node_clone_content(&l_new->entity[i].node, &l_old->entity[i].node);
		}
	}else
		l_new->entity = NULL;

	if(l_new->loop_allocated != 0)
	{
		l_new->loops = malloc((sizeof *l_new->loops) * l_new->loop_allocated);
		for(i = 0; i < l_new->loop_count; i++)
		{
			l_new->loops[i] = l_old->loops[i];
			if(l_new->loops[i].loop_allocated <= l_new->loops[i].loop_size) 
				l_new->loops[i].loop_allocated++;
			l_new->loops[i].loop = malloc((sizeof *l_new->loops[i].loop) * l_new->loops[i].loop_allocated * 2);
			l_new->loops[i].selection = malloc((sizeof *l_new->loops[i].selection) * l_new->loops[i].loop_allocated);
			for(j = 0; j < l_new->loops[i].loop_size * 2; j++)
				l_new->loops[i].loop[j] = l_old->loops[i].loop[j];
			for(j = 0; j < l_new->loops[i].loop_size; j++)
				l_new->loops[i].selection[j] = l_old->loops[i].selection[j];
			l_old->loops[i].triangle_array = NULL;
			l_old->loops[i].pool = NULL;
		}
	}else
		l_new->loops = NULL;
}

HxA2DEditorInstance *hxa_2d_editor_structure_instance_add(HxA2DEditorShape *edit)
{
	HxA2DEditorInstance *l_old, *l_new;
	uint i, j;
	edit->updated = TRUE;
	edit->instance_current++;
	if(edit->instance_current == edit->instance_count)
	{
		if(edit->instance_allocated == edit->instance_count)
		{
			edit->instance_allocated += 32;
			edit->instances = realloc(edit->instances, (sizeof *edit->instances) * edit->instance_allocated);
		}
		edit->instance_count++;
	}else
	{
		for(i = edit->instance_current; i < edit->instance_count; i++)
			hxa_2d_editor_structure_instance_free(&edit->instances[i]);
		edit->instance_count = edit->instance_current + 1;
	}
	l_old = &edit->instances[edit->instance_current - 1];
	l_new = &edit->instances[edit->instance_current];
	hxa_2d_editor_structure_instance_clone(l_old, l_new);
	return l_new;
}


void hxa_2d_editor_undo(HxA2DEditorShape *edit)
{
	uint i;
	if(edit->instance_current > 0)
	{
		for(i = 0; i < edit->instances[edit->instance_current].loop_count; i++)
		{
			if(edit->instances[edit->instance_current].loops[i].pool != NULL)
				r_array_free(edit->instances[edit->instance_current].loops[i].pool);
			edit->instances[edit->instance_current].loops[i].pool = NULL;
			if(edit->instances[edit->instance_current].loops[i].triangle_array != NULL)
				free(edit->instances[edit->instance_current].loops[i].triangle_array);
			edit->instances[edit->instance_current].loops[i].triangle_array = NULL;
		}
		edit->instance_current--;
	}
}

void hxa_2d_editor_redo(HxA2DEditorShape *edit)
{
	uint i;
	if(edit->instance_current + 1 < edit->instance_count)
	{
		for(i = 0; i < edit->instances[edit->instance_current].loop_count; i++)
		{
			if(edit->instances[edit->instance_current].loops[i].pool != NULL)
				r_array_free(edit->instances[edit->instance_current].loops[i].pool);
			edit->instances[edit->instance_current].loops[i].pool = NULL;
			if(edit->instances[edit->instance_current].loops[i].triangle_array != NULL)
				free(edit->instances[edit->instance_current].loops[i].triangle_array);
			edit->instances[edit->instance_current].loops[i].triangle_array = NULL;
		}
		edit->instance_current++;
	}
}


boolean hxa_2d_editor_collission_inside_test(double *loop, uint loop_length, double x, double y)
{
	uint i, ii, count = 0;
	int test, last_test;
	float f;
	loop_length = loop_length << 1;
	
	ii = loop_length - 2;
	last_test = loop[ii] >= x;
	for(i = 0; i < loop_length; i += 2)
	{
		test = loop[i] >= x;
		if(test != last_test)
			if(loop[ii + 1] + (loop[i + 1] - loop[ii + 1]) * (x - loop[ii]) / (loop[i] - loop[ii]) - y > 0)
				count++;
		ii = i;
		last_test = test;
	}
	return count & 1;
}

boolean hxa_2d_editor_collission_test(HxA2DEditorLoop *loop_a, HxA2DEditorLoop *loop_b)
{
	uint i, j;
	if(hxa_2d_editor_collission_inside_test(loop_a->loop, loop_a->loop_size, loop_b->loop[0], loop_b->loop[1]))
		return TRUE;
	if(hxa_2d_editor_collission_inside_test(loop_b->loop, loop_b->loop_size, loop_a->loop[0], loop_a->loop[1]))
		return TRUE;
	for(i = 0; i < loop_a->loop_size; i++)
	{
		for(j = 0; j < loop_b->loop_size; j++)
		{
			if(f_intersect_test2d(&loop_a->loop[i * 2], &loop_a->loop[((i + 1) % loop_a->loop_size) * 2],
								   &loop_b->loop[j * 2], &loop_b->loop[((j + 1) % loop_b->loop_size) * 2]))
				return TRUE;
		}
	}
	return FALSE;
}

void hxa_2d_editor_move_up(HxA2DEditorShape *shape, uint loop_id)
{
	HxA2DEditorLoop loop;
	HxA2DEditorInstance *instance;
	uint i, target;
	instance = &shape->instances[shape->instance_current];
	for(target = loop_id + 1; target < instance->loop_count; target++)
		if(hxa_2d_editor_collission_test(&instance->loops[loop_id], &instance->loops[target]))
			break;
	if(target == instance->loop_count)
		return;
	instance = hxa_2d_editor_structure_instance_add(shape);
	loop = instance->loops[loop_id];
	for(i = loop_id; i < target; i++)
		instance->loops[i] = instance->loops[i + 1];
	instance->loops[i] = loop;
	if(instance->loop_selected == loop_id)
		instance->loop_selected = i;
	else if(instance->loop_selected > loop_id && instance->loop_selected <= target)
		instance->loop_selected++;
}

void hxa_2d_editor_move_down(HxA2DEditorShape *shape, uint loop_id)
{
	HxA2DEditorLoop loop;
	HxA2DEditorInstance *instance;
	uint i, target;
	instance = &shape->instances[shape->instance_current];
	for(target = loop_id - 1; target != ~0; target--)
		if(hxa_2d_editor_collission_test(&instance->loops[loop_id], &instance->loops[target]))
			break;
	if(target == ~0)
		return;
	instance = hxa_2d_editor_structure_instance_add(shape);	
	loop = instance->loops[loop_id];
	for(i = loop_id; i > target; i--)
		instance->loops[i] = instance->loops[i - 1];
	instance->loops[target] = loop;
	if(instance->loop_selected == loop_id)
		instance->loop_selected = target;
	else if(instance->loop_selected > target && instance->loop_selected <= loop_id)
		instance->loop_selected--;
}


HxA2DEditorShape *hxa_2d_editor_init_empty()
{
	HxA2DEditorShape *edit;
	HxA2DEditorInstance *instance;
	uint i;
	edit = malloc(sizeof *edit);
	edit->instances = malloc((sizeof *edit->instances) * 16);
	edit->instance_count = 1;
	edit->instance_allocated = 16;
	edit->instance_current = 0;
	edit->node_create.type = ~0;
	for(i = 0; i < 16; i++)
	{
		edit->state[i] = HXA_2DEES_IDLE;
		edit->grab[i] = NULL;
	}
	instance = edit->instances;
	instance->symetry = MO_ELS_ROTATE;
	instance->symetry_pos[0] = 0;
	instance->symetry_pos[1] = 0;
	instance->loops = NULL;
	instance->loop_count = 0;
	instance->loop_allocated = 0;
	instance->entity = NULL;
	instance->entity_count = 0;
	instance->entity_allocated = 0;
	instance->values = NULL;
	instance->value_count = 0;
	instance->value_allocated = 0;
	instance->reposit_id = -1;
	instance->loop_selected = -1;
	instance->manip_mode = -1;
	instance->manip_start[0] = -0.5;
	instance->manip_start[1] = 0;
	instance->manip_end[0] = 0.5;
	instance->manip_end[1] = 0;
	instance->manip_divisions = 10;
	instance->name[0] = 0;
	return edit;
}


void hxa_2d_editor_free(HxA2DEditorShape *shape)
{
	uint i, j;
	for(i = 0; i < shape->instance_count; i++)
	{
		while(shape->instances[i].loop_count != 0)
			hxa_2d_editor_structure_remove_loop(&shape->instances[i], shape->instances[i].loop_count - 1);
		if(shape->instances[i].loop_allocated != 0)
			free(shape->instances[i].loops);
	}
	if(shape->instance_allocated != 0)
		free(shape->instances);
	free(shape);
}

void hxa_2d_editor_manipulator_set(HxA2DEditorShape *shape, HxA2DEditorManipulatorMode manipulator_mode)
{
	shape->instances[shape->instance_current].manip_mode = manipulator_mode;
}

void hxa_2d_editor_structure_process_matrix(float *matrix, uint lap, IOEditLevelSymetry symetry)
{
	matrix[0] = 1;
	matrix[1] = 0;
	matrix[2] = 0;
	matrix[3] = 0;

	matrix[4] = 0;
	matrix[5] = 1;
	matrix[6] = 0;
	matrix[7] = 0;

	matrix[9] = 0;
	matrix[11] = 0;

	matrix[12] = 0;
	matrix[13] = 0;
	matrix[14] = 0;
	matrix[15] = 1;
	switch(symetry)
	{
		case MO_ELS_ROTATE :
		case MO_ELS_ROTATE_THREE :
		case MO_ELS_ROTATE_FOUR :
			matrix[0] = cos(2 * PI * (float)lap / (float)(symetry + 1));
			matrix[2] = sin(2 * PI * (float)lap / (float)(symetry + 1));
		break;
		case MO_ELS_MIRROR_DOUBLE :
		if(lap >= 2)
		{
			matrix[0] = -1;
			matrix[2] = 0;
		}
		break;
	}
	if(lap % 2 == 1 && symetry >= MO_ELS_MIRROR)
	{	
		matrix[8] = matrix[2];
		matrix[10] = -matrix[0];
	}else
	{	
		matrix[8] = -matrix[2];
		matrix[10] = matrix[0];
	}
}

boolean hxa_2d_editor_updated(HxA2DEditorShape *shape)
{
	uint i;
	if(!shape->updated)
		return FALSE;
	shape->updated = FALSE;
	return TRUE;
}

void hxa_2d_editor_frame(HxA2DEditorShape *shape, float aspect, void *view)
{
	HxA2DEditorInstance *instance;
	uint i, j;
	double min[2] = {100000000.0, 100000000.0}, max[2] = {-100000000.0, -100000000.0}, center[2] = {0, 0}, dist;
	boolean set = FALSE;
	if(shape != NULL)
	{
		instance = &shape->instances[shape->instance_current];
		for(i = 0; i < instance->loop_count; i++)
		{
			if(instance->loops[i].loop_size > 0)
				set = TRUE;
			for(j = 0; j < instance->loops[i].loop_size; j++)
			{
				if(min[0] > instance->loops[i].loop[j * 2])
					min[0] = instance->loops[i].loop[j * 2];
				if(max[0] < instance->loops[i].loop[j * 2])
					max[0] = instance->loops[i].loop[j * 2];
				if(min[1] > instance->loops[i].loop[j * 2 + 1])
					min[1] = instance->loops[i].loop[j * 2 + 1];
				if(max[1] < instance->loops[i].loop[j * 2 + 1])
					max[1] = instance->loops[i].loop[j * 2 + 1];
			
			}
		}
	}
	if(set)
	{
		center[0] = (min[0] + max[0]) * 0.5;
		center[1] = (min[1] + max[1]) * 0.5;
		dist = (max[1] - min[1]) / aspect;
		if(dist < max[0] - min[0])
			dist = max[0] - min[0];
	}else
		dist = 1.0 / aspect;
	seduce_view_center_set(view, center[0], center[1], 0);
	seduce_view_camera_set(view, center[0], center[1], dist);
}