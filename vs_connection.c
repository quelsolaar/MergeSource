/*
 * $Id: vs_connection.c 4533 2008-08-11 15:00:51Z jiri $ 
 *
 * ***** BEGIN BSD LICENSE BLOCK *****
 *
 * Copyright (c) 2005-2008, The Uni-Verse Consortium.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END BSD LICENSE BLOCK *****
 *
 */

#include <stdlib.h>

#include "v_cmd_gen.h"

#if !defined(V_GENERATE_FUNC_MODE)

#include "verse.h"
#include "v_util.h"

#define VS_CONNECTION_CHUNK_SIZE	64

typedef struct{
	VSession	*session;
	unsigned int	session_count;
} VSSubscriptionList;

typedef struct{
	VSession	session;
	uint32		node_id;
	char		name[128];
	char		pass[128];
} VSConnection;

static struct {
	VSConnection	*connection;
	unsigned int	connection_length;
	VSSubscriptionList **list;
	unsigned int	list_length;
	unsigned int	current_session;
} VSConnectionStorage;

void vs_init_connection_storage(void)
{
	VSConnectionStorage.connection = NULL;
	VSConnectionStorage.connection_length = 0;
	VSConnectionStorage.list = NULL;
	VSConnectionStorage.list_length = 0;
	VSConnectionStorage.current_session = 0;
}

void vs_add_new_connection(VSession session, const char *name, const char *pass, VNodeID node_id)
{
	VSConnection	*conn;

	if(VSConnectionStorage.connection_length % VS_CONNECTION_CHUNK_SIZE == 0)
		VSConnectionStorage.connection = realloc(VSConnectionStorage.connection, (sizeof *VSConnectionStorage.connection) * (VSConnectionStorage.connection_length + VS_CONNECTION_CHUNK_SIZE));
	conn = &VSConnectionStorage.connection[VSConnectionStorage.connection_length];

	conn->session = session;
	conn->node_id = node_id;
	v_strlcpy(conn->name, name, sizeof conn->name);
	v_strlcpy(conn->pass, pass, sizeof conn->pass);

	VSConnectionStorage.connection_length++;
}

uint32 vs_get_avatar(void)
{
	return VSConnectionStorage.connection[VSConnectionStorage.current_session].node_id;
}

VSession vs_get_session(void)
{
	return VSConnectionStorage.connection[VSConnectionStorage.current_session].session;
}

const char * vs_get_user_name(void)
{
	return VSConnectionStorage.connection[VSConnectionStorage.current_session].name;
}

const char * vs_get_user_pass(void)
{
	return VSConnectionStorage.connection[VSConnectionStorage.current_session].pass;
}


void vs_remove_connection(void)
{
	unsigned int	i, j;
	VSession	*session;
	VSSubscriptionList *list;

	session = VSConnectionStorage.connection[VSConnectionStorage.current_session].session;
	for(i = 0; i < VSConnectionStorage.list_length; i++)
	{
		list = VSConnectionStorage.list[i];
		for(j = 0; j < list->session_count && list->session[j] != session; j++);
		if(j < list->session_count)
			list->session[j] = list->session[--list->session_count];
	}
	j = --VSConnectionStorage.connection_length;

	if(VSConnectionStorage.current_session < j)
	{
		VSConnectionStorage.connection[VSConnectionStorage.current_session].session = VSConnectionStorage.connection[j].session;
		VSConnectionStorage.connection[VSConnectionStorage.current_session].node_id = VSConnectionStorage.connection[j].node_id;
	}
	else
		VSConnectionStorage.current_session = 0;
}

void vs_set_next_session(void)
{
	if(++VSConnectionStorage.current_session >= VSConnectionStorage.connection_length)
		VSConnectionStorage.current_session = 0;
	if(VSConnectionStorage.connection_length != 0)
		verse_session_set(VSConnectionStorage.connection[VSConnectionStorage.current_session].session);
}

VSSubscriptionList *vs_create_subscription_list(void)
{
	VSSubscriptionList *list;
	list = malloc(sizeof *list);
	if(VSConnectionStorage.list_length % VS_CONNECTION_CHUNK_SIZE == 0)
		VSConnectionStorage.list = realloc(VSConnectionStorage.list, (sizeof *VSConnectionStorage.list) * (VSConnectionStorage.list_length + VS_CONNECTION_CHUNK_SIZE));
	VSConnectionStorage.list[VSConnectionStorage.list_length] = list;
	list->session = NULL;
	list->session_count = 0; 
	VSConnectionStorage.list_length++;
	return list;
}

void vs_destroy_subscription_list(VSSubscriptionList *list)
{
	unsigned int i;

	if(list == NULL)
		return;
	if(list->session != NULL)
		free(list->session);
	for(i = 0; i < VSConnectionStorage.list_length && VSConnectionStorage.list[i] != list; i++)
		;
	if(i < VSConnectionStorage.list_length)
		VSConnectionStorage.list[i] = VSConnectionStorage.list[--VSConnectionStorage.list_length];
	free(list);
}

/* Returns 1 if subscriber was added, 0 if not (typically meaning it was already on the list). */
int vs_add_new_subscriptor(VSSubscriptionList *list)
{
	unsigned int i;
	if(list->session_count % VS_CONNECTION_CHUNK_SIZE == 0)
		list->session = realloc(list->session, (sizeof *list->session) * (list->session_count + VS_CONNECTION_CHUNK_SIZE));
	for(i = 0; i < list->session_count; i++)
		if(list->session[i] == VSConnectionStorage.connection[VSConnectionStorage.current_session].session)
			return 0;
	list->session[list->session_count] = VSConnectionStorage.connection[VSConnectionStorage.current_session].session;
	list->session_count++;
	return 1;
}


void vs_remove_subscriptor(VSSubscriptionList *list)
{
	unsigned int i; 
	VSession *session;
	session = VSConnectionStorage.connection[VSConnectionStorage.current_session].session;
	for(i = 0; i < list->session_count && list->session[i] != session; i++);
	if(i < list->session_count)
		list->session[i] = list->session[--list->session_count];
}

size_t vs_get_subscript_count(const VSSubscriptionList *list)
{
	return list != NULL ? list->session_count : 0;
}

void vs_set_subscript_session(VSSubscriptionList *list, unsigned int session)
{
	verse_session_set(list->session[session]);
}

void vs_reset_subscript_session(void)
{
	verse_session_set(VSConnectionStorage.connection[VSConnectionStorage.current_session].session);
}

#endif
