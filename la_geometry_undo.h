
extern void		init_undo(void);
extern void		undo_event_done(void);
extern void		udg_connect(char *address);
extern void		udg_connect_local(void);

extern void		udg_create_new_modeling_node(void);
extern void		udg_create_new_modeling_layer(void);
extern void		udg_destroy_node(uint32 node_id);
extern void		udg_clone_node(uint32 node_id);

extern boolean	udg_update_geometry(void);
extern boolean	udg_check_g_node(uint32 node_id);
extern void		udg_set_modeling_node(uint32 node_id);
extern uint32	udg_get_modeling_node(void);
extern void		udg_set_modeling_layer(uint layer_id);



extern void		udg_get_geometry(uint32 *vertex_count, uint32 *polygon_count, double **vertex, uint32 **ref, uint32 **crease);
extern egreal	*udg_get_base_layer(void);

extern void		udg_vertex_set(uint32 id, double *state, double x, double y, double z);
extern void		udg_vertex_move(uint32 id, double x, double y, double z);
extern void		udg_vertex_delete(uint32 id);
extern void		udg_get_vertex_pos(double *pos, uint vertex_id);
extern void		udg_polygon_set(uint32 id, uint32 a, uint32 b, uint32 c, uint32 d);
extern void		udg_polygon_delete(uint32 id);
extern void		udg_crease_set(uint32 id, uint32 a, uint32 b, uint32 c, uint32 d);

extern void		udg_set_grid_snap(double *center, double size);
extern boolean	udg_get_grid_snap(double *center, double *size);

extern void		udg_set_select(uint vertex, double value);
extern double	udg_get_select(uint vertex);
extern void		udg_clear_select(double value);

extern uint32	udg_find_empty_slot_vertex(void);
extern uint32	udg_find_empty_slot_polygon(void);

extern void		udg_create_edge(uint vertex0, uint vertex1);
extern uint		*udg_get_edge_data(uint *count);
extern void		udg_destroy_edge(uint id);
extern void		udg_destroy_all_edges(void);

extern uint		udg_get_version(boolean	structure, boolean vertex, boolean select, boolean edge, boolean tags);

extern void		udg_set_distance(uint vertex_a, uint vertex_b);
extern double	udg_get_distance(void);

extern void		udg_undo_geometry(void);
extern void		udg_redo_geometry(void);

typedef struct{
	char	group[16];
	char	tag[16];	
	double	vec[3];
	double	select;
}UNDOTag;

extern UNDOTag	*udg_get_tags(uint *count);
extern void		udg_get_tag_pos(uint tag, double *pos);

extern void		udg_create_tag(double *vec);
extern void		udg_destory_tag(uint tag);
extern void		udg_rename_tag(uint tag);
extern void		udg_rename_group(uint tag);
extern void		udg_move_tag(uint tag, double *vec);
extern void		udg_select_tag(uint tag, double	select);


extern boolean	udg_get_reference_geometry(uint32 *vertex_count, double **vertex);
extern uint		udg_get_reference_version();

extern double la_axis_matrix[16];
extern boolean la_axis_visible;