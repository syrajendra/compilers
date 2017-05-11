#include "common.h"

#define SECTION_STRUCT_ALLOCATE(i) \
		{ \
			ALLOCATE(shdr[i].sh_name); 	\
			ALLOCATE(shdr[i].sh_type); 	\
			ALLOCATE(shdr[i].sh_flags); 	\
			ALLOCATE(shdr[i].sh_addr); 	\
			ALLOCATE(shdr[i].sh_offset); 	\
			ALLOCATE(shdr[i].sh_size); 	\
			ALLOCATE(shdr[i].sh_link); 	\
			ALLOCATE(shdr[i].sh_info); 	\
			ALLOCATE(shdr[i].sh_addralign);\
			ALLOCATE(shdr[i].sh_entsize); 	\
		}

void init_32_bit_section_struct()
{
	Elf32_Shdr shdr[MAX_SECTION_HEADERS];
	unsigned int i;
	for (i=0; i<GET_BYTES(fptr->ehdr.e_shnum); i++) {
		SECTION_STRUCT_ALLOCATE(i);
	}
}

void init_64_bit_section_struct()
{
	Elf64_Shdr shdr[MAX_SECTION_HEADERS];
	unsigned int i;
	for (i=0; i<GET_BYTES(fptr->ehdr.e_shnum); i++) {
		SECTION_STRUCT_ALLOCATE(i);
	}
}

void process_string_table()
{
	unsigned int section_index 	= GET_BYTES(fptr->ehdr.e_shstrndx);
	MAX_BYTES offset 			= GET_BYTES(fptr->shdr[section_index].sh_offset);
	fptr->section_strtable_len 	= GET_BYTES(fptr->shdr[section_index].sh_size);
	fptr->section_strtable 		= (char *) calloc(sizeof(char), fptr->section_strtable_len);
	memcpy(fptr->section_strtable, fptr->mem + offset, fptr->section_strtable_len);
}

void process_section_headers()
{
	MAX_BYTES i, size;
	MAX_BYTES offset;
	if (fptr->is32bit) {
		init_32_bit_section_struct();
	} else {
		init_64_bit_section_struct();
	}
	fptr->num_shdr = GET_BYTES(fptr->ehdr.e_shnum);
	for (i=0; i<fptr->num_shdr; i++) {
		offset =  (GET_BYTES(fptr->ehdr.e_shoff)) + (i * (GET_BYTES(fptr->ehdr.e_shentsize)));
		COPY_FIELD(fptr->shdr[i].sh_name);
		COPY_FIELD(fptr->shdr[i].sh_type);
		COPY_FIELD(fptr->shdr[i].sh_flags);
		COPY_FIELD(fptr->shdr[i].sh_addr);
		COPY_FIELD(fptr->shdr[i].sh_offset);
		COPY_FIELD(fptr->shdr[i].sh_size);
		COPY_FIELD(fptr->shdr[i].sh_link);
		COPY_FIELD(fptr->shdr[i].sh_info);
		COPY_FIELD(fptr->shdr[i].sh_addralign);
		COPY_FIELD(fptr->shdr[i].sh_entsize);
	}
	process_string_table();
}

char *get_section_type(MAX_BYTES sec_type)
{
	switch(sec_type) {
		case SHT_NULL: 		return "SHT_NULL";
		case SHT_PROGBITS:  return "SHT_PROGBITS"; 	/* Program data */
		case SHT_SYMTAB: 	return "SHT_SYMTAB";	/* Symbol table */
		case SHT_STRTAB:	return "SHT_STRTAB";	/* String table */
		case SHT_RELA: 		return "SHT_RELA"; 		/* Relocation entries with addends */
		case SHT_HASH: 		return "SHT_HASH"; 		/* Symbol hash table */
		case SHT_DYNAMIC:   return "SHT_DYNAMIC"; 	/* Dynamic linking information */
		case SHT_NOTE:		return "SHT_NOTE";
		case SHT_NOBITS:	return "SHT_NOBITS"; 	/* Program space with no data (bss) */
		case SHT_REL: 		return "SHT_REL";		/* Relocation entries, no addends */
		case SHT_SHLIB: 	return "SHT_SHLIB"; 	/* Reserved */
		case SHT_LOPROC:	return "SHT_LOPROC";
		case SHT_LOUSER:	  return "SHT_LOUSER";
		case SHT_HIUSER: 	  return "SHT_HIUSER";
		case SHT_GNU_HASH:	  return "SHT_GNU_HASH"; 	/* GNU-style hash table.  */
		case SHT_DYNSYM: 	  return "SHT_DYNSYM"; 	/* Dynamic linker symbol table */
		case SHT_GNU_verneed: return "SHT_GNU_verneed";/* Version needs section.  */
		case SHT_GNU_versym:  return "SHT_GNU_versym";/* Version symbol table.  */

		default: printf("Error: Section type %llx not defined\n", sec_type); exit(-1);
	}
}

void print_sections()
{
	unsigned int i;
	fprintf(stdout, "\tSection header name : Section type : Section size\n");
	for (i=0; i<GET_BYTES(fptr->ehdr.e_shnum); i++) {
		unsigned int index 		= GET_BYTES(fptr->shdr[i].sh_name); // index in to section string table
		char *sec_type 			= get_section_type(GET_BYTES(fptr->shdr[i].sh_type));
		fprintf(stdout, "\t[%d] %s : %s : %llu \n", i, &fptr->section_strtable[index],
							sec_type,
							GET_BYTES(fptr->shdr[i].sh_size)); \
	}
}

void print_section_headers()
{
	fprintf(stdout, ":: Start Section Header (number of sections : %llu offset : %llu) ::\n",
						GET_BYTES(fptr->ehdr.e_shnum),
						GET_BYTES(fptr->ehdr.e_shoff));
	print_sections();
	fprintf(stdout, ":: End Section Header (size :%llu) ::\n", GET_BYTES(fptr->ehdr.e_shentsize));
}
