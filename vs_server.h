/*
 * $Id: vs_server.h 4533 2008-08-11 15:00:51Z jiri $ 
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

extern void			vs_init_connection_storage(void);
extern void			vs_add_new_connection(VSession session, const char *name, const char *pass, VNodeID node_id);
extern void			vs_remove_connection(void);
extern void			vs_set_next_session(void);

typedef void VSSubscriptionList;

extern VSSubscriptionList * vs_create_subscription_list(void);
extern void			vs_destroy_subscription_list(VSSubscriptionList *list);
extern int			vs_add_new_subscriptor(VSSubscriptionList *list);
extern void			vs_remove_subscriptor(VSSubscriptionList *list);
extern unsigned int		vs_get_subscript_count(const VSSubscriptionList *list);
extern void			vs_set_subscript_session(VSSubscriptionList *list, unsigned int session);
extern void			vs_reset_subscript_session(void);
extern uint32		vs_get_avatar(void);
extern VSession		vs_get_session(void);
extern const char *		vs_get_user_name(void);
extern const char *		vs_get_user_pass(void);


typedef struct {
	VNodeID			id;
	VNodeType		type;
	VNodeID			owner;
	char			*name;
	void			*tag_groups;
	uint16			group_count;
	VSSubscriptionList	*subscribers;
} VSNodeHead;

extern void			vs_init_node_storage(void);
extern uint32		vs_add_new_node(VSNodeHead *node, VNodeType type);
extern VSNodeHead *	vs_get_node(unsigned int node_id, VNodeType type);
extern VSNodeHead *	vs_get_node_head(unsigned int node_id);

extern void			create_node_head(VSNodeHead *node, const char *name, unsigned int owner);
extern void			destroy_node_head(VSNodeHead *node);
extern void			vs_send_node_head(VSNodeHead *node);

extern void			vs_h_callback_init(void);	/* "Head", not an actual node type. */
extern void			vs_o_callback_init(void);
extern void			vs_g_callback_init(void);
extern void			vs_m_callback_init(void);
extern void			vs_b_callback_init(void);
extern void			vs_t_callback_init(void);
extern void			vs_c_callback_init(void);
extern void			vs_a_callback_init(void);
extern void			init_callback_node_storage(void);

extern void		vs_master_set_enabled(boolean enabled);
extern void		vs_master_set_address(const char *address);
extern const char *	vs_master_get_address(void);
extern void		vs_master_set_desc(const char *desc);
extern void		vs_master_set_tags(const char *tags);
extern void		vs_master_update(void);
extern void		vs_master_handle_describe(const char *address, const char *message);
