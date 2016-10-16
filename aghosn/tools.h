#ifndef __TOOLS__H__
#define __TOOLS__H__

#include <dune.h>

typedef struct crawl_stats {
	uint32_t pml4_entries;
	uint32_t pdpte_entries;
	uint32_t pdpte_big;

	uint32_t pde_entries;
	uint32_t pde_big;

} crawl_stats_t;

#define GROW_SIZE 512

ptent_t* deep_copy_pgroot(ptent_t *pgroot);
bool has_a_mapping(ptent_t pgroot, void* va);
crawl_stats_t crawl(ptent_t* root);

#endif