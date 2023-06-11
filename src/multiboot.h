#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

typedef enum addr_range_t {
    AR_AVAILABLE = 1,
    AR_UNAVAILABLE = 2,
    AR_ACPI_USABLE = 3,
    AR_PRESERVE_HIBER = 4,
    AR_DEFECTIVE = 5,
} addr_range_t;

typedef struct mmap_addr_range_t {
    uint32_t size;
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
} mmap_addr_range_t;

typedef struct mod_t {
    void *start;
    void *end;
    char *string;
    uint32_t reserved; // or padding, whatever you want it to be
} mod_t;

typedef struct mb_info_t {
    struct flags {
        uint32_t mem : 1;
        uint32_t boot_device : 1;
        uint32_t cmdline : 1;
        uint32_t mods : 1;
        uint32_t aout : 1;
        uint32_t elf : 1;
        uint32_t mmap : 1;
        uint32_t drives : 1;
        uint32_t config_table : 1;
        uint32_t boot_loader_name : 1;
        uint32_t apm_table : 1;
        uint32_t vbe : 1;
        uint32_t framebuffer : 1;
        uint32_t reserved : 32 - 13;
    } __attribute__ ((__packed__)) flags;

    struct mem {
        uint32_t lower;
        uint32_t upper;
    } mem;

    uint32_t boot_device;
    char *cmdline;

    struct mods {
        uint32_t count;
        mod_t *addr;
    } mods;

    union {
        struct aout {
            uint32_t tabsize;
            uint32_t strsize;
            void *addr;
            uint32_t reserved;
        } aout;

        struct elf {
            uint32_t num;
            uint32_t size;
            void *addr;
            uint32_t shndx;
        } elf;
    };

    struct mmap {
        uint32_t length;
        mmap_addr_range_t *addr;
    } mmap;

    struct drives {
        uint32_t length;
        void *addr;
    } drives;

    void *config_table;
    char *boot_loader_name;
    void *apm_table;

    struct vbe {
        void *control_info;
        void *mode_info;
        uint32_t mode;
        uint32_t interface_seg;
        uint32_t interface_off;
        uint32_t interface_len;
    } vbe;
} mb_info_t;

#endif
