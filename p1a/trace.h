#ifndef TRACE_H
#define TRACE_H

#ifdef ENABLE_DTRACE
#include "memcached_dtrace.h"
#else
#define MEMCACHED_ASSOC_DELETE(arg0, arg1)
#define MEMCACHED_ASSOC_DELETE_ENABLED() (0)
#define MEMCACHED_ASSOC_FIND(arg0, arg1, arg2)
#define MEMCACHED_ASSOC_FIND_ENABLED() (0)
#define MEMCACHED_ASSOC_INSERT(arg0, arg1)
#define MEMCACHED_ASSOC_INSERT_ENABLED() (0)
#define MEMCACHED_COMMAND_ADD(arg0, arg1, arg2, arg3, arg4)
#define MEMCACHED_COMMAND_ADD_ENABLED() (0)
#define MEMCACHED_COMMAND_APPEND(arg0, arg1, arg2, arg3, arg4)
#define MEMCACHED_COMMAND_APPEND_ENABLED() (0)
#define MEMCACHED_COMMAND_CAS(arg0, arg1, arg2, arg3, arg4)
#define MEMCACHED_COMMAND_CAS_ENABLED() (0)
#define MEMCACHED_COMMAND_DECR(arg0, arg1, arg2, arg3)
#define MEMCACHED_COMMAND_DECR_ENABLED() (0)
#define MEMCACHED_COMMAND_DELETE(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_DELETE_ENABLED() (0)
#define MEMCACHED_COMMAND_DIV(arg0, arg1, arg2, arg3)
#define MEMCACHED_COMMAND_DIV_ENABLED() (0)
#define MEMCACHED_COMMAND_GET(arg0, arg1, arg2, arg3, arg4)
#define MEMCACHED_COMMAND_GET_ENABLED() (0)
#define MEMCACHED_COMMAND_TOUCH(arg0, arg1, arg2, arg3, arg4)
#define MEMCACHED_COMMAND_TOUCH_ENABLED() (0)
#define MEMCACHED_COMMAND_INCR(arg0, arg1, arg2, arg3)
#define MEMCACHED_COMMAND_INCR_ENABLED() (0)
#define MEMCACHED_COMMAND_MULT(arg0, arg1, arg2, arg3)
#define MEMCACHED_COMMAND_MULT_ENABLED() (0)
#define MEMCACHED_COMMAND_PREPEND(arg0, arg1, arg2, arg3, arg4)
#define MEMCACHED_COMMAND_PREPEND_ENABLED() (0)
#define MEMCACHED_COMMAND_REPLACE(arg0, arg1, arg2, arg3, arg4)
#define MEMCACHED_COMMAND_REPLACE_ENABLED() (0)
#define MEMCACHED_COMMAND_SET(arg0, arg1, arg2, arg3, arg4)
#define MEMCACHED_COMMAND_SET_ENABLED() (0)
#define MEMCACHED_CONN_ALLOCATE(arg0)
#define MEMCACHED_CONN_ALLOCATE_ENABLED() (0)
#define MEMCACHED_CONN_CREATE(arg0)
#define MEMCACHED_CONN_CREATE_ENABLED() (0)
#define MEMCACHED_CONN_DESTROY(arg0)
#define MEMCACHED_CONN_DESTROY_ENABLED() (0)
#define MEMCACHED_CONN_DISPATCH(arg0, arg1)
#define MEMCACHED_CONN_DISPATCH_ENABLED() (0)
#define MEMCACHED_CONN_RELEASE(arg0)
#define MEMCACHED_CONN_RELEASE_ENABLED() (0)
#define MEMCACHED_ITEM_LINK(arg0, arg1, arg2)
#define MEMCACHED_ITEM_LINK_ENABLED() (0)
#define MEMCACHED_ITEM_REMOVE(arg0, arg1, arg2)
#define MEMCACHED_ITEM_REMOVE_ENABLED() (0)
#define MEMCACHED_ITEM_REPLACE(arg0, arg1, arg2, arg3, arg4, arg5)
#define MEMCACHED_ITEM_REPLACE_ENABLED() (0)
#define MEMCACHED_ITEM_UNLINK(arg0, arg1, arg2)
#define MEMCACHED_ITEM_UNLINK_ENABLED() (0)
#define MEMCACHED_ITEM_UPDATE(arg0, arg1, arg2)
#define MEMCACHED_ITEM_UPDATE_ENABLED() (0)
#define MEMCACHED_PROCESS_COMMAND_END(arg0, arg1, arg2)
#define MEMCACHED_PROCESS_COMMAND_END_ENABLED() (0)
#define MEMCACHED_PROCESS_COMMAND_START(arg0, arg1, arg2)
#define MEMCACHED_PROCESS_COMMAND_START_ENABLED() (0)
#define MEMCACHED_SLABS_ALLOCATE(arg0, arg1, arg2, arg3)
#define MEMCACHED_SLABS_ALLOCATE_ENABLED() (0)
#define MEMCACHED_SLABS_ALLOCATE_FAILED(arg0, arg1)
#define MEMCACHED_SLABS_ALLOCATE_FAILED_ENABLED() (0)
#define MEMCACHED_SLABS_FREE(arg0, arg1, arg2)
#define MEMCACHED_SLABS_FREE_ENABLED() (0)
#define MEMCACHED_SLABS_SLABCLASS_ALLOCATE(arg0)
#define MEMCACHED_SLABS_SLABCLASS_ALLOCATE_ENABLED() (0)
#define MEMCACHED_SLABS_SLABCLASS_ALLOCATE_FAILED(arg0)
#define MEMCACHED_SLABS_SLABCLASS_ALLOCATE_FAILED_ENABLED() (0)
#endif

#endif
