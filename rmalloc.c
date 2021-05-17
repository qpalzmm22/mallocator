#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include "rmalloc.h" 

#define MAX_HEAP_SIZE 1024

static rm_header rm_free_list = { 0x0, 0 } ;
static rm_header rm_used_list = { 0x0, 0 } ;

rm_header_ptr p_prev_free_node = &rm_free_list; // points to last node on the free_list
rm_header_ptr p_last_used_node = &rm_used_list;	// points to last node on the used_list





static void * page_boundary;

typedef struct{
	rm_header_ptr heap[MAX_HEAP_SIZE];
	size_t heap_size;
} HeapHeader;
HeapHeader free_h;
HeapHeader *free_heap = &free_h;



void print_heap(){
	int i = 1;
	while(i <= free_heap->heap_size){
		printf("%p with size %ld\n", free_heap->heap[i], free_heap->heap[i]->size);
		i++;
	}
}

// min heap that stores the free memory address in order
void insert_heap(rm_header_ptr free_mem){
	int i = ++(free_heap->heap_size); 

	// while it's not root node and parent has greater value,
	while((i != 1) && (free_mem < free_heap->heap[i/2])){
		// switch parent value with child's value
		free_heap->heap[i] = free_heap->heap[i/2];

		i /= 2;
	}
	free_heap->heap[i] = free_mem;

	print_heap();
}
// Pop min_value
rm_header_ptr pop_min_heap(){
	int parent, child;
	rm_header_ptr item, temp;

	// Nothing in the heap
	if(free_heap->heap_size <= 0){
		printf("Nothing in the heap\n");
		return 0x0;
	}
	item = free_heap->heap[1]; // 루트 노드 값을 반환하기 위해 item에 할당
	temp = free_heap->heap[(free_heap->heap_size)--]; // 마지막 노드를 temp에 할당하고 힙 크기를 하나 감소
	
	// initial value
	parent = 1;
	child = 2;

	// Heapify
	while(child <= free_heap->heap_size){

		// find smallest node among a parent and two children (루트 노드의 왼쪽 자식 노드(index: 2)부터 비교 시작)
		if( (child < free_heap->heap_size) && ((free_heap->heap[child]) < free_heap->heap[child+1]) ){
		child++;
		}
		// 더 큰 자식 노드보다 마지막 노드가 크면, while문 중지
		if( temp <= free_heap->heap[child] ){
		break;
		}

		// 더 큰 자식 노드보다 마지막 노드가 작으면, 부모 노드와 더 큰 자식 노드를 교환
		free_heap->heap[parent] = free_heap->heap[child];
		// 한 단계 아래로 이동
		parent = child;
		child *= 2;
	}

	// 마지막 노드를 재구성한 위치에 삽입
	free_heap->heap[parent] = temp;
	// 최댓값(루트 노드 값)을 반환
	return item;
}

static
void insert_node(rm_header_ptr new_node, rm_header_ptr node_list);

// v_addr->next = null, v_addr->size = s
// returns the ptr of the v_addr.
static
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
	hole = (rm_header_ptr) mmap(0x0, pagesize *n_pages, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

	printf("pagesize : %ld\n", n_pages * pagesize);
	
	if (hole == 0x0)
		return 0x0 ;

		
	// if it's the top most page
	if(rm_free_list.next == 0x0){
		// Remember the page_boundary(lowest it can get)
		page_boundary = (void *)hole + n_pages * pagesize;
	}


	hole->size = n_pages * pagesize - sizeof(rm_header) ;

	return hole ;
}

void * 
rmalloc (size_t size) 
{
	rm_header_ptr hole = 0x0, itr = 0x0 ;
	rm_header_ptr prev_hole = &rm_free_list;
	
	rm_header_ptr min_hole = 0x0;
	rm_header_ptr max_hole = 0x0;

	size_t min_space = 10000;
	size_t max_space = 0;


	for (itr = rm_free_list.next ; itr != 0x0 ; itr = itr->next) {
		size_t surplus = itr->size - size;
		// free list fits or smaller (first fit)
		if ((itr->size == size) || (size + sizeof(rm_header) < itr->size)) {
			hole = itr ;
			break ; 
		}
		// (best fit)
		if(min_space > surplus){
			min_hole = itr;	
			min_space = surplus;
		}

		// (worst fit)
		if(max_space > surplus){
			max_hole = itr;	
			max_space = surplus;
		}
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

// insert front
static
void 
insert_node(rm_header_ptr newNode, rm_header_ptr node_list){
	newNode->next = node_list->next;
	node_list->next = newNode;
}

// Needs to pass previous node to delete current node.
static
void 
delete_node(rm_header_ptr prev_node){
	prev_node->next = prev_node->next->next;
}
static
void 
find_delete_node(rm_header_ptr bigger){
	rm_header_ptr prev_node = &rm_free_list;
	for(rm_header_ptr iter = rm_free_list.next; iter != 0; iter = iter->next){
		if(iter == bigger){
			prev_node->next = iter->next;
		}
		prev_node = iter;
	}
}
void rfree (void * p) 
{
	rm_header_ptr itr ;
	rm_header_ptr prev_node = &rm_used_list;
	for (itr = rm_used_list.next ; itr != 0x0 ; itr = itr->next) {
		if (p == _data(itr)) {
			printf("--- node found ---\n");

			// remove from used_list
			delete_node(prev_node);
			
			// insert front at free_list
			insert_node(itr, &rm_free_list);
			
			break ;
		}
		prev_node = itr;
	}
}



void * rrealloc (void * p, size_t s) 
{
	void * p_orig; 

	// iterate and find p in used_list
	for(rm_header_ptr u_iter = rm_used_list.next ; u_iter != 0x0 ; u_iter = u_iter->next){

		// if p is found
		if(p == _data(u_iter)){
			// iterate and find if there is any range that sits next to the found address.
			
			// range setting
			void * p_old_data =_data(u_iter) + u_iter->size;
			void * p_new_data = _data(u_iter) + s;

			for(rm_header_ptr f_iter = rm_free_list.next ; f_iter != 0x0 ; f_iter = f_iter->next){
				
				// Assume that pages grow up && doesn't randomly drop down.
				// If the expected address exceeds the loweset page boundary, ignore it
				if(p_new_data > page_boundary) {
					printf("execeedede page boundart\n");
					continue;
				}

				// if there is a node that is inclusive in the range, aka if it's not reallocatable
				if(p_new_data > (void*) f_iter && p_old_data < (void*) f_iter){
					printf("memory already in use...\n");

					// Free the existing allocation 
					rfree(_data(u_iter));

					// And make new allocation.
					return _data(rmalloc(s));

				}
				// Looked for all the free nodes 
				if(f_iter->next == 0x0){
					printf("resize current one\n");

					// resize the current node
					u_iter->size = s;
					return _data(u_iter);
				}
			}
		}

		// if not found, just allocate it.	
		if(u_iter->next == 0x0){
			printf("can't find node... just make new one\n");
			_data(rmalloc(s));
		}
	}

	return 0x0 ; 
}


void rmshrink () 
{
	rm_header_ptr smallest = 0x0;
	// O(N^2) maybe???
	// sort the free memories so that free_list is in order 
	for(rm_header_ptr iter = rm_free_list.next; iter != 0x0 ; iter = iter->next){
		printf("printing heap\n");
		insert_heap(iter);
	}
	
	rm_header_ptr smaller, bigger;
	
	
	smaller = pop_min_heap();
	bigger = pop_min_heap();

	if(smallest == 0x0)
		smallest = smaller;
	// pop one by one and see if the add + size is next one
	while(smaller != 0x0 && bigger != 0x0){
		// shrinkable
		printf("smaller : %p, bigger : %p\n", smaller, bigger);
		printf("%p, %p \n", (void *)smaller + smaller->size + sizeof(smaller), (void *)bigger);
		if((void *)smaller + smaller->size + sizeof(smaller) == (void *)bigger){
			printf("Shrinkable :%p \n", bigger);
		
			// resize the small one
			smaller->size = bigger->size + sizeof(bigger);

			// free the bigger one
			find_delete_node(bigger);

			// push in heap again
			insert_heap(smaller);
		}
		smaller = bigger;
		bigger = pop_min_heap();
	}

	// must unmap



}

void rmconfig (rm_option opt) 
{
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
