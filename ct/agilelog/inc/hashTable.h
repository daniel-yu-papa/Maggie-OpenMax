#ifndef _HASHTABLE_H__
#define _HASHTABLE_H__

#include <stdlib.h>

namespace AGILELOG {

/*BEGIN: The Macros for hlist operations*/

#define ht_offsetof(TYPE, MEMBER) ((unsigned int) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - ht_offsetof(type,member) );})

#define hlist_entry(ptr, type, member) container_of(ptr,type,member)

/**
 * hlist_for_each_entry	- iterate over list of given type
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct hlist_node to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry(tpos, pos, head, member)    \
	for (pos = (head)->first;					 \
	     pos &&							 \
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)

#define hlist_for_each_entry_to_tail(tpos, pos, prev, head, member)    \
	for (pos = (head)->first;					 \
	     pos &&							 \
	    ({ prev = pos; 1;}) &&          \
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)
	     
/*END: The Macros for hlist operations*/
    
struct hlist_node {
	struct hlist_node *next, **pprev;
};

struct hlist_head {
	struct hlist_node *first;
};

typedef struct hash_table{
    struct hlist_head head;
}HashTable_t;

typedef struct linked_key_node{
    struct hlist_node node;
    void *nodeValue;
    char nodeStr[64]; //the length of the key should be less than 64
}LinkedKeyNode_t;

class HashTable{
public:
    HashTable(int num);
    ~HashTable();

    void buildHashTable(void *item, const char *str);
    void *getHashItem(const char *str);
    void printHashTable(void);
private:
    unsigned int calcDJBHashValue(char* str, unsigned int len);

    /*handle the hlist.*/
    inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
    {
    	struct hlist_node *first = h->first;
    	n->next = first;
    	if (first)
    		first->pprev = &n->next;
    	h->first = n;
    	n->pprev = &h->first;
    };

    inline void hlist_del(struct hlist_node *n)
    {
    	struct hlist_node *next = n->next;
    	struct hlist_node **pprev = n->pprev;
    	*pprev = next;
    	if (next)
    		next->pprev = pprev;
    	n->next  = NULL;
    	n->pprev = NULL;
    };

    inline void INIT_HLIST_NODE(struct hlist_node *h)
    {
    	h->next = NULL;
    	h->pprev = NULL;
    };
    
    HashTable_t *mpTable;
    int mTableSize;
};

}
#endif