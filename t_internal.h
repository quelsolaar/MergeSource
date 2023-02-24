
#define TESTIFY_MINIMUM_WRITE_SPACE 1024

typedef enum{
	T_TYPE_UINT8,
	T_TYPE_INT8,
	T_TYPE_UINT16,
	T_TYPE_INT16,
	T_TYPE_UINT32,
	T_TYPE_INT32,
	T_TYPE_UINT64,
	T_TYPE_INT64,
	T_TYPE_REAL32,
	T_TYPE_REAL64,
	T_TYPE_RAW,
	T_TYPE_STRING,
	T_TYPE_STRUCT,
	T_TYPE_COUNT
}UTypes;

typedef struct{
	unsigned int ip;
	unsigned short port;
	int			socket;
	THandleType type;
	uint8		*read_buffer;
	uint		read_buffer_used;
	uint		read_buffer_pos;
	uint		read_buffer_size;
	uint		read_marker;
	uint64		read_raw_progress;
	uint8		*write_buffer;
	uint		write_buffer_pos;
	uint		write_buffer_size;
	uint64		write_raw_progress;
	void		*file;
	void		*text_copy;
	char		*file_name;
	boolean		debug_descriptor;
	boolean		debug_header;
	boolean		connected;
}THandle;

#define TESTIFY_DISCOVER_USER_ID_LENGTH 6

typedef struct{
	uint8 user_id[TESTIFY_DISCOVER_USER_ID_LENGTH];
	uint16 relay_port; 
	double timeout;
}TeestifyDiscoverUser;

typedef struct{
	void *mutex;
	TestifyNetworkPeer *peers_public;
	TeestifyDiscoverUser *peers_private;
	uint peer_count;
	uint peer_allocated;
	char *service;
	char channel[64];
	char message[200];
	uint16 port;
	uint16 ref_count;
	boolean joined;
	boolean global;
	boolean local;
	boolean updated;
	double udp_ping_timer;
	void *next;
}TestifyDiscovery;

extern uint testify_unpack_buffer_get(THandle *handle);
extern boolean testify_pack_buffer_clear(THandle *handle);
extern void testify_handle_clear(THandle *handle, uint type);
extern size_t testify_network_stream_receve(THandle *handle, uint8 *buffer, size_t length);
extern size_t testify_network_stream_send(THandle *handle, uint8 *buffer, size_t length);