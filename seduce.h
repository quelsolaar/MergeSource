#ifndef SEDUCE_H
#define	SEDUCE_H

/*
This is the new and improved .h file for the "ui tool kit" used in varius verse related applications.
*/

#include "forge.h"
#include "betray.h"
#include "relinquish.h"
#include "s_text.h"

/*     Initialize Seduce      */
/* -------------------------- */

extern void	seduce_init(void); /* Initializes Seduce */
extern void seduce_viewport_set(float x_start, float y_start, float x_end, float y_end);
extern void seduce_element_endframe(BInputState *input, boolean debug); /* seduce_element_endframe needs to be called at the end of each frame. It will draw mouse pointers and other overlay items and consloidate input. By turning on "debug" Seduce will draw an overlay indication the position of all hit areas of screem created by seduce's colission system */

extern void seduce_animate(BInputState *input, float *timer, boolean active, float speed); /* Animates a value between 0.0 and 1.0. If "active" is TRUE "timer" will go from 0.0 to 1.0 in "speed" seconds, If "active" is FALSE "timer" will go from 1.0 to 0.0 in "speed" seconds*/



/* ------  Text rendering and editing ------
Seduce has a comprehensive text rendeing system. Seduce comes with a number of built in fonts, and you can load more .pff (Polygon font format) fonts by simply placing them in the same directory as the application. New .pff files can be generated from true type fonts using the Seduce font converter. The font system supports advanced kerning, and UTF-8 characters.*/

#ifndef SEDUCE_INTERNAL_FONT
typedef void SeduceRenderFont;
#endif

#define SEDUCE_T_SIZE 0.007 /* default text size */
#define SEDUCE_T_SPACE 0.2 /* default letter spacing*/
#define SEDUCE_T_LINE_SPACEING 2.5 /* default line spacing */

/* ------  Fonts ------
Seduce has a number of built in polygon fonts that can be used to render text.
*/

extern void seduce_font_load(char *name); /* load in a .pff font file */
extern uint seduce_font_count(); /* Get the number of fonts currently installed in seduce */
extern char *seduce_font_name(uint id); /* Get the names of */
extern SeduceRenderFont *seduce_font_get_by_id(uint id); /* load a font by specifying is id. The ids range form 0 to the numer of installed fonts minus 1. The number of installed fornts can be retrived by calling seduce_font_count(). */
extern SeduceRenderFont *seduce_font_get_by_name(char *name); /* load a font by specifying its name, Returns NULL if no fornt with the specifyed name is found. */
extern SeduceRenderFont *seduce_font_default_get(); /* get a handle to the current default font. The default font will be used any time funtion that takes a font as a parameter is given a NULL pointer instead.*/
extern void seduce_font_default_set(SeduceRenderFont *font); /* Set a font as the default font. The default font will be used any time funtion that takes a font as a parameter is given a NULL pointer instead.*/

/* ------  Text line rendering and editing ------
Text funtions for drawing and editing a single line of text.
*/

extern float	seduce_text_line_draw(SeduceRenderFont *font, float pos_x, float pos_y, float letter_size, float letter_spacing, const char *text, float red, float green, float blue, float alpha, uint length); /* Draws a line of text. */
extern uint		seduce_text_line_hit_test(SeduceRenderFont *font, float letter_size, float letter_spacing, const char *text, float pos_x); /* Returns the charactre position that is pos_x to the left, in the string "text" if renderd using font and letter_size and letter_spacing. */
extern float	seduce_text_line_length(SeduceRenderFont *font, float size, float spacing, const char *text, uint end); /* Computes the length of a string renderd using font and letter_size and letter_spacing. If the string is longer then "end" number of characters only the characters before end will be messuers. */

typedef enum{
	S_TIS_IDLE, /* The type in function is idle. */
	S_TIS_ACTIVE, /* The type in function is activly beeing used. */
	S_TIS_DONE /* The use th ethe type in function has just ben completed (mosst often by the user pressing Enter). */
}STypeInState;

extern STypeInState	seduce_text_edit_line(BInputState *input, void *id, SeduceRenderFont *font, char *text, uint buffer_size, float pos_x, float pos_y, float length, float size, char *label, boolean left, void (*done_func)(void *user, char *text), void *user, float red, float green, float blue, float alpha, float active_red, float active_green, float active_blue, float active_alpha); /* Creates an editable text field. Lable will be printed if the text field is empty. The text striing cna either be modifyed as the user types, or if given a done_func a call back can be called when the user completes the typing with a new string. */
extern STypeInState	seduce_text_edit_obfuscated(BInputState *input, void *id, char *text, uint buffer_size, float pos_x, float pos_y, float length, float size, char *label, boolean left, void (*done_func)(void *user, char *text), void *user, float red, float green, float blue, float alpha, float active_red, float active_green, float active_blue, float active_alpha); /* Creates an editable text field where all charcters are obfuscated. Lable will be printed if the text field is empty. The text striing cna either be modifyed as the user types, or if given a done_func a call back can be called when the user completes the typing with a new string. */
extern STypeInState	seduce_text_edit_double(BInputState *input, void *id, SeduceRenderFont *font, double *value, float pos_x, float pos_y, float length, float size, boolean left, void (*done_func)(void *user, double value), void *user, float red, float green, float blue, float alpha, float active_red, float active_green, float active_blue, float active_alpha);/* Creates an editable text feild for typing in 64bit floating point numbers. Lable will be printed if the text field is empty. The text striing cna either be modifyed as the user types, or if given a done_func a call back can be called when the user completes the typing with a new string. */
extern STypeInState	seduce_text_edit_float(BInputState *input, void *id, SeduceRenderFont *font, float *value, float pos_x, float pos_y, float length, float size, boolean left, void (*done_func)(void *user, float value), void *user, float red, float green, float blue, float alpha, float active_red, float active_green, float active_blue, float active_alpha);/* Creates an editable text feild for typing in 32bit floating point numbers.  Lable will be printed if the text field is empty. The text striing cna either be modifyed as the user types, or if given a done_func a call back can be called when the user completes the typing with a new string. */

extern STypeInState	seduce_text_edit_int(BInputState *input, void *id, SeduceRenderFont *font, int *value, float pos_x, float pos_y, float length, float size, boolean left, void (*done_func)(void *user, int value), void *user, float red, float green, float blue, float alpha, float active_red, float active_green, float active_blue, float active_alpha);/* Creates an editable text feild for typing in 32bit signed integer numbers. The text striing cna either be modifyed as the user types, or if given a done_func a call back can be called when the user completes the typing with a new string. */
extern STypeInState	seduce_text_edit_uint(BInputState *input, void *id, SeduceRenderFont *font, uint *value, float pos_x, float pos_y, float length, float size, boolean left, void (*done_func)(void *user, uint value), void *user, float red, float green, float blue, float alpha, float active_red, float active_green, float active_blue, float active_alpha);/* Creates an editable text feild for typing in 64bit unsigend integer numbers.  Lable will be printed if the text field is empty. The text striing cna either be modifyed as the user types, or if given a done_func a call back can be called when the user completes the typing with a new string. */

extern boolean seduce_text_edit_active(void *id); /* Returns True if the id is curtrently active in one of the text editing functions */


/* ------  Text block rendering and editing ------
Text funtions for drawing and editing a blocks of text. Since blocks support text with multiple sices and colors (for things like syntax high-lightin) the setting for text is stord in arrays of STextBlockMode structures.
*/

typedef enum{
	SEDUCE_TBAS_LEFT, /* Left aligned text. */
	SEDUCE_TBAS_RIGHT, /* Right aligned text. */
	SEDUCE_TBAS_CENTER,/* Center aligned text. */
	SEDUCE_TBAS_STRETCH, /* Text stretched out to align with both left and right.*/
	SEDUCE_TBAS_COUNT
}STextBlockAlignmentStyle;

typedef struct{
	uint character_position; /* the first character in the text for this STextBlockMode to be applied to*/
	void *font; /* Font */
	float red; /* Red color component of text */
	float green; /* Green color component of text */ 
	float blue; /* Red color component of text */
	float alpha; /* Alpha component of text */
	float letter_size; /* Sice of charcters */
	float letter_spacing; /* Added spacing between chacters. Use SEDUCE_T_SPACE as default value.*/
}STextBlockMode; /* Describes the styling of a section of text.*/

extern uint		seduce_text_block_draw(float pos_x, float pos_y, float line_size, float height,  float line_spacing, STextBlockAlignmentStyle style, const char *text, uint text_pos, STextBlockMode *modes, uint mode_count); /* Draws a block of mono spaced text. text_pos denotes the starting point in the text string where it should be drawn. This makes it possible to connect multiple blocks togerter and scroll text without needing to update the STextBlockMode array. */
extern float	seduce_text_block_height(float line_size, float line_spacing, STextBlockAlignmentStyle style, const char *text, uint pos, STextBlockMode *modes, uint mode_count, uint end);  /* computes the height needed to fit characters until "end" drawn by seduce_text_block_draw. */
extern uint		seduce_text_box_draw_monospace(SeduceRenderFont *font, float pos_x, float pos_y, float character_size, float space_size, float line_size, const char *text, uint line_count, uint line_length, uint *scroll, STextBlockMode *modes, uint mode_count); /* Draws a block of mono spaced text. */

typedef struct{
	float pos_x; /* The x position of the text block. */
	float pos_y; /* The y position of the text block. */
	float line_size; /* The horizontal size of the text block. */
	float height; /* The verticalo size of the text block. */
	float line_spacing; /* The distance between text lines. */
	STextBlockAlignmentStyle style; /* the  */
}STextBox;

extern STypeInState seduce_text_box_edit(BInputState *input, void *id, char *text, uint buffer_size, STextBox *boxes, uint box_count, STextBlockMode *modes, uint mode_count); /* Alows you to make one or more boxes of text editable. */
extern STypeInState seduce_text_monospace_edit(BInputState *input, void *id, char *text, uint buffer_size, void *font, float pos_x, float pos_y, float character_size, float space_size, float line_size,  uint line_count, uint line_length, uint *scroll, STextBlockMode *modes, uint mode_count);/* Alows you to make one or more boxes of monospace text editable. */

extern boolean	seduce_text_button(BInputState *input, void *id, float pos_x, float pos_y, float center, float size, float spacing, const char *text, float red, float green, float blue, float alpha, float red_select, float green_select, float blue_select, float alpha_select); /* A simple clickable text button. */
extern uint		seduce_text_button_list(BInputState *input, void *id, float pos_x, float pos_y, float length, STextBlockAlignmentStyle style, float size, float spacing, float line_size, const char **texts, uint text_count, uint selected, float red, float green, float blue, float alpha, float red_select, float green_select, float blue_select, float alpha_select); /* A horizontal list of text buttons. "selected" will be highlited. Returns the number of the text beeing clicked. returnes -1 by default. */

/*------- Tool tips  -------
Adds tool tips to any UI element. */

extern void		seduce_tool_tip(BInputState *input, void *id, char *tooltip, char *extended); /* Adds a tool tip to a surface with a specific id. */
extern void		seduce_tool_tip_settings(float text_size, float red, float green, float blue, float alpha); /* set the size and color of tooltips */

/*------- Language translation -------
Seduce has a simple translation system that translates text strings. It automaticly builds a database of all strings and creates checsums so that they can easily be replaced for other languages.
*/

extern char		*seduce_translate(char *text); /* Translates a string, and logs the original string in the database. */
extern void		seduce_translate_load(char *file); /* Loads a translation file containing string translations. */
extern void		seduce_translate_save(char *file); /* Saves a translation file containing string translations. */

/*------- Popup Menus -------
Seduce has a 
*/

boolean seduce_popup_detect_mouse(BInputState *input, void *id, uint button, void (*func)(BInputState *input, float time, void *user), void *user);
boolean seduce_popup_detect_multitouch(BInputState *input, void *id, uint finger_count, void (*func)(BInputState *input, float time, void *user), void *user);
boolean seduce_popup_detect_axis(BInputState *input, uint button, void (*func)(BInputState *input, float time, void *user), void *user);
STypeInState seduce_popup_detect_icon(BInputState *input, void *id, uint icon, float pos_x, float pos_y,  float scale, float time, void (*func)(BInputState *input, float time, void *user), void *user, boolean displace, float *color);
STypeInState seduce_popup_detect_text(BInputState *input, void *id, float pos_x, float pos_y, float center, float size, float spacing, const char *text, void (*func)(BInputState *input, float time, void *user), void *user, boolean displace, float red, float green, float blue, float alpha, float red_select, float green_select, float blue_select, float alpha_select);
STypeInState seduce_popup_detect_button(BInputState *input, void *id, float pos_x, float pos_y, void (*func)(BInputState *input, float time, void *user), void *user, boolean displace);

#define SEDUCE_POP_UP_NO_ACTION -1
#define SEDUCE_POP_UP_DEACTIVATE -2 

typedef enum{
	S_PUT_TOP,
	S_PUT_BOTTOM,
	S_PUT_ANGLE,
	S_PUT_BUTTON,
	S_PUT_IMAGE,
	S_PUT_COUNT
}SPopUpType;

typedef struct{
	SPopUpType	type;
	char		*text;
	union{
		float button_pos[2];
		float angle[2];
		struct{
			float pos[2];
			float size[2];
			uint texture_id;
		}image;
		struct{
			float angle[2];
			float *value;
		}slider_angle;
	}data;
}SUIPUElement;

extern uint seduce_popup_simple(BInputState *input, uint user_id, float pos_x, float pos_y, char **lables, uint element_count, float *time, boolean active, float red, float green, float blue, float red_active, float green_active, float blue_active);
extern uint seduce_popup(BInputState *input, void *id, SUIPUElement *elements, uint element_count, float time);
extern uint seduce_popup_text(BInputState *input, void *id, uint *selected, char **lables, uint element_count, SPopUpType type, float pos_x, float pos_y, float center, float size, float spacing, const char *text, float red, float green, float blue, float alpha, float red_select, float green_select, float blue_select, float alpha_select, boolean release_only);



/*------- Draw icons -------
Seduce has a number of built in icons that can bes used for byttons and other widgets. (These are most likly going to be desesigned to support HxA in the not too distant future.)
*/

extern void seduce_object_3d_draw(BInputState *input, float pos_x, float pos_y, float pos_z, float size, uint id, float fade, float *color); /* draws a single object. */
extern uint seduce_object_3d_count(); /* Returns the number of objects loaded. */
extern char *seduce_object_3d_object_name(uint object); /* returns the name of the object coresponding to an id. */
extern uint seduce_object_3d_object_lookup(char *name); /* Looks up a object id form a name. */

/*------- Widgets -------
Some basic widgets.
*/

extern boolean seduce_widget_button_icon(BInputState *input, void *id, uint icon, float pos_x, float pos_y,  float scale, float time, float *color); /* Draws a button in the form of an icon. */
extern boolean seduce_widget_button_invisible(BInputState *input, void *id, float pos_x, float pos_y, float scale, boolean down_click); /* Creates an invisble button. */
extern boolean seduce_widget_toggle_icon(BInputState *input, void *id, boolean *value, uint icon, float pos_x, float pos_y, float scale, float time); /* Creates an icon button that is a toggle. */
extern STypeInState seduce_widget_slider_line(BInputState *input, void *id, float *value, float pos_a_x, float pos_a_y, float pos_b_x, float pos_b_y, float size, float scale, float min, float max, float time, float *color, boolean snaps); /* Creates a linear slider between two points. The snaps option enables rulers for precise values. */
extern STypeInState seduce_widget_slider_radial(BInputState *input, void *id, float *value, float pos_x, float pos_y, float size, float scale, float min, float max, float time, float *color); /* Creates a radial slider. */ 
extern STypeInState seduce_widget_slider_radius(BInputState *input, void *id, float *value, float pos_x, float pos_y, float size, float time, float *color); /* Creates a radius slider. (Useful for things like brush sizes.) */
extern STypeInState seduce_widget_slider_square(BInputState *input, void *id, float *values, float pos_x, float pos_y, float size_x, float size_y, float size, float scale, float time, float *color);
extern STypeInState seduce_widget_color_wheel_radial(BInputState *input, void *id, float *color, float pos_x, float pos_y, float size, float scale, float time); /* Cretes a color wheel with Hue and Saturation */
extern STypeInState seduce_widget_color_triangle_radial(BInputState *input, void *id, float *color, float pos_x, float pos_y, float size, float scale, float time); /* Creates a color trangle with Value and saturation. */
extern STypeInState seduce_widget_color_square_radial(BInputState *input, void *id, float *color, uint component, float pos_x, float pos_y, float size, float scale, float time); /* creates a color square that is an intersection of the RGB cube at either R, G or B component.*/
extern STypeInState seduce_widget_select_radial(BInputState *input, void *id, uint *selected, char **lables, uint element_count, SPopUpType type, float pos_x, float pos_y, float size, float scale, float time, boolean release_only); /* creates a multi option popup menu. */


/*------- Panels -------
Panels are 
*/

typedef enum{
	SEDUCE_PET_BOOLEAN,
	SEDUCE_PET_TRIGGER,
	SEDUCE_PET_INTEGER,
	SEDUCE_PET_UNSIGNED_INTEGER,
	SEDUCE_PET_INTEGER_BOUND,
	SEDUCE_PET_REAL,
	SEDUCE_PET_REAL_BOUND,
	SEDUCE_PET_RADIUS,
	SEDUCE_PET_2D_POS,
	SEDUCE_PET_3D_POS,
	SEDUCE_PET_4D_POS,
	SEDUCE_PET_QUATERNION,
	SEDUCE_PET_2D_NORMAL,
	SEDUCE_PET_3D_NORMAL,
	SEDUCE_PET_2X2MATRIX,
	SEDUCE_PET_3X3MATRIX,
	SEDUCE_PET_4X4MATRIX,
	SEDUCE_PET_TEXT,
	SEDUCE_PET_PASSWORD,
	SEDUCE_PET_TEXT_BUFFER,
	SEDUCE_PET_COLOR_RGB,
	SEDUCE_PET_COLOR_RGBA,
	SEDUCE_PET_TIME,
	SEDUCE_PET_DATE,
	SEDUCE_PET_SELECT,
	SEDUCE_PET_POPUP,
	SEDUCE_PET_IMAGE,
	SEDUCE_PET_SECTION_START,
	SEDUCE_PET_SECTION_END,
	SEDUCE_PET_CUSTOM,
	SEDUCE_PET_OK_CANCEL,
	SEDUCE_PET_COUNT
}SeducePanelElementType;

typedef enum{
	SEDUCE_PEOCS_UNDECIDED,
	SEDUCE_PEOCS_OK,
	SEDUCE_PEOCS_CANCEL,
	SEDUCE_PEOCS_COUNT
}SeducePanelElementOKCancelState;

typedef struct{
	SeducePanelElementType type;
	char *text;
	char *description;
	union{
		boolean active; 
		boolean trigger; 
		int		integer;
		uint	uinteger;
		struct{
			double value;
			double max;
			double min;
		}real;
		float color[4];
		double vector[4];
		double matrix[16];
		char text[64];
		struct{
			char	*text_buffer;
			uint	buffer_size;
		}buffer;
		double time; /* Seconds in a day */
		struct{
			uint16	year;
			uint8	month;
			uint8	day;
		}date;
		struct{
			char	**text;
			uint	count;
			uint	active;
			SPopUpType style;
		}select;
		struct{
			uint icon;
			void (*func)(BInputState *input, float time, void *user);
			void *user;
			boolean displace;
		}popup;
		struct{
			void *id;
			uint texture_id;
			float aspect;
		}image;
		struct{
			boolean fill;
			float aspect;
			void *user;
			void (*function)(BInputState *input, float pos_x, float pos_y, float width, void *user);
		}custom;
		SeducePanelElementOKCancelState ok_cancel;
	}param;
}SeducePanelElement;

typedef enum{
	SEDUCE_WLS_PANEL,
	SEDUCE_WLS_COMPACT,
	SEDUCE_WLS_COUNT
}SeduceWidgetListStyle;

extern uint seduce_widget_list(BInputState *input, void *id, SeducePanelElement *elements, uint element_count, float time, char *title, float *color, float *background_color, SeduceWidgetListStyle style);

extern float seduce_widget_list_element_background(BInputState *input, float pos_x, float pos_y, float width, float scale, SeducePanelElement *element, uint element_count, uint *selected_element, void *id, float time);
extern void seduce_widget_list_element_list(BInputState *input, float pos_x, float pos_y, float width, float scale, SeducePanelElement *element, uint element_count, uint *selected_element, void *id, float time);

extern void seduce_settings_betray_set(SeducePanelElement *element);
extern void seduce_settings_betray_get(SeducePanelElement *element);


/*------- Surface -------
*/
#ifndef SEDUCE_INTERNAL_BACKGROUND
typedef void SeduceBackgroundObject;
#endif

extern SeduceBackgroundObject *seduce_background_object_allocate();
extern void seduce_primitive_background_object_free(SeduceBackgroundObject *object);
extern void seduce_background_shadow_add(SeduceBackgroundObject *object, float *list, uint count, boolean closed, float size);
extern void seduce_background_shadow_square_add(SeduceBackgroundObject *object, float pos_x, float pos_y, float size_x, float size_y, float size);
extern void seduce_background_shape_add(SeduceBackgroundObject *object, void *id, uint part, float *list, uint count, float surface_r, float surface_g, float surface_b, float surface_a);
extern void seduce_background_quad_add(SeduceBackgroundObject *object, void *id, uint part,
									float a_x, float a_y, float a_z, 
									float b_x, float b_y, float b_z, 
									float c_x, float c_y, float c_z, 
									float d_x, float d_y, float d_z, 
									float surface_r, float surface_g, float surface_b, float surface_a);
extern void seduce_background_square_add(SeduceBackgroundObject *object, void *id, uint part,
									float pos_x, float pos_y, float pos_z, 
									float size_x, float size_y,
									float surface_r, float surface_g, float surface_b, float surface_a);

extern void seduce_background_tri_add(SeduceBackgroundObject *object, void *id, uint part,
									float a_x, float a_y, float a_z, 
									float b_x, float b_y, float b_z, 
									float c_x, float c_y, float c_z, 
									float surface_r, float surface_g, float surface_b, float surface_a);
extern void seduce_background_tri_fade_add(SeduceBackgroundObject *object, void *id, uint part,
									float a_x, float a_y, float a_z, 
									float b_x, float b_y, float b_z, 
									float c_x, float c_y, float c_z, 
									float a_surface_r, float a_surface_g, float a_surface_b, float a_surface_a,
									float b_surface_r, float b_surface_g, float b_surface_b, float b_surface_a,
									float c_surface_r, float c_surface_g, float c_surface_b, float c_surface_a);
extern void seduce_primitive_surface_draw(BInputState *input, SeduceBackgroundObject *object, float time);
extern void seduce_surface_circle_spawn(BInputState *input, float x, float y, float size, void *id, uint part, float red, float green, float blue);
extern void seduce_primitive_background_flare_draw(float x, float y, float z, float time, float size);
extern void seduce_background_image_add(SeduceBackgroundObject *object, void *id, uint part, uint texture_id, 
									float a_x, float a_y, 
									float b_x, float b_y, 
									float c_x, float c_y, 
									float d_x, float d_y, 
									float origo_x, float origo_y, float scale_x, float scale_y);
/*------- Lens shader -------
Draws the Lens shader used for all Quel Solaar logos.
*/
extern RShader *seduce_lens_shader_get(); /* returns the RShader that can be used to draw the shader in arbitrary geometry. */
extern void seduce_lens_draw(float color_a_red, float color_a_green, float color_a_blue, float color_b_red, float color_b_green, float color_b_blue, float color_c_red, float color_c_green, float color_c_blue); /* Draws a round lens. */

/*------- Galleries -------
Galleries fills a rectangular shape with images. The calls either defines the vertical or horizontal size of the rectangle, and image size, and the funtion will compute the size of the rectangle.
*/

extern uint seduce_background_gallery_vertical_draw(BInputState *input, float pos_x, float pos_y, float pos_z, float size_x, float *size_y, float image_height, uint *texture_id, float *aspects, uint image_count, float timer, float *center);
extern uint seduce_background_gallery_horizontal_draw(BInputState *input, float pos_x, float pos_y, float pos_z, float *size_x, float size_y, float image_height, uint *texture_id, float *aspects, uint image_count, float timer, float *center);


/*------- Depricated Surface -------
These functions for drawing surfaces have been deprecated and will be removed.
*/

extern boolean seduce_background_tri_draw(BInputState *input, void *id, uint part,
														float a_x, float a_y, float a_z, 
														float b_x, float b_y, float b_z, 
														float c_x, float c_y, float c_z, 
														float normal_x, float normal_y, float particle_influence,
														float surface_r, float surface_g, float surface_b, float surface_a);

extern boolean seduce_background_quad_draw(BInputState *input, void *id, uint part,
														float a_x, float a_y, float a_z, 
														float b_x, float b_y, float b_z, 
														float c_x, float c_y, float c_z, 
														float d_x, float d_y, float d_z, 
														float normal_x, float normal_y, float particle_influence,
														float surface_r, float surface_g, float surface_b, float surface_a);

extern void seduce_background_polygon_flush(BInputState *input, float *center, float time);

 

extern boolean seduce_background_image_draw(BInputState *input, void *id, float pos_x, float pos_y, float pos_z, float size_x, float size_y, float u_start, float v_start, float u_size, float v_size, float timer, float *center, uint texture_id);

typedef enum{
    S_PT_SPLAT_ONE,
    S_PT_SPLAT_TWO,
    S_PT_SPLAT_THREE,
    S_PT_SPLAT_FOUR,
    S_PT_LIGHT,
    S_PT_CLICK
}SPraticleType;

extern SPraticleType seduce_background_particle_color_allocate(void *id, float red, float green, float blue);

extern void seduce_background_particle_spawn(BInputState *input, float pos_x, float pos_y, float vec_x, float vec_y, float start_age, SPraticleType type);
extern void seduce_background_particle_burst(BInputState *input, float pos_x, float pos_y, uint count, float speed, SPraticleType type);
extern void seduce_background_particle_square(BInputState *input, float pos_x, float pos_y, float size_x, float size_y, uint count, SPraticleType type);


extern void seduce_background_color(float surface_r, float surface_g, float surface_b, float surface_a, 
									 float light_r, float light_g, float light_b, float light_a, 
									 float shade_r, float shade_g, float shade_b, float shade_a);

extern void		seduce_background_negative_draw(float a_x, float a_y, float b_x, float b_y, float c_x, float c_y, float d_x, float d_y, float timer);
extern boolean	seduce_background_square_draw(BInputState *input, void *id, float pos_x, float pos_y, float size_x, float size_y, float split, float tilt, float timer);
extern boolean	seduce_background_shape_draw(BInputState *input, void *id, float a_x, float a_y, float b_x, float b_y, float c_x, float c_y, float d_x, float d_y, float timer, float normal_x, float normal_y, float *center, float transparancy);
extern boolean	seduce_background_shape_matrix_interact(BInputState *input, void *id, float *matrix, boolean scale, boolean rotate);
extern void seduce_draw_circle(float x, float y, float z, float x_normal, float y_normal, float z_normal, float radius, float start, float end, float color_r, float color_g, float color_b, float color_a, float timer);

/*------- Drawing lines -------
The line drawing API draws Depth of field wireframe lines that can be used to build UI elements. 
Lines can either be drawn in retained or immediate mode. You can allocate a line object, then 
load lines in to it and then draw it one ore more times. Once a line object has been drawn, you 
can no longer add lines or modefy existing lines in the object. You can also draw lines in immediate
 mode by simply giving NULL as the object argument. In order to flush and draw the lines, a 
 seduce_primitive_line_draw needs to be drawn with NULL as object. 
*/

typedef void SeduceLineObject; /* An line object handle. */

extern SeduceLineObject *seduce_primitive_line_object_allocate(); /* Allocates a line object. */
extern void seduce_primitive_line_object_free(SeduceLineObject *object); /* Frees a line object. */ 
extern void seduce_primitive_line_draw(SeduceLineObject *object, float red, float green, float blue, float alpha); /* draws a line object. The color of all lines in the object will be multiplied by the collor given to the call. */
extern void seduce_primitive_line_focal_depth_set(float distance); /* Sets the focal depth the lines beein drawn. */
extern void seduce_primitive_line_color_set(float red_a, float green_a, float blue_a, 
											float red_b, float green_b, float blue_b, 
											float red_c, float green_c, float blue_c); 
extern void seduce_primitive_line_animation_set(float scroll, float add, float speed, float timer, float color_brightness);
extern void seduce_primitive_line_add_3d(SeduceLineObject *object,
							float pos_a_x, float pos_a_y, float pos_a_z,
							float pos_b_x, float pos_b_y, float pos_b_z,
							float red_a, float green_a, float blue_a, float alpha_a,
							float red_b, float green_b, float blue_b, float alpha_b);
extern void seduce_primitive_circle_add_3d(SeduceLineObject *object,
							float center_x, float center_y, float center_z,
							float vec_x, float vec_y, float vec_z,
							float normal_x, float normal_y, float normal_z,
							float radius,
							float start, float end,
							float start_u, float length_u,
							float red_a, float green_a, float blue_a, float alpha_a,
							float red_b, float green_b, float blue_b, float alpha_b);
extern void seduce_primitive_spline_add_3d(SeduceLineObject *object,
							float cv_a_x, float cv_a_y, float cv_a_z,
							float cv_b_x, float cv_b_y, float cv_b_z,
							float cv_c_x, float cv_c_y, float cv_c_z,
							float cv_d_x, float cv_d_y, float cv_d_z,
							float start_u, float length_u,
							float red_a, float green_a, float blue_a, float alpha_a,
							float red_b, float green_b, float blue_b, float alpha_b);
extern void seduce_primitive_color_wheel_add_3d(SeduceLineObject *object,
							float center_x, float center_y, float center_z,
							float radius);

/*------- View and projection code -------
*/

typedef enum{
	S_VIS_LINEAR,
	S_VIS_EASE_IN,
	S_VIS_EASE_OUT,
	S_VIS_SMOOTH,
	S_VIS_CUT,
	S_VIS_COUNT
}SViewInterpolationStyle;


typedef struct{
	float matrix[16];
	float model[16];
	float target[3];
	float camera[3];
	float distance;
	uint type;
	float speed;
	SViewInterpolationStyle interpolation_style;
	union{
		struct{
			float			target_target[3];
			float			target_camera[3];
			float			target_up[3];
			float			progress;
		}camera;
		struct{
			float			delta_rot[2];
		}orbit;
		struct{
			float			current_rot[2];
			float			delta_rot[2];
			boolean			slide;
		}rotate;
		struct{
			float			delta_pan[2];
		}pan;
		struct{
			float			delta_distance;
		}distance;
		struct{
			float			target_target[3];
			float			target_camera[3];
			float			target_up[3];
			float			progress;
		}scroll;
		struct{
			float			pitch;
			float			pitch_target;
			float			yaw;
			float			yaw_target;
			float			target[3];
			float			position[3];
			float			speed;
			float			distance_target;
			float			grid_size;
			float			direction;
			float			grab[2];
			uint			view_axis;
			uint			axis;
		}right_button;
	}data;
}SViewData;

extern void		seduce_view_init(SViewData *v);
extern void		seduce_view_set(SViewData *v, RMatrix *m);
extern void		seduce_view_update(SViewData *v, float delta);
extern void		seduce_view_slide(SViewData *v, boolean slide);
extern void		seduce_view_interpolation_style_set(SViewData *v, SViewInterpolationStyle style, float speed);

extern void		seduce_view_change_look_at(SViewData *v, float *target, float *camera, float *up);
extern boolean 	seduce_view_change_right_button(SViewData *v, BInputState *input);
extern void		seduce_view_change_mouse_look(SViewData *v, BInputState *input);
extern boolean 	seduce_view_change_multi_touch(SViewData *v, BInputState *input, void *id);
extern boolean 	seduce_view_change_keys(SViewData *v, BInputState *input, void *id);
extern boolean	seduce_view_change_scroll_wheel(SViewData *v, BInputState *input);

extern void		seduce_view_change_axised_orbit(SViewData *v, BInputState *input);
extern void		seduce_view_change_spin_orbit(SViewData *v, BInputState *input);
extern void		seduce_view_change_spin_axised_orbit(SViewData *v, BInputState *input);

extern void		seduce_view_change_orbit(SViewData *v, BInputState *input, uint button, boolean slide);
extern void		seduce_view_change_orbit_delta(SViewData *v, BInputState *input, uint button, boolean slide);
extern void		seduce_view_orbit(SViewData *v, float x, float y);

extern void		seduce_view_change_rotate(SViewData *v, BInputState *input, uint button, boolean slide);
extern void		seduce_view_change_rotate_delta(SViewData *v, BInputState *input, uint button, boolean slide);
extern void		seduce_view_rotate(SViewData *v, BInputState *input, float x, float y);

extern void		seduce_view_change_pan(SViewData *v, BInputState *input, uint button, boolean slide);
extern void		seduce_view_change_pan_delta(SViewData *v, BInputState *input, uint button, boolean slide);
extern void		seduce_view_pan(SViewData *v, float x, float y);
extern void		seduce_view_pan_to_set(SViewData *v, float position_x, float position_y, float position_z);

extern void		seduce_view_change_distance(SViewData *v, BInputState *input, uint button, boolean slide);
extern void		seduce_view_change_distance_delta(SViewData *v, BInputState *input, uint button, boolean slide);
extern void		seduce_view_change_distance_scroll(SViewData *v, BInputState *input);
extern void		seduce_view_distance(SViewData *v, float x, float y);
extern void		seduce_view_distance_to_set(SViewData *v, float distance);


extern void		seduce_view_change_distance_scroll_zoom(SViewData *v, BInputState *input);


extern void		seduce_view_center_set(SViewData *v, float position_x, float position_y, float position_z);
extern void		seduce_view_camera_set(SViewData *v, float position_x, float position_y, float position_z);
extern void		seduce_view_up_set(SViewData *v, float up_x, float up_y, float up_z);
extern void		seduce_view_pan_to_set(SViewData *v, float position_x, float position_y, float position_z);
extern void		seduce_view_direction_set(SViewData *v, float normal_x, float normal_y, float normal_z);


extern void		seduce_view_change_zoom(SViewData *v, float delta_x, float delta_y);
extern void		seduce_view_center_getf(SViewData *v, float *center);
extern void		seduce_view_center_getd(SViewData *v, double *center);
extern void		seduce_view_distance_camera_set(SViewData *v, double distance);
extern double	seduce_view_distance_camera_get(SViewData *v);

extern void		seduce_view_camera_getf(SViewData *v, float *camera);
extern void		seduce_view_camera_getd(SViewData *v, double *camera);
extern void		seduce_view_camera_vector_getf(SViewData *v, float *camera, float x, float y);
extern void		seduce_view_camera_vector_getd(SViewData *v, double *camera, double x, double y);
extern void		seduce_view_grid_size_set(SViewData *v, double grid_size);

extern void		seduce_view_model_matrixd(SViewData *v, double *matrix);
extern void		seduce_view_model_matrixf(SViewData *v, float *matrix);
extern void		seduce_view_sprite_matrixf(SViewData *v, float *matrix);

extern void		seduce_view_projectiond(SViewData *v, double *output, double x, double y);
extern void		seduce_view_projectionf(SViewData *v, float *output, float x, float y);
extern void		seduce_view_projection_vertexf(SViewData *v, float *output, float *vertex, float x, float y);
extern void		seduce_view_projection_vertexd(SViewData *v, double *output, double *vertex, double x, double y);
extern void		seduce_view_projection_screend(SViewData *v, double *output, double x, double y, double z);
extern void		seduce_view_projection_screenf(SViewData *v, float *output, float x, float y, float z);
extern double	seduce_view_projection_screen_distanced(SViewData *v, double space_x, double space_y, double space_z, double screen_x, double screen_y);
extern void		seduce_view_projection_planed(SViewData *v, double *dist, uint axis, double pointer_x, double pointer_y , double depth);
extern double	seduce_view_projection_lined(SViewData *v, double *dist, uint axis, double pointer_x, double pointer_y, double *pos);

/*------- Select lasso -------
*/

STypeInState seduce_select_draw(BInputState *input, void *id, uint user, float red, float green, float blue);
boolean seduce_select_test(float x, float y, float *pos_a, float *pos_b);
boolean seduce_select_query_pos(BInputState *input, uint user_id, float x, float y);
boolean seduce_select_query_id(BInputState *input, uint user_id, void *id);

/*------- Manipulator -------
*/

extern STypeInState	seduce_manipulator_point(BInputState *input, RMatrix *m, float *pos, void *id, float scale); /* Creates a manipulator for moving a point in space. */
extern STypeInState	seduce_manipulator_pos_xyz(BInputState *input, RMatrix *m, float *pos, void *id, float *snap, boolean snap_active, boolean x, boolean y, boolean z, float scale, float time); /* Creates a manipulator for moving a point in space using 3 separate handles representing the X, Y and Z axis. */
extern STypeInState	seduce_manipulator_point_plane(BInputState *input, RMatrix *m, float *pos, void *id, float *snap, boolean snap_active, uint axis, float scale); /* Creates a manipulator for moving a point in 2 out of the 3 dimentions.*/
extern STypeInState	seduce_manipulator_point_axis(BInputState *input, RMatrix *m, float *pos, void *id, float *snap, boolean snap_active, uint axis, float scale);/* Creates a manipulator for moving a point along one axis in 3 dimentions.*/
extern STypeInState	seduce_manipulator_point_vector(BInputState *input, RMatrix *m, float *pos, void *id, float *snap, boolean snap_active, float *vector, float scale);/* Creates a manipulator for moving a point along a vector in 3 dimentions.*/

extern STypeInState	seduce_manipulator_normal(BInputState *input, RMatrix *m, float *pos, float *normal, void *id, float scale, float time); /* Creates a manipulator for defining a normalized vector in 3D space. */
extern STypeInState	seduce_manipulator_radius(BInputState *input, RMatrix *m, float *pos, float *radius, void *id, float time);/* Creates a manipulator for defining a radius in 3D space. */
extern STypeInState	seduce_manipulator_scale(BInputState *input, RMatrix *m, float *pos, float *size, void *id, float *snap, boolean snap_active, boolean x, boolean y, boolean z, float scale, float time);/* Creates a manipulator for scaling 3 size values in 1, 2 or 3 axis. */
extern STypeInState	seduce_manipulator_rotate(BInputState *input, RMatrix *matrix, float *pos, float *rotation_matrix, void *id, float *snap, boolean snap_active, float scale, float time);/* Creates a manipulator for 3D rotation of a 4x4 matrix. */

extern boolean	seduce_manipulator_square_centered(BInputState *input, RMatrix *m, float *pos, float *size, void *id, float *snap, boolean snap_active, boolean aspect_lock, float scale, float time); /* Creates a set of manipulators for scaling a rectangle, using edge and corner handles. */
extern boolean	seduce_manipulator_square_cornered(BInputState *input, RMatrix *m, float *down_left, float *up_right, void *id, float *snap, boolean snap_active, boolean aspect_lock, float scale, float time);/* Creates a set of manipulators for scaling and moving a rectangle, using edge and corner handles. */

extern boolean	seduce_manipulator_cube_centered(BInputState *input, RMatrix *m, float *pos, float *size, void *id, float *snap, boolean snap_active, float scale, float time); /* Creates a set of manipulators for scaling a cube, using edge, surface and corner handles. */
extern boolean	seduce_manipulator_cube_cornered(BInputState *input, RMatrix *m, float *min, float *max, void *id, float *snap, boolean snap_active, float scale, float time); /* Creates a set of manipulators for scaling and moving a cube, using edge, surface and corner handles. */

extern STypeInState	seduce_manipulator_slider(BInputState *input, void *id, float *scroll, float *pos_a, float *pos_b, float full_length, float section_length, float size, float red, float green, float blue);/* Creates slider between 2 points in 3d space. */

/*------- Colission elements -------
Seduce uses an id based colission system. In order to build custom widgets that use the same colission system, seduce provides a set of functionality for using it. When execution is set to BAM_DRAW, functions can be called to create coliders. Later execution is set to BAM_EVENT the colission system can be queried to test if an element is accessabel.
*/

extern void seduce_element_add_point(BInputState *input, void *id, uint part, float *pos, float size); /* Adds a point shaped element. */
extern void seduce_element_add_line(BInputState *input, void *id, uint part, float *a, float *b, float size);/* Adds a line segment shaped element. */
extern void seduce_element_add_triangle(BInputState *input, void *id, uint part, float *a, float *b, float *c);/* Adds a triangle shaped element. */
extern void seduce_element_add_quad(BInputState *input, void *id, uint part, float *a, float *b, float *c, float *d);/* Adds a quad shaped element. */
extern void seduce_element_add_rectangle(BInputState *input, void *id, uint part, float pos_x, float pos_y, float size_x, float size_y);/* Adds an axis aligned rectangle shaped element. */
extern void seduce_element_add_surface(BInputState *input, void *id); /* creates alocal surface at a matrix space that can be used for local interactions. */

extern void *seduce_element_colission_test(float *pos, uint *part, uint user_id);
extern void *seduce_element_pointer_id(BInputState *input, uint pointer, uint *part); /* Returns the id and part colliding with a specific pointer */
extern void *seduce_element_pointer_down_click_id(BInputState *input, uint pointer, uint *part); /* Returns the id and part that a specific pointer  was coliding with last time it was activated. */
extern uint seduce_element_pointer(BInputState *input, void *id, uint *part); /* return the id of a pointer colliding with an id / part. */
extern void *seduce_element_selected_id(uint user_id, float *pos, uint *part); /* return the selected if of a specific user id. */
extern uint seduce_element_primary_axis(BInputState *input, uint user_id); /* retuyrns the primary axis used by a specific user for non pointer based interaction with UI emelents. */
extern boolean seduce_element_surface_project(BInputState *input, void *id, float *output, uint axis, float pointer_x, float pointer_y); /* transforma an xy coordinate in to the local space of an id. */
extern boolean seduce_element_active(BInputState *input, void *id, void *part); /* Returns TRUE or FALSE is a specific id / part is active. */

extern boolean seduce_element_position_get(void *id, float *pos); /* finds the location of an id and sets pos to it. returns TRUE if sucsessful, or FALSE if it fails.*/

extern void seduce_element_user_exclusive_begin(uint user_id); /* Begins the creation of elements, that are only acessable to a single users input. (useful for popup menus)*/
extern void seduce_element_user_exclusive_end(); /* Ends the exclusivity set by seduce_element_user_exclusive_begin. */
#endif
