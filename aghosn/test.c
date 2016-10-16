#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include <asm/vmx.h>
#include "tools.h"

static void pgflt_handler(uintptr_t addr, uint64_t fec, struct dune_tf *tf)
{
        bool was_user = (tf->cs & 0x3);

        printf((was_user)? "Was in user mode\n" : "Was not in user mode\n");

        //dune_dump_trap_frame(tf);
        abort();

       /* if (was_user) {
                printf("sandbox: got unexpected G3 page fault at addr %lx, fec %lx\n", addr, fec);
                dune_dump_trap_frame(tf);
                dune_ret_from_user(-EFAULT);
        } else {
                ret = dune_vm_lookup(pgroot, (void *) addr, CREATE_NORMAL, &pte);
                assert(!ret);
                *pte = PTE_P | PTE_W | PTE_ADDR(dune_va_to_pa((void *) addr));
        }*/
}

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

	/*printf("The cr3 value is %p\n", read_cr3());
	uint64_t test = 0xdeadbeef;
	printf("The address on the stack %p for deadbeef.\n", &test);
	has_a_mapping(pgroot, &test);

	crawl_stats_t result = crawl(pgroot);
	printf("The result is %u pml4 entries, %u pdpte entries, and %u pde entries.\n", result.pml4_entries, result.pdpte_entries, result.pde_entries);
	printf("And the bigs %u and %u\n", result.pdpte_big, result.pde_big);

	printf("Let's remove the write access\n");
	ptent_t* newPg = remove_access_RW(pgroot);*/
	dune_register_pgflt_handler(pgflt_handler);
	// int* t = NULL;
	// int res = *t;
	// printf("Result %d\n", res);
	ptent_t* newPg = remove_access_RW(pgroot);

	//TODO like in sthread with st_tf, and jump to user.
	struct dune_tf tf;

	load_cr3((unsigned long) newPg);

	dune_jump_to_user(&tf);

	load_cr3(pgroot);

	printf("Reached the end!\n");
	// int a = 3;
	// printf("Can still print %d\n", a);	
	return 0;
}
