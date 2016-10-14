#include <stdio.h>
#include "test.h"
#include <asm/vmx.h>
#include "tools.h"


uint32_t read_cr0() {
	uint32_t cr0 = 0xdeadbeef;
	__asm__ __volatile__ (
		"mov %%cr0, %%rax\n\t"
		"mov %%eax, %0\n\t"
		: "=m" (cr0)
		: /* no input */
		: "%rax" 
	);
	return cr0;
}

uint64_t read_cr3() {
	uint64_t cr3 = 0xdeadbeef;
	__asm__ __volatile__ (
		"mov %%cr3, %%rax\n\t"
		"mov %%rax, %0\n\t"
		: "=m" (cr3)
		: /* no input */
		: "%rax"
	);

	return cr3;
}

uint64_t read_cr4() {
	uint64_t cr4 = 0xdeadbeef;
	__asm__ __volatile__ (
		"mov %%cr4, %%rax\n\t"
		"mov %%rax, %0\n\t"
		: "=m" (cr4)
		: /* no input */
		: "%rax"
	);

	return cr4;
}

int main() {
	printf("This is a simple test!\n");

	int ret = dune_init(false);
	if (ret)
		return ret;

	ret = dune_enter();
	if (ret)
		return ret;

	println("I'm in dune mode !");
	// if (!pgroot)
	// 	println("Page root is null.");
	// else
	// 	println("Page root is actually live!");

	
	// struct dune_tf *tf = NULL;
	// struct thread_arg *a = NULL;

	// printf("The address of pgroot %p\n", pgroot);
	// printf("The size of pgroot %lu\n", sizeof(pgroot));
	// printf("The size of what it points to %d\n", sizeof(*pgroot));
	
	// ptent_t* copy = memalign(PGSIZE, PGSIZE);

	// printf("The address of copy %p\n", copy);
	// printf("The size of copy %lu\n", sizeof(copy));
	// printf("The size of what it points to %d\n", sizeof(*copy));

	// copy = memcpy(copy, pgroot, PGSIZE);

	// load_cr3((unsigned long) copy);
	
	// printf("After the load.\n");
	// // uint32_t cr0 = 0xdeadbeef;
	// __asm__ __volatile__ (
	// 	"mov %%cr0, %%rax\n\t"
	// 	"mov %%eax, %0\n\t"
	// 	: "=m" (cr0)
	// 	: /* no input */
	// 	: "%rax" 
	// 	);

	// ptent_t* trial = deep_copy_pgroot(pgroot);
	// printf("Managed to do it.\n");

	// uint32_t cr0 = read_cr0();
	// printf("cr0 = 0x%8.8X\n", cr0 >> 31);
	// printf("cr0.pG %d\n", IS_CR0_PG(cr0));
	// if (IS_CR0_PG(cr0) != 0)
	// 	printf("Has PG enabled.\n");
	// else 
	// 	println("Paging not enabled.\n");

	// println("Checking that cr3 is pgroot.");
	// if (read_cr3() == (uint64_t)pgroot)
	// 	println("Yeah that's the case");
	// else
	// 	println("Nope");

	// if (cr0 & 0x1)
	// 	printf("Protection is enabled.\n");
	// else
	// 	printf("Protection is disabled\n.");

	/*printf("Before trying something bold\n.");
	asm("xorl 0x1 %cr0" );*/

	// uint32_t cr4 = read_cr4();
	// if(cr4 & (1 << 5)) /*PAE*/
	// 	printf("PAE is enabled\n");
	// else 
	// 	printf("PAE disabled.\n");

	//load_cr3(copy);
	//printf("After the load.\n");

	
	// unsigned low, high;
	// asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(guest_ia32_efer));
	// unsigned result = low | ((unsigned long)high << 32);
	// printf("%lu\n", result);
	// void* test = dune_page_alloc();
	printf("The cr3 value is %p\n", read_cr3());
	uint64_t test = 0xdeadbeef;
	printf("The address on the stack %p for deadbeef.\n", &test);
	has_a_mapping(pgroot, &test);

	crawl_stats_t result = crawl(pgroot);
	printf("The result is %u pml4 entries, %u pdpte entries, and %u pde entries.\n", result.pml4_entries, result.pdpte_entries, result.pde_entries);
	printf("And the bigs %u and %u\n", result.pdpte_big, result.pde_big);
		
	return 0;
}
