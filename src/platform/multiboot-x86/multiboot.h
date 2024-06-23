#pragma once

#include <stdint.h>

struct multiboot_header {
    uint32_t flags;

    uint32_t mem_lower;
    uint32_t mem_upper;

    uint32_t boot_device;

    const char *cmdline;

    uint32_t mods_count;
    struct module_entry *mods_addr;

    uint32_t syms[4];

    uint32_t mmap_length;
    void *mmap_addr;

    uint32_t drives_length;
    void *drives_addr;

    void *config_table;

    const char *boot_loader_name;

    void *apm_table;

    void *vbe_control_info;
    void *vbe_mode_info;

    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

    void *framebuffer_addr;
    uint32_t unused;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    union {
        struct {
            struct color_desc *palette_addr;
            uint8_t num_colors;
        } indexed;
        struct {
            uint8_t red_field_position;
            uint8_t red_mask_size;
            uint8_t green_field_position;
            uint8_t green_mask_size;
            uint8_t blue_field_position;
            uint8_t blue_mask_size;
        } rgb;
    } color_info;
};

struct color_desc {
    uint8_t red_value;
    uint8_t green_value;
    uint8_t blue_value;
};

#define INDEXED_COLOR 0
#define DIRECT_RGB 1
#define TEXT_MODE 2

struct mmap_entry {
    uint32_t size;
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
};

#define AVAILABLE_RAM 1

struct module_entry {
    void *start;
    void *end;
    const char *string;
    uint32_t reserved;
};
