#ifndef _FREEBSD_H_
#define _FREEBSD_H_ 1

#include <elf.h>
#include <sys/endian.h>
#include <limits.h>

#define PT_NUM      			8       /* Number of defined types */
#define STT_ARM_TFUNC   		STT_LOPROC /* A Thumb function.  */
#define STT_HP_OPAQUE   		(STT_LOOS + 0x1)
#define STT_HP_STUB     		(STT_LOOS + 0x2)
#define ELFOSABI_GNU     		3   /* Object uses GNU ELF extensions.  */
#define STB_GNU_UNIQUE  		10      /* Unique symbol.  */
#define DT_TLSDESC_PLT  		0x6ffffef6
#define DT_TLSDESC_GOT  		0x6ffffef7
#define DT_GNU_CONFLICT 		0x6ffffef8  /* Start of conflict section */
#define DT_GNU_PRELINKED 		0x6ffffdf5 /* Prelinking timestamp */
#define DT_GNU_CONFLICTSZ 		0x6ffffdf6    /* Size of conflict section */
#define DT_GNU_LIBLIST  		0x6ffffef9  /* Library list */
#define DT_GNU_LIBLISTSZ 		0x6ffffdf7 /* Size of library list */

/* Processor specific values for the Phdr p_type field.  */
#define PT_ARM_EXIDX        (PT_LOPROC + 1) /* ARM unwind segment.  */
#endif