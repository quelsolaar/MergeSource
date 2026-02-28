
/* la_neighor.c */

extern uint *la_compute_neighbor(uint *ref, uint ref_count, uint vertex_count, egreal *vertex);

/* la_tool_cloapse.c */

extern void		la_t_collapse_two_vertexes(uint vertex_a, uint vertex_b);
extern void		la_t_collapse_selected_vertexes(void);
extern void		la_t_weld_selected_vertexes(void);

/* la_tool_draw.c */


extern void		la_t_init_draw_line(void);
extern void		la_t_new_draw_line(float x, float y);
extern void		la_t_draw_line_add(float x, float y, boolean add);
extern boolean  la_t_draw_select_menu_test(void);

typedef enum{
	SM_SELECT,
	SM_DESELECT,
	SM_SUB,
	SM_ADD
}SelectionMode;

extern boolean la_t_draw_line_test_delete(void);
extern boolean la_t_draw_line_draw_delete_overlay(void);
extern boolean la_t_draw_line_test_select(SelectionMode mode);


/* la_tool_edge_connector.c */

extern void		la_t_init_edge_connector(void);
extern boolean la_t_edge_connector(uint *edge);

/* la_tool_manipulator.c */

extern void		la_t_tm_init(void);
extern void		la_t_tm_place(double x, double y, double z);
extern void		la_t_tm_get_pos(double *pos);
extern void		la_t_tm_get_vector(double *vector);
extern void		la_t_tm_hide(boolean hide);
extern boolean	la_t_tm_hiden(void);
extern boolean	la_t_tm_grab(BInputState *input);
extern void		la_t_tm_view_center(void);
extern boolean	la_t_tm_test_center(BInputState *input);
extern boolean	la_t_tm_manipulate(BInputState *input, double *snap, uint snap_type);
extern void 	grab_one_vertex(BInputState *input, uint id, double *pos);
extern float	*la_t_tm_matrix_get(void);

/* la_tool_poly_select.c */

extern void		la_t_face_vector(double *origo, double *vector, uint v0, uint v1, uint v2);

extern void		la_t_extrude(uint vertex_count, void (*func)(double *output, uint vertex_id, void *data), void *data);
extern void		la_t_detach_selected_polygons(void);
extern void		la_t_duplicate_selected_polygons(void);
extern void		la_t_flip_selected_polygons(void);
extern void		la_t_mirror(double *pos, double *vector);
extern void		la_t_flatten(double *origo, double *vector);
extern void		la_t_symmetry(double *origo, double *vector);
extern void		la_t_delete_selection(void);

/* la_tool_reshape.c */

extern void 	la_t_reshape_hull_start(BInputState *input, uint *edge);
extern void 	la_t_reshape_hull(BInputState *input);
extern void 	la_t_reshape_hull_draw(void);
extern void		la_t_select_hull(uint *edge);

/* la_tool_revolve.c */

extern void 	la_t_revolve(uint *start_edge, uint revolve);
extern void		la_t_tube(uint *start_edge, uint revolve);
extern void 	la_t_select_open_edge(void);
extern void 	la_t_crease_selected(uint32 crease_value);
extern void		la_t_wrap_around(uint *start_edge, uint revolve);

/* la_tool_select.c */
extern uint		la_t_poly_egde_test(uint polygon, double x, double y);
extern void		la_t_poly_select(uint polygon);
extern void		la_t_smooth_select(void);
extern void		la_t_revert_to_base(void);

typedef enum{
	LA_PSST_PLANE,
	LA_PSST_SMOOTH,
	LA_PSST_CONVEX,
	LA_PSST_COUNT
}LAPolySurfaceSeletcType;

extern void		la_t_poly_surface_select(uint polygon, LAPolySurfaceSeletcType type);

/* la_tool_split.c */

extern void 	la_t_edge_splitter_start(BInputState *input, uint *edge);
extern void 	la_t_edge_splitter(BInputState *input);
extern void		la_t_poly_spliter(uint id);

/* la_tool_deply.c */

extern void 	la_t_deploy(uint poly);
extern void		la_t_polygon_select_fill(uint poly);
extern void		la_t_vertex_select_fill(uint vertex);

/* la_tool_slice.c */

extern void		la_t_slice(double *pos, double *vector, boolean del);

/* la_tool_center.c */

extern void		la_t_center_geometry(void);
extern void		la_t_center_manipulator(void);

/* la_tool_neighbor.c */

extern void		la_t_poly_triangulate(void);
extern void		la_t_poly_auto_crease(void);
extern void		la_t_poly_find_quads(void);

/* la_tool_cut_paste.c */

extern void		la_t_copy(egreal *pos);
extern void		la_t_paste(egreal *pos);
extern void		la_t_copy_to_new_geometry();
extern void		la_t_paste_to_new_geometry();

/* la_tool_cleanup.c */

extern void		la_t_model_vertex_delete();
extern void		la_t_model_cleanup();

/* la_tool_hole.c */

extern uint		la_t_edge_fill_find_edge_poly(uint edge_a, uint edge_b, boolean single);
extern void		la_t_edge_fill_strip(uint edge_a, uint edge_b);
extern void		la_t_edge_fill_hole(uint edge_a, uint edge_b);
extern boolean	la_t_edge_select(uint edge_a, uint edge_b);
extern void		lo_t_hole_debug();

/* la_projection.c */

extern void		la_view_axis_matrix_poly_set(uint polygon, uint edge);
extern void		la_view_axis_matrix_reset();
extern void		la_view_axis_matrix_edge_set(uint edge_a, uint edge_b);

/* la_tool_group.c */

extern void		la_t_group_draw();
extern void		la_t_group_store();
extern void		la_t_group_update(BInputState *input, boolean active);
extern boolean	la_t_group_interact(BInputState *input);