#define OBSERVATORY_NAME_SIZE 64

typedef struct{
	char platform[64];
	char build[64];
}ObsBuild;

typedef enum{
	OBSERVER_TDT_HEADLINE,
	OBSERVER_TDT_TODO,
	OBSERVER_TDT_DONE,
	OBSERVER_TDT_COUNT
}ObsToDoType; 

typedef struct{
	ObsToDoType type;
	char *string;
	boolean grouped;
	uint date[3];
}ObsToDo;

typedef struct{
	char name[64];
	char header[64];
	char owner[64];
	char release_define[64];
	char todo[64];
	char help_string[64];
	char *descrition;
	char **prefixes;
	uint prefix_count;
	char **assets;
	uint asset_count;
	ObsBuild *builds;
	uint build_count;
	uint color[3];
	ObsToDo *todo_parsed;
	uint todo_count;
	void *release_array;
	uint documented;
	uint undocumented;
	AJsonValue *log_array;
}ObsModule;

typedef struct{
	size_t size;
	uint64 time;
	uint file_start;
	char path[1024];
	char *file;
}ObsFile;

typedef enum{
	OBSERVATORY_IT_DEFINE,
	OBSERVATORY_IT_STRUCT,
	OBSERVATORY_IT_ENUM,
	OBSERVATORY_IT_FUNCTION,
	OBSERVATORY_IT_GROUP,
	OBSERVATORY_IT_MEMBER,
	OBSERVATORY_IT_COUNT
}ObsItemType;

typedef struct{
	char *file;
	char *source;
	ObsItemType type;
	char *comment;
	char name[OBSERVATORY_NAME_SIZE];
}ObsItem;

typedef struct{
	char *name;
	char **source_paths;
	uint source_path_count;
	ObsFile *source_files;
	uint source_file_count;
	char **builds_paths;
	uint builds_path_count;
	ObsFile *build_files;
	uint build_file_count;
	ObsModule *modules;
	uint module_count;
	uint header_count;
	ObsItem *items;
	uint item_count;
	AJsonValue *release_root;
	AJsonValue *log_root;
}ObsState;


/* observatory_files.c  */

extern ObsFile *observatory_files_gather(ObsFile *file, uint *count, char *path);
extern char *observatory_files_read(ObsState *state, char *name, size_t *size);
extern void observatory_files_dependency_list(FILE *f, ObsState *state, uint module_id);
extern void observatory_files_module_file_list(FILE *f, ObsState *state, uint module_id);
extern boolean oservatory_files_file_in_module(ObsState *state, uint module_id, ObsFile *file);
extern void observatory_files_help(FILE *f, ObsState *state, uint module_id);
extern void observatory_files_documentation(FILE *f, ObsState *state);
extern void observatory_files_module_make_file(ObsState *state, uint module_id);

/* observatory_headers.c - generates documentation form C header files. */

extern uint observatory_headers_parce_file(ObsState *settings, uint module_id, ObsItem *items);
extern void observatory_headers_generate_html(char *path, ObsItem *items, uint count, ObsModule *modules, uint modules_count);
extern void observatory_headers_menu(FILE *f, ObsItem *items, uint item_count, uint active_module, ObsModule *modules, uint module_count);
extern void observatory_headers_print_text(FILE *f, char *text, ObsItem *items, uint item_count);
extern void observatory_headers_html_footer(FILE *f);


extern void observatory_header_functions(ObsState *settings, uint module_id);
extern void observatory_header_defines(ObsState *settings, uint module_id);

extern char *observatory_headers_load_file(char *path, uint *size); /* Remove me! */
extern void observatory_headers_module_description(FILE *f, ObsItem *items, uint count, ObsModule *modules, uint modules_count, uint active_module);
extern void observatory_headers_module_index(FILE *f, ObsItem *items, uint count, ObsModule *modules, uint modules_count, uint active_module);


/* observatoryn_todo.c - parse todo lists. */

extern boolean observatory_todo_parse(ObsState *settings, uint module_id);
extern void observatory_todo_card(FILE *f, ObsState *state, uint module_id, ObsToDoType type);
extern boolean observatory_todo_deadline(FILE *f, ObsToDo *todo, uint *date);
extern void observatory_todo_gen_deadlines(FILE *f, ObsState *state);

/* observatory_html.c - parse todo lists. */


extern void observatory_html_init();
extern void observatory_html_set_path(char *path);
extern void observatory_html_string_to_link(char *copy, char *file_name);
extern FILE *observatory_html_create_page(char *file_name, ObsState *settings, uint active_module, boolean cards);
extern void observatory_html_complete(FILE *f, boolean cards);
extern void observatory_html_set_accent_color(uint red, uint green, uint blue);
extern void observatory_html_card_begin(FILE *f, char *banner);
extern void observatory_html_card_end(FILE *f);
extern void observatory_html_headline(FILE *f, char *headline_one, char *headline_two);
extern void observatory_html_text(FILE *f, char *text);
extern void observatory_html_bullet_point_begin(FILE *f, char *headline);
extern void observatory_html_bullet_point(FILE *f, char *text, char *url, char *hightlight, unsigned char red, unsigned char green, unsigned char blue);
extern void observatory_html_bullet_point_end(FILE *f);
extern void observatory_html_key_value(FILE *f, char *key, char *value, char *url);
extern void observatory_html_test(ObsState *settings);

/* observatory_release.c - parse todo lists. */

extern void observatory_release_load(ObsState *state);
extern void observatory_release_save(ObsState *state);
extern char *observatory_release_extract_module(ObsState *state, uint module_id, char *string);
extern boolean observatory_release_detect(ObsState *state, uint module_id, int64 system_time);
extern boolean observatory_release_list(FILE *f, ObsState *state, uint count);
extern void observatory_release_module_list(FILE *f, ObsState *state, uint module_id);
extern void observatory_release_developer_status(FILE *f, ObsState *state);

/* observatory_work_log.c - parse work logs. */

extern void observatory_work_log_load(ObsState *state, int64 time_stamp);
extern void observatory_work_log_module(FILE *f, ObsState *state, uint module_id);
extern void observatory_work_log(FILE *f, ObsState *state, uint days);