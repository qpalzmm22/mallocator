#include <stdio.h>
#include "rmalloc.h"

int 
main ()
{
	void *p1, *p2, *p3, *p4 ;
	
	rmprint() ;

	p1 = rmalloc(2000) ; 
	printf("rmalloc(2000):%p\n", p1) ; 

	p2 = rmalloc(2000) ; 
	printf("rmalloc(2500):%p\n", p2) ; 
	rmprint() ;

	rfree(p1) ;
	rfree(p2) ;
	printf("rfree(%p)\n", p1) ; 
	rmprint() ;
	rmshrink () ;
	// p4 = rrealloc(p2, 8000) ; 
	// printf("rrealloc(8000):%p\n", p4) ; 
	rmprint() ;
	
	test();

}
