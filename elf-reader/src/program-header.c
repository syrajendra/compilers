#include "common.h"

#define PROGRAM_STRUCT_ALLOCATE(i) \
		{ \
			ALLOCATE(phdr[i].p_type); 	\
			ALLOCATE(phdr[i].p_offset); \
			ALLOCATE(phdr[i].p_vaddr); 	\
			ALLOCATE(phdr[i].p_paddr); 	\
			ALLOCATE(phdr[i].p_filesz); \
			ALLOCATE(phdr[i].p_memsz); 	\
			ALLOCATE(phdr[i].p_flags); 	\
			ALLOCATE(phdr[i].p_align); 	\
		}

void init_32_bit_program_struct()
{
	Elf32_Phdr phdr[MAX_PROGRAM_HEADERS];
	unsigned int i;
	for (i=0; i<GET_BYTES(fptr->ehdr.e_phnum); i++) {
		fprintf(stdout, "Program %d\n", i);
		PROGRAM_STRUCT_ALLOCATE(i);
	}
}

void init_64_bit_program_struct()
{
	Elf64_Phdr phdr[MAX_PROGRAM_HEADERS];
	unsigned int i;
	for (i=0; i<GET_BYTES(fptr->ehdr.e_phnum); i++) {
		PROGRAM_STRUCT_ALLOCATE(i);
	}
}

void process_program_headers()
{
	MAX_BYTES i, size;
	MAX_BYTES offset;
	if (fptr->is32bit) {
		init_32_bit_program_struct();
	} else {
		init_64_bit_program_struct();
	}
	fptr->num_phdr = GET_BYTES(fptr->ehdr.e_phnum);
	for (i=0; i<fptr->num_phdr; i++) {
		offset =  (GET_BYTES(fptr->ehdr.e_phoff)) + (i * (GET_BYTES(fptr->ehdr.e_phentsize)));
		if (fptr->is32bit) {
			COPY_FIELD(fptr->phdr[i].p_type);
			COPY_FIELD(fptr->phdr[i].p_offset);
			COPY_FIELD(fptr->phdr[i].p_vaddr);
			COPY_FIELD(fptr->phdr[i].p_paddr);
			COPY_FIELD(fptr->phdr[i].p_filesz);
			COPY_FIELD(fptr->phdr[i].p_memsz);
			COPY_FIELD(fptr->phdr[i].p_flags);
			COPY_FIELD(fptr->phdr[i].p_align);
		} else {
			COPY_FIELD(fptr->phdr[i].p_type);
			COPY_FIELD(fptr->phdr[i].p_flags);
			COPY_FIELD(fptr->phdr[i].p_offset);
			COPY_FIELD(fptr->phdr[i].p_vaddr);
			COPY_FIELD(fptr->phdr[i].p_paddr);
			COPY_FIELD(fptr->phdr[i].p_filesz);
			COPY_FIELD(fptr->phdr[i].p_memsz);
			COPY_FIELD(fptr->phdr[i].p_align);
		}
	}
}

void print_segment_type(unsigned int i, MAX_BYTES seg_type,
						MAX_BYTES seg_file_offset,
						MAX_BYTES seg_virtual_addr,
						MAX_BYTES seg_physical_addr)
{
	char *str;
	switch(seg_type) {
		case PT_LOAD:
				if (seg_file_offset == 0) {
					fprintf(stdout, "\t[%d] Text segment : 0x%llx : 0x%llx\n", i, seg_virtual_addr, seg_physical_addr);
				} else {
					fprintf(stdout, "\t[%d] Data segment : 0x%llx : 0x%llx\n", i, seg_virtual_addr, seg_physical_addr);
				}
				break;
		case PT_INTERP:
			{
				str = (char *)fptr->mem + seg_file_offset;
				fprintf(stdout, "\t[%d] Interpreter : %s\n", i, str);
				break;
			}
		case PT_NOTE:
				fprintf(stdout, "\t[%d] Note segment : 0x%llx : 0x%llx\n", i, seg_virtual_addr, seg_physical_addr);
				break;
		case PT_DYNAMIC:
				fprintf(stdout, "\t[%d] Dynamic segment : 0x%llx : 0x%llx\n", i, seg_virtual_addr, seg_physical_addr);
				break;
		case PT_PHDR:
				fprintf(stdout, "\t[%d] Phdr segment : 0x%llx : 0x%llx\n", i, seg_virtual_addr, seg_physical_addr);
				break;
		case PT_NULL:
				fprintf(stdout,"\t[%d] Program header table entry unused segment\n", i);
				break;
		case PT_TLS:
				fprintf(stdout, "\t[%d] Thread-local storage segment\n", i);
				break;
		case PT_GNU_EH_FRAME:
				fprintf(stdout, "\t[%d] GCC .eh_frame_hdr segment\n", i);
				break;
		case PT_GNU_STACK:
				fprintf(stdout, "\t[%d] Indicates stack executability segment\n", i);
				break;
		case PT_GNU_RELRO:
				fprintf(stdout, "\t[%d] Read-only after relocation segment\n", i);
				break;
		case PT_LOPROC:
				fprintf(stdout, "\t[%d] Start of processor-specific segment\n", i);
				break;
		case PT_HIPROC:
				fprintf(stdout, "\t[%d] End of processor-specific segment\n", i);
				break;
		case PT_SHLIB:
				fprintf(stdout, "\t[%d] Reserved segment\n", i);
				break;
		case PT_NUM:
				fprintf(stdout, "\t[%d] Number of defined types segment\n", i);
				break;
		case PT_LOOS:
				fprintf(stdout, "\t[%d] Start of OS specific segment\n", i);
				break;
		case PT_HIOS:
				fprintf(stdout, "\t[%d] End of OS specific segment\n", i);
				break;
		case PT_ARM_EXIDX:
				fprintf(stdout, "\t[%d] Exception unwind table segment\n", i);
				break;
		default:
			fprintf(stdout, "Segment not handled %llx\n", seg_type);
	}
}

void print_segments()
{
	MAX_BYTES i;
	fprintf(stdout, "\tSegment name : virtual addr : physical addr\n");
	for (i=0; i<GET_BYTES(fptr->ehdr.e_phnum); i++) {
		print_segment_type(i,
							GET_BYTES(fptr->phdr[i].p_type),
							GET_BYTES(fptr->phdr[i].p_offset),
							GET_BYTES(fptr->phdr[i].p_vaddr),
							GET_BYTES(fptr->phdr[i].p_paddr));
	}
}

void print_program_headers()
{
	fprintf(stdout, ":: Start Program Header (number of programs : %llu offset : %llu) ::\n",
						GET_BYTES(fptr->ehdr.e_phnum),
						GET_BYTES(fptr->ehdr.e_phoff));
	print_segments();
	fprintf(stdout, ":: End Program Header (size :%llu) ::\n", GET_BYTES(fptr->ehdr.e_phentsize));
}

void cleanup_program_header()
{
	unsigned int i;
	for (i=0; i<fptr->num_phdr; i++) {
		free(fptr->phdr[i].p_type);
		free(fptr->phdr[i].p_offset);
		free(fptr->phdr[i].p_vaddr);
		free(fptr->phdr[i].p_paddr);
		free(fptr->phdr[i].p_filesz);
		free(fptr->phdr[i].p_memsz);
		free(fptr->phdr[i].p_flags);
		free(fptr->phdr[i].p_align);
	}
}
