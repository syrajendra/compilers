#include "common.h"

// $ hexdump -n 4 bin/native/Linux/x86_64/elf-reader -C # -s as offset

file_info_t *fptr;

void cleanup_structs()
{
	unsigned int i;
	for (i=0; i<GET_BYTES(fptr->ehdr.e_phnum); i++) {
		free(fptr->phdr[i].p_type);
		free(fptr->phdr[i].p_flags);
		free(fptr->phdr[i].p_offset);
		free(fptr->phdr[i].p_vaddr);
		free(fptr->phdr[i].p_paddr);
		free(fptr->phdr[i].p_filesz);
		free(fptr->phdr[i].p_memsz);
		free(fptr->phdr[i].p_align);
	}
	for (i=0; i<GET_BYTES(fptr->ehdr.e_shnum); i++) {
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
	for (i=0; i<fptr->num_sym; i++) {
		free(fptr->sym[i].st_name);
		free(fptr->sym[i].st_value);
		free(fptr->sym[i].st_size);
		free(fptr->sym[i].st_info);
		free(fptr->sym[i].st_other);
		free(fptr->sym[i].st_shndx);
	}
	free(fptr->sym);
	for (i=0; i<fptr->num_dsec; i++) {
		free(fptr->dsec[i].d_tag);
		free(fptr->dsec[i].d_un.d_val);
	}
	free(fptr->dsec);
	for (i=0; i<fptr->num_dsyminfo; i++) {
		free(fptr->dsyminfo[i].si_boundto);
		free(fptr->dsyminfo[i].si_flags);
	}
	free(fptr->dsyminfo);
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

void cleanup()
{
	cleanup_structs();
	free(fptr->section_strtable);
	free(fptr->sym_strtable);
	free(fptr->dsec_strtable);
	close(fptr->fd);
	munmap(fptr->mem, fptr->st.st_size);
	free(fptr);
}

void print_all_headers()
{
	print_elf_header();
	print_program_headers();
	print_section_headers();
}

void parse_arguments(int argc, char *argv[], unsigned int *opt)
{
	int ch;

	while ((ch = getopt(argc, argv, "ahlSsenrudfDi")) != -1) {
		switch (ch) {
			case 'a' : opt['a'] = 1; break; // show all headers
			case 'h' : opt['h'] = 1; break; // show elf headers
			case 'l' : opt['l'] = 1; break; // show program headers
			case 'S' : opt['S'] = 1; break; // show section headers
			case 's' : opt['s'] = 1; break; // show complete symbol table
			case 'n' : opt['n'] = 1; break; // show notes
			case 'r' : opt['r'] = 1; break; // show relocation section
			case 'u' : opt['u'] = 1; break; // show unwind section
			case 'd' : opt['d'] = 1; break; // show dynamic section
			case 'f' : opt['f'] = 1; break; // show file info
			case 'D' : opt['D'] = 1; break; // show only dynamic symbols
			default: printf("Unknown option %c\n", ch); exit(-1);
		}
	}
}

unsigned int ispid(char *path)
{
	unsigned int i;
	for (i=0; i<strlen(path); i++) {
		if (!isdigit(path[i])) return 0;
	}
	return 1;
}

int main(int argc, char *argv[])
{
	unsigned int opt[256] = {0};

	fptr = (file_info_t *) calloc(1, sizeof(file_info_t));
	fptr->mem = NULL;

	if (argc < 2) {
		fprintf(stderr, "Usage:\n%s <options> <elf-file>\n", argv[0]);
		exit(1);
	}
	parse_arguments(argc, argv, opt);

	strcpy(fptr->path, argv[optind++]);
	if(ispid(fptr->path)) {
		fptr->pid = atol(fptr->path);
	} else {
		open_map_file();
	}

	// elf header processing
	process_elf_header();
	// program header processing
	process_program_headers(); // meaningful only for exe & .so files
	// section header processing
	process_section_headers();
	// .symtab symbol table processing
	process_symbol_table();
	// .dynamic section processing
	process_dynamic_section();
	// .dynsym section processing
	process_dynamic_symbol_table();
	// extra symbol info
	process_dynamic_symbolinfo_section();

	if (opt['f'] == 1)  print_file_info();
	if (opt['a'] == 1)  print_all_headers();
	if (opt['h'] == 1)  print_elf_header();
	if (opt['l'] == 1)  print_program_headers();
	if (opt['S'] == 1)  print_section_headers();
	if (opt['s'] == 1)  print_symbol_table();
	if (opt['d'] == 1)  print_dynamic_section();
	if (opt['D'] == 1)  print_dynamic_symbol_table();
	if (argc == 2) 		print_file_info();

	//verify_fields();
	//verify_field();
	cleanup();
	return 0;
}





