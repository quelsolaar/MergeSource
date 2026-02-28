/*
extern void p_init(void);
extern void p_view_change_start(BInputState *input);
extern void p_view_change(BInputState *input);

extern void p_projection_update(void);

extern void p_set_view_center(double *center);
extern void p_get_view_center(double *center);
extern void p_get_view_camera(double *camera);
extern double p_get_distance_camera(void);
extern void p_set_grid_size(double grid_size);

extern void p_get_model_matrix(double *matrix);
extern void p_view_set(void);

extern void p_get_projection(double *output, float x, float y);
extern void p_get_projection_vertex(double *output, double *vertex, double x, double y);
extern void p_get_projection_screen(double *output, double x, double y, double z);
extern double p_get_projection_screen_distance(double space_x, double space_y, double space_z, double screen_x, double screen_y);
extern void p_get_projection_plane(double *dist, uint axis, double pointer_x, double pointer_y , double depth);
extern double p_get_projection_line(double *dist, uint axis, double pointer_x, double pointer_y, double *pos);


typedef enum{
	LA_ST_VERTEX,
	LA_ST_EDGE,
	LA_ST_SURFACE
}LASnapType;

extern void p_get_projection_vertex_with_axis(double *output, double *start, double pointer_x, double pointer_y, boolean snap, double *closest, LASnapType snap_type);
extern uint p_get_projection_axis(void);
extern void p_get_projection_line_snap(double *output, uint axis, double direction, double *start, double *snap, LASnapType snap_type);

extern boolean p_find_closest_tag(double *pos, double distance, double x, double y);

*/
extern boolean p_find_closest_edge(uint *edge, double *distance, double x, double y);
extern uint p_find_click_tag(double x, double y);
extern boolean p_find_click_tag_lable(double x, double y);

extern RMatrix la_interface_matrix;
extern RMatrix la_world_matrix;

typedef enum{
	SUI_ST_NONE,
	SUI_ST_VERTEX_FAR,
	SUI_ST_VERTEX_CLOSE,
	SUI_ST_LINE,
	SUI_ST_TANGENT
}SUISnapType;

extern boolean	p_find_closest_vertex(uint *closest, uint *selected, double *distance, double *selected_distance, double x, double y, boolean include_selected);
extern uint		p_find_closest_polygon(BInputState *input, double *mid, double *normal, boolean *selected);
extern boolean	p_find_line_intersect(float *start, float *end, void (*func)(uint id));
extern void		seduce_view_projection_line_snap(double *output, uint axis, double direction, double *start, double *snap, SUISnapType snap_type);
extern void		la_view_projection_vertex_with_axis(BInputState *input, RMatrix *matrix, double *output, double *start, double pointer_x, double pointer_y, boolean snap, double *closest, SUISnapType snap_type);
extern boolean	seduce_view_edge_test(double *a, double *b, double x, double y);

extern void lo_projection_cache_init();
extern void lo_projection_cache_update(RMatrix *matrix);
extern void lo_projection_cache_update_test(RMatrix *matrix);
extern void lo_projection_cache_draw();