/*
 * Copyright (c) 2010, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#define MI_TABLE_AS_ARRAY	0
#define MI_TABLE_AS_TREE	1

#include <osdep_internal.h>
#include <if_llc.h>
#include <if_upperproto.h>
#include <if_media.h>
#include <ieee80211_var.h>
#include <ieee80211_defines.h>
#include <ieee80211_mitbl.h>
#include <ieee80211_extap.h>

#if MI_TABLE_AS_TREE == MI_TABLE_AS_ARRAY
#	error "Define either one of MI_TABLE_AS_ARRAY or MI_TABLE_AS_TREE"
#endif
static inline void mi_node_print(mi_node_t *);

static inline void
mi_node_print(mi_node_t *n)
{
	printk(eamstr " ", eamac(n->h_dest));
	if (n->ip_ver == ATH_MITBL_IPV4) {
		printk(eaistr "\n", eaip(n->ip));
	} else {
		printk(eastr6 "\n", eaip6(n->ip));
	}
}
#if MI_TABLE_AS_ARRAY
#define NUM_MITBL_ENTRIES	32
static mi_node_t mi_tbl[NUM_MITBL_ENTRIES];

static mi_node_t *mi_tbl_lkup2(mi_node_t *, u_int8_t *, int);
void mi_tbl_dump(void *);

mi_node_t *
mi_tbl_add(EXTAP_DBG_PARM mi_node_t **root, u_int8_t *ip, u_int8_t *mac, int ipver)
{
	int		i, len = mi_ip_len(ipver);
	mi_node_t	*a;

	/*
	 * Non unicast addresses should not be added. Since the
	 * addresses being added come from ARP request/response
	 * (for IPv4), or Neighbor Discovery/Advertisement
	 * (for IPv6) packets, the input need not be validated.
	 */
	if ((a = mi_tbl_lkup2(*root, ip, ipver)) != NULL) {
		eadbg3("%s(%d): overwriting " eamstr " for " eaistr "\n",
			f, l, eamac(mac), eaip(ip));

		/*
		 * EV #98259, overwrite as the mac address
		 * might have changed.
		 */
		OS_MEMCPY(a->h_dest, mac, ETH_ALEN);

		return a;
	}

	for (i = 0; i < NUM_MITBL_ENTRIES; i++) {
		if (mi_node_is_free(&mi_tbl[i])) {
			OS_MEMCPY(mi_tbl[i].h_dest, mac, ETH_ALEN);
			OS_MEMCPY(mi_tbl[i].ip, ip, len);
			mi_tbl[i].ip_ver = ipver;
			mi_tbl[i].len = len;
			eadbg3("%s(%d): saving " eamstr " for " eaistr "\n",
				f, l, eamac(mac), eaip(ip));

			return &mi_tbl[i];
		}
	}

	eadbg3("%s(%d): cant save " eamstr " for " eaistr "\n",
		f, l, eamac(mac), eaip(ip));
	return NULL;
}

mi_node_t *
mi_tbl_lkup2(mi_node_t *root, u_int8_t *ip, int ipver)
{
	int		i, len = mi_ip_len(ipver);

	for (i = 0; i < NUM_MITBL_ENTRIES; i++) {
		if (!mi_node_is_free(&mi_tbl[i]) &&
		    (mi_tbl[i].ip_ver == ipver) &&
		    (OS_MEMCMP(ip, mi_tbl[i].ip, len) == 0)) {
			return &mi_tbl[i];
		}
	}
	return NULL;
}

u_int8_t *
mi_tbl_lkup(EXTAP_DBG_PARM mi_node_t *root, u_int8_t *ip, int ipver)
{
	mi_node_t	*a = mi_tbl_lkup2(root, ip, ipver);
	if (a) {
		return a->h_dest;
	}
	eadbg3("%s(%d): lkup failed for " eaistr "\n",
		f, l, eaip(ip));
	return NULL;
}

void
mi_tbl_del(EXTAP_DBG_PARM mi_node_t **root, u_int8_t *ip, int ipver)
{
	mi_node_t	*a = mi_tbl_lkup2(*root, ip, ipver);
	if (a) {
		mi_node_free(a);
		eadbg3("%s(%d): deleting " eamstr " for " eaistr "\n",
			f, l, eamac(a->h_dest), eaip(ip));
		return;
	}
	eadbg3("%s(%d): delete failed for " eaistr "\n", f, l, eaip(ip));
	return;
}

void
mi_tbl_dump(void *arg)
{
	int		i;

	printk("%s: array\n", __func__);
	for (i = 0; i < NUM_MITBL_ENTRIES; i++) {
		if (mi_tbl[i].len != ATH_MITBL_NONE) {
			mi_node_print(&mi_tbl[i]);
		}
	}

	return;
}

void
mi_tbl_purge(mi_node_t **root)
{
	int		i;

	for (i = 0; i < NUM_MITBL_ENTRIES; i++) {
		mi_tbl[i].len = ATH_MITBL_NONE;
	}

	return;
}
#endif /* MI_TABLE_AS_ARRAY */

#if MI_TABLE_AS_TREE

static __inline__ mi_node_t *mi_node_alloc(void);
static __inline__ void mi_node_free(mi_node_t *);
static void mi_set_key(mi_node_t *, u_int8_t *, int, u_int8_t *, int);
static int mi_prefix_match(u_int8_t *, u_int8_t *, int);
static int mi_addr_bit_set(u_int8_t *, int, int);

static __inline__ mi_node_t *
mi_node_alloc(void)
{
	mi_node_t *fn;

	if ((fn = (mi_node_t *)OS_MALLOC(NULL, sizeof(mi_node_t), GFP_KERNEL)) != NULL) {
		OS_MEMSET(fn, 0, sizeof(mi_node_t));
	}
	return fn;
}

static __inline__ void
mi_node_free(mi_node_t *fn)
{
	OS_FREE(fn);
}

static void
mi_set_key(mi_node_t *node, u_int8_t *ipaddr, int bit, u_int8_t *h_dest, int ip_ver)
{
	int i;

	if (h_dest == NULL) {	/* Intermediate Node */
		for (i = 0; i < (bit / 8); i++) {
			node->ip[i] = ipaddr[i];
		}
		if (bit % 8) {
			node->ip[i] =
			    (ipaddr[i] >> (8 - (bit % 8))) <<
			    (8 - (bit % 8));
		}
		node->len = bit;
	} else {
		if (ip_ver == ATH_MITBL_IPV4) {
			for (i = 0; i < 16; i++) {
				if (i < 4) {
					node->ip[i] = ipaddr[i];
				} else {
					node->ip[i] = 0;
				}
			}
			node->len = 32;
		} else if (ip_ver == ATH_MITBL_IPV6) {
			for (i = 0; i < 16; i++) {
				node->ip[i] = ipaddr[i];
			}
			node->len = 128;
		}

		for (i = 0; i < ETH_ALEN; i++) {
			node->h_dest[i] = h_dest[i];
		}
	}
	node->ip_ver = ip_ver;

}

static int
mi_prefix_match(u_int8_t *key, u_int8_t *ipaddr, int ip_ver)
{
	int i, j, bit = 0;

	if (ip_ver == ATH_MITBL_IPV4) {
		for (i = 0; i < 4; i++) {
			if (key[i] == ipaddr[i]) {
				bit = bit + 8;
			} else {
				break;
			}
		}

		/* Same ip addr exists in the tree */
		if (bit == 32)
			return bit;

		for (j = 1; j < 8; j++) {

			if ((key[i] &
			     ((unsigned char) ((0xFF >> j) << j))) ==
			    (ipaddr[i] &
			     ((unsigned char) ((0xFF >> j) << j)))) {
				bit = bit + (8 - j);
				eadbg1("\n %d Value of bit:%d", __LINE__, bit);
				return bit;
			}
		}
	} else if (ip_ver == ATH_MITBL_IPV6) {
		for (i = 0; i < 16; i++) {
			if (key[i] == ipaddr[i]) {
				bit = bit + 8;
			} else {
				break;
			}
		}

		/* Already the same ip addr exist in the tree */
		if (bit == 128)
			return bit;

		for (j = 1; j < 8; j++) {
			if ((key[i] & ((unsigned char) ((0xFF >> j) << j))) ==
			    (ipaddr[i] & ((unsigned char) ((0xFF >> j) << j)))) {
				bit = bit + (8 - j);
				eadbg1("\n%d Value of bit:%d", __LINE__, bit);
				return bit;
			}

		}
	}
	return bit;
}


mi_node_t *
mi_tbl_add(EXTAP_DBG_PARM mi_node_t **root, u_int8_t *ipaddr, u_int8_t *h_dest, int ip_ver)
{

	mi_node_t *fn, *in, *ln;
	int bit = 0, i;

	//if(ipv6_addr_type((struct in6_addr *)ipaddr) != IPV6_ADDR_UNICAST)
/*
	if (ipaddr[0] == 254) {
		eadbg1("\nMulticast addr...");
		return NULL;
	}
*/
	/* insert node in tree */

	if (ip_ver == ATH_MITBL_IPV4) {
		for (i = 0; i < 4; i++) {
			if (ipaddr[i] != 0) {
				break;
			}
		}
		if (i == 4) {
			return NULL;
		}
	} else if (ip_ver == ATH_MITBL_IPV6) {
		for (i = 0; i < 16; i++) {
			if (ipaddr[i] != 0) {
				break;
			}
		}
		if (i == 16) {
			return NULL;
		}
	}

	if (*root == NULL) {
		*root = mi_node_alloc();
		/* Modify for static analysis, prevent *root is NULL */
		if (*root == NULL)
		{
			return NULL;
		}
		
		for (i = 0; i < 6; i++)
			(*root)->h_dest[i] = h_dest[i];
		(*root)->left = NULL;
		(*root)->right = NULL;
		(*root)->parent = NULL;
		(*root)->ip_ver = ip_ver;
		if ((*root)->ip_ver == ATH_MITBL_IPV4) {
			for (i = 0; i < 4; i++)
				(*root)->ip[i] = ipaddr[i];
			(*root)->len = 32;
		} else if ((*root)->ip_ver == ATH_MITBL_IPV6) {
			for (i = 0; i < 16; i++)
				(*root)->ip[i] = ipaddr[i];
			(*root)->len = 128;
		}
		return (*root);
	}

	fn = *root;

	do {

		/*
		 *      Prefix match returns no. of bits matched with the key
		 */
		if (fn->ip_ver && (fn->ip_ver != ip_ver)) {
			bit = 0;
			/* create a new child node and intermediate node */
			in = mi_node_alloc();
			ln = mi_node_alloc();
			/* Modify for static analysis, prevent in/ln is NULL */
			if ((in == NULL) || (ln == NULL))
			{
				if (in) mi_node_free(in);
				if (ln) mi_node_free(ln);
				return NULL;
			}
			fn->parent = in;
			ln->parent = in;
			in->parent = NULL;
			if (ip_ver == ATH_MITBL_IPV4) {
				in->left = ln;
				in->right = fn;
                        } else if (ip_ver == ATH_MITBL_IPV6) {
				in->left = fn;
				in->right = ln;
                        }
			ln->left = NULL;
			ln->right = NULL;
			mi_set_key(ln, ipaddr, 0, h_dest, ip_ver);
			*root = in;
			return ln;
		} else if (!(fn->ip_ver) && !(fn->len)) {	/* Handle ip4 and IPV6 . Root node will have fn->len=0 and fn->ip_ver=0   */
			if (ip_ver == ATH_MITBL_IPV4) {
				fn = fn->left;
			} else if (ip_ver == ATH_MITBL_IPV6) {
				fn = fn->right;
			}
			continue;
		} else {
			bit = mi_prefix_match(fn->ip, ipaddr, ip_ver);
			if (bit > (fn->len)) {
				bit = fn->len;
			}
		}

		eadbg1("\nValue of fn->len = %d bit = %d", fn->len, bit);

		if (bit == fn->len) {
			if (ip_ver == ATH_MITBL_IPV4) {
				if (bit == 32) {
					eadbg1("\nIPV4:Overwriting");

					/*
					 * EV #98259, overwrite as the mac address
					 * might have changed.
					 */
					for (i = 0; i < ETH_ALEN; i++)
						fn->h_dest[i] = h_dest[i];

					break;
				}

				if (ipaddr[(bit / 8)] &
				    ((0x80) >> (bit % 8))) {
					eadbg1("\nIPV4:Descend to right node...");
					fn = fn->right;
				} else {
					fn = fn->left;
					eadbg1("\nIPV4:Descend in left node...");
				}
			}
			if (ip_ver == ATH_MITBL_IPV6) {
				if (bit == 128) {
					eadbg1("\nIPV6 : Overwriting");

					/*
					 * EV #98259, overwrite as the mac address
					 * might have changed.
					 */
					for (i = 0; i < ETH_ALEN; i++)
						fn->h_dest[i] = h_dest[i];

					break;
				}

				if (ipaddr[(bit / 8)] &
				    ((0x80) >> (bit % 8))) {
					eadbg1("\nIPV6: Descend to right node...");
					fn = fn->right;
				} else {
					fn = fn->left;
					eadbg1("\nIPV6: Descend left node...");
				}
			}

			continue;
		} else {
			/* create a new child node and intermediate node */
			in = mi_node_alloc();
			ln = mi_node_alloc();

			in->parent = fn->parent;
			//in->ip_ver = 0;
			fn->parent = in;
			ln->parent = in;

			if ((in->parent) != NULL) {
				if (((in->parent)->left) == fn) {
					eadbg1("\nAdded in left node");
					(in->parent)->left = in;
				} else {
					eadbg1("\nAdded in right node");
					(in->parent)->right = in;
				}
			} else {
				*root = in;
			}

			if (ip_ver == ATH_MITBL_IPV4) {
				if (ipaddr[(bit / 8)] &
				    ((0x80) >> (bit % 8))) {
					in->right = ln;
					in->left = fn;
				} else {
					in->left = ln;
					in->right = fn;
				}
			} else if (ip_ver == ATH_MITBL_IPV6) {
				if (ipaddr[(bit / 8)] &
				    ((0x80) >> (bit % 8))) {
					in->right = ln;
					in->left = fn;
				} else {
					in->left = ln;
					in->right = fn;
				}
			}

			ln->left = NULL;
			ln->right = NULL;

			/* Need to set the key for in and ln */

			mi_set_key(in, ipaddr, bit, NULL, ip_ver);
			mi_set_key(ln, ipaddr, 0, h_dest, ip_ver);

			eadbg1("root=%p @ %d", *root, __LINE__);
			return ln;
		}
	} while (fn);

	return NULL;
}

static int
mi_addr_bit_set(u_int8_t *ipaddr, int len, int ip_ver)
{
	if (ip_ver == ATH_MITBL_IPV4) {
		return (ipaddr[(len / 8)] & ((0x80) >> (len % 8)));
	} else if (ip_ver == ATH_MITBL_IPV6) {
		return (ipaddr[(len / 8)] & ((0x80) >> (len % 8)));
	}
	return 0;
}

void
mi_tbl_del(EXTAP_DBG_PARM mi_node_t **root, u_int8_t *ipaddr, int ip_ver)
{

	mi_node_t *fn, *in, *ln, *next;
	int dir = 0, bit = 0;

	/*
	 *      Descend on a tree
	 */

	if (*root == NULL) {
		eadbg1("\nThere is no entry in the Table");
		return;
	}
	fn = *root;

	for (;;) {
		if (fn->right && fn->left) {
			if (!fn->len && !fn->ip_ver) {
				if (ip_ver == ATH_MITBL_IPV4) {
					dir = 0;
				} else if (ip_ver == ATH_MITBL_IPV6) {
					dir = 1;
				}
			} else {
				dir = mi_addr_bit_set(ipaddr, fn->len, ip_ver);	// return 0 for left
			}
			next = dir ? fn->right : fn->left;
			fn = next;
			continue;
		} else {
			/* To identify no match found */
			bit = mi_prefix_match(fn->ip, ipaddr, ip_ver);
			if ((ip_ver == ATH_MITBL_IPV4) && (bit != 32)) {
				return;
			} else if ((ip_ver == ATH_MITBL_IPV6) && (bit != 128)) {
				return;
			} else {
				in = fn->parent;

				if (in == NULL) {	// Tree has only 1 entry
					mi_node_free(fn);
					*root = NULL;
					return;
				}

				if (in->right == fn) {
					ln = in->left;
				} else {
					ln = in->right;
				}

				ln->parent = in->parent;

				if ((in->parent) != NULL) {
					if ((in->parent)->left == in) {
						(in->parent)->left = ln;
					} else {
						(in->parent)->right = ln;
					}
				} else {
					*root = ln;
				}

				mi_node_free(in);
				mi_node_free(fn);
				return;
			}
		}

	}

}

u_int8_t *
mi_tbl_lkup(EXTAP_DBG_PARM mi_node_t *root, u_int8_t *ipaddr, int ip_ver)
{
	mi_node_t *fn, *next;
	int dir = 0, i, bit;


	/*
	 *      Descend on a tree
	 */
	if (root == NULL) {
		eadbg1("\nThere is no entry in the Table");
		return NULL;
	}

	fn = root;
	next = root;

	for (;;) {
		if (fn->right && fn->left) {
			if (!fn->len && !fn->ip_ver) {
				if (ip_ver == ATH_MITBL_IPV4) {
					dir = 0;
				} else if (ip_ver == ATH_MITBL_IPV6) {
					dir = 1;
				}
				} else {
					dir = mi_addr_bit_set(ipaddr, fn->len, ip_ver);	// return 0 for left
				}
				next = dir ? fn->right : fn->left;
				fn = next;
				continue;
			} else {
				/* To identify no match found */
				bit = mi_prefix_match(fn->ip, ipaddr, ip_ver);
				if ((ip_ver == ATH_MITBL_IPV4) && (bit != 32)) {
					eadbg1("\nNo Match found");
					return NULL;
				} else if ((ip_ver == ATH_MITBL_IPV6) && (bit != 128)) {
					eadbg1("\nNo Match found");
				return NULL;
			}

			eadbg1("\nMatch found");
			for (i = 5; i >= 0; i--) {
				eadbg1("%x:", next->h_dest[i]);
			}
			return next->h_dest;
		}

		//fn = next;
	}

	return NULL;

}

void
mi_tbl_purge(mi_node_t **root)
{
    if(NULL == root || NULL == *root) {
        return;
    }
	if((*root)->left) {
		mi_tbl_purge(&((*root)->left));
		mi_tbl_purge(&((*root)->right));
	}

	if((*root)->parent == NULL) {
		mi_node_free(*root);
		*root = NULL;
	} else {
		mi_node_free(*root);
	}
}

void
mi_tbl_dump(void *arg)
{
	mi_node_t		*root = (mi_node_t *)arg;
	if(!root) {
		return;
	}
	if(!root->left) {
		mi_node_print(root);
#if 0
		printk
		    ("\n\n ****root->left->len=%d root->left->ip_ver=%d\n",
		     root->left->len, root->left->ip_ver);

		printk("\n**root->left->h_dest = %x:%x:%x:%x:%x:%x \n",
		       root->left->h_dest[5], root->left->h_dest[4],
		       root->left->h_dest[3], root->left->h_dest[2],
		       root->left->h_dest[1], root->left->h_dest[0]);

		printk("\n ****root->left->ipaddr = %d:%d:%d:%d \n",
		       root->left->ip[0],
		       root->left->ip[1],
		       root->left->ip[2],
		       root->left->ip[3]);

		printk
		    ("\n\n ****root->right->len=%d root->right->ip_ver=%d\n",
		     root->right->len, root->right->ip_ver);

		printk("\n**root->right->h_dest = %x:%x:%x:%x:%x:%x \n",
		       root->right->h_dest[5], root->right->h_dest[4],
		       root->right->h_dest[3], root->right->h_dest[2],
		       root->right->h_dest[1], root->right->h_dest[0]);

		printk("\n ****root->right->ipaddr = %d:%d:%d:%d \n",
		       root->right->ip[0],
		       root->right->ip[1],
		       root->right->ip[2],
		       root->right->ip[3]);
#endif
	} else {
		mi_tbl_dump(root->left);
		mi_tbl_dump(root->right);
	}

}

#if 0
main()
{

	char choice = 'y';
	int option;

	mi_node_t *root = NULL;
	u_int8_t ipaddr[16];
	unsigned char temp[17];
	unsigned char h_dest[ETH_ALEN];
	int ip_ver, i, j;


	while (choice == 'y') {
		eadbg1("\n Enter you option 1) Add 2) Delete 3) Search :");
		scanf("%d", &option);
		switch (option) {
		case 1:
			eadbg1("Enter the mac addr,ip_ver:");
			for (i = 5; i >= 0; i--) {
				scanf("%x", &j);
				h_dest[i] = (unsigned char) j;
			}
			scanf("%d", &ip_ver);

			for (i = 5; i >= 0; i--) {
				eadbg1("%x", h_dest[i]);
				eadbg1(":");
			}
			eadbg1("\n");

			if (ip_ver == 1) {
				eadbg1("Enter the ipv4 addr:");
				for (i = 3; i >= 0; i--) {
					scanf("%d", &j);
					ipaddr[i] = (unsigned char) j;
				}
			} else if (ip_ver == 2) {
				eadbg1("Enter the ipv6 addr:");
				for (i = 15; i >= 0; i--) {
					scanf("%d", &j);
					ipaddr[i] = (unsigned char) j;
				}
			}
			eadbg1("\n");
			for (i = ((ip_ver == 1) ? 3 : 15); i >= 0; i--) {
				eadbg1("ipaddr[%d] = %d:\n", i, ipaddr[i]);
			}
			if (root == NULL) {
				root = mi_node_alloc();
				for (i = 0; i < 6; i++)
					root->h_dest[i] = h_dest[i];
				root->left = NULL;
				root->right = NULL;
				root->parent = NULL;
				root->ip_ver = ip_ver;
				if (root->ip_ver == 1) {
					for (i = 0; i < 4; i++)
						root->ip[i] = ipaddr[i];
					root->len = 32;
				} else {
					for (i = 0; i < 16; i++)
						root->ip[i] = ipaddr[i];
					root->len = 128;
				}
			} else {
				mi_add(&root, ipaddr, h_dest, ip_ver);
				//print_tree(root);
			}
			break;
		case 2:
			eadbg1("Enter the ip_ver:");
			scanf("%d", &ip_ver);

			if (ip_ver == 1) {
				eadbg1("Enter the ipv4 addr:");
				for (i = 3; i >= 0; i--) {
					scanf("%d", &j);
					ipaddr[i] = (unsigned char) j;
				}
			} else if (ip_ver == 2) {
				eadbg1("Enter the ipv6 addr:");
				for (i = 15; i >= 0; i--) {
					scanf("%d", &j);
					ipaddr[i] = (unsigned char) j;
				}
			}


			if (root == NULL) {
				eadbg1("There is no entry in the Table");
			} else {
				if (root != NULL) {
					mi_tbl_del(&root, ipaddr, ip_ver);
				} else {
					eadbg1("\n No nodes present");
				}
			}
			break;
		case 3:
			eadbg1("Enter the ip_ver:");
			scanf("%d", &ip_ver);

			if (ip_ver == 1) {
				eadbg1("Enter the ipv4 addr:");
				for (i = 3; i >= 0; i--) {
					scanf("%d", &j);
					ipaddr[i] = (unsigned char) j;
				}
			} else if (ip_ver == 2) {
				eadbg1("Enter the ipv6 addr:");
				for (i = 15; i >= 0; i--) {
					scanf("%d", &j);
					ipaddr[i] = (unsigned char) j;
				}
			}

			if (root == NULL) {
				eadbg1("There is no entry in the Table");
			} else {
				mi_lkup(root, ipaddr, ip_ver);
			}
			break;

		default:

			eadbg1("Invalid Option");
			print_tree(root);
			continue;
		}

	}


}
#endif
#endif /* MI_TABLE_AS_TREE */
