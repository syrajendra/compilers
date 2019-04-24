#include <map>
#include "common.h"

void print_dict()
{
	unsigned int i;
	for (i=0; i<fptr->dict_size; i++)
		fprintf(stderr, "%s : %p : %d\n", fptr->dict[i].fieldstr, fptr->dict[i].addr, fptr->dict[i].len);
}
void cleanup_dict()
{
	unsigned int i;
	for (i=0; i<fptr->dict_size; i++)
		if (fptr->dict[i].fieldstr)
			free(fptr->dict[i].fieldstr);
}

int search_field(unsigned char *addr)
{
	unsigned int i;
	for (i=0; i<fptr->dict_size; i++)
		if (fptr->dict[i].addr == addr)  return i;
	return -1;
}

void put_field_size(const char *fieldstr, unsigned char *addr, unsigned int len)
{
	int index = search_field(addr);
	if (-1 == index) {
		if (fptr->dict_size >= MAX_DICT) {
			fprintf(stderr, "Error: Maximum limit of dictionary reached increase this MAX_DICT  = %d\n", MAX_DICT);
			exit(-1);
		}
		fptr->dict[fptr->dict_size].addr = addr;
		fptr->dict[fptr->dict_size].len  = len;
		fptr->dict[fptr->dict_size].fieldstr = strdup(fieldstr);
		fptr->dict_size++;
	} else {
		if (fptr->dict[index].len != len) {
			fprintf(stderr, "Error: Dictionary already has this element with different length\n");
			fprintf(stderr, "Fields[%d]: %p:%s(%d) != %p:%s(%d)\n", index, addr, fieldstr, len, fptr->dict[index].addr, fptr->dict[index].fieldstr, fptr->dict[index].len);
			//print_dict();
			exit(-1);
		}
	}
}

void put_field_offset(unsigned char *addr, MAX_BYTES offset)
{
	int index = search_field(addr);
	if (-1 != index) {
		fptr->dict[index].offset = offset;
	} else {
		fprintf(stderr, "Error: Failed to find dict addr to put offset\n");
		exit(-1);
	}
}

unsigned int get_field_size(unsigned char *addr)
{
	int index = search_field(addr);
	if (-1 != index) {
		return fptr->dict[index].len;
	} else {
		fprintf(stderr, "Error: Failed to find pop dict for size\n");
		exit(-1);
	}
}

MAX_BYTES get_field_offset(unsigned char *addr)
{
	int index = search_field(addr);
	if (-1 != index) {
		return fptr->dict[index].offset;
	} else {
		fprintf(stderr, "Error: Failed to find pop dict for offset\n");
		exit(-1);
	}
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

int formated_fprintf(FILE *stream, const char *format, ...)
{
	int ret;
	va_list ap;
	va_start(ap, format);
	ret = vfprintf(stream, format, ap);
	va_end(ap);
	return ret;
}

void print_perror(const char *syscall, const char *str)
{
	char msg[PATH_MAX];
	sprintf(msg, "Error: %s %s", syscall, str);
	perror(msg);
}

void open_map_file()
{
	fptr->fd = open(fptr->path, O_RDONLY);
	if (fptr->fd < 0) {
		print_perror("open", fptr->path);
		exit(-1);
	}

	if (fstat(fptr->fd, &fptr->st) < 0 ) {
		perror("fstat");
		exit(-1);
	}

	fptr->mem = (unsigned char *)mmap(NULL, fptr->st.st_size, PROT_READ, MAP_PRIVATE, fptr->fd, 0);
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


