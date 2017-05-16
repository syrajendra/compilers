#include "common.h"

#define DYNAMIC_SYMBOLINFO_SECTION_STRUCT_ALLOCATE(i) \
		{ \
			ALLOCATE(dsyminfo[i].si_boundto); \
			ALLOCATE(dsyminfo[i].si_flags); \
		}

void init_32_bit_dynamic_symbolinfo_section_struct(MAX_BYTES num_dsyminfo)
{
	MAX_BYTES i;
	Elf32_Syminfo *dsyminfo = (Elf32_Syminfo *) calloc(num_dsyminfo, sizeof(Elf32_Syminfo));
	fptr->dsyminfo 		= (RElf_Syminfo *) calloc(num_dsyminfo, sizeof(RElf_Syminfo));
	for (i=0; i<num_dsyminfo; i++) {
		DYNAMIC_SYMBOLINFO_SECTION_STRUCT_ALLOCATE(i);
	}
	free(dsyminfo);
}

void init_64_bit_dynamic_symbolinfo_section_struct(MAX_BYTES num_dsyminfo)
{
	MAX_BYTES i;
	Elf64_Syminfo *dsyminfo = (Elf64_Syminfo *) calloc(num_dsyminfo, sizeof(Elf64_Syminfo));
	fptr->dsyminfo 		= (RElf_Syminfo *) calloc(num_dsyminfo, sizeof(RElf_Syminfo));
	for (i=0; i<num_dsyminfo; i++) {
		DYNAMIC_SYMBOLINFO_SECTION_STRUCT_ALLOCATE(i);
	}
	free(dsyminfo);
}

void process_dynamic_symbolinfo_section()
{
	MAX_BYTES i, offset = 0, size = 0, found = 0, s_size;

	for (i=0; i<fptr->num_dsec; i++) {
		if (GET_BYTES(fptr->dsec[i].d_tag) == DT_SYMINENT) { /* Entry size of syminfo */
			found = 1;
		} else if (GET_BYTES(fptr->dsec[i].d_tag) == DT_SYMINSZ) { /* Size of syminfo table (in bytes) */
			size 	= GET_BYTES(fptr->dsec[i].d_un.d_val);
		} else if (GET_BYTES(fptr->dsec[i].d_tag) == DT_SYMINFO) {
			copy_data((unsigned char *)&offset, fptr->mem + GET_BYTES(fptr->dsec[i].d_un.d_val), size);
		}
	}
	if (found && offset && size) {
		if (fptr->is32bit) {
			s_size = sizeof(Elf32_Syminfo);
			fptr->num_dsyminfo = size / sizeof(Elf32_Syminfo);
			init_32_bit_dynamic_symbolinfo_section_struct(fptr->num_dsyminfo);
		} else {
			s_size = sizeof(Elf64_Syminfo);
			fptr->num_dsyminfo = size / sizeof(Elf64_Syminfo);
			init_64_bit_dynamic_symbolinfo_section_struct(fptr->num_dsyminfo);
		}
		fprintf(stdout, "Number of dynamic symbolinfo sections : %lld\n", fptr->num_dsyminfo);
		for (i=0; i<fptr->num_dsyminfo; i++) {
			offset +=  (i * s_size);
			COPY_FIELD(fptr->dsyminfo[i].si_boundto);
			COPY_FIELD(fptr->dsyminfo[i].si_flags);
		}
	}
}

void print_dynamic_symbol_info()
{

}

void cleanup_symbol_info()
{
	unsigned int i;
	for (i=0; i<fptr->num_dsyminfo; i++) {
		free(fptr->dsyminfo[i].si_boundto);
		free(fptr->dsyminfo[i].si_flags);
	}
	free(fptr->dsyminfo);
}
