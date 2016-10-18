#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include <asm/vmx.h>
#include "tools.h"
#include <sys/mman.h>

int GLOBAL_TESTING = 3;

static void pgflt_handler(uintptr_t addr, uint64_t fec, struct dune_tf *tf)
{
        bool was_user = (tf->cs & 0x3);

        printf((was_user)? "Was in user mode\n" : "Was not in user mode\n");

        if (was_user) {
        	printf("CR2 %p and fec %lu \n", addr, fec);
        	fflush(stdout);
        	dune_dump_trap_frame(tf);
        	abort();
        }
        
        load_cr3(pgroot);
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


void my_function() {

	printf("Hello world!\n");
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

	dune_register_pgflt_handler(pgflt_handler);

	ptent_t *cppgroot;

	/* 	1. We copy the pageroot
	 	2. We create a local variable
	 	3. Verify it has a mapping in both pgroot.
	 	4. Identify its page table
	 	5. Revoke it inside the copy of the page table.
	 	6. load copy of pgroot.*/
	
	cppgroot = deep_copy_pgroot(pgroot, cppgroot);

	if (!has_a_mapping(pgroot, &GLOBAL_TESTING) 
		|| !has_a_mapping(cppgroot, &GLOBAL_TESTING)) {
		printf("The given address(a) %p lacks one or both mappings.\n", 
			&GLOBAL_TESTING);
		exit(1);
	}

	printf("Revoking access for its page.\n");

	location_t locA = get_location(cppgroot, &GLOBAL_TESTING);
	
	printf("Address is %p.\n", &GLOBAL_TESTING);
	printf("i: %d, j:%d, k: %d, l: %d.\n", locA.i, locA.j, locA.k, locA.l);

	
	make_read_only(cppgroot, locA);
	//dune_procmap_dump();

	printf("Loading the copied cr3.\n");
	load_cr3((unsigned long) cppgroot);
	printf("After the load.\n");

	printf("Printing %d\n", GLOBAL_TESTING);
	GLOBAL_TESTING = 4;
	//dune_procmap_dump();
	printf("Survived!!! %d\n", GLOBAL_TESTING);

	//dune_page_stats();

	struct sthread th;

	th.st_stack = mmap(NULL, 10 * PGSIZE, PROT_READ | PROT_WRITE, 
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); 

	printf((has_a_mapping(pgroot, th.st_stack))? "Stack is mapped\n": "Stack not mapped\n");

	printf((has_a_mapping(cppgroot, th.st_stack))? "Weird has a mapping there too.\n" : "No mapping.\n");

	th.st_pgroot = pgroot;

	struct dune_tf tf;

	memset(&tf, 0, sizeof(tf));

	tf.rip = (unsigned long) my_function;
	tf.rsp = (unsigned long) th.st_stack + (10 * PGSIZE);
	printf("Stack base is %p\n", tf.rsp);

	tf.rflags = 0x02;

	printf("Before the jump.\n");
	dune_jump_to_user(&tf);
	printf("Back from the jump.\n");
	
	return 0;
}
