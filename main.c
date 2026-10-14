#include <stdint.h>
#include "kernel.h"

void kmain(void) {
    terminal_initialize();
    
    terminal_writestring("Hello, OS World from a structured kernel!\n");
    terminal_writestring("Everything is running smoothly.\n");
    kernel_shell();
    while (1) {
        __asm__ volatile ("hlt");
    }
}