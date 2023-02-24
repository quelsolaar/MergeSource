#include <math.h>
#include <stdlib.h>
#include "seduce.h"
#include "s_draw_3d.h"



extern void seduce_object_3d_init();
extern void seduce_font_init();
extern void seduce_text_init();
extern void seduce_3d_make();
extern void seduce_background_init();
extern void seduce_element_init();
extern void seduce_text_edit_init();
extern void seduce_font_load_all();
extern void seduce_select_init(void);
extern void seduce_draw_line_init();
extern void seduce_primitive_line_init();
extern void seduce_background_circles_init();

void seduce_init(void)
{
	seduce_view_init(NULL);
	seduce_object_3d_init();
	seduce_font_load_all();
	seduce_text_init();
	seduce_background_init();
	seduce_element_init();
	seduce_text_edit_init();
	seduce_select_init();
	seduce_primitive_line_init();
	r_shader_set(r_shader_presets_get(P_SP_COLOR_UNIFORM));
	seduce_draw_line_init();
	seduce_background_circles_init();
}


void seduce_animate(BInputState *input, float *timer, boolean active, float speed)
{
	if(input->mode == BAM_MAIN)
	{
		if(active)
		{
			*timer += input->delta_time * speed;
			if(*timer > 1.0)
				*timer = 1.0;
		}else
		{
			*timer -= input->delta_time * speed;
			if(*timer < 0.0)
				*timer = 0.0;
		}
	}
}
