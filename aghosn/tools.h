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


typedef struct location {
	uint32_t i; 
	uint32_t j;
	uint32_t k;
	uint32_t l;
} location_t;

typedef struct sc {
    unsigned long   sc_mem[8];
	unsigned long	sc_fd[8];
	unsigned char	sc_sys[16];
} sc_t;

struct sthread {
	int		st_id;
	int		st_state;
	void		*st_ret;
	void		*st_stack;
	ptent_t		*st_pgroot;
	struct dune_tf	st_tf;
	sc_t		st_sc;
	unsigned char	*st_writable;
	unsigned long	st_walk;
	struct sthread	*st_next;
};

#define GROW_SIZE 512

ptent_t* deep_copy_pgroot(ptent_t *pgroot, ptent_t *cppgroot);
bool has_a_mapping(ptent_t pgroot, void* va);
crawl_stats_t crawl(ptent_t* root);
int make_read_only(ptent_t* root, location_t l);
location_t get_location(ptent_t* root, void *va);

#endif