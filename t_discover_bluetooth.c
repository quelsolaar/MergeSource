#ifdef _WIN32

#include <stdio.h>
#include <winsock2.h>
#include <ws2bth.h>
#include <assert.h>
#include "testify.h"

#define BLUETOOTH_DATA_LENGTH 1024
#define BLUETOOTH_LISTEN_BACKLOG 4
#define BLUETOOTH_PORT 5555 // can also use BT_PORT_ANY;

#define NEXT_ALIGNED_ADDRESS(previous_byte_position, previous_type, next_type) \
    ((((previous_byte_position + sizeof(previous_type)) + _Alignof(next_type) - 1) / _Alignof(next_type)) * _Alignof(next_type))

WSAQUERYSET *testify_discover_bluetooth_allocate(size_t size)
{
    WSAQUERYSET *query_set;
    query_set = malloc(size);
    if(query_set == NULL)
        return NULL;
    memset(query_set, 0, size);
    query_set->dwNameSpace = NS_BTH;
    query_set->dwSize = size;
    return query_set;
}

void testify_discover_bluetooth_update()
{
    INT             iRetryCount, iResult;
    BOOL            bContinueLookup = FALSE, bRemoteDeviceFound = FALSE;
    ULONG           flags = 0;
    DWORD           size;
    HANDLE          handle = NULL;
    WSAQUERYSET     *query_set;
    size = sizeof(*query_set);
    query_set = testify_discover_bluetooth_allocate(size);
    flags = LUP_CONTAINERS | LUP_RETURN_NAME | LUP_RETURN_ADDR;
    handle = NULL;
    iResult = WSALookupServiceBegin(query_set, flags, &handle);
    if(NO_ERROR != iResult || NULL == handle)
        return;

    while(TRUE)
    {
        if(NO_ERROR == WSALookupServiceNext(handle, flags, &size, query_set))
        {
            if(query_set->lpszServiceInstanceName != NULL)
            {
                char name[32];
                unsigned int i, pos;
                for(pos = i = 0; pos < 32 - 6 - 1 && query_set->lpszServiceInstanceName[i] != 0; i++)
                    pos += f_uint32_to_utf8(query_set->lpszServiceInstanceName[i], &name[pos]);
                name[pos] = 0;
                printf("name: %s\n", name);
            /*   PSOCKADDR_BTH pRemoteBtAddr
                CopyMemory(pRemoteBtAddr,
                                (PSOCKADDR_BTH) pWSAQuerySet->lpcsaBuffer->RemoteAddr.lpSockaddr,
                                sizeof(*pRemoteBtAddr));*/
            }
        }else
        {
            if(WSAEFAULT == WSAGetLastError())
            {
                free(query_set);
                query_set = testify_discover_bluetooth_allocate(size);
            }
            break;
        }
    }
    WSALookupServiceEnd(handle);
    flags |= LUP_FLUSHCACHE;
}

#endif
