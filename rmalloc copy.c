#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include "rmalloc.h" 


static rm_header rm_free_list = { 0x0, 0 } ;
static rm_header rm_used_list = { 0x0, 0 } ;

rm_header_ptr p_prev_free_node = &rm_free_list;
rm_header_ptr p_prev_used_node = &rm_used_list;
/* 
 * USE mmap to allocate memory by page
 * 1. first fit
 * 2. best fit
 * 3. worst fit 
 */

#define ADDRESS_SIZE 1<<33

#define GET_FREE_ADDR(v_addr, size)  (void *)v_addr - size 						// free_addr - size
#define GET_INIT_FREE(pg_addr, pg_size) pg_addr + pg_size - sizeof(rm_header)	// returns address of initial free node at new page. 


// v_addr->next = null, v_addr->size = s
// returns the ptr of the v_addr. 
rm_header_ptr create_node(void * v_addr, size_t s){
	printf("(create_node) Creating node at %p with size %ld\n", v_addr, s);
	rm_header_ptr newnode = (rm_header_ptr) v_addr;
	newnode->next = 0x0;
	newnode->size = s;

	
	return newnode;
}

// mmap more size by pgsize
void call_more_page(size_t pg_size, rm_header_ptr end_node){
	size_t pgsize = sysconf(_SC_PAGE_SIZE);
	printf("Calling more pages(%ld) \n", pgsize);
	void * pg_front = mmap(0x0, pg_size / pgsize + pgsize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

	// link to last node of free_list
	end_node->next = create_node(GET_INIT_FREE(pg_front, pg_size), pg_size - sizeof(rm_header) );

}


void * rmalloc (size_t s) 
{
	void *pg_free;

	// Initilize first page, bring pages in lazy manner.
	if(rm_free_list.next == 0x0){
		printf("Initialize free list\n");
		size_t pgsize = sysconf(_SC_PAGE_SIZE);

		pg_free = mmap(0x0, pgsize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
		printf("init at %p\n", pg_free);
		rm_free_list.next = create_node( GET_INIT_FREE(pg_free, pgsize), pgsize - sizeof(rm_header));
	}

	// Allocate s size on free list with iterating
	for(rm_header_ptr iter = rm_free_list.next ; iter != 0x0 ; iter = iter->next){
		printf("Iterating free list at :%p, size : %ld\n", iter, iter->size);

		// consider two cases : 1. no page available 2. no v_size found. What to consider first ...
		
		// if size == s
		if(s == iter->size){
			printf("size == request\n");
			// switch free to used and add it at the end of the list
			p_prev_used_node->next = create_node(GET_FREE_ADDR(iter, iter->size), s);
			p_prev_used_node = p_prev_used_node->next;

			// remove from free list
			if(iter->next == 0x0){
			    //free(zeroize the area)

				p_prev_free_node->next = 0x0;
			}
			else{
				p_prev_free_node->next = iter->next;
				//free(zeroize the area)
			}
			break;
		}

		// if s < size
		if(s < iter->size){
			printf("request < size at %p\n", iter);
			printf("iter->size %ld: \n", iter->size);
			// insert

			p_prev_used_node->next = create_node(GET_FREE_ADDR(iter, iter->size), s);
			printf("HEere\n");
			p_prev_used_node = p_prev_used_node->next;

			// reduce the size by s + sizeof(node)

			iter->size = iter->size - (s + sizeof(rm_header));
			break;
		}

		// if s > pgsize and not found in iteration
		if(iter->next == 0x0){
			call_more_page(s, p_prev_free_node->next);
		}


		// recording previous free node
		p_prev_free_node = iter;
	}

	
	// first fit
	// iterate free_list and find a first free space and allocate area 

	return p_prev_free_node->next ; // erase this
}

void rfree (void * p) 
{
	// TODO 
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

// int main(){
// 	//dont use this one
// 	rmalloc(10);
// 	rmprint();
// 	rmalloc(100);
// 	rmprint();
// 	rmalloc(2000);
// 	rmprint();
// }


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
