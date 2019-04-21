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
	if (fptr->num_shdr) process_string_table();
}

char *get_section_type(MAX_BYTES sec_type)
{
	switch(sec_type) {
		case SHT_NULL: 			return "SHT_NULL";
		case SHT_PROGBITS:  	return "SHT_PROGBITS"; 	/* Program data */
		case SHT_SYMTAB: 		return "SHT_SYMTAB";	/* Symbol table */
		case SHT_STRTAB:		return "SHT_STRTAB";	/* String table */
		case SHT_RELA: 			return "SHT_RELA"; 		/* Relocation entries with addends */
		case SHT_HASH: 			return "SHT_HASH"; 		/* Symbol hash table */
		case SHT_DYNAMIC:   	return "SHT_DYNAMIC"; 	/* Dynamic linking information */
		case SHT_NOTE:			return "SHT_NOTE";
		case SHT_NOBITS:		return "SHT_NOBITS"; 	/* Program space with no data (bss) */
		case SHT_REL: 			return "SHT_REL";		/* Relocation entries, no addends */
		case SHT_SHLIB: 		return "SHT_SHLIB"; 	/* Reserved */
		case SHT_LOPROC:	  	return "SHT_LOPROC";
		case SHT_LOUSER:	  	return "SHT_LOUSER";
		case SHT_HIUSER: 	  	return "SHT_HIUSER";
		case SHT_GNU_HASH:	  	return "SHT_GNU_HASH"; 	/* GNU-style hash table.  */
		case SHT_DYNSYM: 	  	return "SHT_DYNSYM"; 	/* Dynamic linker symbol table */
		case SHT_GNU_verneed: 	return "SHT_GNU_verneed";/* Version needs section.  */
		case SHT_GNU_versym:  	return "SHT_GNU_versym";/* Version symbol table.  */
		case SHT_ARM_EXIDX: 	return "SHT_ARM_EXID"; /* ARM unwind section.  */
		case SHT_INIT_ARRAY: 	return "SHT_INIT_ARRAY"; /* Array of constructors */
		case SHT_FINI_ARRAY: 	return "SHT_FINI_ARRAY"; /* Array of destructors */
		case SHT_ARM_PREEMPTMAP: return "SHT_ARM_PREEMPTMAP"; /* Preemption details.  */
		case SHT_ARM_ATTRIBUTES: return "SHT_ARM_ATTRIBUTES"; /* ARM attributes section.  */
		default: printf("Error: Section type %llx not defined\n", sec_type); exit(-1);
	}
}

char *get_section_use(char *name)
{
	if (!strcmp(name, ".bss")) 		return "Uninitialized data";
	if (!strcmp(name, ".data")) 	return "Initialized data";
	if (!strcmp(name, ".interp")) 	return "Program interpreter path name";
	if (!strcmp(name, ".rodata")) 	return "Read-only data (constants and literals)";
	if (!strcmp(name, ".text")) 	return "Executable code";
	if (!strcmp(name, ".comment")) 	return "Version control information";
	if (!strcmp(name, ".dynamic")) 	return "Dynamic linking tables";
	if (!strcmp(name, ".dynstr")) 	return "String table for .dynamic section";
	if (!strcmp(name, ".dynsym")) 	return "Symbol table for dynamic linking";
	if (!strcmp(name, ".got")) 		return "Holds addresses of variables which are relocated upon loading";
	if (!strcmp(name, ".hash")) 	return "Symbol hash table";
	if (!strcmp(name, ".note")) 	return "Note section";
	if (!strcmp(name, ".plt")) 		return "Holds the trampoline/linkage code";
	if (!strcmp(name, ".shstrtab")) return "Section name string table";
	if (!strcmp(name, ".strtab")) 	return "String table";
	if (!strcmp(name, ".symtab")) 	return "Linker symbol table";
	if (!strcmp(name, ".gnu.hash")) return "GNU extension to hash table for symbols";
	if (!strcmp(name, ".rela.dyn")) return "Runtime/Dynamic relocation table";
	if (!strcmp(name, ".ctors")) 	return "Functions which are marked constructor or C++ constructor";
	if (!strcmp(name, ".dtors")) 	return "Functions which are marked destructor or C++ destructor";
	if (!strcmp(name, ".fini")) 	return "Code which will be executed when program exits normally.";
	if (!strcmp(name, ".gnu.version")) 	 return "Version of symbols";
	if (!strcmp(name, ".gnu.version_r")) return "Version references of symbols";
	if (!strcmp(name, ".got.plt") || !strcmp(name, ".plt.got")) 	return "Holds addresses of functions in dynamic libraries";
	if (!strcmp(name, ".init")) 	return "Code which will be executed when program initializes";
	if (!strcmp(name, ".jcr")) 		return "Java class registration information";
	if (!strcmp(name, ".note.ABI-tag")) return "Linux specific note";
	if (!strcmp(name, ".note.gnu.build-id")) return "A unique build ID";
	if (!strncmp(name, ".eh_frame",8)) return "Frame unwind information (EH = Exception Handling)";
	if (!strncmp(name, ".debug_", 6))	return "Debug info";
	if (!strcmp(name, ".ARM.exidx")) return "ARM unwind section";
	if (!strcmp(name, ".ARM.attributes")) return "ARM object attributes";
	if (!strcmp(name, ".init_array")) return "Pointers to functions which will be executed when program starts";
	if (!strcmp(name, ".fini_array")) return "Pointers to functions which will be executed when program exits normally";
	if (!strcmp(name, ".note.tag")) return "Note tag section";
	return "";
}

char *get_section_flags(MAX_BYTES sec_flags)
{
	static char buff[256];
	memset(buff, '\0', 256);
	if(sec_flags & SHF_WRITE) 		strcat(buff, "Write ");
	if(sec_flags & SHF_ALLOC) 		strcat(buff, "Alloc ");
	if(sec_flags & SHF_EXECINSTR) 	strcat(buff, "Exe ");
	if(sec_flags & SHF_TLS) 		strcat(buff, "Tls ");
	if(sec_flags & SHF_STRINGS) 	strcat(buff, "Strings ");
	if(sec_flags & SHF_MERGE) 		strcat(buff, "Merge ");
	return buff;
}


void print_sections()
{
	unsigned int i;
	fprintf(stdout, "\t%2s | %18s | %18s | %18s | %s\n", "No","Section name", "Section type", "Flags", "Use");
	for (i=0; i<GET_BYTES(fptr->ehdr.e_shnum); i++) {
		unsigned int index 		= GET_BYTES(fptr->shdr[i].sh_name); // index in to section string table
		char *sec_type 			= get_section_type(GET_BYTES(fptr->shdr[i].sh_type));
		fprintf(stdout, "\t%2d | %18s | %18s | %18s | %s \n", i,
							&fptr->section_strtable[index],
							sec_type,
							get_section_flags(GET_BYTES(fptr->shdr[i].sh_flags)),
							get_section_use(&fptr->section_strtable[index]));
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

void cleanup_section_header()
{
	unsigned int i;
	for (i=0; i<fptr->num_shdr; i++) {
		free(fptr->shdr[i].sh_name);
		free(fptr->shdr[i].sh_type);
		free(fptr->shdr[i].sh_flags);
		free(fptr->shdr[i].sh_addr);
		free(fptr->shdr[i].sh_offset);
		free(fptr->shdr[i].sh_size);
		free(fptr->shdr[i].sh_link);
		free(fptr->shdr[i].sh_info);
		free(fptr->shdr[i].sh_addralign);
		free(fptr->shdr[i].sh_entsize);
	}
	free(fptr->section_strtable);
}
