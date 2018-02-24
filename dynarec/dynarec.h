#ifndef __DYNAREC_H__
#define __DYNAREC_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
/* Just to tell emacs to stop indenting everything because of the
   block above */
}
#endif

/* PSX RAM size in bytes: 2MB */
#define PSX_RAM_SIZE               0x200000U
/* BIOS ROM size in bytes: 512kB */
#define PSX_BIOS_SIZE              0x80000U
/* Base address for the BIOS ROM */
#define PSX_BIOS_BASE              0x1FC00000U
/* Scratchpad size in bytes: 1kB */
#define PSX_SCRATCHPAD_SIZE        1024U
/* Base address for the scratchpad */
#define PSX_SCRATCHPAD_BASE        0x1F800000U

/* Log in base 2 of the page size*/
#define DYNAREC_PAGE_SIZE_SHIFT    9U
/* Length of a recompilation page in bytes */
#define DYNAREC_PAGE_SIZE          (1U << DYNAREC_PAGE_SIZE_SHIFT)
/* Number of instructions per page */
#define DYNAREC_PAGE_INSTRUCTIONS  (DYNAREC_PAGE_SIZE / 4U)

/* Total number of dynarec pages in RAM */
#define DYNAREC_RAM_PAGES          (PSX_RAM_SIZE / DYNAREC_PAGE_SIZE)
/* Total number of dynarec pages in BIOS ROM */
#define DYNAREC_BIOS_PAGES         (PSX_BIOS_SIZE / DYNAREC_PAGE_SIZE)

/* Total number of dynarec pages for the system */
#define DYNAREC_TOTAL_PAGES        (DYNAREC_RAM_PAGES + DYNAREC_BIOS_PAGES)

#ifndef ARRAY_SIZE
# define ARRAY_SIZE(_a) (sizeof(_a) / sizeof((_a)[0]))
#endif


#ifdef DYNAREC_DEBUG
#define DYNAREC_LOG(fmt, ...)     \
   fprintf(stderr, "[DYNAREC]: " fmt, __VA_ARGS__)
#else
#define DYNAREC_LOG(fmt, ...) do{} while (0)
#endif


struct dynarec_state;

typedef int32_t (*dynarec_store_cback)(struct dynarec_state *state,
                                       uint32_t val,
                                       uint32_t addr,
                                       int32_t counter);

struct dynarec_state {
   /* Current value of the PC */
   uint32_t            pc;
   /* Region mask, it's used heavily in the dynarec'd code so it's
      convenient to have it accessible in this struct. */
   uint32_t            region_mask[8];
   /* Pointer to the PSX RAM */
   uint32_t           *ram;
   /* Pointer to the PSX scratchpad */
   uint32_t           *scratchpad;
   /* Pointer to the PSX BIOS */
   const uint32_t     *bios;
   /* Called when a SW to an unsupported memory address is
      encountered. Returns the new counter value. */
   dynarec_store_cback memory_sw;
   /* Private data for the caller */
   void               *priv;
   /* All general purpose CPU registers except R0 */
   uint32_t            regs[31];
   /* Executable region of memory containing the dynarec'd code */
   uint8_t            *map;
   /* Length of the map */
   uint32_t            map_len;
   /* Keeps track of whether each page is valid or needs to be
      recompiled */
   uint8_t             page_valid[DYNAREC_TOTAL_PAGES];
};


extern struct dynarec_state *dynarec_init(uint32_t *ram,
                                          uint32_t *scratchpad,
                                          const uint32_t *bios,
                                          dynarec_store_cback memory_sw);

extern void dynarec_delete(struct dynarec_state *state);
extern void dynarec_set_next_event(struct dynarec_state *state,
                                   int32_t cycles);
extern void dynarec_set_pc(struct dynarec_state *state,
                           uint32_t pc);
extern int32_t dynarec_run(struct dynarec_state *state,
                           int32_t cycles_to_run);

#ifdef __cplusplus
}
#endif

#endif /* __DYNAREC_H__ */
