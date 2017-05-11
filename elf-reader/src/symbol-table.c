#include "common.h"

#define SYMBOL_STRUCT_ALLOCATE(i) \
		{ \
			ALLOCATE(sym[i].st_name); \
			ALLOCATE(sym[i].st_value); \
			ALLOCATE(sym[i].st_size); \
			ALLOCATE(sym[i].st_info); \
			ALLOCATE(sym[i].st_other); \
			ALLOCATE(sym[i].st_shndx); \
		}

void init_32_bit_dynamic_symbol_struct(MAX_BYTES num_symbols)
{
	MAX_BYTES i;
	Elf32_Sym *sym  = (Elf32_Sym *) calloc(num_symbols, sizeof(Elf32_Sym));
	fptr->sym 		= (RElf_Sym *) calloc(num_symbols, sizeof(RElf_Sym));
	for (i=0; i<num_symbols; i++) {
		SYMBOL_STRUCT_ALLOCATE(i);
	}
	free(sym);
}

void init_64_bit_dynamic_symbol_struct(MAX_BYTES num_symbols)
{
	MAX_BYTES i;
	Elf64_Sym *sym  = (Elf64_Sym *) calloc(num_symbols, sizeof(Elf64_Sym));
	fptr->sym 		= (RElf_Sym *) calloc(num_symbols, sizeof(RElf_Sym));
	for (i=0; i<num_symbols; i++) {
		SYMBOL_STRUCT_ALLOCATE(i);
	}
	free(sym);
}

void read_symbol_table(char *s_name, MAX_BYTES s_offset, MAX_BYTES se_size, MAX_BYTES s_size)
{
	MAX_BYTES i, offset, size;
	fptr->num_sym = s_size / se_size;
	if (fptr->is32bit) {
		init_32_bit_dynamic_symbol_struct(fptr->num_sym);
	} else {
		init_64_bit_dynamic_symbol_struct(fptr->num_sym);
	}
	for (i=0; i<fptr->num_sym; i++) {
		offset =  s_offset + (i * se_size);
		COPY_FIELD(fptr->sym[i].st_name);
		COPY_FIELD(fptr->sym[i].st_value);
		COPY_FIELD(fptr->sym[i].st_size);
		COPY_FIELD(fptr->sym[i].st_info);
		COPY_FIELD(fptr->sym[i].st_other);
		COPY_FIELD(fptr->sym[i].st_shndx);
	}
}

void process_symbol_table()
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
		if (s_type == SHT_SYMTAB) { // symbol table exist
			read_symbol_table(s_name, s_offset, se_size, s_size);
			MAX_BYTES stratb_link	= GET_BYTES(fptr->shdr[i].sh_link);
			MAX_BYTES link_offset 	= GET_BYTES(fptr->shdr[stratb_link].sh_offset);
			fptr->sym_strtable_len 	= GET_BYTES(fptr->shdr[stratb_link].sh_size);
			fptr->sym_strtable 		= (char *) calloc(sizeof(char), fptr->sym_strtable_len);
			memcpy(fptr->sym_strtable, fptr->mem + link_offset, fptr->sym_strtable_len);
			break;
		}
	}
}

char *get_symbol_type (unsigned int type)
{
	static char buff[32];
	switch (type) {
		case STT_NOTYPE:	return "NOTYPE";
		case STT_OBJECT:	return "OBJECT";
		case STT_FUNC:		return "FUNC";
		case STT_SECTION:	return "SECTION";
		case STT_FILE:		return "FILE";
		case STT_COMMON:	return "COMMON";
		case STT_TLS:		return "TLS";
		case STT_RELC:      return "RELC";
		case STT_SRELC:     return "SRELC";
		default:
		{
			if (type >= STT_LOPROC && type <= STT_HIPROC) {
				if (GET_BYTES(fptr->ehdr.e_machine) == EM_ARM && type == STT_ARM_TFUNC)
					return "THUMB_FUNC";
				if (GET_BYTES(fptr->ehdr.e_machine) == EM_SPARCV9 && type == STT_REGISTER)
					return "REGISTER";
				if (GET_BYTES(fptr->ehdr.e_machine) == EM_PARISC && type == STT_PARISC_MILLI)
					return "PARISC_MILLI";
				snprintf (buff, sizeof (buff), "<processor specific>: %d", type);
			} else if (type >= STT_LOOS && type <= STT_HIOS) {
				if (GET_BYTES(fptr->ehdr.e_machine) == EM_PARISC) {
					if (type == STT_HP_OPAQUE)
						return "HP_OPAQUE";
					if (type == STT_HP_STUB)
						return "HP_STUB";
				}
				if (type == STT_GNU_IFUNC
					&& (fptr->ehdr.e_ident[EI_OSABI] == ELFOSABI_GNU
					|| fptr->ehdr.e_ident[EI_OSABI] == ELFOSABI_FREEBSD
					/* GNU is still using the default value 0.  */
					|| fptr->ehdr.e_ident[EI_OSABI] == ELFOSABI_NONE))
					return "IFUNC";
				snprintf (buff, sizeof (buff), "<OS specific>: %d", type);
			} else
				snprintf (buff, sizeof (buff), "<unknown>: %d", type);
			return buff;
		}
	}
}

char *get_symbol_binding (unsigned int binding)
{
	static char buff[32];

	switch (binding)
	{
		case STB_LOCAL:		return "LOCAL";
		case STB_GLOBAL:	return "GLOBAL";
		case STB_WEAK:		return "WEAK";
		default:
		{
			if (binding >= STB_LOPROC && binding <= STB_HIPROC)
				snprintf (buff, sizeof (buff), "<processor specific>: %d", binding);
			else if (binding >= STB_LOOS && binding <= STB_HIOS) {
				if (binding == STB_GNU_UNIQUE
					&& (fptr->ehdr.e_ident[EI_OSABI] == ELFOSABI_GNU
					/* GNU is still using the default value 0.  */
					|| fptr->ehdr.e_ident[EI_OSABI] == ELFOSABI_NONE))
						return "UNIQUE";
				snprintf (buff, sizeof (buff), "<OS specific>: %d", binding);
			} else
				snprintf (buff, sizeof (buff), "<unknown>: %d", binding);
			return buff;
		}
	}
}

char *get_symbol_visibility (unsigned int visibility)
{
	switch (visibility) {
		case STV_DEFAULT:	return "DEFAULT";
		case STV_INTERNAL:	return "INTERNAL";
		case STV_HIDDEN:	return "HIDDEN";
		case STV_PROTECTED: return "PROTECTED";
		default:
			fprintf(stderr, "Unrecognized visibility value: %u", visibility);
			return "<unknown>";
	}
}
void print_symbol_table()
{
	MAX_BYTES i;
	fprintf(stdout, "Number of symbols in symbol table: %lld\n", fptr->num_sym);
	if (fptr->num_sym) {
		fprintf(stdout, "Num : Value : Type : Bind : Visibility: Name\n");
		for (i=0; i<fptr->num_sym; i++) {
			MAX_BYTES val = GET_BYTES(fptr->sym[i].st_value);
			char *type = get_symbol_type(ELF_ST_TYPE (GET_BYTES(fptr->sym[i].st_info)));
			char *bind = get_symbol_binding(ELF_ST_BIND (GET_BYTES(fptr->sym[i].st_info)));
			char *visb = get_symbol_visibility(ELF_ST_VISIBILITY (GET_BYTES(fptr->sym[i].st_info)));
			fprintf(stderr, "%llx: 0x%llx %s %s %s %s\n", i, val, type, bind, visb,
								&fptr->sym_strtable[GET_BYTES(fptr->sym[i].st_name)]);
		}
	}
}
