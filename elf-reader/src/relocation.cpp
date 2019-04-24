#include "common.h"

#define RELOCATION_STRUCT_ALLOCATE(i) \
		{ \
			ALLOCATE(rel[i].r_offset); 	\
			ALLOCATE(rel[i].r_info); 	\
		}

#define RELOCATION_STRUCT_ALLOCATE_ADDEND(i) \
		{ \
			ALLOCATE(rel[i].r_offset); 	\
			ALLOCATE(rel[i].r_info); 	\
			ALLOCATE(rel[i].r_addend);  \
		}

static void allocate_rel(MAX_BYTES num_rel)
{
	if (!fptr->rel) {
		fptr->rel 		= (RElf_Rela *) calloc(num_rel, sizeof(RElf_Rela));
		fptr->num_rel 	= num_rel;
	} else {
		RElf_Rela *tmp 	= (RElf_Rela *) calloc(fptr->num_rel + num_rel, sizeof(RElf_Rela));
		memcpy(tmp, fptr->rel, fptr->num_rel*sizeof(RElf_Rela));
		free(fptr->rel);
		fptr->rel 	   	= tmp;
		fptr->num_rel  += num_rel;
	}
}

void init_32_bit_relocation_struct(MAX_BYTES num_rel, unsigned int addend)
{
	MAX_BYTES i;
	allocate_rel(num_rel);

	if (addend) {
		Elf32_Rela *rel 	= (Elf32_Rela *) calloc(num_rel, sizeof(Elf32_Rela));
		for (i=0; i<num_rel; i++) {
			RELOCATION_STRUCT_ALLOCATE_ADDEND(i);
		}
		free(rel);
	} else {
		Elf32_Rel *rel 	= (Elf32_Rel *) calloc(num_rel, sizeof(Elf32_Rel));
		for (i=0; i<num_rel; i++) {
			RELOCATION_STRUCT_ALLOCATE(i);
		}
		free(rel);
	}
}

void init_64_bit_relocation_struct(MAX_BYTES num_rel, unsigned int addend)
{
	MAX_BYTES i;
	allocate_rel(num_rel);

	if (addend) {
		Elf64_Rela *rel 	= (Elf64_Rela *) calloc(num_rel, sizeof(Elf64_Rela));
		for (i=0; i<num_rel; i++) {
			RELOCATION_STRUCT_ALLOCATE_ADDEND(i);
		}
		free(rel);
	} else {
		Elf64_Rel *rel 	= (Elf64_Rel *) calloc(num_rel, sizeof(Elf64_Rel));
		for (i=0; i<num_rel; i++) {
			RELOCATION_STRUCT_ALLOCATE(i);
		}
		free(rel);
	}
}

void read_relocation_section(MAX_BYTES num_rel, MAX_BYTES s_offset, MAX_BYTES se_size, unsigned int addend)
{
	MAX_BYTES i, offset, size;
	if (fptr->is32bit) {
		init_32_bit_relocation_struct(num_rel, addend);
	} else {
		init_64_bit_relocation_struct(num_rel, addend);
	}
	for (i=0; i<num_rel; i++) {
		offset =  s_offset + (i * se_size);
		COPY_FIELD(fptr->rel[i].r_offset);
		COPY_FIELD(fptr->rel[i].r_info);
		if(addend) {
			COPY_FIELD(fptr->rel[i].r_addend);
		}
	}
}

void process_relocations()
{
	MAX_BYTES i, s_type, s_size, se_size, s_offset, num_rel = 0, addend = 0;
	for (i=0; i<fptr->num_shdr; i++) {
		s_offset= GET_BYTES(fptr->shdr[i].sh_offset);
		s_size 	= GET_BYTES(fptr->shdr[i].sh_size);
		se_size = GET_BYTES(fptr->shdr[i].sh_entsize);
		s_type 	= GET_BYTES(fptr->shdr[i].sh_type);
		if (s_type == SHT_RELA || s_type == SHT_REL) { // .relocation section exist
			if (s_size) {
				num_rel  = s_size / se_size;
				if(s_type == SHT_RELA) addend = 1;
				read_relocation_section(num_rel, s_offset, se_size, addend);
			}
		}
	}
}

unsigned int get_relocation_type(MAX_BYTES r_info)
{
	if (fptr->is32bit) {
		return ELF32_R_TYPE (r_info);
	} else {
		return ELF64_R_TYPE (r_info);
	}
}


const char *get_relocation_type_str(MAX_BYTES type)
{
	switch(GET_BYTES(fptr->ehdr.e_machine)) {
		case EM_X86_64:
		{
			switch(type) {
				case R_X86_64_NONE: return "R_X86_64_NONE";
				case R_X86_64_GLOB_DAT: return "R_X86_64_GLOB_DAT";
				case R_X86_64_RELATIVE: return "R_X86_64_RELATIVE";
				default: return "DoNo type";
			}
		}
		case EM_ARM:
		{
			switch(type) {
				case R_ARM_JUMP_SLOT: return "R_ARM_JUMP_SLOT";
				default: return "DoNo type";
			}
		}
		case EM_386:
		{
			switch(type) {
				case R_386_GLOB_DAT: return "R_386_GLOB_DAT";
				default: return "DoNo type";
			}
		}
		default: return "DoNo machine";
	}
}

MAX_BYTES get_reloc_symindex(MAX_BYTES r_info)
{
	if (fptr->is32bit) {
		return ELF32_R_SYM(r_info);
	} else {
		return ELF64_R_SYM(r_info);
	}
}

void print_relocation_section()
{
	MAX_BYTES i;
	fprintf(stdout, "Number of relocation entries : %llu \n", fptr->num_rel);
	fprintf(stdout, "%10s | %10s | %18s | %10s | %s\n", "Offset", "Info", "Type", "Sym. Value","Sym.Name + Addend");
	for (i=0; i<fptr->num_rel; i++) {
		MAX_BYTES r_offset 	= GET_BYTES(fptr->rel[i].r_offset);
		MAX_BYTES r_info 	= GET_BYTES(fptr->rel[i].r_info);
		const char *r_type 		= get_relocation_type_str(get_relocation_type(r_info));
		MAX_BYTES sym_index = get_reloc_symindex(r_info);
		MAX_BYTES sym_value = GET_BYTES(fptr->dsym[sym_index].st_value);
		char *sym_name 		= &fptr->dsym_strtable[GET_BYTES(fptr->dsym[sym_index].st_name)];
		fprintf(stdout, "%10llx | %10llx | %18s | %10llx | %s \n", r_offset, r_info, r_type, sym_value, sym_name);
	}
}

void cleanup_relocation_info()
{
	MAX_BYTES i;
	for (i=0; i<fptr->num_rel; i++) {
		free(fptr->rel[i].r_offset);
		free(fptr->rel[i].r_info);
		if (fptr->rel[i].r_addend) free(fptr->rel[i].r_addend);
	}
	free(fptr->rel);
}
