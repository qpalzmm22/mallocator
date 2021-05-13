#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include "rmalloc.h" 


rm_header rm_free_list = { 0x0, 0 } ;
rm_header rm_used_list = { 0x0, 0 } ;

rm_option g_opt = FirstFit;

static int* pg_free = 0x0;

/* 
 * USE mmap to allocate memory by page
 * 1. first fit
 * 2. best fit
 * 3. worst fit 
 */


rm_header* create_node(void * v_addr, size_t s){
	printf("Creating node at %p with size %ld\n", v_addr, s);
	printf("HI\n");
	rm_header * newfree = (rm_header *)v_addr;
	printf("1B");
	newfree->size = s;
	printf("1C");
	newfree->next = 0x0;
	printf("1D");

	return newfree;
}


void * rmalloc (size_t s) 
{
	int *mapped = NULL;
	long int a=0;
	int* newmapped = 0x0;
	newmapped = mmap(0x0, 50, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	printf("p: %p : %ld\n", newmapped,a++);
	newmapped = mmap(0x0, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	printf("p: %p : %ld\n", newmapped,a++);
	newmapped = mmap(0x0, sysconf(_SC_PAGE_SIZE)+2, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	printf("p: %p : %ld\n", newmapped,a++);
	newmapped = mmap(0x0, sysconf(_SC_PAGE_SIZE)+3, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	printf("p: %p : %ld\n", newmapped,a++);
	printf("p: %p\n", newmapped);
	printf("p: %p\n",NULL);

	static rm_header * lst_used_node = 0x0;
	rm_header * prev_free_node;

	// Initilize first page, bring pages in lazy manner.
	if(rm_free_list.next == 0x0){
		printf("Initialize free list\n");
		pg_free = mmap(0x0, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
		
		rm_free_list.next = create_node(pg_free + sysconf(_SC_PAGE_SIZE) - sizeof(rm_header), sysconf(_SC_PAGE_SIZE) - sizeof(rm_header));
		rm_free_list.size = sysconf(_SC_PAGE_SIZE);

		// initialize prev_free_node
		prev_free_node = &rm_free_list;

	}

	// Allocate s size on free list with iterating
	for(rm_header * iter = rm_free_list.next; iter != 0x0; iter = iter->next){
		printf("Iterating free list at :%p, size : %ld\n", iter, iter->size);

		// consider two cases : 1. no page available 2. no v_size found. What to consider first ...
		
		// if size == s
		if(s == iter->size){
			printf("size == request\n");
			// switch free to used and add it at the end of the list
			lst_used_node->next = create_node(iter + iter->size, s);
			lst_used_node = lst_used_node->next;

			// remove from free list
			if(iter->next == 0x0){
			    //free(zeroize the area)

				prev_free_node = 0x0;
			}
			else{
				prev_free_node->next = iter->next;
				//free(zeroize the area)
			}
		}

		// if s < size
		if(s < iter->size){
			printf("request < size %p\n", iter );
			
			lst_used_node->next = create_node(iter, s);
			printf("1");
			lst_used_node = lst_used_node->next;
			printf("2");
			// reduce the size by s + sizeof(node)
			iter->size = iter->size - (s + sizeof(rm_header));
			printf("3");
		}

		// if s > pgsize


		// recording previous free node
		// if(iter->next != 0x0){
			
		// }
		prev_free_node = iter;
	}

	
	// first fit
	// iterate free_list and find a first free space and allocate area 

	return 0x0 ; // erase this
}

void rfree (void * p) 
{
	// TODO 
}

void * rrealloc (void * p, size_t s) 
{
	// TODO
	return 0x0 ; // erase this 
}

void rmshrink () 
{
	// TODO
}

void rmconfig (rm_option opt) 
{
	
}

int main(int argc, char** argv)
{
	rmalloc(10);
	rmprint();
	rmalloc(20);
	rmprint();
	return 0;
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
