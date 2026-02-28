#ifdef NOT_DEFINED

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


#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <linux/fs.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdatomic.h>
#include <linux/io_uring.h>
#define QUEUE_DEPTH 1
#define BLOCK_SZ    1024
/* Macros for barriers needed by io_uring */
#define io_uring_smp_store_release(p, v)            \

    atomic_store_explicit((_Atomic typeof(*(p)) *)(p), (v), \


                  memory_order_release)
#define io_uring_smp_load_acquire(p)                \


    atomic_load_explicit((_Atomic typeof(*(p)) *)(p),   \


                 memory_order_acquire)
int ring_fd;
unsigned *sring_tail, *sring_mask, *sring_array,


            *cring_head, *cring_tail, *cring_mask;
struct io_uring_sqe *sqes;
struct io_uring_cqe *cqes;
char buff[BLOCK_SZ];
off_t offset;
/*


 * System call wrappers provided since glibc does not yet


 * provide wrappers for io_uring system calls.
* */
int io_uring_setup(unsigned entries, struct io_uring_params *p)
{
    return (int) syscall(__NR_io_uring_setup, entries, p);
}
int io_uring_enter(int ring_fd, unsigned int to_submit, unsigned int min_complete, unsigned int flags)
{
    return (int) syscall(__NR_io_uring_enter, ring_fd, to_submit, min_complete, flags, NULL, 0);
}
int app_setup_uring(void) {
    struct io_uring_params p;
    void *sq_ptr, *cq_ptr;
    /* See io_uring_setup(2) for io_uring_params.flags you can set */
    memset(&p, 0, sizeof(p));
    ring_fd = io_uring_setup(QUEUE_DEPTH, &p);
    if (ring_fd < 0) {
        perror("io_uring_setup");
        return 1;
    }
    /*
     * io_uring communication happens via 2 shared kernel-user space ring
     * buffers, which can be jointly mapped with a single mmap() call in
     * kernels >= 5.4.
     */
    int sring_sz = p.sq_off.array + p.sq_entries * sizeof(unsigned);
    int cring_sz = p.cq_off.cqes + p.cq_entries * sizeof(struct io_uring_cqe);
    /* Rather than check for kernel version, the recommended way is to
     * check the features field of the io_uring_params structure, which is a
     * bitmask. If IORING_FEAT_SINGLE_MMAP is set, we can do away with the
     * second mmap() call to map in the completion ring separately.
     */
    if (p.features & IORING_FEAT_SINGLE_MMAP) {
        if (cring_sz > sring_sz)
            sring_sz = cring_sz;
        cring_sz = sring_sz;
    }
    /* Map in the submission and completion queue ring buffers.
     *  Kernels < 5.4 only map in the submission queue, though.
     */
    sq_ptr = mmap(0, sring_sz, PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_POPULATE,
                  ring_fd, IORING_OFF_SQ_RING);
    if (sq_ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    if (p.features & IORING_FEAT_SINGLE_MMAP) {
        cq_ptr = sq_ptr;
    } else {
        /* Map in the completion queue ring buffer in older kernels separately */
        cq_ptr = mmap(0, cring_sz, PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_POPULATE,
                      ring_fd, IORING_OFF_CQ_RING);
        if (cq_ptr == MAP_FAILED) {
            perror("mmap");
            return 1;
        }
    }
    /* Save useful fields for later easy reference */
    sring_tail = sq_ptr + p.sq_off.tail;
    sring_mask = sq_ptr + p.sq_off.ring_mask;
    sring_array = sq_ptr + p.sq_off.array;
    /* Map in the submission queue entries array */
    sqes = mmap(0, p.sq_entries * sizeof(struct io_uring_sqe),
                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,
                   ring_fd, IORING_OFF_SQES);

    if (sqes == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    /* Save useful fields for later easy reference */
    cring_head = cq_ptr + p.cq_off.head;
    cring_tail = cq_ptr + p.cq_off.tail;
    cring_mask = cq_ptr + p.cq_off.ring_mask;
    cqes = cq_ptr + p.cq_off.cqes;
    return 0;
}
/*
* Read from completion queue.
* In this function, we read completion events from the completion queue.
* We dequeue the CQE, update and head and return the result of the operation.
* */
int read_from_cq() {
    struct io_uring_cqe *cqe;
    unsigned head;
    /* Read barrier */
    head = io_uring_smp_load_acquire(cring_head);
    /*
    * Remember, this is a ring buffer. If head == tail, it means that the
    * buffer is empty.
    * */
    if (head == *cring_tail)
        return -1;
    /* Get the entry */
    cqe = &cqes[head & (*cring_mask)];
    if (cqe->res < 0)
        fprintf(stderr, "Error: %s\n", strerror(abs(cqe->res)));
    head++;
    /* Write barrier so that update to the head are made visible */
    io_uring_smp_store_release(cring_head, head);
    return cqe->res;
}
/*
* Submit a read or a write request to the submission queue.
* */
int submit_to_sq(int fd, int op) {
    unsigned index, tail;
    /* Add our submission queue entry to the tail of the SQE ring buffer */
    tail = *sring_tail;
    index = tail & *sring_mask;
    struct io_uring_sqe *sqe = &sqes[index];
    /* Fill in the parameters required for the read or write operation */
    sqe->opcode = op;
    sqe->fd = fd;
    sqe->addr = (unsigned long) buff;
    if (op == IORING_OP_READ) {
        memset(buff, 0, sizeof(buff));
        sqe->len = BLOCK_SZ;
    }
    else {
        sqe->len = strlen(buff);
    }
    sqe->off = offset;
    sring_array[index] = index;
    tail++;
    /* Update the tail */
    io_uring_smp_store_release(sring_tail, tail);
    /*
    * Tell the kernel we have submitted events with the io_uring_enter()
    * system call. We also pass in the IOURING_ENTER_GETEVENTS flag which
    * causes the io_uring_enter() call to wait until min_complete
    * (the 3rd param) events complete.
    * */
    int ret =  io_uring_enter(ring_fd, 1,1, IORING_ENTER_GETEVENTS);

    if(ret < 0) {
        perror("io_uring_enter");
        return -1;
    }
    return ret;
}


int main(int argc, char *argv[]) {
    int res;
    /* Setup io_uring for use */
    if(app_setup_uring()) {
        fprintf(stderr, "Unable to setup uring!\n");
        return 1;
    }
    /*
    * A while loop that reads from stdin and writes to stdout.
    * Breaks on EOF.
    */
    while (1) {
        /* Initiate read from stdin and wait for it to complete */
        submit_to_sq(STDIN_FILENO, IORING_OP_READ);
        /* Read completion queue entry */
        res = read_from_cq();
        if (res > 0) {
            /* Read successful. Write to stdout. */
            submit_to_sq(STDOUT_FILENO, IORING_OP_WRITE);
            read_from_cq();
        } else if (res == 0) {
            /* reached EOF */
            break;
        }
        else if (res < 0) {
            /* Error reading file */

            fprintf(stderr, "Error: %s\n", strerror(abs(res)));
            break;
        }
        offset += res;
    }
    return 0;
}
#endif