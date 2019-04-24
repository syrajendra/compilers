#include "common.h"

#define MY_LITTLE_ENDIAN(type) \
			{ \
				for (i=0, j=0; i<size; i++, j+=8) {  \
					tmp |= ((type) src[i] << j); \
				} \
			}

#define MY_BIG_ENDIAN(type) \
			{ \
				for (i=size-1, j=0; i>=0; i--, j+=8) { \
					tmp |= ((type) src[i] << j); \
				} \
			}

MAX_BYTES read_in_host_endianess(unsigned char *src, size_t size)
{
	unsigned int i, j;
	unsigned int max_size = sizeof(MAX_BYTES);
	MAX_BYTES tmp = 0;
	//fprintf(stderr, "max_size = %d size = %ld\n", max_size, size);

	if (size > 4 && max_size == 4) size = 4;
	if (fptr->islittleendian == 1) {
		//fprintf(stderr, "little endian size = %ld\n", size);
		switch(size) {
			case 1: tmp = src[0];
			case 2:
			{
				MY_LITTLE_ENDIAN(uint16_t);
				tmp = le16toh(tmp);
				break;
			}
			case 4:
			{
				MY_LITTLE_ENDIAN(uint32_t);
				tmp = le32toh(tmp);
				break;
			}
			case 8:
			{
				MY_LITTLE_ENDIAN(uint64_t);
				tmp = le64toh(tmp);
				break;
			}
			default:
				fprintf(stderr, "Error: read endianess failed %ld\n", size);
				exit(-1);
		}
	} else {
		switch(size) {
			case 1: tmp = src[0];
			case 2:
			{
				MY_BIG_ENDIAN(uint16_t);
				tmp = be16toh(tmp);
				break;
			}
			case 4:
			{
				MY_BIG_ENDIAN(uint32_t);
				tmp = be32toh(tmp);
				break;
			}
			case 8:
			{
				MY_BIG_ENDIAN(uint64_t);
				tmp = be64toh(tmp);
				break;
			}
			default:
				fprintf(stderr, "Error: read endianess failed %ld\n", size);
				exit(-1);
		}
	}
	return tmp;
}

void comapre_fields(unsigned char *f1, unsigned char *f2)
{
	unsigned int i, size = get_field_size(f2);
	fprintf(stderr, "Data %llx %llx\n", *(MAX_BYTES *)f1, *(MAX_BYTES *)f2);
	if (*(typeof(f1) *)f1 != *(typeof(f1) *)f2) {
		fprintf(stderr, "Failed compare field ");
		fprintf(stderr, "\n");
		for (i=0; i<size; i++)
			fprintf(stderr, "%d : f1=0x%x f2=0x%x\n", i, f1[i], f2[i]);
		exit(-1);
	} else {
		fprintf(stdout, "Passed field size compared : %ld \n", sizeof(typeof(f1)));
	}
}

void copy_data(unsigned char *dest, unsigned char *src, size_t size)
{
	switch (size) {
		case 1: memcpy(dest, src, size); break;
		case 2:
			{
				d16.ui = read_in_host_endianess(src, size);
				memcpy(dest, d16.arr, size);
				break;
			}
		case 4:
			{
				d32.ui = read_in_host_endianess(src, size);
				memcpy(dest, d32.arr, size);
				break;
			}
		case 8:
			{
				d64.ui = read_in_host_endianess(src, size);
				memcpy(dest, d64.arr, size);
				break;
			}
		default: printf("Size %ld not supported for copy\n", size); exit(-1);
	}
}

void verify_field()
{
	unsigned int i;

	Elf64_Ehdr *ehdr = (Elf64_Ehdr *) fptr->mem;
	Elf64_Shdr *shdr = (Elf64_Shdr *) &fptr->mem[ehdr->e_shoff];
	Elf64_Phdr *phdr = (Elf64_Phdr *) &fptr->mem[ehdr->e_phoff];

	for (i=0; i <9; i++) {
		fprintf(stderr, " ptype = %x size:%ld\n", phdr[i].p_type, sizeof(phdr[i].p_type));
	}
}

/*
#define COMAPRE_FIELD(f1, f2) \
		{ \
			typeof(f1) f3 = *(typeof(f1) *)f2; \
			fprintf(stderr, "f1 %lx  f3 %lx\n", f1, f3); \
			if (f1 != f3) { \
				fprintf(stderr, "Failed compare field size compared : %ld\n", sizeof(typeof(f1))); \
				exit(-1); \
			} else { \
				fprintf(stdout, "Passed field size compared : %ld \n", sizeof(f1)); \
			} \
		}

void verify_field()
{
	unsigned int i;

	Elf64_Ehdr *ehdr = (Elf64_Ehdr *) fptr->mem;
	Elf64_Shdr *shdr = (Elf64_Shdr *) &fptr->mem[ehdr->e_shoff];
	Elf64_Phdr *phdr = (Elf64_Phdr *) &fptr->mem[ehdr->e_phoff];

	d64.ui = phdr[0].p_paddr;
	fprintf(stderr, "Actual data ui : %lx\n", (MAX_BYTES)d64.ui);
	for (i=0; i <8; i++) {
		fprintf(stderr, "Org: [%d]=%x %x\n", i, d64.arr[i], fptr->phdr[0].p_paddr[i]);
	}
	COMAPRE_FIELD(phdr[0].p_paddr, fptr->phdr[0].p_paddr);
}

void verify_fields()
{
	MAX_BYTES i;

	Elf64_Ehdr *ehdr = (Elf64_Ehdr *) fptr->mem;
	Elf64_Shdr *shdr = (Elf64_Shdr *) &fptr->mem[ehdr->e_shoff];
	Elf64_Phdr *phdr = (Elf64_Phdr *) &fptr->mem[ehdr->e_phoff];
	//comapre_fields((unsigned char *)&ehdr->e_shstrndx, fptr->ehdr.e_shstrndx);

	//printf("Section Offset : %ld \n", ((void *)shdr - (void *)ehdr));
	//printf("Program Offset : %ld \n", ((void *)phdr - (void *)ehdr));
	//unsigned char *shstrtab= &fptr->mem[shdr[ehdr->e_shstrndx].sh_offset];

	//printf("Number of section headers : %d\n", ehdr->e_shnum);
	//for(i=0; i<ehdr->e_shnum; i++) {
	//	fprintf(stderr, "Section-%ld : my offset : %ld\n", i, ((void *)&shdr[i] - (void *)ehdr));
	//	fprintf(stderr, "Name=%s : Size=%ld bytes : Offset=%ld\n",
	//						&shstrtab[shdr[i].sh_name],
	//						shdr[i].sh_size,
	//						shdr[i].sh_offset);
	//}
	for (i=0; i<ehdr->e_phnum; i++) {
		print_segment_type((phdr[i].p_type),
							(phdr[i].p_offset),
							(phdr[i].p_vaddr),
							(phdr[i].p_paddr));
	}
	//printf("Number of program headers : %d\n", ehdr->e_phnum);
	//for(i=0; i<ehdr->e_phnum; i++) {
	//	fprintf(stderr, "%lx %lx\n", phdr[i].p_paddr, GET_BYTES(fptr->phdr[i].p_paddr));
	//	comapre_fields((unsigned char *)&phdr[i].p_paddr, fptr->phdr[i].p_paddr);
	//}
}
*/