
extern void la_do_init(void);
extern void la_do_edge_select(double *vertex_a, double *vertex_b);
extern void la_do_edge_split(double *vertex_a, double *vertex_b, double pos);
extern void la_do_edge_delete(double *a, double *b, double c_x, double c_y, double c_z);
extern void la_do_polygon_delete(uint polygon);
extern void la_do_polygon_delete_clear();
extern void la_do_polygon_delete_execute();
extern void la_do_edge_delete_air(double *vertex_a, double *vertex_b);

extern void la_t_draw_line_draw_overlay(double *start, double *end);

extern void la_do_active_vertex(BInputState *input, double *vertex, boolean move);
extern void la_do_xyz_lines(double *start, boolean snap);
extern void la_do_draw_snap_edge(uint *edge);
extern void la_do_active_polygon(double *snap);
extern void la_do_draw(double *start, double *end, boolean snap, double *closest);
extern void la_do_owerlay(BInputState *input);
extern void la_do_draw_closest_edge(uint *edge, double x, double y, boolean snap);

typedef struct{
	void		*poly_pool;
	void		*line_pool;
	void		*vertex_pool;
	void		*base_pool;
	float		base_length;
	uint		surface_version;
	float		max[3];
	float		min[3];
}LoOverlay;

extern void la_do_mesh_clear(LoOverlay *overlay);
extern void la_do_mesh_free(LoOverlay *overlay);
// extern void la_do_mesh_update(LoOverlay *overlay, uint32 length, uint32 ref_length, double *vertex, uint32 *ref, uint32 *crease, double *select, uint version);
extern void la_do_mesh_surface(LoOverlay *overlay);