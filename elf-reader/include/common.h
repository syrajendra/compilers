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

#define MAX_DICT 			2048
#define MAX_SECTION_HEADERS 128
#define MAX_PROGRAM_HEADERS 128
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
	unsigned char	*p_flags;		/* Segment flags */
	unsigned char	*p_offset;		/* Segment file offset */
	unsigned char	*p_vaddr;		/* Segment virtual address */
	unsigned char	*p_paddr;		/* Segment physical address */
	unsigned char	*p_filesz;		/* Segment size in file */
	unsigned char	*p_memsz;		/* Segment size in memory */
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

typedef struct sym { // symbol table
  	unsigned char	*st_name;		/* Symbol name, index in string tbl */
	unsigned char	*st_info;		/* Type and binding attributes */
  	unsigned char	*st_other;		/* No defined meaning, 0 */
	unsigned char	*st_shndx;		/* Associated section index */
  	unsigned char	*st_value;		/* Value of the symbol */
  	unsigned char	*st_size;		/* Associated symbol size */
} RElf_Sym;

typedef struct d_syminfo { // dynamic symbol info
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

typedef struct file_info {
	char 			path[PATH_MAX];
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

	RElf_Sym 		*sym; 			// Symbol table
	MAX_BYTES 		num_sym; 		// number of symbols

	char 			*sym_strtable;		// all symbol - string table
	MAX_BYTES 		sym_strtable_len; 	//

	RElf_Dyn 		*dsec; 			// .dynamic section
	MAX_BYTES 		num_dsec;

	char 			*dsec_strtable; 	// .dynamic - string table
	MAX_BYTES 		dsec_strtable_len; 	// length of .dynamic string table

	RElf_Syminfo 	*dsyminfo; 		// dynamic symbol info sections
	MAX_BYTES 		num_dsyminfo; 	// number of dynamic symbol info

	char 			*dsym_strtable; 	// only dynamic - string table
	MAX_BYTES 		dsym_strtable_len; 	// length of dynamic string table
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
