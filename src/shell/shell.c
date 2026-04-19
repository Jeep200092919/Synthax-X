#include <stdint.h>
#include <stddef.h>
#include "shell/shell.h"
#include "drivers/vga.h"
#include "drivers/keyboard.h"
#include "utils/string.h"
#include "mem/pmm.h"

#define SHELL_CMD_MAX 256

static char cmd_buf[SHELL_CMD_MAX];
static size_t cmd_len;

static void cmd_help(void) {
    vga_write("Available commands:\n");
    vga_write("  help     - show this message\n");
    vga_write("  clear    - clear the screen\n");
    vga_write("  mem      - show memory usage\n");
    vga_write("  version  - show kernel version\n");
}

static void cmd_mem(void) {
    uint64_t total = pmm_total_bytes();
    uint64_t used  = pmm_used_bytes();
    uint64_t freeb = pmm_free_bytes();

    vga_write("Total: ");
    vga_write_dec(total / (1024ULL * 1024ULL));
    vga_write(" MB\n");

    vga_write("Used:  ");
    vga_write_dec(used / 1024ULL);
    vga_write(" KB\n");

    vga_write("Free:  ");
    vga_write_dec(freeb / (1024ULL * 1024ULL));
    vga_write(" MB\n");
}

static void cmd_version(void) {
    vga_write("Synthax X OS - 64-bit Kernel\n");
}

void shell_execute_command(char *input) {
    if (input[0] == '\0') return;

    if (strcmp(input, "help") == 0) {
        cmd_help();
    } else if (strcmp(input, "clear") == 0) {
        vga_clear_screen();
    } else if (strcmp(input, "mem") == 0) {
        cmd_mem();
    } else if (strcmp(input, "version") == 0) {
        cmd_version();
    } else {
        vga_write("Unknown command: ");
        vga_write(input);
        vga_write("\n");
    }
}

void shell_prompt(void) {
    vga_write("synthax> ");
}

void shell_init(void) {
    cmd_len = 0;
    cmd_buf[0] = '\0';
    vga_write("\nSynthax X Shell - type 'help' for commands.\n");
}

void shell_run(void) {
    shell_prompt();
    for (;;) {
        char c = keyboard_getchar();

        if (c == '\n') {
            vga_putc('\n');
            cmd_buf[cmd_len] = '\0';
            shell_execute_command(cmd_buf);
            cmd_len = 0;
            cmd_buf[0] = '\0';
            shell_prompt();
        } else if (c == '\b') {
            if (cmd_len > 0) {
                cmd_len--;
                cmd_buf[cmd_len] = '\0';
                vga_putc('\b');
            }
        } else if (c >= 0x20 && c < 0x7F) {
            if (cmd_len < SHELL_CMD_MAX - 1) {
                cmd_buf[cmd_len++] = c;
                vga_putc(c);
            }
        }
    }
}
