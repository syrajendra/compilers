#ifndef _COMMON_H_
#define _COMMON_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <ctype.h>

#ifdef __linux__
	#include "linux.h"
#elif __FreeBSD__
	#include "freebsd.h"
#else
	#error "unknown platform"
#endif

#if __POINTER_WIDTH__ == 64
	typedef unsigned long long MAX_BYTES;
#else
	typedef unsigned long MAX_BYTES;
#endif

typedef struct dict {
	unsigned char 	*addr;
	unsigned int 	 len;
	MAX_BYTES 		offset;
} dict_t;

#define MAX_DICT 			5000
#define MAX_SECTION_HEADERS 512
#define MAX_PROGRAM_HEADERS 512
#define STT_RELC			8		/* Complex relocation expression */
#define STT_SRELC			9		/* Signed Complex relocation expression */
#define STT_REGISTER		13		/* global reg reserved to app. */
/* Identifies the entry point of a millicode routine.  */
#define STT_PARISC_MILLI	13
/* This macro disassembles and assembles a symbol's visibility into
   the st_other field.  The STV_ defines specify the actual visibility.  */
#define ELF_ST_VISIBILITY(v)		((v) & 0x3)

#define DT_FEATURE	0x6ffffdfc
#define DT_USED		0x7ffffffe

#define SHN_IA_64_ANSI_COMMON SHN_LORESERVE
#define EM_L1OM		180	/* Intel L1OM */
#define EM_K1OM		181	/* Intel K1OM */
/* Like SHN_COMMON but the symbol will be allocated in the .lbss
   section.  */
#define SHN_X86_64_LCOMMON 	(SHN_LORESERVE + 2)

/* Small data area common symbol.  */
#define SHN_TIC6X_SCOMMON	SHN_LORESERVE
#define EM_TI_C6000	140	/* Texas Instruments TMS320C6000 DSP family */


/* ELF Header */
typedef struct elf_ehdr {
	unsigned char		*e_ident;   /* ELF "magic number" */
	unsigned char		*e_type;    /* Identifies object file type */
	unsigned char		*e_machine;	/* Specifies required architecture */
	unsigned char		*e_version;	/* Identifies object file version */
	unsigned char		*e_entry;	/* Entry point virtual address */
	unsigned char		*e_phoff;	/* Program header table file offset */
	unsigned char		*e_shoff;	/* Section header table file offset */
	unsigned char		*e_flags;	/* Processor-specific flags */
	unsigned char		*e_ehsize;	/* ELF header size in bytes */
	unsigned char		*e_phentsize;	/* Program header table entry size */
	unsigned char		*e_phnum;	/* Program header table entry count */
	unsigned char		*e_shentsize;	/* Section header table entry size */
	unsigned char		*e_shnum;	/* Section header table entry count */
	unsigned char		*e_shstrndx;	/* Section header string table index */
} RElf_Ehdr;

/* Program header */
typedef struct elf_phdr {
	unsigned char	*p_type;		/* Identifies program segment type */
	unsigned char	*p_offset;		/* Segment file offset */
	unsigned char	*p_vaddr;		/* Segment virtual address */
	unsigned char	*p_paddr;		/* Segment physical address */
	unsigned char	*p_filesz;		/* Segment size in file */
	unsigned char	*p_memsz;		/* Segment size in memory */
	unsigned char	*p_flags;		/* Segment flags */
	unsigned char	*p_align;		/* Segment alignment, file & memory */
} RElf_Phdr;

/* Section header */
typedef struct elf_shdr {
	unsigned char	*sh_name;		/* Section name, index in string tbl */
	unsigned char	*sh_type;		/* Type of section */
	unsigned char	*sh_flags;		/* Miscellaneous section attributes */
	unsigned char	*sh_addr;		/* Section virtual addr at execution */
	unsigned char	*sh_offset;		/* Section file offset */
	unsigned char	*sh_size;		/* Size of section in bytes */
	unsigned char	*sh_link;		/* Index of another section */
	unsigned char	*sh_info;		/* Additional section information */
	unsigned char	*sh_addralign;	/* Section alignment */
	unsigned char	*sh_entsize;	/* Entry size if section holds table */
} RElf_Shdr;

typedef struct sym { // .symtab symbol table
	unsigned char	*st_name;		/* Symbol name, index in string tbl */
	unsigned char	*st_value;		/* Value of the symbol */
	unsigned char	*st_size;		/* Associated symbol size */
	unsigned char	*st_info;		/* Type and binding attributes */
	unsigned char	*st_other;		/* No defined meaning, 0 */
	unsigned char	*st_shndx;		/* Associated section index */
} RElf_Sym;

/* The syminfo section if available contains additional information about every dynamic symbol */
typedef struct d_syminfo {
	unsigned char *si_boundto;
	unsigned char *si_flags;
} RElf_Syminfo;

typedef struct d_sec { // .dynamic section
	unsigned char *d_tag;
	union {
		unsigned char *d_val;
		unsigned char *d_ptr;
	} d_un;
} RElf_Dyn;

/* Relocation Entries */

typedef struct rela {
	unsigned char *r_offset;	/* Location at which to apply the action */
	unsigned char *r_info;		/* Index and Type of relocation */
	unsigned char *r_addend;	/* Constant addend used to compute value */
} RElf_Rela;

/* Version definition sections.  */
typedef struct verdef {
	unsigned char *vd_version;     /* Version revision */
	unsigned char *vd_flags;       /* Version information */
	unsigned char *vd_ndx;         /* Version Index */
	unsigned char *vd_cnt;         /* Number of associated aux entries */
	unsigned char *vd_hash;        /* Version name hash value */
	unsigned char *vd_aux;         /* Offset in bytes to verdaux array */
	unsigned char *vd_next;        /* Offset in bytes to next verdef entry */
} RElf_Verdef;

/* Auxialiary version information.  */
typedef struct verdaux {
	unsigned char *vda_name;       /* Version or dependency names */
	unsigned char *vda_next;       /* Offset in bytes to next verdaux entry */
} RElf_Verdaux;


/* Version dependency section.  */
typedef struct verneed {
	unsigned char *vn_version;     /* Version of structure */
	unsigned char *vn_cnt;         /* Number of associated aux entries */
	unsigned char *vn_file;        /* Offset of filename for this dependency */
	unsigned char *vn_aux;         /* Offset in bytes to vernaux array */
	unsigned char *vn_next;        /* Offset in bytes to next verneed entry */
} RElf_Verneed;

/* Auxiliary needed version information.  */
typedef struct vernaux {
	unsigned char *vna_hash;       /* Hash value of dependency name */
	unsigned char *vna_flags;      /* Dependency specific information */
	unsigned char *vna_other;      /* Unused */
	unsigned char *vna_name;       /* Dependency name string offset */
	unsigned char *vna_next;       /* Offset in bytes to next vernaux entry */
} RElf_Vernaux;

/* Auxiliary vector.  */
typedef struct auxv {
	unsigned char *a_type;      /* Entry type */
	union {
		unsigned char *a_val;   /* Integer value */
	} a_un;
} RElf_auxv_t;

/* Note section */
typedef struct nhdr {
	unsigned char *n_namesz;    /* Length of the note's name.  */
	unsigned char *n_descsz;    /* Length of the note's descriptor.  */
	unsigned char *n_type;      /* Type of the note.  */
} RElf_Nhdr;

/* Move records.  */
typedef struct move {
	unsigned char *m_value;       /* Symbol value.  */
	unsigned char *m_info;        /* Size and index.  */
	unsigned char *m_poffset;     /* Symbol offset.  */
	unsigned char *m_repeat;      /* Repeat count.  */
	unsigned char *m_stride;      /* Stride info.  */
} RElf_Move;

typedef struct file_info {
	char 			path[PATH_MAX];
	pid_t 			pid;
	int 		 	fd;
	struct stat 	st;
	unsigned char 	*mem;
	unsigned int 	is32bit;
	unsigned int 	islittleendian;
	RElf_Ehdr 		ehdr;  			// single elf header

	RElf_Shdr 		shdr[MAX_SECTION_HEADERS]; // many section headers
	unsigned int 	num_shdr;

	char 			*section_strtable;
	MAX_BYTES 		section_strtable_len;

	RElf_Phdr 		phdr[MAX_PROGRAM_HEADERS]; // many program headers
	unsigned int 	num_phdr;

	dict_t 			dict[MAX_DICT];
	unsigned int    dict_size;

	RElf_Sym 		*sym; 				// Symbol table
	MAX_BYTES 		num_sym; 			// number of symbols

	char 			*sym_strtable;		// all symbol - string table
	MAX_BYTES 		sym_strtable_len; 	//

	RElf_Sym 		*dsym; 				// Dynamic symbol table
	MAX_BYTES 		num_dsym; 			// number of dynamic symbols

	char 			*dsym_strtable; 	// only dynamic - string table
	MAX_BYTES 		dsym_strtable_len; 	// length of dynamic string table

	RElf_Dyn 		*dsec; 				// .dynamic section
	MAX_BYTES 		num_dsec;

	char 			*dsec_strtable; 	// .dynamic - string table
	MAX_BYTES 		dsec_strtable_len; 	// length of .dynamic string table

	RElf_Syminfo 	*dsyminfo; 			// dynamic symbol info sections
	MAX_BYTES 		num_dsyminfo; 		// number of dynamic symbol info

	RElf_Rela 		*rel; 				// relocation entries
	MAX_BYTES 		num_rel;

} file_info_t;

extern file_info_t *fptr;

union {
	uint16_t ui;
	uint8_t  arr[2];
} d16;

union {
	uint32_t ui;
	uint8_t  arr[4];
} d32;

union {
	uint64_t ui;
	uint8_t  arr[8];
} d64;


void put_field_size(unsigned char *addr, unsigned int len);
void put_field_offset(unsigned char *addr, MAX_BYTES offset);
unsigned int get_field_size(unsigned char *addr);
MAX_BYTES get_field_offset(unsigned char *addr);
MAX_BYTES get_bytes(unsigned char *field);
int formated_fprintf(FILE *stream, const char *format, ...);
void open_map_file();
void copy_data(unsigned char *dest, unsigned char *src, size_t size);
void print_bytes(unsigned char *data, unsigned int num_bytes);

void print_file_info();

// ELF header functions
void process_elf_header();
void print_elf_header();

// Program header functions
void process_program_headers();
void print_program_headers();

// Section header functions
void process_section_headers();
void print_section_headers();

// Symbol table functions
void process_symbol_table();
void print_symbol_table();

// Dynamic section functions
void process_dynamic_section();
void print_dynamic_section();

// Extra symbol info
void process_dynamic_symbolinfo_section();

// Symbol table
void process_dynamic_symbol_table();
void print_dynamic_symbol_table();

// Relocation info
void process_relocations();
void print_relocation_section();

char *get_section_type(MAX_BYTES sec_type);
char *get_symbol_index_type (unsigned int type);
char *get_symbol_visibility (unsigned int visibility);
char *get_symbol_binding (unsigned int binding);
char *get_symbol_type (unsigned int type);

// cleanup
void cleanup_elf_header();
void cleanup_section_header();
void cleanup_program_header();
void cleanup_symbol_table();
void cleanup_dynamic_symbol_table();
void cleanup_dynamic_section();
void cleanup_symbol_info();
void cleanup_relocation_info();

#define GET_BYTES(field) get_bytes(field)

#define COPY_FIELD(field) \
		{ \
			size = get_field_size(field); \
			copy_data(field, fptr->mem + offset, size); \
			put_field_offset(field, offset); \
			offset += size; \
		}

#define ALLOCATE(x) \
		{ \
			fptr->x = (unsigned char *) calloc(sizeof(x), sizeof(unsigned char)); \
			if (fptr->x) { \
				put_field_size(fptr->x, sizeof(x)*sizeof(unsigned char)); \
			} else { \
				fprintf(stderr, "Failed to calloc \n"); \
				exit(-1); \
			} \
		}

#define PRINT_BYTES(...) \
		{ \
			unsigned int num_bytes 	= get_field_size(field); \
			fprintf (stdout, __VA_ARGS__); \
			print_bytes(field, num_bytes); \
		}
#endif
