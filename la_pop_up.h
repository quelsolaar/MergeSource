
typedef enum{
	LA_PUER_NONE,
	LA_PUER_OBJECT_SELECT,
	LA_PUER_SETTINGS,
	LA_PUER_RENAME,
	LA_PUER_COUNT
}LAPUEmptyReturn;

extern LAPUEmptyReturn la_pu_empty(BInputState *input, boolean active);
extern boolean la_pu_object_selection(BInputState *input, boolean active);
extern void     la_pu_vertex(BInputState *input, uint vertex, boolean active);
extern boolean la_pu_manipulator(BInputState *input, boolean active);
extern boolean la_pu_edge(BInputState *input, uint *edge, boolean active);
extern boolean la_pu_polygon(BInputState *input, uint polygon, boolean active);
extern uint la_pu_select(BInputState *input, boolean active);
extern void la_pu_type(BInputState *input);
extern boolean la_draw_settings_menu(BInputState *input, boolean active);
extern boolean la_draw_rename_menu(BInputState *input, boolean active);

extern void lo_preview_update();