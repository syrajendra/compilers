#include "common.h"

#define ELF_STRUCT_ALLOCATE() \
		{ \
			ALLOCATE(ehdr.e_ident); 	\
			ALLOCATE(ehdr.e_type); 		\
			ALLOCATE(ehdr.e_machine); 	\
			ALLOCATE(ehdr.e_version); 	\
			ALLOCATE(ehdr.e_entry); 	\
			ALLOCATE(ehdr.e_phoff); 	\
			ALLOCATE(ehdr.e_shoff); 	\
			ALLOCATE(ehdr.e_flags); 	\
			ALLOCATE(ehdr.e_ehsize); 	\
			ALLOCATE(ehdr.e_phentsize); \
			ALLOCATE(ehdr.e_phnum); 	\
			ALLOCATE(ehdr.e_shentsize); \
			ALLOCATE(ehdr.e_shnum); 	\
			ALLOCATE(ehdr.e_shstrndx); 	\
		}

void init_32_bit_elf_struct()
{
	Elf32_Ehdr ehdr;
	ELF_STRUCT_ALLOCATE();
}

void init_64_bit_elf_struct()
{
	Elf64_Ehdr ehdr;
	ELF_STRUCT_ALLOCATE();
}

void find_endianess()
{
	if (fptr->mem[EI_DATA] == ELFDATA2LSB) {
		fptr->islittleendian = 1;
	} else if (fptr->mem[EI_DATA] == ELFDATA2MSB) {
		fptr->islittleendian = 0;
	} else {
		fprintf(stderr, "Error: Endianness unknown %d\n", (int)fptr->mem[EI_CLASS]);
		exit(-1);
	}
}

void find_byte_order()
{
	if (fptr->mem[EI_CLASS] == ELFCLASS32) { // 5th bit 1 mean 32 bit
		fptr->is32bit = 1;
	} else if (fptr->mem[EI_CLASS] == ELFCLASS64) { // 5th bit 2 mean 64 bit
		fptr->is32bit = 0;
	} else {
		fprintf(stderr, "Error: Neither 32 nor 64 bit architecture %d\n", (int)fptr->mem[EI_CLASS]);
		exit(-1);
	}
}

void print_abi()
{
	const char *str;
	switch(fptr->mem[EI_OSABI]) {
		case ELFOSABI_NONE: str = "UNIX System V ABI"; break;
		case ELFOSABI_FREEBSD: str = "FreeBSD"; break;
		default : str = "default";
	}
	fprintf(stdout, "\tOS ABI : %s { 1 byte }\n", str);
	fprintf(stdout, "\t\t[%d] = 0x%x\n", EI_OSABI, fptr->mem[EI_OSABI]);

	fprintf(stdout, "\tOS ABI version { 1 byte }\n");
	fprintf(stdout, "\t\t[%d] = 0x%x\n", EI_ABIVERSION, fptr->mem[EI_ABIVERSION]);
}

void print_file_info()
{
	fprintf(stdout, ":: Start of File Info :: %s\n", fptr->path);
	fprintf(stdout, "\tFormat : ELF : { 4 bytes }\n");
	fprintf(stdout, "\t\t[%d] = 0x%x\n", EI_MAG0, ELFMAG0);
	fprintf(stdout, "\t\t[%d] = 0x%x = %c\n", EI_MAG1, fptr->mem[EI_MAG1], ELFMAG1);
	fprintf(stdout, "\t\t[%d] = 0x%x = %c\n", EI_MAG2, fptr->mem[EI_MAG2], ELFMAG2);
	fprintf(stdout, "\t\t[%d] = 0x%x = %c\n", EI_MAG3, fptr->mem[EI_MAG3] ,ELFMAG3);
	fprintf(stdout, "\tArchitecture : %d bit { 1 byte }\n",(fptr->is32bit) ? 32 : 64);
	fprintf(stdout, "\t\t[%d] = 0x%d\n", EI_CLASS, (int)fptr->mem[EI_CLASS]);
	fprintf(stdout, "\tByte order : %s  { 1 byte }\n", (fptr->islittleendian) ?  "little endian": "big endian");
	fprintf(stdout, "\t\t[%d] = 0x%d\n", EI_DATA, (int)fptr->mem[EI_DATA]);
	fprintf(stdout, "\tFile version : { 1 byte }\n");
	fprintf(stdout, "\t\t[%d] = 0x%x\n", EI_VERSION, fptr->mem[EI_VERSION]);
	print_abi();
	fprintf(stdout, ":: End of File Info (size: 2 bytes) ::\n");
}

void is_elf_file()
{
	if (fptr->mem[EI_MAG0] != ELFMAG0 &&
		fptr->mem[EI_MAG1] != ELFMAG1 &&
		fptr->mem[EI_MAG2] != ELFMAG2 &&
		fptr->mem[EI_MAG3] != ELFMAG3) {
		fprintf(stderr, "Error: File %s is not in ELF format\n", fptr->path);
		fprintf(stderr, "Magic byte : %x\n", fptr->mem[0]);
		exit(-1);
	}
	find_byte_order();
	find_endianess();
}

void process_elf_header()
{
	MAX_BYTES offset = 0, size;

	is_elf_file();
	if (fptr->is32bit) {
		init_32_bit_elf_struct();
	} else {
		init_64_bit_elf_struct();
	}

	// Copy first 16 bytes as is
	put_field_offset(fptr->ehdr.e_ident, offset);
	memcpy(fptr->ehdr.e_ident, fptr->mem, EI_NIDENT);
	offset += EI_NIDENT;

	// Copy fields based on endianess
	COPY_FIELD(fptr->ehdr.e_type);
	COPY_FIELD(fptr->ehdr.e_machine);
	COPY_FIELD(fptr->ehdr.e_version);
	COPY_FIELD(fptr->ehdr.e_entry);
	COPY_FIELD(fptr->ehdr.e_phoff);
	COPY_FIELD(fptr->ehdr.e_shoff);
	COPY_FIELD(fptr->ehdr.e_flags);
	COPY_FIELD(fptr->ehdr.e_ehsize);
	COPY_FIELD(fptr->ehdr.e_phentsize);
	COPY_FIELD(fptr->ehdr.e_phnum);
	COPY_FIELD(fptr->ehdr.e_shentsize);
	COPY_FIELD(fptr->ehdr.e_shnum);
	COPY_FIELD(fptr->ehdr.e_shstrndx);

	if (GET_BYTES(fptr->ehdr.e_phnum) >= MAX_PROGRAM_HEADERS) {
		fprintf(stderr, "Error: Maximum limit of program header reached increase this MAX_PROGRAM_HEADERS = %d\n", MAX_PROGRAM_HEADERS);
		exit(-1);
	}

	if (GET_BYTES(fptr->ehdr.e_shnum) >= MAX_SECTION_HEADERS) {
		fprintf(stderr, "Error: Maximum limit of section header reached increase this MAX_SECTION_HEADERS = %d\n", MAX_SECTION_HEADERS);
		exit(-1);
	}
}

void print_elf_type()
{
	unsigned char *field 	= fptr->ehdr.e_type;
	unsigned int type 		= *field;

	const char *type_str;
	switch(type) {
		case ET_NONE:
			type_str = "Unknown";
			break;
		case ET_REL:
			type_str = "Relocatable object file";
			break;
		case ET_EXEC:
			type_str = "Executable";
			break;
		case ET_DYN:
			type_str = "Shared library";
			break;
		case ET_CORE:
			type_str = "Core";
			break;
	}
	PRINT_BYTES("\tType : %s { %d bytes }\n", type_str, num_bytes);
}

void print_elf_machine()
{
	const char *str = "";
	unsigned char *field 	= fptr->ehdr.e_machine;
	switch (*field) {
		case EM_X86_64: str = "AMD x86-64 architecture"; break;
		default: 		str = "default";
	}
	PRINT_BYTES("\tMachine : %s { %d bytes }\n", str, num_bytes);
}

void print_elf_object_file_version()
{
	unsigned char *field 	= fptr->ehdr.e_version;
	PRINT_BYTES("\tObject file version : { %d bytes }\n", num_bytes);
}

void print_elf_header_info()
{
	unsigned char *field = fptr->ehdr.e_entry;
	PRINT_BYTES("\tElf entry point virtual address : 0x%llu { %d bytes }\n", GET_BYTES(field), num_bytes);
	field = fptr->ehdr.e_ehsize;
	PRINT_BYTES("\tElf header size : %llu bytes { %d bytes }\n", GET_BYTES(field), num_bytes);
}

void print_prgm_header_table_info()
{
	unsigned char *field 	= fptr->ehdr.e_phoff;
	PRINT_BYTES("\tProgram header table file offset : %llu { %d bytes }\n", GET_BYTES(field), num_bytes);
	field = fptr->ehdr.e_phnum;
	PRINT_BYTES("\tProgram header table entry count : %llu { %d bytes }\n", GET_BYTES(field), num_bytes);
	field = fptr->ehdr.e_phentsize;
	PRINT_BYTES("\tProgram header table entry size : %llu bytes { %d bytes }\n", GET_BYTES(field), num_bytes);
}

void print_section_header_table_info()
{
	unsigned char *field 	= fptr->ehdr.e_shoff;
	PRINT_BYTES("\tSection header table file offset : %llu { %d bytes }\n", GET_BYTES(field), num_bytes);
	field = fptr->ehdr.e_shnum;
	PRINT_BYTES("\tSection header table entry count : %llu { %d bytes }\n", GET_BYTES(field), num_bytes);
	field = fptr->ehdr.e_shentsize;
	PRINT_BYTES("\tSection header table entry size : %llu bytes { %d bytes }\n", GET_BYTES(field), num_bytes);
	field = fptr->ehdr.e_shstrndx;
	PRINT_BYTES("\tSection header string table index : %llu { %d bytes }\n", GET_BYTES(field), num_bytes);
}


void print_elf_header()
{
	fprintf(stdout, ":: Start of ELF Header ::\n");
	print_elf_type();
	print_elf_machine();
	print_elf_object_file_version();
	print_elf_header_info();
	print_prgm_header_table_info();
	print_section_header_table_info();
	fprintf(stdout, ":: End of ELF Header (size : %llu bytes) ::\n", GET_BYTES(fptr->ehdr.e_ehsize));
}

void cleanup_elf_header()
{
	free(fptr->ehdr.e_ident);
	free(fptr->ehdr.e_type);
	free(fptr->ehdr.e_machine);
	free(fptr->ehdr.e_version);
	free(fptr->ehdr.e_entry);
	free(fptr->ehdr.e_phoff);
	free(fptr->ehdr.e_shoff);
	free(fptr->ehdr.e_flags);
	free(fptr->ehdr.e_ehsize);
	free(fptr->ehdr.e_phentsize);
	free(fptr->ehdr.e_phnum);
	free(fptr->ehdr.e_shentsize);
	free(fptr->ehdr.e_shnum);
	free(fptr->ehdr.e_shstrndx);
}
