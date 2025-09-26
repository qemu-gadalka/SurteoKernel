#include <stdint.h>
#include <stddef.h> 

volatile char* vga_buffer = (volatile char*)0xB8000;
static uint16_t vga_position = 0;
#define VGA_COLOR 0x07
#define PS2_STATUS_PORT 0x64

void outb(unsigned short port, unsigned char data);
unsigned char inb(unsigned short port);
void outw(uint16_t port, uint16_t data); 

void cls(void); 
void echo(const char* str);
void reboot(void);
void shutdown(void);
void reboot_to_bootloader(void);
void sleep(uint32_t milliseconds);

void outb(unsigned short port, unsigned char data) {
    asm volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

unsigned char inb(unsigned short port) {
    unsigned char data;
    asm volatile ("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

void outw(uint16_t port, uint16_t data) {
    asm volatile ("outw %0, %1" : : "a"(data), "Nd"(port));
}

void cls(void) {
    uint16_t* video_memory = (uint16_t*)0xB8000;
    uint16_t clear_char = (uint16_t)0x0720; 
    for (int i = 0; i < 80 * 25; ++i) {
        video_memory[i] = clear_char; 
    }
    vga_position = 0;
}

void echo(const char* str) {
    while (*str) {
        if (vga_position >= 80 * 25) {
            vga_position = 0;
            cls(); 
        }

        if (*str == '\n') {
            vga_position += 80 - (vga_position % 80);
        } else if (*str == '\b') {
            if (vga_position > 0) {
                vga_position--;
                vga_buffer[vga_position * 2] = ' ';
                vga_buffer[vga_position * 2 + 1] = VGA_COLOR;
            }
        } else {
            vga_buffer[vga_position * 2] = *str;
            vga_buffer[vga_position * 2 + 1] = VGA_COLOR;
            vga_position++;
        }
        str++;
    }
}

void sleep(uint32_t milliseconds) {
    for (uint32_t ms = 0; ms < milliseconds; ms++) {
        for (volatile uint32_t i = 0; i < 285000; i++) {
            asm volatile ("nop");
        }
    }
}

void reboot(void) {
    echo("\nRebooting...\n");
    sleep(100);
    outb(0xCF9, 0x06); 
    
    while(1) {
        asm volatile ("hlt"); 
    }
    echo("\nReboot failed!\nYour PC does not support ACPI, sorry\n");
}

void shutdown(void) {
    echo("\nShutting down...\n");
    sleep(100);
    outw(0xB004, 0x2000); 
    outw(0x604, 0x2000); 
    
    while(1) {
        asm volatile ("hlt"); 
    }

    echo("\nShutdown failed!\nYour PC does not support ACPI, sorry\n");
}

void reboot_to_bootloader(void) {
    echo("\nRebooting to Bootloader...\n");
    sleep(100);

    while (inb(PS2_STATUS_PORT) & 0x02) { 

    }
    outb(0x64, 0xFE); 
    
    while(1) {
        asm volatile ("hlt"); 
    }
}

void kernel_main(uint32_t memory_map_address) {
    cls();

    uint8_t old_color = VGA_COLOR;
    uint8_t orange_color = 0x06;

    const char* log_prefix = "[SURTEO LOG]";
    const char* log_suffix = " Surteo loaded.\n";

    for (const char* p = log_prefix; *p; ++p) {
        vga_buffer[vga_position * 2] = *p;
        vga_buffer[vga_position * 2 + 1] = orange_color;
        vga_position++;
    }

    for (const char* p = log_suffix, log2_suffix; *p; ++p) {
        if (*p == '\n') {
            vga_position += 80 - (vga_position % 80);
        } else {
            vga_buffer[vga_position * 2] = *p;
            vga_buffer[vga_position * 2 + 1] = old_color;
            vga_position++;
        }
    }
    
    while(1) {
        asm volatile ("hlt"); 
    }
}
