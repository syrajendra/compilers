#include "common.h"

void put_field_size(unsigned char *addr, unsigned int len)
{
	fptr->dict[fptr->dict_size].addr = addr;
	fptr->dict[fptr->dict_size].len  = len;
	fptr->dict_size++;
	if (fptr->dict_size >= MAX_DICT) {
		fprintf(stderr, "Error: Maximum limit of dictionary reached increase this MAX_DICT  = %d\n", MAX_DICT);
		exit(-1);
	}
}

void put_field_offset(unsigned char *addr, MAX_BYTES offset)
{
	unsigned int i = 0;
	for (i=0; i<fptr->dict_size; i++) {
		if (fptr->dict[i].addr == addr)  {
			fptr->dict[i].offset = offset;
			return;
		}
	}
	fprintf(stderr, "Error: Failed to find dict addr to put offset\n");
	exit(-1);
}

unsigned int get_field_size(unsigned char *addr)
{
	unsigned int i = 0;
	for (i=0; i<fptr->dict_size; i++) {
		if (fptr->dict[i].addr == addr) return fptr->dict[i].len;
	}
	fprintf(stderr, "Error: Failed to find pop dict for size\n");
	exit(-1);
}

#define GET_DATA(field, TYPE) *((TYPE *)field)

MAX_BYTES get_bytes(unsigned char *field)
{
	unsigned int size = get_field_size(field);
	switch(size) {
		case 1: return GET_DATA(field, uint8_t);
		case 2: return GET_DATA(field, uint16_t);
		case 4: return GET_DATA(field, uint32_t);
		case 8: return GET_DATA(field, uint64_t);
		default:
			fprintf(stderr, "Error: get bytes does not know size %d\n", size);
			exit(-1);
	}
}

MAX_BYTES get_field_offset(unsigned char *addr)
{
	unsigned int i = 0;
	for (i=0; i<fptr->dict_size; i++) {
		if (fptr->dict[i].addr == addr) return fptr->dict[i].offset;
	}
	fprintf(stderr, "Error: Failed to find pop dict for offset\n");
	exit(-1);
}

int formated_fprintf(FILE *stream, const char *format, ...)
{
	int ret;
	va_list ap;
	va_start(ap, format);
	ret = vfprintf(stream, format, ap);
	va_end(ap);
	return ret;
}

void open_map_file()
{
	fptr->fd = open(fptr->path, O_RDONLY);
	if (fptr->fd < 0) {
		perror("open");
		exit(-1);
	}

	if (fstat(fptr->fd, &fptr->st) < 0 ) {
		perror("fstat");
		exit(-1);
	}

	fptr->mem = mmap(NULL, fptr->st.st_size, PROT_READ, MAP_PRIVATE, fptr->fd, 0);
	if (fptr->mem == MAP_FAILED) {
		perror("mmap");
		exit(-1);
	}
}

void print_bytes(unsigned char *data, unsigned int num_bytes)
{
	unsigned int i, j;
	MAX_BYTES offset = get_field_offset(data);
	for(i=0, j=0; i<num_bytes; i++, j+=8) {
		fprintf(stdout, "\t\t[%llu] = 0x%x = %d\n", offset+i, data[i], data[i]);
	}
}


