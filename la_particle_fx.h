extern void la_pfx_image_init(uint texture_size);
extern void la_pfx_init(uint particle_count);
extern void la_pfx_create_spark(double *pos);
extern void la_pfx_create_intro_spark(double *pos);
extern void la_pfx_create_aches(double *pos);
extern void la_pfx_create_dust_line(double *pos_a, double *pos_b);
extern void la_pfx_create_dust_selected_vertexes(double *mid);
extern void la_pfx_create_aches_surface(double *v0, double *v1, double *v2, double *v3);
extern void la_pfx_create_bright(double *pos);
extern void la_pfx_video_flare(void);
extern void la_pfx_select_vertex(void);
extern void la_pfx_draw(boolean intro);
extern void la_pfx_draw_intro(void);
extern uint la_pfx_surface_material(void);

extern void la_pfx_manipulator_flare(void);
extern void la_pfx_sparks(float delta);

extern void la_create_shadow_edge(float size, uint count, float *shadow, float *color, float *square);
extern void la_draw_set_vec2(float *array, uint pos, float a, float b);
extern void la_draw_set_vec3(float *array, uint pos, float a, float b, float c);
extern void la_draw_set_vec4(float *array, uint pos, float a, float b, float c, float d);


extern void lo_pfx_draw_selected(BInputState *input, uint snap_type, uint selected_vertex, uint polygon_snap, uint *edge_snap, boolean action);
extern void lo_pfx_wiggle_line_add(float ax, float ay, float az, float bx, float by, float bz, boolean major);
extern void lo_pfx_draw_snap(BInputState *input, uint snap_type, double  *snap, double *pos, boolean action);
extern void la_pfx_test(float delta);

extern void lo_pfx_draw_local_space(BInputState *input);