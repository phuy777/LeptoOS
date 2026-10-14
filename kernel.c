#include "kernel.h"
#include <stddef.h>
#include <stdint.h>

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

uint8_t vga_entry_color(enum vga_color fg , enum vga_color bg){
    return fg | bg << 4;
}
uint16_t vga_entry(unsigned char uc, uint8_t color){
    return (uint16_t) uc | (uint16_t) color << 8;
}
size_t strlen(const char* str){
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
        
    }
    return len;
}
void terminal_initialize(void){
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) 0xB8000;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for(size_t x = 0; x < VGA_WIDTH; x++ ){
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ' , terminal_color);
        }
    }
}
void set_color(uint8_t color){
    terminal_color = color;
}
void terminal_putentryat(char c , uint8_t color ,size_t x, size_t y){
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}
static void terminal_scroll(void) {
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t src_index = y * VGA_WIDTH + x;
            const size_t dst_index = (y - 1) * VGA_WIDTH + x;
            terminal_buffer[dst_index] = terminal_buffer[src_index];
        }
    }

    for (size_t x = 0; x < VGA_WIDTH; x++) {
        const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
    }

    terminal_row = VGA_HEIGHT - 1;
}

void terminal_putchar(char c){
    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
        } else if (terminal_row > 0) {
            terminal_row--;
            terminal_column = VGA_WIDTH - 1;
        }
        terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        return;
    }

    if (c == '\r') {
        terminal_column = 0;
        return;
    }

    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT) {
            terminal_scroll();
        }
        return;
    }

    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT) {
            terminal_scroll();
        }
    }
}
void terminal_write(const char* data, size_t size){
    if (data == NULL) {
        return;
    }

    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}
void terminal_writestring(const char* data){
    if (data == NULL) {
        return;
    }

    terminal_write(data, strlen(data));
}
static inline uint8_t inb(uint16_t port){
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
uint8_t keyboard_read_scancode(){
    while(!(inb(0x64) & 0x01));
    return inb(0x60);
}
const char keyboard_scancode_to_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};
int string_cmp(const char* str1, const char* str2){
    if (str1 == NULL && str2 == NULL) {
        return 0;
    }
    if (str1 == NULL) {
        return -1;
    }
    if (str2 == NULL) {
        return 1;
    }

    while (*str1 != '\0' && (*str1 == *str2)) {
        str1++;
        str2++;
    }

    return (int)(*(const unsigned char*)str1) - (int)(*(const unsigned char*)str2);
}
void execute_command(char* cmd) {
    if (cmd == NULL) {
        return;
    }

    if (string_cmp(cmd, "help") == 0) {
        terminal_writestring("Available commands:\n");
        terminal_writestring("  help   - Display this help list\n");
        terminal_writestring("  clear  - Clear the screen\n");
        terminal_writestring("  hello  - Say hello to the OS\n");
    } else if (string_cmp(cmd, "clear") == 0) {
        terminal_initialize();
    } else if (string_cmp(cmd, "hello") == 0) {
        terminal_writestring("Hello developer! Welcome to your custom OS shell.\n");
    } else if (cmd[0] != '\0') {
        terminal_writestring("Unknown command. Type 'help' for options.\n");
    }
}
void kernel_shell(void) {
    char cmd_buffer[128] = {0};
    size_t cmd_index = 0;

    terminal_writestring("\nmy_os> ");

    while (1) {
        uint8_t scancode = keyboard_read_scancode();

        if (scancode & 0x80) {
            continue;
        }
        if (scancode >= sizeof(keyboard_scancode_to_ascii)) {
            continue;
        }

        char c = keyboard_scancode_to_ascii[scancode];

        if (c == '\n' || c == '\r') {
            cmd_buffer[cmd_index] = '\0';
            terminal_putchar('\n');
            execute_command(cmd_buffer);
            cmd_index = 0;
            terminal_writestring("my_os> ");
        } else if (c == '\b') {
            if (cmd_index > 0) {
                cmd_index--;
                cmd_buffer[cmd_index] = '\0';
                terminal_writestring("\b \b");
            }
        } else if (c != 0 && cmd_index < sizeof(cmd_buffer) - 1) {
            cmd_buffer[cmd_index++] = c;
            char str[2] = {c, '\0'};
            terminal_writestring(str);
        }
    }
}
