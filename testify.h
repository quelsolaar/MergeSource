#include "forge.h"

#ifndef TESTIFY_H
#define TESTIFY_H

/*  -------------- Testify --------------------   
Testify is a library for sereielizing data in to bit streams, and then unpacking them. Data can be packed/unpacked form memory, a network connection or a file. Testify packs all data in big endian order and hides any difference between big or small endianess of the CPU. While Testify is written to be very fast it also has extensive debugging capabillities. By enabling the debug state of a Stream Testify can be made to pack significant ammounts of meta data arround each entry. By doing this the receving end can match the incomming data with the expected data makig it trivial to find pack/unpack miss-match buggs. By building Testify without the define TESTIFY_DEBUG all debugging code can be dissabled and maximum preformance can be obtained.*/

#define U_BUFFER_SIZE 2048

#define TESTIFY_DEBUG

typedef enum{
	T_HT_STREAMING_SERVER,
	T_HT_STREAMING_CONNECTION,
	T_HT_PACKET_PEER,
	T_HT_FILE_READ,
	T_HT_FILE_WRITE,
	T_HT_BUFFER
}THandleType;


typedef struct{
	uint32 ip;
	uint16 port;
}TestifyNetworkAddress;

#define TESTIFY_NETWORK_PEER_MESSAGE_LENGTH 200

typedef enum{
	TESTIFY_DS_NONE,
	TESTIFY_DS_ADDED,
	TESTIFY_DS_REMOVED,
	TESTIFY_DS_COUNT
}TDiscoverDiscoverState;

typedef struct{
	TestifyNetworkAddress address;
	char message[TESTIFY_NETWORK_PEER_MESSAGE_LENGTH];
	boolean local;
	TDiscoverDiscoverState state;
}TestifyNetworkPeer;

#include "t_internal.h"

/* ---------- TCP Networking --------
Handles the creation and mangement of TCP streams.*/


extern THandle *testify_network_stream_address_create(const char *host_name, uint16 port); /* Creates a TCP connection that can be both read or written to, connecting to the DNS address given as "host_name". By default the connection will attempt to connect to "port", unless the host_name is frormated in the following way <dns name>:<port>, then the port number will be read from the host_name. If no host name is given, the function will open up the port, and wait for incomming connections.*/
extern int		testify_network_stream_send_force(THandle *handle); /* Tells a TCP connectiuon to flush all out going data to the network. */
extern THandle *testify_network_stream_wait_for_connection(THandle *listen, TestifyNetworkAddress *from); /*Polls a TCP handle "listen" for incomming connections. If a new connection is obtained a new handle for that connection will be returned. If the pointer from is given, the function will fill out the structure with the IP address and port of the incomming connection if one is obtained.*/
extern boolean	testify_network_stream_connected(THandle *handle); /*Returns TRUE is the tcp connection is still active.*/

/* ---------- UDP Networking --------
Handles the creation and mangement of UDP datagrams.*/

extern boolean	testify_network_address_lookup(TestifyNetworkAddress *address, char *dns_name, uint16 default_port); /* look up a DNS address and turn it in to a IP and port. Returns TRUE if sucessfull. The default port will be set unless the dns_name contains a port in the following format hostname:portnumber .*/
extern boolean	testify_network_address_compare(TestifyNetworkAddress *a, TestifyNetworkAddress *b);
extern THandle *testify_network_datagram_create(uint16 port); /* Opens a port for receving and sending UDP traffic on a specific port. */
extern int		testify_network_datagram_send(THandle *handle, TestifyNetworkAddress *to); /* Sends the data collected in handle to the IP and address specifyed in "to".*/
extern int		testify_network_receive(THandle *handle, TestifyNetworkAddress *from);


/* ---------- Network adapters --------
Query the netwqork adapters and their settings.*/

extern uint		testify_network_adapter_count(); /* WQuery the number of active network adapters in the system. */
extern uint32	testify_network_adapter_ipv4_local_address(uint id); /* Find the local IP v4 address of a specific adapter. */
extern uint32	testify_network_adapter_ipv4_global_address(uint id); /* Find the global IP v4 address of a specific adapter. May or may not be samer as local address. */
extern uint32	testify_network_adapter_ipv4_mask(uint id); /* Find the local IP v4 mask of a specific adapter. */
extern uint32	testify_network_adapter_ipv4_gateway(uint id); /* Find the local IP v4 gateway of a specific adapter. */
extern char		*testify_network_adapter_name(uint id); /* Find the local name of a specific adapter. */
extern uint8	*testify_network_adapter_mac_address(uint id); /* Find the MAC address of a specific adapter. A pointer to 6 bytes will be returned. */
extern boolean	testify_network_ipv4_local(uint32 ip_address); /* Find the local IP v4 gateway of a specific adapter. */

/* ---------- File Management --------
Handles the creation and mangement of binary file handles.*/

extern THandle *testify_file_load(char *path); /* Opens a Testify handle from a file name for reading. */
extern THandle *testify_file_save(char *path); /* Opens a Testify handle from a file name for writing. */
extern uint64	testify_file_size(THandle *handle); /* Returns the size of the file. */
extern void		testify_file_position_set(THandle *handle, uint64 pos); /* Sets the current write position to "pos". */
extern uint64 	testify_file_position_get(THandle *handle); /* Returns the current write position. */

/* ---------- Memmory buffers --------
Handles the creation and mangement of in memory FIFO channels.*/

extern THandle *testify_buffer_create(); /* Creates a handle to a Testify FIFO Memory buffer. */
extern void *testify_buffer_get(THandle *handle, uint32 *size); /*Gives the user read direct memory access to the packed data. If the handle is set to debug mode it will contain debug header information.*/
extern void testify_buffer_set(THandle *handle, void *data, uint32 size); /*Lets the user give a pointer to "data" of "Size" number of bytes that will be added to the buffer. */


/* ---------- Handle Utillities --------
*/

extern THandleType	testify_type(THandle *handle); /* Returns the type of a Testify handle. */
extern void		testify_free(THandle *handle); /* frees a handle */
extern uint		testify_network_wait(THandle **handles, boolean *read, boolean *write, unsigned int handle_count, unsigned int microseconds); /* querys if network handles are ready to read and write. Can handle multiple handles as speccified by the array "handles" and handle count. The function will block for at most milliseconds and wait for any resource to become availbale. Once a resource gets available it will return. */

/* ---------- Debugging --------
Testify has an extencive debugging iunfrastructure desinged to enable inrosprection of binary data stream and the abillity to debug miss matches of how a */

extern void		testify_debug_mode_set(THandle *handle, boolean debug, char *text_copy_name); /* Allows applications to set the debug mode of a handle. if "debug" is set to TRUE, meta data will be packed with each data containing its type and name. Once this is unpacked the receving end can allert the developer of a miss match ocurrs. A handle can also automaticly be set to debug if it reads in data from a handle set to debug. Data traffic will be significantly larger and slower with the debug mode truned on and it is therefor only intended to be used when debugging. The Entire Debugging system can be removed using macros for maximum preformance by not defining TESTIFY_DEBUG.*/
extern void		testify_network_debug_incomming(THandle *handle); /* prints ot the current state of the incomming buffer */
extern void		testify_network_debug_outbound(THandle *handle); /* prints ot the current state of the incomming buffer */

#define TESTIFY_MAX_NETWORK_ADDRESS_STRING_SIZE 22

extern void		testify_network_debug_address_print(TestifyNetworkAddress *address, char *buffer);

/* ---------- Markers --------
Markers allow a application to set down a marker, then parse forward, and then if need be revert to the marker position in case not enought data has arrived to complete the parse. Markers are especialy imporatnt whjen dealing with network handles, but it is always good practice to use them to make sure that the data one tries to retrive is intact. 
*/

extern void		testify_restart_marker_set(THandle *handle); /* Sets the marker at the current spot in the stream, forcing the handle not to free any data from this point forward in order to be able to rewind to this point. */
extern void		testify_restart_marker_release(THandle *handle); /* Releases the currently set marker. The handle may free data before the current stream position. */
extern void		testify_restart_marker_reset(THandle *handle); /*Re sets the current read position to the marker.*/

extern boolean	testify_retivable(THandle *handle, uint size); /* Returns TRUE or FALSE if "size" number of bytes are retrivable. If the handle is set to debug mode the function will only count actiual payload not debug meta data.*/
extern boolean	testify_retivable_terminated(THandle *handle); /* Returns TRUE or FALSE if a null terminated string can be obtained form the stream. */

/* ---------- Data packing --------
Functions used to pack data in to a stram. If TESTIFY_DEBUG is not defined, the param "name" will be removed with a macro. In debug mode (where both TESTIFY_DEBUG is defined AND the bug mode of the stream has been set to TRUE testify_debug_mode_set) the name, and data type will be checked to make sure they match the name and type that was packed. If this test fails the application will terminate on unpack and write out a Error raport to standard out.*/


extern void		testify_pack_uint8(THandle *handle, uint8 value, char *name); /* Packs one uint8 "value" in to the stream. "name" is a string that needs to be matched when unpacking if the handle is running in debug mode. */
extern void		testify_pack_int8(THandle *handle, int8 value, char *name); /* Packs one int8 "value" in to the stream. "name" is a string that needs to be matched when unpacking if the handle is running in debug mode. */
extern void		testify_pack_uint16(THandle *handle, uint16 value, char *name); /* Packs one uint16 "value" in to the stream. "name" is a string that needs to be matched when unpacking if the handle is running in debug mode. */
extern void		testify_pack_int16(THandle *handle, int16 value, char *name); /* Packs one int16 "value" in to the stream. "name" is a string that needs to be matched when unpacking if the handle is running in debug mode. */
extern void		testify_pack_uint32(THandle *handle, uint32 value, char *name); /* Packs one uint32 "value" in to the stream. "name" is a string that needs to be matched when unpacking if the handle is running in debug mode. */
extern void		testify_pack_int32(THandle *handle, int32 value, char *name); /* Packs one int32 "value" in to the stream. "name" is a string that needs to be matched when unpacking if the handle is running in debug mode. */
extern void		testify_pack_uint64(THandle *handle, uint64 value, char *name); /* Packs one uint64 "value" in to the stream. "name" is a string that needs to be matched when unpacking if the handle is running in debug mode. */
extern void		testify_pack_int64(THandle *handle, int64 value, char *name);	 /* Packs one int64 "value" in to the stream. "name" is a string that needs to be matched when unpacking if the handle is running in debug mode. */
extern void		testify_pack_real32(THandle *handle, float value, char *name); /* Packs one float "value" in to the stream. "name" is a string that needs to be matched when unpacking if the handle is running in debug mode. */
extern void		testify_pack_real64(THandle *handle, double value, char *name); /* Packs one double "value" in to the stream. "name" is a string that needs to be matched when unpacking if the handle is running in debug mode. */
extern boolean	testify_pack_string(THandle *handle, char *value, char *name); /* Packs one uint8 "value" in to the stream. "name" is a string that needs to be matched when unpacking if the handle is running in debug mode. */
extern uint64	testify_pack_raw(THandle *handle, uint8 *data, uint64 lengt, char *name); /* Packs a raw array of bytes in to a handle. Returns the number of bytes that got packed as this may be less then size */

/* ---------- Data unpacking --------
Functions used to unpack data in to a stram. If TESTIFY_DEBUG is not defined, the param "name" will be removed with a macro. In debug mode (where both TESTIFY_DEBUG is defined AND the bug mode of the stream has been set to TRUE testify_debug_mode_set) the name, and data type will be checked to make sure they match the name and type that was packed. If this test fails the application will terminate and write out a Error raport to standard out.*/

extern uint8	testify_unpack_uint8(THandle *handle, char *name); /* Unpacking one uint8 from "handle". Name is the name of the value expected to be found next in line in the data stream. */
extern int8		testify_unpack_int8(THandle *handle, char *name); /* Unpacking one int8 from "handle". Name is the name of the value expected to be found next in line in the data stream. */
extern uint16	testify_unpack_uint16(THandle *handle, char *name); /* Unpacking one uint16 from "handle". Name is the name of the value expected to be found next in line in the data stream. */
extern int16	testify_unpack_int16(THandle *handle, char *name); /* Unpacking one int16 from "handle". Name is the name of the value expected to be found next in line in the data stream. */
extern uint32	testify_unpack_uint32(THandle *handle, char *name); /* Unpacking one uint32 from "handle". Name is the name of the value expected to be found next in line in the data stream. */
extern int32	testify_unpack_int32(THandle *handle, char *name); /* Unpacking one int32 from "handle". Name is the name of the value expected to be found next in line in the data stream. */
extern uint64	testify_unpack_uint64(THandle *handle, char *name); /* Unpacking one uint64 from "handle". Name is the name of the value expected to be found next in line in the data stream. */
extern int64	testify_unpack_int64(THandle *handle, char *name); /* Unpacking one int64 from "handle". Name is the name of the value expected to be found next in line in the data stream. */
extern float	testify_unpack_real32(THandle *handle, char *name); /* Unpacking one float from "handle". Name is the name of the value expected to be found next in line in the data stream. */
extern double	testify_unpack_real64(THandle *handle, char *name); /* Unpacking one double from "handle". Name is the name of the value expected to be found next in line in the data stream. */
extern boolean	testify_unpack_string(THandle *handle, char *value, uint buffer_size, char *name); /* Unpacking a string to the buffer "string" with maximum "buffer_size" number of bytes from "handle". Name is the name of the value expected to be found next in line in the data stream. */
extern char		*testify_unpack_string_allocate(THandle *handle, char *name); /* Unpacking a string from "handle" and returning a pointer to the string. The application is responcible for freeing the string. Name is the name of the value expected to be found next in line in the data stream. */
extern real64	testify_unpack_raw(THandle *handle, uint8 *buffer, uint64 buffer_length, char *name); /* Unpacks a raw array of bytes in to a handle. Returns the number of bytes that got unpacked as this may be less then size */

/* ---------- Peer discovery --------
Testify has a peer discovery system that lets users advertice a global service and then find IP addresses of other peers advertising servises. */

extern TestifyNetworkPeer *testify_discover(char *service, uint16 port, uint *count, uint8 *seed, uint seed_length, char *message, boolean global);


TestifyDiscovery	*testify_discover_create(char *service, uint16 port, char *message, boolean global);
void				testify_discover_destroy(TestifyDiscovery *discovery);
void				testify_discover_message_set(TestifyDiscovery *discovery, char *message);
TestifyNetworkPeer  *testify_discover_peers_get(TestifyDiscovery *discovery, uint *count);
void				testify_discover_peers_return(TestifyDiscovery *discovery);

boolean				testify_discover_updated(TestifyDiscovery *discovery);

/* ---------- Code Generation --------
Testify . */

extern void testify_struct_generate(char *path, char *struct_definition, char *prefix);

#include "t_debug.h"

#endif