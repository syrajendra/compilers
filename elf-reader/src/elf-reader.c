#include "common.h"

// $ hexdump -n 4 bin/native/Linux/x86_64/elf-reader -C # -s as offset

file_info_t *fptr;

void cleanup()
{
	cleanup_elf_header();
	cleanup_section_header();
	cleanup_program_header();
	cleanup_symbol_table();
	cleanup_dynamic_symbol_table();
	cleanup_dynamic_section();
	cleanup_symbol_info();
	cleanup_relocation_info();
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
	// .rel & .rela section processing
	process_relocations();

	if (opt['f'] == 1)  print_file_info();
	if (opt['a'] == 1)  print_all_headers();
	if (opt['h'] == 1)  print_elf_header();
	if (opt['l'] == 1)  print_program_headers();
	if (opt['S'] == 1)  print_section_headers();
	if (opt['s'] == 1)  print_symbol_table();
	if (opt['d'] == 1)  print_dynamic_section();
	if (opt['D'] == 1)  print_dynamic_symbol_table();
	if (opt['r'] == 1)  print_relocation_section();
	if (argc == 2) 		print_file_info();

	//verify_fields();
	//verify_field();
	cleanup();
	return 0;
}





