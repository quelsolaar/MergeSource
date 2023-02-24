#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "betray.h"
#include "seduce.h"
#include "testify.h"
#include "opa_internal.h"

typedef struct{
	THandle *connection;
	boolean initialized;
	OPAProject project;
	TestifyNetworkAddress address;
}OPAConnection;

OPAConnection *opa_connections = NULL;
uint opa_connection_count = 0;
uint opa_connection_allocated = 0 ;

THandle *opa_host_handle = NULL;

extern boolean opa_parse_init(OPAProject *project, THandle *handle);
extern void opa_parse_incomming(OPAProject *project, THandle *handle);
extern void opa_widget_draw(BInputState *input, THandle *handle, OPAProject *project);
extern void opa_plot_draw(BInputState *input, THandle *handle, OPAProject *project, RMatrix *camera);
extern void opa_parse_retrive(OPAProject *project, THandle *handle);
extern void opa_save(OPAProject *project, THandle *handle);

void opa_options_init(OPADisplayOptions *options, char *name, uint array_length)
{
	uint i;
	options->expand = FALSE;
	options->show = TRUE;
	options->matrix = FALSE;
	options->columns = 1;
	options->width = 0.2;
	options->scroll = 0;
	options->min[0] = 0.0;
	options->min[1] = 0.0;
	options->max[0] = 1.0;
	options->max[1] = 1.0;	
	options->down_left[0] = -0.5;
	options->down_left[1] = -0.5;
	options->down_left[2] = 0.0;
	options->up_right[0] = 0.5;
	options->up_right[1] = 0.5;
	options->up_right[2] = 0.0;
	options->manipulator = NULL;

	for(i = 0; i < OPA_COLUMN_MAX_COUNT; i++)
		options->column_data[i] = 255;
	if(f_text_filter(name, "col"))
	{
		if(array_length % 3 == 0)
		{
			options->columns = 3;
		}else if(array_length % 4 == 0)
		{
			options->columns = 4;
		}
	}else if(f_text_filter(name, "matri"))
	{
		for(i = 1; i < 31 && array_length < i * i; i++);
		if(array_length == i * i)
		{
			options->columns = i;
		}else if(array_length % 4 == 0)
		{
			options->columns = 4;
		}
	}else if(f_text_filter(name, "pos") || f_text_filter(name, "vec"))
	{
		if(array_length % 3 == 0)
		{
			options->columns = 3;
		}else if(array_length % 2 == 0)
		{
			options->columns = 2;
		}
	}
	options->expand = array_length >  options->columns * options->columns || array_length == 1;
}

void opa_project_init(OPAProject *project)
{
	uint i, j;
	for(i = 0; i < project->type_count; i++)
		for(j = 0; j < project->types[i].member_count; j++)
			opa_options_init(&project->types[i].members[j].options, project->types[i].members[j].value_name, project->types[i].members[j].array_length);
}

void opa_input_handler(BInputState *input, void *user_pointer)
{
	THandle *connection;
	TestifyNetworkAddress incomming;
	static RMatrix matrix, camera;
	static float view[3] = {0, 0, 1}, m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	uint x, y;
	float aspect;
	uint i;

	if(input->mode == BAM_DRAW)
	{
		r_framebuffer_clear(0, 0, 0, 0, TRUE, TRUE); 
		aspect = betray_screen_mode_get(&x, &y, NULL);
		r_viewport(0, 0, x, y);
		r_matrix_identity(&matrix);
		betray_view_vantage(view);
		r_matrix_frustum(&matrix, -0.01 - view[0] * 0.01, 0.01 - view[0] * 0.01, -0.01 * aspect - view[1] * 0.01, 0.01 * aspect - view[1] * 0.01, 0.01 * view[2], 100.0); /* set frustum */
		r_matrix_translate(&matrix, -view[0], -view[1], -view[2]);
		camera = matrix;
		seduce_view_set(NULL, &camera);
		r_matrix_translate(&camera, 0, 0, 1);
		r_matrix_set(&matrix);
		r_matrix_push(&matrix);
	}

	if(input->mode == BAM_MAIN)
	{
		connection =  testify_network_stream_wait_for_connection(opa_host_handle, &incomming);
		if(connection != NULL)
		{
			printf("got connection");
			if(opa_connection_count == opa_connection_allocated)
			{
				opa_connection_allocated += 8;
				opa_connections = realloc(opa_connections, (sizeof *opa_connections) * opa_connection_allocated);
			}
			opa_connections[opa_connection_count].connection = connection;
			opa_connections[opa_connection_count].initialized = FALSE;
			opa_connections[opa_connection_count].project;
			opa_connections[opa_connection_count].address = incomming;
			opa_connection_count++;
		}
		for(i = 0; i < opa_connection_count; i++)
		{
			if(testify_network_stream_connected(opa_connections[i].connection))
			{
				if(!opa_connections[i].initialized)
				{
					opa_connections[i].initialized = opa_parse_init(&opa_connections[i].project, opa_connections[i].connection);
					opa_project_init(&opa_connections[i].project);
				}else
				{
					opa_parse_incomming(&opa_connections[i].project, opa_connections[i].connection);
				}
			}else
			{
				opa_connections[i--] = opa_connections[--opa_connection_count];
			}
			opa_parse_retrive(&opa_connections[i].project, opa_connections[i].connection);
			opa_save(&opa_connections[i].project, opa_connections[i].connection);
		}
	}
	if(seduce_view_change_right_button(NULL, input) ||
		seduce_view_change_multi_touch(NULL, input, NULL) ||
		seduce_view_change_keys(NULL, input, NULL))
		return;
//	seduce_background_shape_matrix_interact(input, NULL, m, TRUE, FALSE);
	if(input->mode == BAM_DRAW)
	{
		r_matrix_push(NULL);
		r_matrix_matrix_mult(NULL, m);
	}


	for(i = 0; i < opa_connection_count; i++)
	{
		opa_widget_draw(input, opa_connections[i].connection, &opa_connections[i].project);
	}
	if(input->mode == BAM_MAIN) /* Update the camera over time */
	{
		seduce_view_update(NULL, input->delta_time);
	}
	if(input->mode == BAM_DRAW)
	{
		r_matrix_pop(NULL);
	}
	for(i = 0; i < opa_connection_count; i++)
		opa_plot_draw(input, opa_connections[i].connection, &opa_connections[i].project, &camera);



	
	
//	seduce_popup_detect_mouse(input, NULL, 2, light_organ_popup_func, NULL); /* Middle mouse button (2) activates popup */
//	seduce_popup_detect_multitouch(input, NULL, 5, light_organ_popup_func, NULL); /* Putting 5 mult touch points on screen at the same time activates popup */
//	seduce_popup_detect_axis(input, BETRAY_BUTTON_FACE_Y, light_organ_popup_func, NULL); /* Controller button BETRAY_BUTTON_FACE_Y activates popup */
	seduce_element_endframe(input, FALSE); /* Change FALSE in to TRUE to display hit detection debugger*/
}

void opa_tokenizer_test();

typedef int TestType;

int main(int argc, char **argv)
{

	if(!betray_support_context(B_CT_OPENGL))
	{
		printf("OPA Requires B_CT_OPENGL to be available, Program exit.\n");
		exit(0);
	}
	seduce_translate_load("opa_translation.txt");
	betray_init(B_CT_OPENGL, argc, argv, 1800, 1000, 16, FALSE, "OPA - Memmory debugger");	
	

/*	opa_preprosessor("./testfile.h");
	exit(0);
/*	{
		OPAProject project;
		opa_parse(&project, "./");
		return;
		opa_tokenizer_test();
		opa_preprosessor("./test_file.c");
	//	opa_parse(&project, "../../Mergesource/");
		return;
	//	opa_tokenizer_test();
	//	opa_parse(&project, "./");
	}*/

	opa_host_handle = testify_network_stream_address_create(NULL, 0xa113);
	if(opa_host_handle == NULL)
	{
		printf("OPA Error: Network Bind failed\n");
		return;
	}
	r_init(betray_gl_proc_address_get());
	seduce_init();
	seduce_font_default_set(seduce_font_get_by_name("verdana"));
//	seduce_font_default_set(seduce_font_get_by_name("arial"));

	betray_action_func_set(opa_input_handler, NULL);
	betray_launch_main_loop();
	seduce_translate_save("opa_translation.txt");
	return TRUE;
}

