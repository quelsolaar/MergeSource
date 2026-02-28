#include "hxa.h"

#ifndef HXA_2D_EDITOR_INTERNAL

typedef void HxA2DEditorShape;

#endif

typedef enum{
	HXA_2DEMM_STRETCH,
	HXA_2DEMM_SCALE,
	HXA_2DEMM_ROTATE,
	HXA_2DEMM_ROTATE_AND_STRETCH,
	HXA_2DEMM_ROTATE_AND_SCALE,
	HXA_2DEMM_MIRROR,
	HXA_2DEMM_RULER,
	HXA_2DEMM_GRID,
	HXA_2DEMM_CIRCLE,
	HXA_2DEMM_OFF,
	HXA_2DEMM_COUNT
}HxA2DEditorManipulatorMode;

extern char					*hxa_2d_editor_manipulator_names[];

extern void					hxa_2d_editor_manipulator_set(HxA2DEditorShape *shape, HxA2DEditorManipulatorMode manipulator_mode); /* set the active manipulator tool */
extern void					hxa_2d_editor_draw(BInputState *input, HxA2DEditorShape *edit, boolean draw_shape, char *(*material_func)(uint32 material, float *color, void *user), void *user, double scale); /* deraw the UI for the editor */
extern void					hxa_2d_editor_draw_surface(BInputState *input, HxA2DEditorShape *edit, char *(*material_func)(uint32 material, float *color, void *user), void *user);

extern void					hxa_2d_editor_edit_add_new_shape(HxA2DEditorShape *edit, uint32 material); /* set ytthe edit to add a new shape */
extern void					hxa_2d_editor_edit_add_new_entity(HxA2DEditorShape *edit, HXANode *node, char *name);

extern HxA2DEditorShape		*hxa_2d_editor_init_empty(); /* chartes and empty shape that can be used to draw */
extern HxA2DEditorShape		*hxa_2d_editor_hxa_node_load(HXANode *node);
extern void					hxa_2d_editor_hxa_node_load_entity(HxA2DEditorShape *edit, HXANode *node, float pos_x, float pos_y, char *name);
extern boolean				hxa_2d_editor_updated(HxA2DEditorShape *shape);
extern void					hxa_2d_editor_hxa_node_save(HxA2DEditorShape *shape, HXANode *node);
extern void					hxa_2d_editor_frame(HxA2DEditorShape *shape, float aspect, void *view);
extern void					hxa_2d_editor_free(HxA2DEditorShape *shape);
extern void					hxa_2d_editor_material_replace(HxA2DEditorShape *edit, uint32 material, uint32 replacement);

extern void					hxa_2d_editor_undo(HxA2DEditorShape *edit);
extern void					hxa_2d_editor_redo(HxA2DEditorShape *edit);

extern uint					hxa_2d_editor_entity_count(HxA2DEditorShape *edit);
extern HXANode				*hxa_2d_editor_entity_get(HxA2DEditorShape *edit, uint id, float *pos, boolean *selected);
extern void					hxa_2d_editor_entity_delete_selected(HxA2DEditorShape *edit);


