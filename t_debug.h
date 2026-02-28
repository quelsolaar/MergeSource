#ifndef TESTIFY_INTERNAL

extern void		testify_restart_marker_reset_internal(THandle *handle, char *file, uint line);
#define testify_restart_marker_reset(n) testify_restart_marker_reset_internal(n, __FILE__, __LINE__)

#ifdef TESTIFY_DEBUG

extern void		testify_pack_uint8_internal(THandle *handle, uint8 value, char *name, char *file, uint line);
extern void		testify_pack_int8_internal(THandle *handle, int8 value, char *name, char *file, uint line);
extern void		testify_pack_uint16_internal(THandle *handle, uint16 value, char *name, char *file, uint line);
extern void		testify_pack_int16_internal(THandle *handle, int16 value, char *name, char *file, uint line);
extern void		testify_pack_uint32_internal(THandle *handle, uint32 value, char *name, char *file, uint line);
extern void		testify_pack_int32_internal(THandle *handle, int32 value, char *name, char *file, uint line);
extern void		testify_pack_uint64_internal(THandle *handle, uint64 value, char *name, char *file, uint line);
extern void		testify_pack_int64_internal(THandle *handle, int64 value, char *name, char *file, uint line);	
extern void		testify_pack_real32_internal(THandle *handle, float value, char *name, char *file, uint line);
extern void		testify_pack_real64_internal(THandle *handle, double value, char *name, char *file, uint line);
extern void		testify_pack_string_internal(THandle *handle, char *value, char *name, char *file, uint line);
extern uint64	testify_pack_raw_internal(THandle *handle, uint8 *data, uint64 length, char *name, char *file, uint line);
extern uint8	testify_unpack_uint8_internal(THandle *handle, char *name, char *file, uint line);
extern int8		testify_unpack_int8_internal(THandle *handle, char *name, char *file, uint line);
extern uint16	testify_unpack_uint16_internal(THandle *handle, char *name, char *file, uint line);
extern int16	testify_unpack_int16_internal(THandle *handle, char *name, char *file, uint line);
extern uint32	testify_unpack_uint32_internal(THandle *handle, char *name, char *file, uint line);
extern int32	testify_unpack_int32_internal(THandle *handle, char *name, char *file, uint line);
extern uint64	testify_unpack_uint64_internal(THandle *handle, char *name, char *file, uint line);
extern int64	testify_unpack_int64_internal(THandle *handle, char *name, char *file, uint line);
extern float	testify_unpack_real32_internal(THandle *handle, char *name, char *file, uint line);
extern double	testify_unpack_real64_internal(THandle *handle, char *name, char *file, uint line);
extern uint		testify_unpack_string_internal(THandle *handle, char *value, uint buffer_size, char *name, char *file, uint line);
extern char		*testify_unpack_string_allocate_internal(THandle *handle, char *name, char *file, uint line);
extern real64	testify_unpack_raw_internal(THandle *handle, uint8 *buffer, uint64 buffer_length, char *name, char *file, uint line);

extern void		testify_pack_vector_uint8_internal(THandle *handle, uint8 value, char *name, uint length, char *file, uint line);
extern void		testify_pack_vector_int8_internal(THandle *handle, int8 value, char *name, uint length, char *file, uint line);
extern void		testify_pack_vector_uint16_internal(THandle *handle, uint16 value, char *name, uint length, char *file, uint line);
extern void		testify_pack_vector_int16_internal(THandle *handle, int16 value, char *name, uint length, char *file, uint line);
extern void		testify_pack_vector_uint32_internal(THandle *handle, uint32 value, char *name, uint length, char *file, uint line);
extern void		testify_pack_vector_int32_internal(THandle *handle, int32 value, char *name, uint length, char *file, uint line);
extern void		testify_pack_vector_uint64_internal(THandle *handle, uint64 value, char *name, uint length, char *file, uint line);
extern void		testify_pack_vector_int64_internal(THandle *handle, int64 value, char *name, uint length, char *file, uint line);	
extern void		testify_pack_vector_real32_internal(THandle *handle, float value, char *name, uint length, char *file, uint line);
extern void		testify_pack_vector_real64_internal(THandle *handle, double value, char *name, uint length, char *file, uint line);
extern void		testify_pack_vector_string_internal(THandle *handle, char *value, char *name, uint length, char *file, uint line);
extern uint8	testify_unpack_vector_uint8_internal(THandle *handle, char *name, uint length, char *file, uint line);
extern int8		testify_unpack_vector_int8_internal(THandle *handle, char *name, uint length, char *file, uint line);
extern uint16	testify_unpack_vector_uint16_internal(THandle *handle, char *name, uint length, char *file, uint line);
extern int16	testify_unpack_vector_int16_internal(THandle *handle, char *name, uint length, char *file, uint line);
extern uint32	testify_unpack_vector_uint32_internal(THandle *handle, char *name, uint length, char *file, uint line);
extern int32	testify_unpack_vector_int32_internal(THandle *handle, char *name, uint length, char *file, uint line);
extern uint64	testify_unpack_vector_uint64_internal(THandle *handle, char *name, uint length, char *file, uint line);
extern int64	testify_unpack_vector_int64_internal(THandle *handle, char *name, uint length, char *file, uint line);
extern float	testify_unpack_vector_real32_internal(THandle *handle, char *name, uint length, char *file, uint line);
extern double	testify_unpack_vector_real64_internal(THandle *handle, char *name, uint length, char *file, uint line);
extern uint		testify_unpack_vector_string_internal(THandle *handle, char *value, uint buffer_size, char *name, uint length, char *file, uint line);



#define testify_pack_uint8(n, m, l) testify_pack_uint8_internal(n, m, l, __FILE__, __LINE__)
#define testify_unpack_uint8(n, m) testify_unpack_uint8_internal(n, m, __FILE__, __LINE__)
#define testify_pack_int8(n, m, l) testify_pack_int8_internal(n, m, l, __FILE__, __LINE__)
#define testify_unpack_int8(n, m) testify_unpack_int8_internal(n, m, __FILE__, __LINE__)

#define testify_pack_uint16(n, m, l) testify_pack_uint16_internal(n, m, l, __FILE__, __LINE__)
#define testify_unpack_uint16(n, m) testify_unpack_uint16_internal(n, m, __FILE__, __LINE__)
#define testify_pack_int16(n, m, l) testify_pack_int16_internal(n, m, l, __FILE__, __LINE__)
#define testify_unpack_int16(n, m) testify_unpack_int16_internal(n, m, __FILE__, __LINE__)

#define testify_pack_uint32(n, m, l) testify_pack_uint32_internal(n, m, l, __FILE__, __LINE__)
#define testify_unpack_uint32(n, m) testify_unpack_uint32_internal(n, m, __FILE__, __LINE__)
#define testify_pack_int32(n, m, l) testify_pack_int32_internal(n, m, l, __FILE__, __LINE__)
#define testify_unpack_int32(n, m) testify_unpack_int32_internal(n, m, __FILE__, __LINE__)

#define testify_pack_uint64(n, m, l) testify_pack_uint64_internal(n, m, l, __FILE__, __LINE__)
#define testify_unpack_uint64(n, m) testify_unpack_uint64_internal(n, m, __FILE__, __LINE__)
#define testify_pack_int64(n, m, l) testify_pack_int64_internal(n, m, l, __FILE__, __LINE__)
#define testify_unpack_int64(n, m) testify_unpack_int64_internal(n, m, __FILE__, __LINE__)

#define testify_pack_real32(n, m, l) testify_pack_real32_internal(n, m, l, __FILE__, __LINE__)
#define testify_unpack_real32(n, m) testify_unpack_real32_internal(n, m, __FILE__, __LINE__)
#define testify_pack_real64(n, m, l) testify_pack_real64_internal(n, m, l, __FILE__, __LINE__)
#define testify_unpack_real64(n, m) testify_unpack_real64_internal(n, m, __FILE__, __LINE__)

#define testify_pack_string(n, m, l) testify_pack_string_internal(n, m, l, __FILE__, __LINE__)
#define testify_unpack_string(n, m, l, k) testify_unpack_string_internal(n, m, l, k, __FILE__, __LINE__)
#define testify_unpack_string_allocate(n, m) testify_unpack_string_allocate_internal(n, m, __FILE__, __LINE__)

#define testify_pack_raw(n, m, l, k) testify_pack_raw_internal(n, m, l, k, __FILE__, __LINE__)
#define testify_unpack_raw(n, m, l, k) testify_unpack_raw_internal(n, m, l, k, __FILE__, __LINE__)


#else

extern void		testify_pack_uint8_internal(THandle *handle, uint8 value);
extern void		testify_pack_int8_internal(THandle *handle, int8 value);
extern void		testify_pack_uint16_internal(THandle *handle, uint16 value);
extern void		testify_pack_int16_internal(THandle *handle, int16 value);
extern void		testify_pack_uint32_internal(THandle *handle, uint32 value);
extern void		testify_pack_int32_internal(THandle *handle, int32 value);
extern void		testify_pack_uint64_internal(THandle *handle, uint64 value);
extern void		testify_pack_int64_internal(THandle *handle, int64 value);	
extern void		testify_pack_real32_internal(THandle *handle, float value);
extern void		testify_pack_real64_internal(THandle *handle, double value);
extern void		testify_pack_string_internal(THandle *handle, char *value);
extern uint64	testify_pack_raw_internal(THandle *handle, uint8 *data, uint64 length);
extern uint8	testify_unpack_uint8_internal(THandle *handle);
extern int8		testify_unpack_int8_internal(THandle *handle);
extern uint16	testify_unpack_uint16_internal(THandle *handle);
extern int16	testify_unpack_int16_internal(THandle *handle);
extern uint32	testify_unpack_uint32_internal(THandle *handle);
extern int32	testify_unpack_int32_internal(THandle *handle);
extern uint64	testify_unpack_uint64_internal(THandle *handle);
extern int64	testify_unpack_int64_internal(THandle *handle);
extern float	testify_unpack_real32_internal(THandle *handle);
extern double	testify_unpack_real64_internal(THandle *handle);
extern uint		testify_unpack_string_internal(THandle *handle, char *value, uint buffer_size);
extern char		*testify_unpack_string_allocate_internal(THandle *handle);
extern real64	testify_unpack_raw_internal(THandle *handle, uint8 *buffer, uint64 buffer_length);

extern void		testify_pack_vector_uint8_internal(THandle *handle, uint8 value, uint length);
extern void		testify_pack_vector_int8_internal(THandle *handle, int8 value, uint length);
extern void		testify_pack_vector_uint16_internal(THandle *handle, uint16 value, uint length);
extern void		testify_pack_vector_int16_internal(THandle *handle, int16 value, uint length);
extern void		testify_pack_vector_uint32_internal(THandle *handle, uint32 value, uint length);
extern void		testify_pack_vector_int32_internal(THandle *handle, int32 value, uint length);
extern void		testify_pack_vector_uint64_internal(THandle *handle, uint64 value,uint length);
extern void		testify_pack_vector_int64_internal(THandle *handle, int64 value, uint length);	
extern void		testify_pack_vector_real32_internal(THandle *handle, float value,uint length);
extern void		testify_pack_vector_real64_internal(THandle *handle, double value, uint length);
extern void		testify_pack_vector_string_internal(THandle *handle, char *value, uint length);
extern uint8	testify_unpack_vector_uint8_internal(THandle *handle, uint length);
extern int8		testify_unpack_vector_int8_internal(THandle *handle, uint length);
extern uint16	testify_unpack_vector_uint16_internal(THandle *handle, uint length);
extern int16	testify_unpack_vector_int16_internal(THandle *handle, uint length);
extern uint32	testify_unpack_vector_uint32_internal(THandle *handle, uint length);
extern int32	testify_unpack_vector_int32_internal(THandle *handle, uint length);
extern uint64	testify_unpack_vector_uint64_internal(THandle *handle, uint length);
extern int64	testify_unpack_vector_int64_internal(THandle *handle, uint length);
extern float	testify_unpack_vector_real32_internal(THandle *handle, uint length);
extern double	testify_unpack_vector_real64_internal(THandle *handle, uint length);
extern uint		testify_unpack_vector_string_internal(THandle *handle, char *value, uint buffer_size, uint length);

#define testify_pack_uint8(n, m, l) testify_pack_uint8_internal(n, m)
#define testify_unpack_uint8(n, m) testify_unpack_uint8_internal(n)
#define testify_pack_int8(n, m, l) testify_pack_int8_internal(n, m)
#define testify_unpack_int8(n, m) testify_unpack_int8_internal(n)

#define testify_pack_uint16(n, m, l) testify_pack_uint16_internal(n, m)
#define testify_unpack_uint16(n, m) testify_unpack_uint16_internal(n)
#define testify_pack_int16(n, m, l) testify_pack_int16_internal(n, m)
#define testify_unpack_int16(n, m) testify_unpack_int16_internal(n)

#define testify_pack_uint32(n, m, l) testify_pack_uint32_internal(n, m)
#define testify_unpack_uint32(n, m) testify_unpack_uint32_internal(n)
#define testify_pack_int32(n, m, l) testify_pack_int32_internal(n, m)
#define testify_unpack_int32(n, m) testify_unpack_int32_internal(n)

#define testify_pack_real32(n, m, l) testify_pack_real32_internal(n, m)
#define testify_unpack_real32(n, m) testify_unpack_real32_internal(n)
#define testify_pack_real64(n, m, l) testify_pack_real64_internal(n, m)
#define testify_unpack_real64(n, m) testify_unpack_real64_internal(n)

#define testify_pack_string(n, m, l) testify_pack_string_internal(n, m, l)
#define testify_unpack_string(n, m, l, k) testify_unpack_string_internal(n, m, l)
#define testify_unpack_string_allocate(n, m) testify_unpack_string_allocate_internal(n)

#define testify_pack_raw(n, m, l, k) testify_pack_raw_internal(n, m, l)
#define testify_unpack_raw(n, m, l, k) testify_unpack_raw_internal(n, m, l)


#endif
#endif
