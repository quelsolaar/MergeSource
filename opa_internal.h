

typedef enum{
	OPA_TT_DELETED,
	OPA_TT_SPACE,
	OPA_TT_NAME,
	OPA_TT_OPERATOR,
	OPA_TT_OPERATOR_EQUAL,
	OPA_TT_OPERATOR_ASSIGN,
	OPA_TT_OPERATOR_NOT,
	OPA_TT_OPERATOR_NOT_ASSIGN,
	OPA_TT_OPERATOR_PLUS_PLUS,
	OPA_TT_OPERATOR_PLUS_ASSIGN,
	OPA_TT_OPERATOR_PLUS,
	OPA_TT_OPERATOR_MINUS_MINUS,
	OPA_TT_OPERATOR_MINUS_ASSIGN,
	OPA_TT_OPERATOR_MINUS,
	OPA_TT_OPERATOR_MULTIPLY_ASSIGN,
	OPA_TT_OPERATOR_MULTIPLY,
	OPA_TT_OPERATOR_DIVIDE_ASSIGN,
	OPA_TT_OPERATOR_DIVIDE,
	OPA_TT_OPERATOR_MODULO_ASSIGN,
	OPA_TT_OPERATOR_MODULO,
	OPA_TT_OPERATOR_OR_OR_ASSIGN,
	OPA_TT_OPERATOR_OR_OR,
	OPA_TT_OPERATOR_OR_ASSIGN,
	OPA_TT_OPERATOR_OR,
	OPA_TT_OPERATOR_AND_AND_ASSIGN,
	OPA_TT_OPERATOR_AND_AND,
	OPA_TT_OPERATOR_AND_ASSIGN,
	OPA_TT_OPERATOR_AND,
	OPA_TT_OPERATOR_LEFT_SHIFT_ASSIGN,
	OPA_TT_OPERATOR_LEFT_SHIFT,
	OPA_TT_OPERATOR_LESS_THEN_ASSIGN,
	OPA_TT_OPERATOR_LESS_THEN,
	OPA_TT_OPERATOR_RIGHT_SHIFT,
	OPA_TT_OPERATOR_MORE_THEN_ASSIGN,
	OPA_TT_OPERATOR_MORE_THEN,
	OPA_TT_OPERATOR_XOR_ASSIGN,
	OPA_TT_OPERATOR_XOR,
	OPA_TT_OPERATOR_QUESTION_ASSIGN,
	OPA_TT_OPERATOR_QUESTION,
	OPA_TT_OPERATOR_BITWISE_NOT_ASSIGN,
	OPA_TT_OPERATOR_BITWIZE_NOT,
	OPA_TT_HASHTAG,
	OPA_TT_NEWLINE,
	OPA_TT_SINGLE_CHARACTER_START,
	OPA_TT_SEMICOLON,
	OPA_TT_COLON,
	OPA_TT_COMMA,
	OPA_TT_PERIOD,
	OPA_TT_OPEN_PARENTHESIS,
	OPA_TT_CLOSE_PARENTHESIS,
	OPA_TT_OPEN_SCOPE,
	OPA_TT_CLOSE_SCOPE,
	OPA_TT_OPEN_BRACKET,
	OPA_TT_CLOSE_BRACKET,
	OPA_TT_INTEGER,
	OPA_TT_REAL,
	OPA_TT_KEYWORD_AUTO,
	OPA_TT_KEYWORD_BREAK,
	OPA_TT_KEYWORD_CASE,
	OPA_TT_KEYWORD_CHAR,
	OPA_TT_KEYWORD_CONST,
	OPA_TT_KEYWORD_CONTINUE,
	OPA_TT_KEYWORD_DEFAULT,
	OPA_TT_KEYWORD_DO,
	OPA_TT_KEYWORD_DOUBLE,
	OPA_TT_KEYWORD_ELSE,
	OPA_TT_KEYWORD_ENUM,
	OPA_TT_KEYWORD_EXTERN,
	OPA_TT_KEYWORD_FLOAT,
	OPA_TT_KEYWORD_FOR,
	OPA_TT_KEYWORD_GOTO,
	OPA_TT_KEYWORD_IF,
	OPA_TT_KEYWORD_INT,
	OPA_TT_KEYWORD_LONG,
	OPA_TT_KEYWORD_REGISTER,
	OPA_TT_KEYWORD_RETURN,
	OPA_TT_KEYWORD_SHORT,
	OPA_TT_KEYWORD_SIGNED,
	OPA_TT_KEYWORD_SIZEOF,
	OPA_TT_KEYWORD_STATIC,
	OPA_TT_KEYWORD_STRUCT,
	OPA_TT_KEYWORD_SWITCH,
	OPA_TT_KEYWORD_TYPEDEF,
	OPA_TT_KEYWORD_UNION,
	OPA_TT_KEYWORD_UNSIGNED,
	OPA_TT_KEYWORD_VOID,
	OPA_TT_KEYWORD_VOLATILE,
	OPA_TT_KEYWORD_WHILE
}OPATokenType;

typedef struct{
	OPATokenType type;
	uint line;
	uint start;
	char *text;
	uint length;
	union{
		double real;
		int64 integer;
	}value;
	void *next;
	void *prev;
}OPAToken;


typedef struct{
	OPAToken *tokens;
	uint token_count;
	uint8 *buffer;
}OPATokenFile;

/* -------------------- */

typedef enum{
	OPA_TYPE_SINGED_CHAR,
	OPA_TYPE_UNSINGED_CHAR,
	OPA_TYPE_SIGNED_SHORT,
	OPA_TYPE_UNSIGNED_SHORT,
	OPA_TYPE_SINGED_INT,
	OPA_TYPE_UNSINGED_INT,
	OPA_TYPE_SIGNED_LONG_LONG,
	OPA_TYPE_UNSIGNED_LONG_LONG,
	OPA_TYPE_FLOAT,
	OPA_TYPE_DOUBLE,
	OPA_TYPE_VOID,
	OPA_TYPE_FUNCTION,
	OPA_TYPE_COUNT,	
}OPAType;

#define OPA_COLUMN_MAX_COUNT 256

typedef enum{
	OPA_CD_NONE,
	OPA_CD_PLOT_1D,
	OPA_CD_PLOT_2D,
	OPA_CD_PLOT_3D,
	OPA_CD_COUNT
}OPAColumnDraw;

typedef struct{
	boolean expand;
	boolean show;
	boolean matrix;
	uint columns;
	float width;
	uint8 column_data[OPA_COLUMN_MAX_COUNT];
	uint scroll;
	float min[2];
	float max[2];
	void *manipulator;
	boolean first;
	float down_left[3];
	float up_right[3];
}OPADisplayOptions;

typedef struct{
	char value_name[32];
	uint offset;
	uint indirection;
	uint base_type;
	uint array_length;
	uint enum_value;
	OPADisplayOptions options;
}OPAMember;

typedef enum{
	OPA_C_STRUCT,
	OPA_C_UNION,
	OPA_C_ENUM,
	OPA_C_BASE_TYPE,
	OPA_C_ERROR,
	OPA_C_REFERENCE_START
}OPAConstructType;

typedef struct{
	char type_name[64];
	OPAConstructType construct;
	uint member_count;
	OPAMember *members;
	uint size_of;
	uint alignment;
}OPAConstruct;

typedef struct{
	uint64 pointer;
	uint original_type;
	uint type;
	uint indirection;
	uint8 *data;
	uint data_size;
	float matrix[16];
	uint offset;
	uint parent;
	OPADisplayOptions options;
	char path[256];
	uint line;
	float length;
	boolean paused;
	boolean hidden;
}OPAMemory;

typedef struct{
	OPAConstruct *types;
	uint type_count;
	uint type_allocated;
	char path[1024];
	char *includes;
	uint include_count;
	OPAMemory *memmory;
	uint memory_count;
	uint memory_allocated;
	uint update_current;
}OPAProject;

extern char *opa_preprosessor(char *file_name);
extern void opa_parse(OPAProject *project, char *path);
extern char *opa_file_load(char *file);
extern void opa_tokenizer_test();
extern void opa_tokenizer(OPATokenFile *tokens, uint8 *buffer, uint buffer_size);