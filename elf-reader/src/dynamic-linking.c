#include "common.h"

#define DYNAMIC_SECTION_STRUCT_ALLOCATE(i) \
		{ \
			ALLOCATE(dsec[i].d_tag); \
			ALLOCATE(dsec[i].d_un.d_val); \
		}

void init_32_bit_dynamic_section_struct(MAX_BYTES num_dsection)
{
	MAX_BYTES i;
	Elf32_Dyn *dsec = (Elf32_Dyn *) calloc(num_dsection, sizeof(Elf32_Dyn));
	fptr->dsec 		= (RElf_Dyn *) calloc(num_dsection, sizeof(RElf_Dyn));
	for (i=0; i<num_dsection; i++) {
		DYNAMIC_SECTION_STRUCT_ALLOCATE(i);
	}
	free(dsec);
}

void init_64_bit_dynamic_section_struct(MAX_BYTES num_dsection)
{
	MAX_BYTES i;
	Elf64_Dyn *dsec = (Elf64_Dyn *) calloc(num_dsection, sizeof(Elf64_Dyn));
	fptr->dsec 		= (RElf_Dyn *) calloc(num_dsection, sizeof(RElf_Dyn));
	for (i=0; i<num_dsection; i++) {
		DYNAMIC_SECTION_STRUCT_ALLOCATE(i);
	}
	free(dsec);
}

void read_dynamic_section(MAX_BYTES s_offset, MAX_BYTES se_size)
{
	MAX_BYTES i, offset, size;
	if (fptr->is32bit) {
		init_32_bit_dynamic_section_struct(fptr->num_dsec);
	} else {
		init_64_bit_dynamic_section_struct(fptr->num_dsec);
	}
	for (i=0; i<fptr->num_dsec; i++) {
		offset =  s_offset + (i * se_size);
		COPY_FIELD(fptr->dsec[i].d_tag);
		COPY_FIELD(fptr->dsec[i].d_un.d_val);
	}
}

void process_dynamic_section()
{
	MAX_BYTES s_offset, s_size, se_size, s_type, i;
	char *s_name;
	for (i=0; i<GET_BYTES(fptr->ehdr.e_shnum); i++) {
		s_name 			= &fptr->section_strtable[GET_BYTES(fptr->shdr[i].sh_name)];
		s_offset   		= GET_BYTES(fptr->shdr[i].sh_offset);
		s_size 			= GET_BYTES(fptr->shdr[i].sh_size);
		se_size 		= GET_BYTES(fptr->shdr[i].sh_entsize);
		s_type 			= GET_BYTES(fptr->shdr[i].sh_type);
		if (s_type == SHT_DYNAMIC) { // .dynamic section exist
			fptr->num_dsec  = s_size / se_size;
			read_dynamic_section(s_offset, se_size);
			MAX_BYTES stratb_link	= GET_BYTES(fptr->shdr[i].sh_link);
			MAX_BYTES link_offset 	= GET_BYTES(fptr->shdr[stratb_link].sh_offset);
			fptr->dsec_strtable_len = GET_BYTES(fptr->shdr[stratb_link].sh_size);
			fptr->dsec_strtable 	= (char *) calloc(sizeof(char), fptr->dsec_strtable_len);
			memcpy(fptr->dsec_strtable, fptr->mem + link_offset, fptr->dsec_strtable_len);
			break;
		}
	}
}


char *get_dynamic_type (unsigned long type)
{
  	switch (type) {
		case DT_NULL:		return "NULL";
		case DT_NEEDED:		return "NEEDED";
		case DT_PLTRELSZ:	return "PLTRELSZ";
		case DT_PLTGOT:		return "PLTGOT";
		case DT_HASH:		return "HASH";
		case DT_STRTAB:		return "STRTAB";
		case DT_SYMTAB:		return "SYMTAB";
		case DT_RELA:		return "RELA";
		case DT_RELASZ:		return "RELASZ";
		case DT_RELAENT:	return "RELAENT";
		case DT_STRSZ:		return "STRSZ";
		case DT_SYMENT:		return "SYMENT";
		case DT_INIT:		return "INIT";
		case DT_FINI:		return "FINI";
		case DT_SONAME:		return "SONAME";
		case DT_RPATH:		return "RPATH";
		case DT_SYMBOLIC:	return "SYMBOLIC";
		case DT_REL:		return "REL";
		case DT_RELSZ:		return "RELSZ";
		case DT_RELENT:		return "RELENT";
		case DT_PLTREL:		return "PLTREL";
		case DT_DEBUG:		return "DEBUG";
		case DT_TEXTREL:	return "TEXTREL";
		case DT_JMPREL:		return "JMPREL";
		case DT_BIND_NOW:   return "BIND_NOW";
		case DT_INIT_ARRAY: return "INIT_ARRAY";
		case DT_FINI_ARRAY: 	return "FINI_ARRAY";
		case DT_INIT_ARRAYSZ: 	return "INIT_ARRAYSZ";
		case DT_FINI_ARRAYSZ: 	return "FINI_ARRAYSZ";
		case DT_RUNPATH:    	return "RUNPATH";
		case DT_FLAGS:      	return "FLAGS";

		case DT_PREINIT_ARRAY: return "PREINIT_ARRAY";
		case DT_PREINIT_ARRAYSZ: return "PREINIT_ARRAYSZ";

		case DT_CHECKSUM:	return "CHECKSUM";
		case DT_PLTPADSZ:	return "PLTPADSZ";
		case DT_MOVEENT:	return "MOVEENT";
		case DT_MOVESZ:	return "MOVESZ";
		case DT_FEATURE:	return "FEATURE";
		case DT_POSFLAG_1:	return "POSFLAG_1";
		case DT_SYMINSZ:	return "SYMINSZ";
		case DT_SYMINENT:	return "SYMINENT"; /* aka VALRNGHI */

		case DT_ADDRRNGLO:  return "ADDRRNGLO";
		case DT_CONFIG:	return "CONFIG";
		case DT_DEPAUDIT:	return "DEPAUDIT";
		case DT_AUDIT:	return "AUDIT";
		case DT_PLTPAD:	return "PLTPAD";
		case DT_MOVETAB:	return "MOVETAB";
		case DT_SYMINFO:	return "SYMINFO"; /* aka ADDRRNGHI */

		case DT_VERSYM:	return "VERSYM";

		case DT_TLSDESC_GOT: return "TLSDESC_GOT";
		case DT_TLSDESC_PLT: return "TLSDESC_PLT";
		case DT_RELACOUNT:	return "RELACOUNT";
		case DT_RELCOUNT:	return "RELCOUNT";
		case DT_FLAGS_1:	return "FLAGS_1";
		case DT_VERDEF:	return "VERDEF";
		case DT_VERDEFNUM:	return "VERDEFNUM";
		case DT_VERNEED:	return "VERNEED";
		case DT_VERNEEDNUM:	return "VERNEEDNUM";

		case DT_AUXILIARY:	return "AUXILIARY";
		case DT_USED:	return "USED";
		case DT_FILTER:	return "FILTER";

		case DT_GNU_PRELINKED: return "GNU_PRELINKED";
		case DT_GNU_CONFLICT: return "GNU_CONFLICT";
		case DT_GNU_CONFLICTSZ: return "GNU_CONFLICTSZ";
		case DT_GNU_LIBLIST: return "GNU_LIBLIST";
		case DT_GNU_LIBLISTSZ: return "GNU_LIBLISTSZ";
		case DT_GNU_HASH:	return "GNU_HASH";
		default:
			fprintf(stderr, "Error: Dynamic type not handled : 0x%lx\n", type);
			exit(-1);
	}
	return "<unknown>";
}

char *get_lib_name(MAX_BYTES d_tag, MAX_BYTES d_val, char *d_type)
{
	static char buff[PATH_MAX];
	char *name = &fptr->dsec_strtable[d_val];
	switch(d_tag) {
		case DT_NEEDED:  sprintf(buff, "Shared library [%s]", name); break;
		case DT_SONAME:  sprintf(buff, "Library soname [%s]", name); break;
		case DT_RPATH: 	 sprintf(buff, "Library rpath  [%s]", name); break;
		case DT_RUNPATH: sprintf(buff, "Library runpath[%s]", name); break;
		default:
		{
			unsigned int size = strlen(d_type);
			if ((d_type[size-2] == 'S' && d_type[size-1] == 'Z') ||
				(d_type[size-3] == 'E' && d_type[size-2] == 'N' && d_type[size-1] == 'T'))
				sprintf(buff, "%llu bytes", d_val);
			else if (d_type[size-3] == 'N' && d_type[size-2] == 'U' && d_type[size-1] == 'M')
				sprintf(buff, "%llu", d_val);
			else
				sprintf(buff, "0x%llx", d_val);
		}
	}
	return buff;
}

void print_dynamic_section()
{
	MAX_BYTES i;
	fprintf(stdout, "Number of dynamic sections : %llu \n", fptr->num_dsec);
	fprintf(stdout, "%10s | %10s | %s\n", "Tag", "Type", "Name/Value");
	for (i=0; i<fptr->num_dsec; i++) {
		MAX_BYTES d_tag = GET_BYTES(fptr->dsec[i].d_tag);
		MAX_BYTES d_val = GET_BYTES(fptr->dsec[i].d_un.d_val);
		char *d_type 	= get_dynamic_type(d_tag);
		fprintf(stdout, "%#10llx | %10s | %s\n", d_tag, d_type, get_lib_name(d_tag, d_val, d_type));
	}
}
