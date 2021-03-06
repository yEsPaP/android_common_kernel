#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <sys/time.h>
#include <set>
#include <map>
#include <string>
#include <limits.h>

#if 0
#include "aot.h"
#else

#ifdef __linux__
  #define _ALIGNAS(n)	__attribute__((aligned(n)))
  #define RB_NODE_ALIGN	(sizeof(long))
#else
#ifdef _WIN32
  #define _ALIGNAS(n)	__declspec(align(n))
  #ifdef _M_IX86
    #define RB_NODE_ALIGN	4
  #elif defined _M_X64
    #define RB_NODE_ALIGN	8
  #endif
#endif	/* _WIN32 */
#endif /* __linux__ */

#ifdef __linux__
  #include <alloca.h>
  #define ALLOCA(x)	alloca(x)
#else
  #ifdef _WIN32
    #include <malloc.h>
    #define ALLOCA(x)	_malloca(x)
  #endif
#endif

#ifdef __linux__
  #define container_of(ptr, type, member) ({			\
  	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
  	(type *)( (char *)__mptr - offsetof(type,member) );})
#else
  #ifdef _WIN32
    #define container_of(ptr, type, member) (type *)( (char *)(ptr) - offsetof(type,member) )
  #endif
#endif

typedef uintptr_t (*get_function_address_t)(const char* fsym);

struct _ALIGNAS(RB_NODE_ALIGN) rb_node {
	uintptr_t  __rb_parent_color;
	struct rb_node *rb_right;
	struct rb_node *rb_left;
};
/* The alignment might seem pointless, but allegedly CRIS needs it */

struct rb_root {
	struct rb_node *rb_node;
};

#include "interval_tree.h"
#include "interval_tree_generic.h"

struct interval_tree_node {
	struct rb_node rb;
	uintptr_t start;	/* Start of interval */
	uintptr_t last;	/* Last location _in_ interval */
	uintptr_t __subtree_last;
	void* mptr;
};

#define START(node) ((node)->start)
#define LAST(node)  ((node)->last)

extern "C" {
	void __rb_erase_color(struct rb_node *parent, struct rb_root *root,
		void (*augment_rotate)(struct rb_node *old, struct rb_node *__new));
	void __rb_insert_augmented(struct rb_node *node, struct rb_root *root,
		void (*augment_rotate)(struct rb_node *old, struct rb_node *__new));
}

INTERVAL_TREE_DEFINE(struct interval_tree_node, rb,
		     uintptr_t, __subtree_last,
		     START, LAST,, interval_tree)

void* malloc (size_t size);
void* calloc (size_t num, size_t size);
void free(void *ptr);

struct interval_tree_node * interval_tree_iter_first(struct rb_root *root, uintptr_t start, uintptr_t last);
struct interval_tree_node *	interval_tree_iter_next(struct interval_tree_node *node, uintptr_t start, uintptr_t last);
struct rb_node* interval_tree_insert(struct interval_tree_node *node, struct rb_root *root);

struct task_struct {
	unsigned char padding0[60];
	unsigned int cpu;
	unsigned char padding1[16];
	struct task_struct* last_wakee;
	unsigned char padding2[12];
	int prio;
	unsigned char padding3[1248];
	int pid;
	int tgid;
	unsigned char padding4[8];
	struct task_struct* real_parent;
	struct task_struct* parent;
	unsigned char padding5[32];
	struct task_struct* group_leader;
	unsigned char padding6[608];
	struct task_struct* pi_top_task;
	unsigned char padding7[592];
	struct task_struct* oom_reaper_list;
	unsigned char padding8[4464];
} __attribute__((packed));
#endif

#include "stringset.h"

#define TIME_MARK_START(start_marker)		\
		struct timeval  tv_mark_##start_marker;	\
		gettimeofday(&tv_mark_##start_marker, 0)

#define TIME_CHECK_FMT(start_marker,end_marker,fmt)	do {	\
		struct timeval  tv_mark_##start_marker##_##end_marker;	\
		gettimeofday(&tv_mark_##start_marker##_##end_marker, 0);	\
		printf(fmt,	\
		(double) (tv_mark_##start_marker##_##end_marker.tv_usec - tv_mark_##start_marker.tv_usec) / 1000000 +	\
		         (double) (tv_mark_##start_marker##_##end_marker.tv_sec - tv_mark_##start_marker.tv_sec) );	\
	} while(0)

struct flatten_header {
	size_t memory_size;
	size_t ptr_count;
	size_t fptr_count;
	size_t root_addr_count;
	uintptr_t this_addr;
	size_t fptrmapsz;
	size_t mcount;
	uint64_t magic;
};

enum flatten_option {
	option_silent = 0x01
};

struct blstream {
	struct blstream* next;
	struct blstream* prev;
	void* data;
	size_t size;
	size_t index;
	size_t alignment;
};

/* We got def from aot.h. */
/*
#define _ALIGNAS(n)	__attribute__((aligned(n)))
#define RB_NODE_ALIGN	(sizeof(long))

struct _ALIGNAS(RB_NODE_ALIGN) rb_node {
	uintptr_t  __rb_parent_color;
	struct rb_node *rb_right;
	struct rb_node *rb_left;
};

struct rb_root {
	struct rb_node *rb_node;
};
*/

/* Root address list */
struct root_addrnode {
	struct root_addrnode* next;
	uintptr_t root_addr;
};

struct FLCONTROL {
	struct blstream* bhead;
	struct blstream* btail;
	struct rb_root fixup_set_root;
	struct rb_root imap_root;
	struct flatten_header	HDR;
	struct root_addrnode* rhead;
	struct root_addrnode* rtail;
	struct root_addrnode* last_accessed_root;
	int debug_flag;
	unsigned long option;
	void* mem;
};

#define RB_ROOT	{ 0, }

struct FLCONTROL FLCTRL = {
		.bhead = 0,
		.btail = 0,
		.fixup_set_root = RB_ROOT,
		.imap_root = RB_ROOT,
		.rhead = 0,
		.rtail = 0,
		.last_accessed_root=0,
		.debug_flag=0,
		.option=0,
		.mem = 0,
};

#define FLATTEN_MAGIC 0x464c415454454e00ULL

#define ROOT_POINTER_NEXT(PTRTYPE)	((PTRTYPE)(root_pointer_next()))
#define ROOT_POINTER_SEQ(PTRTYPE,n)	((PTRTYPE)(root_pointer_seq(n)))
#define FLATTEN_MEMORY_START	((unsigned char*)FLCTRL.mem+FLCTRL.HDR.ptr_count*sizeof(size_t)+FLCTRL.HDR.fptr_count*sizeof(size_t)+FLCTRL.HDR.mcount*2*sizeof(size_t))

void* root_pointer_next() {
	
	assert(FLCTRL.rhead!=0);

	if (FLCTRL.last_accessed_root==0) {
		FLCTRL.last_accessed_root = FLCTRL.rhead;
	}
	else {
		if (FLCTRL.last_accessed_root->next) {
			FLCTRL.last_accessed_root = FLCTRL.last_accessed_root->next;
		}
		else {
			assert(0);
		}
	}

	if (FLCTRL.last_accessed_root->root_addr==(size_t)-1) {
		return 0;
	}
	else {
		if (interval_tree_iter_first(&FLCTRL.imap_root,0,ULONG_MAX)) {	/* We have allocated each memory fragment individually */
			struct interval_tree_node *node = interval_tree_iter_first(&FLCTRL.imap_root,FLCTRL.last_accessed_root->root_addr,FLCTRL.last_accessed_root->root_addr+8);
			assert(node);
			size_t node_offset = FLCTRL.last_accessed_root->root_addr-node->start;
			return (unsigned char*)node->mptr+node_offset;
		}
		else {
			return (FLATTEN_MEMORY_START+FLCTRL.last_accessed_root->root_addr);
		}
	}
}

void* root_pointer_seq(size_t index) {

	assert(FLCTRL.rhead!=0);

	FLCTRL.last_accessed_root = FLCTRL.rhead;

	size_t i=0;
	for (i=0; i<index; ++i) {
		if (FLCTRL.last_accessed_root->next) {
			FLCTRL.last_accessed_root = FLCTRL.last_accessed_root->next;
		}
		else {
			assert(0);
		}
	}

	if (FLCTRL.last_accessed_root->root_addr==(size_t)-1) {
		return 0;
	}
	else {
		if (interval_tree_iter_first(&FLCTRL.imap_root,0,ULONG_MAX)) {	/* We have allocated each memory fragment individually */
			struct interval_tree_node *node = interval_tree_iter_first(&FLCTRL.imap_root,FLCTRL.last_accessed_root->root_addr,FLCTRL.last_accessed_root->root_addr+8);
			assert(node);
			size_t node_offset = FLCTRL.last_accessed_root->root_addr-node->start;
			return (unsigned char*)node->mptr+node_offset;
		}
		else {
			return (FLATTEN_MEMORY_START+FLCTRL.last_accessed_root->root_addr);
		}
	}
}

void root_addr_append(uintptr_t root_addr) {
    struct root_addrnode* v = (struct root_addrnode*)calloc(1,sizeof(struct root_addrnode));
    assert(v!=0);
    v->root_addr = root_addr;
    if (!FLCTRL.rhead) {
        FLCTRL.rhead = v;
        FLCTRL.rtail = v;
    }
    else {
        FLCTRL.rtail->next = v;
        FLCTRL.rtail = FLCTRL.rtail->next;
    }
}

void fix_unflatten_memory(struct flatten_header* hdr, void* memory) {
	size_t i;
	void* mem = (unsigned char*)memory+hdr->ptr_count*sizeof(size_t)+hdr->fptr_count*sizeof(size_t)+hdr->mcount*2*sizeof(size_t);
	for (i=0; i<hdr->ptr_count; ++i) {
		size_t fix_loc = *((size_t*)memory+i);
		uintptr_t ptr = (uintptr_t)( *((void**)((unsigned char*)mem+fix_loc)) );
		/* Make the fix */
		*((void**)((unsigned char*)mem+fix_loc)) = (unsigned char*)mem + ptr;
	}
}

std::map<uintptr_t,std::string> fptrmap;

void unflatten_init() {
}

int unflatten_create(FILE* f, get_function_address_t gfa) {

	TIME_MARK_START(unfl_b);
	size_t readin = 0;
	size_t rd = fread(&FLCTRL.HDR,sizeof(struct flatten_header),1,f);
	if (rd!=1) return -1; else readin+=sizeof(struct flatten_header);
	if (FLCTRL.HDR.magic!=FLATTEN_MAGIC) {
		fprintf(stderr,"Invalid magic while reading flattened image\n");
		return -1;
	}
	size_t* root_addr_array = (size_t*)malloc(FLCTRL.HDR.root_addr_count*sizeof(size_t));
	assert(root_addr_array);
	rd = fread(root_addr_array,sizeof(size_t),FLCTRL.HDR.root_addr_count,f);
	if (rd!=FLCTRL.HDR.root_addr_count) return -1; else readin+=sizeof(size_t)*FLCTRL.HDR.root_addr_count;
	size_t memsz = FLCTRL.HDR.memory_size+FLCTRL.HDR.ptr_count*sizeof(size_t)+FLCTRL.HDR.fptr_count*sizeof(size_t)+FLCTRL.HDR.mcount*2*sizeof(size_t);
	FLCTRL.mem = malloc(memsz);
	assert(FLCTRL.mem);
	rd = fread(FLCTRL.mem,1,memsz,f);
	if (rd!=memsz) return -1; else readin+=rd;
	if ((FLCTRL.HDR.fptr_count>0)&&(FLCTRL.HDR.fptrmapsz>0)&&(gfa)) {
		unsigned char* fptrmapmem = (unsigned char*)malloc(FLCTRL.HDR.fptrmapsz);
		assert(fptrmapmem);
		rd = fread(fptrmapmem,1,FLCTRL.HDR.fptrmapsz,f);
		if (rd!=FLCTRL.HDR.fptrmapsz) return -1; else readin+=rd;
		size_t fptrnum = *((size_t*)fptrmapmem);
		fptrmapmem+=sizeof(size_t);
		for (size_t kvi=0; kvi<fptrnum; ++kvi) {
			uintptr_t addr = *((uintptr_t*)fptrmapmem);
			fptrmapmem+=sizeof(uintptr_t);
			size_t sz = *((size_t*)fptrmapmem);
			fptrmapmem+=sizeof(size_t);
			std::string sym((const char*)fptrmapmem,sz);
			fptrmapmem+=sz;
			fptrmap.insert(std::pair<uintptr_t,std::string>(addr,sym));
		}
		free(fptrmapmem-FLCTRL.HDR.fptrmapsz);
	}
	if ((FLCTRL.option&option_silent)==0) {
		printf("# Unflattening done. Summary:\n");
		TIME_CHECK_FMT(unfl_b,read_e,"  Image read time: %fs\n");
	}
	TIME_MARK_START(create_b);
	size_t* minfoptr = (size_t*)(((unsigned char*)FLCTRL.mem)+FLCTRL.HDR.ptr_count*sizeof(size_t)+FLCTRL.HDR.fptr_count*sizeof(size_t));
	unsigned char* memptr = ((unsigned char*)FLCTRL.mem)+FLCTRL.HDR.ptr_count*sizeof(size_t)+FLCTRL.HDR.fptr_count*sizeof(size_t)+FLCTRL.HDR.mcount*2*sizeof(size_t);
	for (size_t i=0; i<FLCTRL.HDR.mcount; ++i) {
		size_t index = *minfoptr++;
		size_t size = *minfoptr++;
		struct interval_tree_node *node = (struct interval_tree_node*)calloc(1,sizeof(struct interval_tree_node));
		node->start = index;
		node->last = index+size-1;
		void* fragment = malloc(size);
		assert(fragment!=0);
		memcpy(fragment,memptr+index,size);
		node->mptr = fragment;
		struct rb_node* rb = interval_tree_insert(node, &FLCTRL.imap_root);
	}
	if ((FLCTRL.option&option_silent)==0) {
		TIME_CHECK_FMT(create_b,create_e,"  Creating memory time: %fs\n");
	}
	TIME_MARK_START(fix_b);
	for (size_t i=0; i<FLCTRL.HDR.root_addr_count; ++i) {
		size_t root_addr_offset = root_addr_array[i];
		root_addr_append(root_addr_offset);
	}
	free(root_addr_array);
	for (size_t i=0; i<FLCTRL.HDR.ptr_count; ++i) {
		void* mem = (unsigned char*)FLCTRL.mem+FLCTRL.HDR.ptr_count*sizeof(size_t)+FLCTRL.HDR.fptr_count*sizeof(size_t)+FLCTRL.HDR.mcount*2*sizeof(size_t);
		size_t fix_loc = *((size_t*)FLCTRL.mem+i);
		struct interval_tree_node *node = interval_tree_iter_first(&FLCTRL.imap_root,fix_loc,fix_loc+8);
		assert(node);
		size_t node_offset = fix_loc-node->start;
		uintptr_t ptr = (uintptr_t)( *((void**)((unsigned char*)mem+fix_loc)) );
		struct interval_tree_node *ptr_node = interval_tree_iter_first(&FLCTRL.imap_root,ptr,ptr+8);
		assert(ptr_node);
		size_t ptr_node_offset = ptr-ptr_node->start;
		/* Make the fix */
		*((void**)((unsigned char*)node->mptr+node_offset)) = (unsigned char*)ptr_node->mptr + ptr_node_offset;
	}
	if ((FLCTRL.HDR.fptr_count>0)&&(gfa)) {
		unsigned char* mem = (unsigned char*)FLCTRL.mem+FLCTRL.HDR.ptr_count*sizeof(size_t)+FLCTRL.HDR.fptr_count*sizeof(size_t)+FLCTRL.HDR.mcount*2*sizeof(size_t);
		for (size_t fi=0; fi<FLCTRL.HDR.fptr_count; ++fi) {
			size_t fptri = ((uintptr_t*)((unsigned char*)FLCTRL.mem+FLCTRL.HDR.ptr_count*sizeof(size_t)))[fi];
			struct interval_tree_node *node = interval_tree_iter_first(&FLCTRL.imap_root,fptri,fptri+8);
			assert(node);
			size_t node_offset = fptri-node->start;
			uintptr_t fptrv = *((uintptr_t*)((unsigned char*)node->mptr+node_offset));
			if (fptrmap.find(fptrv)!=fptrmap.end()) {
				uintptr_t nfptr = (*gfa)(fptrmap[fptrv].c_str());
				// Fix function pointer
				*((void**)((unsigned char*)node->mptr+node_offset)) = (void*)nfptr;
			}
			else {
			}
		}
	}
	if ((FLCTRL.option&option_silent)==0) {
		TIME_CHECK_FMT(fix_b,fix_e,"  Fixing memory time: %fs\n");
		TIME_CHECK_FMT(unfl_b,fix_e,"  Total time: %fs\n");
		printf("  Total bytes read: %zu\n",readin);
		printf("  Number of allocated fragments: %zu\n",FLCTRL.HDR.mcount);
	}

	return 0;
}

int unflatten_read(FILE* f, get_function_address_t gfa) {

	TIME_MARK_START(unfl_b);
	size_t readin = 0;
	size_t rd = fread(&FLCTRL.HDR,sizeof(struct flatten_header),1,f);
	if (rd!=1) return -1; else readin+=sizeof(struct flatten_header);
	if (FLCTRL.HDR.magic!=FLATTEN_MAGIC) {
		fprintf(stderr,"Invalid magic while reading flattened image\n");
		return -1;
	}
	size_t i;
	for (i=0; i<FLCTRL.HDR.root_addr_count; ++i) {
		size_t root_addr_offset;
		size_t rd = fread(&root_addr_offset,sizeof(size_t),1,f);
		if (rd!=1) return -1; else readin+=sizeof(size_t);
		root_addr_append(root_addr_offset);
	}
	size_t memsz = FLCTRL.HDR.memory_size+FLCTRL.HDR.ptr_count*sizeof(size_t)+FLCTRL.HDR.fptr_count*sizeof(size_t)+FLCTRL.HDR.mcount*2*sizeof(size_t);
	FLCTRL.mem = malloc(memsz);
	assert(FLCTRL.mem);
	rd = fread(FLCTRL.mem,1,memsz,f);
	if (rd!=memsz) return -1; else readin+=rd;
	if ((FLCTRL.HDR.fptr_count>0)&&(FLCTRL.HDR.fptrmapsz>0)&&(gfa)) {
		unsigned char* fptrmapmem = (unsigned char*)malloc(FLCTRL.HDR.fptrmapsz);
		assert(fptrmapmem);
		rd = fread(fptrmapmem,1,FLCTRL.HDR.fptrmapsz,f);
		if (rd!=FLCTRL.HDR.fptrmapsz) return -1; else readin+=rd;
		size_t fptrnum = *((size_t*)fptrmapmem);
		fptrmapmem+=sizeof(size_t);
		for (size_t kvi=0; kvi<fptrnum; ++kvi) {
			uintptr_t addr = *((uintptr_t*)fptrmapmem);
			fptrmapmem+=sizeof(uintptr_t);
			size_t sz = *((size_t*)fptrmapmem);
			fptrmapmem+=sizeof(size_t);
			std::string sym((const char*)fptrmapmem,sz);
			fptrmapmem+=sz;
			fptrmap.insert(std::pair<uintptr_t,std::string>(addr,sym));
		}
		free(fptrmapmem-FLCTRL.HDR.fptrmapsz);
	}
	if ((FLCTRL.option&option_silent)==0) {
		printf("# Unflattening done. Summary:\n");
		TIME_CHECK_FMT(unfl_b,read_e,"  Image read time: %fs\n");
	}
	TIME_MARK_START(fix_b);
	fix_unflatten_memory(&FLCTRL.HDR,FLCTRL.mem);
	if ((FLCTRL.HDR.fptr_count>0)&&(gfa)) {
		unsigned char* mem = (unsigned char*)FLCTRL.mem+FLCTRL.HDR.ptr_count*sizeof(size_t)+FLCTRL.HDR.fptr_count*sizeof(size_t)+FLCTRL.HDR.mcount*2*sizeof(size_t);
		for (size_t fi=0; fi<FLCTRL.HDR.fptr_count; ++fi) {
			size_t fptri = ((uintptr_t*)((unsigned char*)FLCTRL.mem+FLCTRL.HDR.ptr_count*sizeof(size_t)))[fi];
			uintptr_t fptrv = *((uintptr_t*)(mem+fptri));
			if (fptrmap.find(fptrv)!=fptrmap.end()) {
				uintptr_t nfptr = (*gfa)(fptrmap[fptrv].c_str());
				// Fix function pointer
				*((void**)(mem+fptri)) = (void*)nfptr;
			}
			else {
			}
		}
	}
	if ((FLCTRL.option&option_silent)==0) {
		TIME_CHECK_FMT(fix_b,fix_e,"  Fixing memory time: %fs\n");
		TIME_CHECK_FMT(unfl_b,fix_e,"  Total time: %fs\n");
		printf("  Total bytes read: %zu\n",readin);
	}
	return 0;
}

void unflatten_fini() {
	FLCTRL.rtail = FLCTRL.rhead;
    while(FLCTRL.rtail) {
    	struct root_addrnode* p = FLCTRL.rtail;
    	FLCTRL.rtail = FLCTRL.rtail->next;
    	free(p);
    }
    free(FLCTRL.mem);
    fptrmap.clear();
    // TODO: clear interval tree nodes and memory fragments
}

struct point {
	double x;
	double y;
	unsigned n;
	struct point** other;
};

struct figure {
	const char* name;
	unsigned n;
	struct point* points;
};

struct B {
	unsigned char T[4];
};

struct A {
	unsigned long X;
	struct B* pB;
};

struct my_list_head {
	struct my_list_head* prev;
	struct my_list_head* next;
};

struct intermediate {
	struct my_list_head* plh;
};

struct my_task_struct {
	int pid;
	struct intermediate* im;
	struct my_list_head u;
	float w;
};

typedef struct struct_B {
	int i;
} my_B;

typedef struct struct_A {
	unsigned long ul;
	my_B* pB0;
	my_B* pB1;
	my_B* pB2;
	my_B* pB3;
	char* p;
} /*__attribute__((aligned(64)))*/ my_A;

void print_struct_task_offsets(struct task_struct* t) {
	printf("task_struct.last_wakee: %zu\n",offsetof(struct task_struct,last_wakee));
	printf("task_struct.real_parent: %zu\n",offsetof(struct task_struct,real_parent));
	printf("task_struct.parent: %zu\n",offsetof(struct task_struct,parent));
	printf("task_struct.group_leader: %zu\n",offsetof(struct task_struct,group_leader));
	printf("task_struct.pi_top_task: %zu\n",offsetof(struct task_struct,pi_top_task));
	printf("task_struct.oom_reaper_list: %zu\n",offsetof(struct task_struct,oom_reaper_list));
	printf("task_struct.pid: %zu\n",offsetof(struct task_struct,pid));
	printf("task_struct.tgid: %zu\n",offsetof(struct task_struct,tgid));
	printf("task_struct.prio: %zu\n",offsetof(struct task_struct,prio));
	printf("task_struct.cpu: %zu\n",offsetof(struct task_struct,cpu));
	printf("task_struct size: %zu\n",sizeof(struct task_struct));
}

void walk_print_task_struct(struct task_struct* T,std::set<struct task_struct*>& visited) {
	visited.insert(T);
	printf("T[%d:%d], cpu %u, prio %d\n",T->pid,T->tgid,T->cpu,T->prio);
	if ((T->last_wakee!=0)&&(visited.find(T->last_wakee)==visited.end())) {
		walk_print_task_struct(T->last_wakee,visited);
	}
	if ((T->real_parent!=0)&&(visited.find(T->real_parent)==visited.end())) {
		walk_print_task_struct(T->real_parent,visited);
	}
	if ((T->parent!=0)&&(visited.find(T->parent)==visited.end())) {
		walk_print_task_struct(T->parent,visited);
	}
	if ((T->group_leader!=0)&&(visited.find(T->group_leader)==visited.end())) {
		walk_print_task_struct(T->group_leader,visited);
	}
	if ((T->pi_top_task!=0)&&(visited.find(T->pi_top_task)==visited.end())) {
		walk_print_task_struct(T->pi_top_task,visited);
	}
	if ((T->oom_reaper_list!=0)&&(visited.find(T->oom_reaper_list)==visited.end())) {
		walk_print_task_struct(T->oom_reaper_list,visited);
	}
}

struct fptr_test_struct {
	int i;
	long l;
	char* s;
	int (*sf)(struct kflat *kflat, size_t num_strings, int debug_flag);
	struct blstream* (*ef)(struct kflat* kflat, const void* data, size_t size);
	int (*gf)(struct kflat* kflat);
};

int kflat_stringset_module_test(struct kflat *kflat, size_t num_strings, int debug_flag) {
	printf("HOST::kflat_stringset_module_test()\n");
	return 0;
}

int binary_stream_append(struct kflat* kflat, const void* data, size_t size) {
	printf("HOST::binary_stream_append()\n");
	return 0;
}

int binary_stream_calculate_index(struct kflat* kflat) {
	printf("HOST::binary_stream_calculate_index()\n");
	return 0;
}

bool endswith (std::string const &s, std::string const &what) {
    if (s.length() >= what.length()) {
        return (0 == s.compare (s.length() - what.length(), what.length(), what));
    } else {
        return false;
    }
}

uintptr_t get_fpointer_test_function_address(const char* fsym) {
	std::string sf(fsym);
	if (endswith(sf,"::kflat_stringset_module_test")) {
		return (uintptr_t)&kflat_stringset_module_test;
	}
	else if (endswith(sf,"::binary_stream_append")) {
		return (uintptr_t)&binary_stream_append;
	}
	else if (endswith(sf,"::binary_stream_calculate_index")) {
		return (uintptr_t)&binary_stream_calculate_index;
	}

	return 0;
}

uintptr_t print_function_address(const char* fsym) {
	printf("HOST: %s\n",fsym);
	return 0;
}

struct CC {
	int i;
};

struct BB {
	long s;
	long n;
	int* pi;
	struct CC* pC;
};

struct MM {
	const char* s;
	struct BB arrB[4];
	long* Lx;
};

void print_struct_BB(const struct BB* pB) {
	printf("%ld:%ld",pB->s,pB->n);
	if (pB->pi) {
		printf(" [ ");
		for (long i=0; i<pB->n; ++i) {
			printf("%d ",pB->pi[i]);
		}
		printf("]");
	}
	printf("\n");
	if (pB->pC) {
		printf("C: %d\n",pB->pC->i);
	}
}

int main(int argc, char* argv[]) {

	FILE* in = fopen(argv[1], "r");
	assert(in);

	size_t size;
	fread(&size,sizeof(size_t),1,in);
	printf("Size of flatten image: %zu\n",size);

	unflatten_init();
	if (!strncmp(argv[2],"CREAT",5)) {
		argv[2] = argv[2]+6;
		if (!strcmp(argv[2],"CURRENTTASKM")) {
			assert(unflatten_create(in,print_function_address) == 0);
		}
		else {
			assert(unflatten_create(in,get_fpointer_test_function_address) == 0);
		}
	}
	else {
		if (!strcmp(argv[2],"CURRENTTASKM")) {
			assert(unflatten_read(in,print_function_address) == 0);
		}
		else {
			assert(unflatten_read(in,get_fpointer_test_function_address) == 0);
		}
	}

	if (argc>=3) {
		if (!strcmp(argv[2],"SIMPLE")) {
			printf("sizeof(struct A): %zu\n",sizeof(struct A));
			printf("sizeof(struct B): %zu\n",sizeof(struct B));
			const struct A* pA = ROOT_POINTER_NEXT(const struct A*);
			printf("pA->X: %016lx\n",pA->X);
			printf("pA->pB->T: [%02x%02x%02x%02x]\n",pA->pB->T[0],pA->pB->T[1],pA->pB->T[2],pA->pB->T[3]);
		}
		else if (!strcmp(argv[2],"CIRCLE")) {
			
			const struct figure* circle = ROOT_POINTER_NEXT(const struct figure*);
			unsigned i, j;
			double length = 0, circumference = 0;
			unsigned edge_number = 0;
			for (i = 0; i < circle->n - 1; ++i) {
				for (j = i; j < circle->n - 1; ++j) {
					if (circle->points[i].other[j]) {

						double path_len = sqrt(  pow(circle->points[i].x-circle->points[i].other[j]->x,2) +
								pow(circle->points[i].y-circle->points[i].other[j]->y,2) );
						length += path_len;

						if (j == i)
							circumference += path_len;
						if ((i == 0) && (j == circle->n - 2))
							circumference += path_len;

						unsigned u;
						for (u = 0; u < circle->n - 1; ++u) {
							if (circle->points[i].other[j]->other[u] == &circle->points[i]) {
								circle->points[i].other[j]->other[u] = 0;
							}
						}
						edge_number++;
					}
				}
			}

			printf("Number of edges/diagonals: %u\n", edge_number);
			printf("Sum of lengths of edges/diagonals: %.17f\n", length);
			printf("Half of the circumference: %.17f\n", circumference / 2);
		}
		else if (!strcmp(argv[2],"CURRENTTASK")) {
			struct task_struct *T = ROOT_POINTER_NEXT(struct task_struct*);
			print_struct_task_offsets(T);
			printf("\n");
			printf("# root PID: %d\n",T->pid);
			std::set<struct task_struct*> visited;
			walk_print_task_struct(T,visited);
		}
		else if (!strcmp(argv[2],"CURRENTTASKM")) {
			struct task_struct *T = ROOT_POINTER_NEXT(struct task_struct*);
			print_struct_task_offsets(T);
			printf("\n");
			printf("# root PID: %d\n",T->pid);
			std::set<struct task_struct*> visited;
			walk_print_task_struct(T,visited);
		}
		else if (!strcmp(argv[2],"OVERLAPLIST")) {
			struct my_task_struct *T = ROOT_POINTER_NEXT(struct my_task_struct*);
			printf("pid: %d\n",T->pid);
			printf("T: %lx\n",(uintptr_t)T);
			printf("T->im->plh: %lx\n",(uintptr_t)T->im->plh);
			printf("T->u.prev: %lx\n",(uintptr_t)T->u.prev);
			printf("T->u.next: %lx\n",(uintptr_t)T->u.next);
			printf("w: %f\n",T->w);
		}
		else if (!strcmp(argv[2],"OVERLAPPTR")) {
			unsigned char* p = ROOT_POINTER_NEXT(unsigned char*);
			(void)p;
			my_A* pA = ROOT_POINTER_NEXT(my_A*);

			printf("%d %d %d %d\n",pA->pB0->i,pA->pB1->i,pA->pB2->i,pA->pB3->i);
			printf("%lx\n",(uintptr_t)pA->p);
			printf("%s\n",pA->p);
		}
		else if ((!strcmp(argv[2],"STRINGSET"))||(!strcmp(argv[2],"STRINGSETM"))) {
			const struct rb_root* root = ROOT_POINTER_NEXT(const struct rb_root*);
			printf("stringset size: %zu\n",stringset_count(root));
			stringset_nprint(root,10);
		}
		else if (!strcmp(argv[2],"POINTER")) {
			double*** ehhh = ROOT_POINTER_SEQ(double***, 0);
			printf("The magic answer to the ultimate question of life?: %f\n", ***ehhh);
		}
		else if (!strcmp(argv[2],"FPOINTER")) {
			const struct fptr_test_struct* p = ROOT_POINTER_NEXT(const struct fptr_test_struct*);
			printf("%d\n",p->i);
			printf("%ld\n",p->l);
			printf("%s\n",p->s);
			p->sf(0,0,0);
			p->ef(0,0,0);
			p->gf(0);
		}
		else if (!strcmp(argv[2],"STRUCTARRAY")) {
			const struct MM* pM = ROOT_POINTER_SEQ(const struct MM*,2);
			printf("\n");
			printf("pM->s: %s\n",pM->s);
			for (int i=0; i<4; ++i) {
				print_struct_BB(&pM->arrB[i]);
			}
			printf("pM->Lx: %p\n",pM->Lx);
				}
			}

	unflatten_fini();
	fclose(in);

    return 0;
}
