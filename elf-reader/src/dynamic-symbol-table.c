#include "common.h"

#define SYMBOL_STRUCT_ALLOCATE(i) \
		{ \
			ALLOCATE(dsym[i].st_name); \
			ALLOCATE(dsym[i].st_value); \
			ALLOCATE(dsym[i].st_size); \
			ALLOCATE(dsym[i].st_info); \
			ALLOCATE(dsym[i].st_other); \
			ALLOCATE(dsym[i].st_shndx); \
		}

static void init_32_bit_symbol_struct(MAX_BYTES num_dsymbols)
{
	MAX_BYTES i;
	Elf32_Sym *dsym  = (Elf32_Sym *) calloc(num_dsymbols, sizeof(Elf32_Sym));
	fptr->dsym 		= (RElf_Sym *) calloc(num_dsymbols, sizeof(RElf_Sym));
	for (i=0; i<num_dsymbols; i++) {
		SYMBOL_STRUCT_ALLOCATE(i);
	}
	free(dsym);
}

static void init_64_bit_symbol_struct(MAX_BYTES num_dsymbols)
{
	MAX_BYTES i;
	Elf64_Sym *dsym  = (Elf64_Sym *) calloc(num_dsymbols, sizeof(Elf64_Sym));
	fptr->dsym 		= (RElf_Sym *) calloc(num_dsymbols, sizeof(RElf_Sym));
	for (i=0; i<num_dsymbols; i++) {
		SYMBOL_STRUCT_ALLOCATE(i);
	}
	free(dsym);
}

void read_dynamic_symbol_table(char *s_name, MAX_BYTES s_offset, MAX_BYTES se_size, MAX_BYTES s_size)
{
	MAX_BYTES i, offset, size;
	fptr->num_dsym = s_size / se_size;
	if (fptr->is32bit) {
		init_32_bit_symbol_struct(fptr->num_dsym);
	} else {
		init_64_bit_symbol_struct(fptr->num_dsym);
	}
	for (i=0; i<fptr->num_dsym; i++) {
		offset =  s_offset + (i * se_size);
		COPY_FIELD(fptr->dsym[i].st_name);
		COPY_FIELD(fptr->dsym[i].st_value);
		COPY_FIELD(fptr->dsym[i].st_size);
		COPY_FIELD(fptr->dsym[i].st_info);
		COPY_FIELD(fptr->dsym[i].st_other);
		COPY_FIELD(fptr->dsym[i].st_shndx);
	}
}

void process_dynamic_symbol_table()
{
	MAX_BYTES s_offset, s_size, se_size, s_type, i;
	char *s_name;
	for (i=0; i<GET_BYTES(fptr->ehdr.e_shnum); i++) {
		s_name 		= &fptr->section_strtable[GET_BYTES(fptr->shdr[i].sh_name)];
		s_offset   	= GET_BYTES(fptr->shdr[i].sh_offset);
		s_size 		= GET_BYTES(fptr->shdr[i].sh_size);
		se_size 	= GET_BYTES(fptr->shdr[i].sh_entsize);
		s_type 		= GET_BYTES(fptr->shdr[i].sh_type);
		//fprintf(stderr, "Type: %s Name: %s\n",get_section_type(GET_BYTES(fptr->shdr[i].sh_type)), s_name);
		if (s_type == SHT_DYNSYM) { // dynamic symbol table exist
			read_dynamic_symbol_table(s_name, s_offset, se_size, s_size);
			MAX_BYTES stratb_link	= GET_BYTES(fptr->shdr[i].sh_link);
			MAX_BYTES link_offset 	= GET_BYTES(fptr->shdr[stratb_link].sh_offset);
			fptr->dsym_strtable_len 	= GET_BYTES(fptr->shdr[stratb_link].sh_size);
			fptr->dsym_strtable 		= (char *) calloc(sizeof(char), fptr->dsym_strtable_len);
			memcpy(fptr->dsym_strtable, fptr->mem + link_offset, fptr->dsym_strtable_len);
			break;
		}
	}
}

void print_dynamic_symbol_table()
{
	MAX_BYTES i;
	fprintf(stdout, "Number of dynamic symbols in symbol table: %lld\n", fptr->num_dsym);
	if (fptr->num_dsym) {
		fprintf(stdout, "%5s | %18s | %8s | %8s | %10s | %4s | Name\n", "Num", "Value" ,"Type", "Bind","Visibility", "Ndx");
		for (i=0; i<fptr->num_dsym; i++) {
			MAX_BYTES val = GET_BYTES(fptr->dsym[i].st_value);
			char *type = get_symbol_type(ELF_ST_TYPE (GET_BYTES(fptr->dsym[i].st_info)));
			char *bind = get_symbol_binding(ELF_ST_BIND (GET_BYTES(fptr->dsym[i].st_info)));
			char *visb = get_symbol_visibility(ELF_ST_VISIBILITY (GET_BYTES(fptr->dsym[i].st_info)));
			char *sndx = get_symbol_index_type(GET_BYTES(fptr->dsym[i].st_shndx));
			fprintf(stderr, "%5lld | %#18llx | %8s | %8s | %10s | %s | %s\n", i, val, type, bind, visb, sndx,
								&fptr->dsym_strtable[GET_BYTES(fptr->dsym[i].st_name)]);
		}
	}
}

void cleanup_dynamic_symbol_table()
{
	MAX_BYTES i;
	for (i=0; i<fptr->num_dsym; i++) {
		free(fptr->dsym[i].st_name);
		free(fptr->dsym[i].st_value);
		free(fptr->dsym[i].st_size);
		free(fptr->dsym[i].st_info);
		free(fptr->dsym[i].st_other);
		free(fptr->dsym[i].st_shndx);
	}
	free(fptr->dsym);
	free(fptr->dsym_strtable);
}


