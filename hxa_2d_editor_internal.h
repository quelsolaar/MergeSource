
#include "tweeker.h"
#define MO_EDITOR_SNAP 0.00001
#define HXA_2D_EDITOR_MIN_EDGE_SIZE 0.0002

typedef enum{
	MO_ELS_NONE,
	MO_ELS_ROTATE,
	MO_ELS_ROTATE_THREE,
	MO_ELS_ROTATE_FOUR,
	MO_ELS_MIRROR,
	MO_ELS_MIRROR_DOUBLE,
	MO_ELS_COUNT,
}IOEditLevelSymetry;

typedef struct{
	double *loop;
	uint loop_size;
	uint loop_allocated;
	boolean *selection;
	uint32 material;
	double matrix[16];
	double rotate;
	double scale;
	float *triangle_array;
	void *pool;
}HxA2DEditorLoop;

typedef struct{
	double pos[2];
	int int_pos[2];
	char name[32];
	uint loop_id;
	HXANode node;
	boolean selected;
	boolean active;
}IOEditLevelEntity;

typedef struct{
	uint class_id;
	uint value_id;
	int64 value;
}IOEditLevelValue;

typedef struct{
	HxA2DEditorLoop *loops;
	uint loop_count;
	uint loop_allocated;
	uint loop_selected;
	IOEditLevelEntity *entity;
	uint entity_count;
	uint entity_allocated;
	IOEditLevelValue *values;
	uint value_count;
	uint value_allocated;
	IOEditLevelSymetry symetry;
	double symetry_pos[2];
//	float start_pos[2];
	uint reposit_id;
	uint manip_mode;
	double manip_start[2];
	double manip_end[2];
	double manip_divisions;
	char name[64]; 
}HxA2DEditorInstance;

typedef enum{
	HXA_2DEES_IDLE,
	HXA_2DEES_EMPTY,
	HXA_2DEES_MANIPULATOR,
	HXA_2DEES_NOT_SELECTED,
	HXA_2DEES_SURFACE,
	HXA_2DEES_SELECTION,
	HXA_2DEES_SELECT,
	HXA_2DEES_ADD_SHAPE,
	HXA_2DEES_ADD_ENTITY
}HXA2DEditorEditState;


typedef struct{
	HxA2DEditorInstance *instances;
	uint instance_count;
	uint instance_allocated;
	uint instance_current;
	HXA2DEditorEditState state[16];
	void *grab[16];
	uint32 material_create;
	HXANode node_create;
	char node_name[32];
	boolean updated;
}HxA2DEditorShape;



extern HxA2DEditorShape mo_level_edit;

/* structure */



extern void			hxa_2d_editor_structure_add_loop(HxA2DEditorInstance *level, uint32 material, double x, double y, double size, uint allocate);
extern void			hxa_2d_editor_structure_entity_add(HxA2DEditorInstance *level, float x, float y, HXANode *node, char *node_name);
extern void			hxa_2d_editor_structure_remove_loop(HxA2DEditorInstance *level, uint loop);
extern HxA2DEditorInstance	*hxa_2d_editor_structure_instance_add(HxA2DEditorShape *edit);

extern void			hxa_2d_editor_structure_process_matrix(double *matrix, uint lap, IOEditLevelSymetry symetry);

extern boolean		hxa_2d_editor_collission_test(HxA2DEditorLoop *loop_a, HxA2DEditorLoop *loop_b);
extern void			hxa_2d_editor_move_up(HxA2DEditorShape *shape, uint loop_id);
extern void			hxa_2d_editor_move_down(HxA2DEditorShape *shape, uint loop_id);
/* edit */

extern void			hxa_2d_editor_edit_guideline_snap(HxA2DEditorInstance *level, double *pos, double pointer_x, double pointer_y, double scale, boolean manipulator);
extern boolean		hxa_2d_editor_edit_loop_valid_test(double *array, uint size);

/* manipulator */

extern void			hxa_2d_editor_manipulator(BInputState *input, HxA2DEditorShape *edit, HxA2DEditorInstance *level, double scale);

#define HXA_2D_EDITOR_INTERNAL
#include "hxa_2d_editor.h"

