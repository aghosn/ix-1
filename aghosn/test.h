#ifndef __AGHOSN_TEST_H__
#define __AGHOSN_TEST_H__
#include <dune.h>

#define println(x)		\
	do { 				\
		printf((x)); 		\
		printf("\n");	\
	} while(0)


#define GET_BIT(x,i) (((x) >> i) & 1)

#define CR0_PG 	0x80000000 /*Paging*/

#define IS_CR0_PG(x) (((uint32_t)x) & ((uint32_t)CR0_PG))
//#define CR0_WP(x)	(GET_BIT((x), __CR0__WP__))


uint32_t read_cr0();
uint64_t read_cr3();

#endif