#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include "rmalloc.h" 


static rm_header rm_free_list = { 0x0, 0 } ;
static rm_header rm_used_list = { 0x0, 0 } ;

rm_header_ptr p_prev_free_node = &rm_free_list; // points to last node on the free_list
rm_header_ptr p_last_used_node = &rm_used_list;	// points to last node on the used_list



void insert_node(rm_header_ptr new_node, rm_header node_list);

// v_addr->next = null, v_addr->size = s
// returns the ptr of the v_addr. 
rm_header_ptr create_node(void * v_addr, size_t s){
	printf("(create_node) Creating node at %p with size %ld\n", v_addr, s);
	rm_header_ptr newnode = (rm_header_ptr) v_addr;
	newnode->next = 0x0;
	newnode->size = s;

	return newnode;
}

// size section of rm_header 
static 
void * 
_data (rm_header_ptr e)
{
	return ((void *) e) + sizeof(rm_header) ;
}

static 
void 
sm_container_split (rm_header_ptr prev_hole, size_t size)
{
	rm_header_ptr hole = prev_hole->next;
	// remainder address
	rm_header_ptr remainder = (rm_header_ptr) (_data(hole) + size) ;

	// (free list)
	remainder->size = hole->size - size - sizeof(rm_header) ;
	remainder->next = hole->next ;
	prev_hole->next = remainder;

	// (used list)
	hole->next = rm_used_list.next;
	hole->size = size ;
	rm_used_list.next = hole;
}

static 
void * 
retain_more_memory (size_t size)
{
	rm_header_ptr hole ;
	size_t pagesize = getpagesize() ;
	size_t n_pages = 0;

	n_pages = (sizeof(rm_header) + size ) / pagesize  + 1 ;
	hole = (rm_header_ptr) mmap(0x0, n_pages *n_pages, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

	printf("pagesize : %ld\n", pagesize);
	
	if (hole == 0x0)
		return 0x0 ;
	
	hole->size = n_pages * pagesize - sizeof(rm_header) ;
	printf("pagesize : %ld\n", hole->size);
	return hole ;
}

void * 
rmalloc (size_t size) 
{
	rm_header_ptr hole = 0x0, itr = 0x0 ;
	rm_header_ptr prev_hole = &rm_free_list;

	for (itr = rm_free_list.next ; itr != 0x0 ; itr = itr->next) {
		
		// free list fits or smaller
		if ((itr->size == size) || (size + sizeof(rm_header) < itr->size)) {
			hole = itr ;
			break ; 
		}
		prev_hole = itr;
	}

	// if couldn't find a hole
	if (hole == 0x0) {
		hole = retain_more_memory(size) ;
		if (hole == 0x0)
			return 0x0 ;
		
		hole->next = rm_free_list.next;
		rm_free_list.next = hole;

		prev_hole = &rm_free_list;

	}
	//if hole is found
	if (size < hole->size)
		// split the area
		sm_container_split(prev_hole, size) ;
	return _data(hole) ;
}

void insert_node(rm_header_ptr newNode, rm_header node_list){
	newNode->next = node_list.next;
	node_list.next = newNode;
}

void delete_node(){}
void rfree (void * p) 
{
	// TODO 
	rm_header_ptr itr ;
	for (itr = rm_used_list.next ; itr != 0x0 ; itr = itr->next) {
		if (p == _data(itr)) {

			// insert front at free list
			//insert_node(itr, rm_free_list);
			break ;
		}
	}
}



void * rrealloc (void * p, size_t s) 
{
	void * p_orig; 

	// iterate and find p in used_list
	for(rm_header_ptr u_iter = rm_used_list.next ; u_iter != 0x0 ; u_iter = u_iter->next){
		
		// if p is found
		if(p == u_iter){
			for(rm_header_ptr f_iter = rm_free_list.next ; f_iter != 0x0 ; f_iter = f_iter->next){
				p + sizeof(rm_header) + s;
				// if(){

				// }
			// check if reallocatable (iterate free list and find if (p + sizeof(rm_header + size)) is between(free_add - size , free_add))
			break;
			}
		}

		
		// if not found, just alloc			
		if(u_iter->next == 0x0);
		
	}

	// if size == 0 just allocate with 0 address
	if(s == 0){
		// remove from
		return NULL;
	}

	return 0x0 ; 
}

void rmshrink () 
{
	// TODO
}

void rmconfig (rm_option opt) 
{
	// TODO
}

void 
rmprint () 
{
	rm_header_ptr itr ;
	int i ;

	printf("==================== rm_free_list ====================\n") ;
	for (itr = rm_free_list.next, i = 0 ; itr != 0x0 ; itr = itr->next, i++) {
		printf("%3d:%p:%8d:", i, ((void *) itr) + sizeof(rm_header), (int) itr->size) ;

		int j ;
		char * s = ((char *) itr) + sizeof(rm_header) ;
		for (j = 0 ; j < (itr->size >= 8 ? 8 : itr->size) ; j++) 
			printf("%02x ", s[j]) ;
		printf("\n") ;
	}
	printf("\n") ;

	printf("==================== rm_used_list ====================\n") ;
	for (itr = rm_used_list.next, i = 0 ; itr != 0x0 ; itr = itr->next, i++) {
		printf("%3d:%p:%8d:", i, ((void *) itr) + sizeof(rm_header), (int) itr->size) ;

		int j ;
		char * s = ((char *) itr) + sizeof(rm_header) ;
		for (j = 0 ; j < (itr->size >= 8 ? 8 : itr->size) ; j++) 
			printf("%02x ", s[j]) ;
		printf("\n") ;
	}
	printf("\n") ;
}
