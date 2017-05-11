#ifndef _LINUX_H_
#define _LINUX_H_ 1

#include <elf.h>
#include <linux/limits.h>
#include <endian.h>

#define ELF_ST_BIND(val)		(((unsigned int)(val)) >> 4)
#define ELF_ST_TYPE(val)		((val) & 0xF)
#define ELF_ST_INFO(bind,type)	(((bind) << 4) + ((type) & 0xF))

#endif