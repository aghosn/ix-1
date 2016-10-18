#include <stdio.h>
#include <stdlib.h>

#include "tools.h"


#define PTE_DEF_FLAGS	(PTE_P | PTE_W | PTE_U)

static inline int pte_present(ptent_t pte)
{
	return (PTE_FLAGS(pte) & PTE_P);
}

static inline int pte_RW(ptent_t pte) {

	return (PTE_FLAGS(pte) & PTE_W);
}


static inline int pte_big(ptent_t pte)
{
	return (PTE_FLAGS(pte) & PTE_PS);
}

static inline void * alloc_page(void)
{
	struct page *pg = dune_page_alloc();
	if (!pg)
		return NULL;

	return (void *) dune_page2pa(pg);
}

/*TODO deep copy is useless since old virtual will not be valid.*/
ptent_t* deep_copy_pgroot(ptent_t *pgroot, ptent_t *cppgroot) {

	//Create the copy for the page root.
	cppgroot = memalign(PGSIZE, PGSIZE);
	cppgroot = memcpy(cppgroot, pgroot, PGSIZE);

	//Creating copies for pml4
	ptent_t *o_pml4 = pgroot, *o_pdpte, *o_pde, *o_pte;
	ptent_t *pml4 = cppgroot, *pdpte, *pde, *pte;


	for (int i = 0; i < GROW_SIZE; i++) {
		if (!pte_present(o_pml4[i])) {
			pml4[i] = o_pml4[i];
			continue;
		}

		//It is present so we have to create the copy.
		o_pdpte = (ptent_t*) PTE_ADDR(o_pml4[i]);
		pdpte = alloc_page();
		memset(pdpte, 0, PGSIZE);
		//TODO copy the flags.
		pml4[i] = PTE_ADDR(pdpte) | PTE_FLAGS(o_pml4[i]);

		for (int j = 0; j < GROW_SIZE; j++) {
			//TODO Handle big too!
			if (!pte_present(o_pdpte[j])) {
				pdpte[j] = o_pdpte[i];
				continue;
			}

			if (pte_big(o_pdpte[j])) {
				pdpte[j] = o_pdpte[j];
				continue;
			}

			o_pde = (ptent_t*) PTE_ADDR(o_pdpte[j]);
			pde = alloc_page();
			memset(pde, 0 , PGSIZE);
			//TODO flags?
			pdpte[j] = PTE_ADDR(pde) | PTE_FLAGS(o_pdpte[j]);

			for (int k = 0; k < GROW_SIZE; k++) {
				//TODO handle big too.
				if (!pte_present(o_pde[k])) {
					pde[k] = o_pde[k];
					continue;
				}

				if (pte_big(o_pde[k])) {
					pde[k] = o_pde[k];
					continue;
				}

				o_pte = (ptent_t*) PTE_ADDR(o_pde[k]);
				pte = alloc_page();
				memset(pte, 0, PGSIZE);

				pde[k] = PTE_ADDR(pte) | PTE_FLAGS(o_pde[k]);
				
				for (int l = 0; l < GROW_SIZE; l++) {

					if (pte_present(o_pte[l])) {
						pte[l] = o_pte[l];
					} 
				}	
			}
		}

	}

	return cppgroot;
}


//TOOD what I actually need is to keep the same mappings but change access policies
//They should not be relevant for the addressing.

bool has_a_mapping(ptent_t pgroot, void* va) {
	int i, j, k, l;
	ptent_t *pml4 = (ptent_t*) pgroot, *pdpte, *pde, *pte;

	i = PDX(3, va);
	j = PDX(2, va);
	k = PDX(1, va);
	l = PDX(0, va);

	if (!pte_present(pml4[i])) {
		// printf("pdpte is Not present.");
		return false;
	} 

	// printf("The value for i: %d\n", i);
	pdpte = (ptent_t*) PTE_ADDR(pml4[i]);
	// printf("Now pdpte is %p\n", pdpte);

	if (!pte_present(pdpte[j])){
		// printf("The pde is not present!\n");
		return false;
	}


	// printf("The pde is present.\n");
	if (pte_big(pdpte[j])) {
		// printf("Using the big scheme.\n");
		return false;
	}

	pde = (ptent_t*) PTE_ADDR(pdpte[j]);
	// printf("The pde %p \n", pde);

	if (!pte_present(pde[k])) {
		// printf("The pte is not present.\n");
		return false;
	}

	if (pte_big(pde[k])) {
		// printf("Using the big scheme.\n");
		return false;
	}

	pte = (ptent_t *) PTE_ADDR(pde[k]);
	// printf("pte is present and is %p.\n", pte);
	
	ptent_t* out = &pte[l];
	// printf("The l is %d", l);
	// printf("The result is %p\n", out);

	return true;
}

location_t get_location(ptent_t* root, void *va) {
	location_t res = {PDX(3, va), PDX(2, va), PDX(1, va), PDX(0, va)};
	return res;
}



crawl_stats_t crawl(ptent_t* root) {
	crawl_stats_t res = {0, 0, 0, 0, 0};
	
	if (!root)
		return res;

	printf("Let's go for a crawl.\n");

	ptent_t* pml4 = root, *pdpte, *pde, *pte;
	
	//Page memory level 4
	for (int i = 0; i < GROW_SIZE; i++) {
		if (!pte_present(pml4[i]))
			continue;
		
		pdpte = (ptent_t*) PTE_ADDR(pml4[i]);
		res.pml4_entries++;

		// if (pte_RW(pml4[i])) {
		// 	printf("Has rigths RW for %d\n", i);
		// }
		
		//Page directory pointers level 3
		for (int j = 0; j < GROW_SIZE; j++) {
			if (!pte_present(pdpte[j]))
				continue;

			if (pte_big(pdpte[j])) {
				res.pdpte_big++;
				continue;
			}


			pde = (ptent_t*) PTE_ADDR(pdpte[j]);
			res.pdpte_entries++;

			//Page directories level 2
			for (int k = 0; k < GROW_SIZE; k++) {
				if(!pte_present(pde[k]))
					continue;

				if (pte_big(pde[k])) {
					res.pde_big++;
					continue;
				}

				//Page table level 1
				pte = (ptent_t*) PTE_ADDR(pde[k]);
				res.pde_entries++;

				/*printf("Size of pte %u\n", sizeof(pte));
				printf("Size of what it points to %u\n", sizeof(*pte));*/
			}
		}
	}
	return res;
}



int make_read_only(ptent_t* root, location_t l) {
	if (!root)
		return -1;

	if(!pte_present(root[l.i]))
		return -2;

	ptent_t *pdpte = PTE_ADDR(root[l.i]);
	if(!pte_present(pdpte[l.j]))
		return -3;

	ptent_t *pde =  PTE_ADDR(pdpte[l.j]);
	if(!pte_present(pde[l.k]))
		return -4;

	ptent_t *pte = PTE_ADDR(pde[l.k]);
	if (!pte_present(pte[l.l]))
		return -5;

	pte[l.l] &= ~PTE_W;

	return 0;
}

















