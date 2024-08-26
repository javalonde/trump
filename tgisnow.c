#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atari.h>
#include <conio.h>
#include <peekpoke.h>
#include "bitmap.c"

#define BITMAP_WIDTH 35
#define BITMAP_HEIGHT 32
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 80
#define BYTES_PER_LINE (BITMAP_WIDTH / 4)
#define SCREEN_BYTES_PER_LINE (SCREEN_WIDTH / 4)
#define BITMAP_SIZE (BITMAP_WIDTH * BITMAP_HEIGHT / 4)
#define BUFFER_SIZE (SCREEN_BYTES_PER_LINE * BITMAP_HEIGHT)

// Makra do zapisu do pamięci
#define poke(addr, val) (*(unsigned char*) (addr) = (val))
#define pokew(addr, val) (*(unsigned int*) (addr) = (val))

void mover(int n1, const unsigned char *n2, int n3, int n4, int n5) {
    pokew(0xCB, n1);
    pokew(0xCD, (unsigned int)n2);
    poke(0xCF, n3);
    poke(0xD2, n4);
    poke(0xD0, n5);

    asm("  LDX $D2");
    asm("_label4:");
    asm("  LDY $CF");
    asm("  DEY");
    asm("_label1:");
    asm("  LDA ($CD),Y");
    asm("  STA ($CB),Y");
    asm("  DEY");
    asm("  CPY #$FF");
    asm("  BNE _label1");
    asm("  CLC");
    asm("  LDA $CB");
    asm("  ADC $D0");
    asm("  STA $CB");
    asm("  BCC _label2");
    asm("  INC $CC");
    asm("_label2:");
    asm("  CLC");
    asm("  LDA $CD");
    asm("  ADC $CF");
    asm("  STA $CD");
    asm("  BCC _label3");
    asm("  INC $CE");
    asm("_label3:");
    asm("  DEX");
    asm("  BNE _label4");
}

void activate_display_list(char* dl) {
    POKE(0x230, (unsigned int)dl & 0xFF);
    POKE(0x231, ((unsigned int)dl >> 8) & 0xFF);
}

void wait_for_vblank() {
    while (PEEK(0xD40B) != 0) {}
}

char* create_display_list(unsigned char* bitmap_data) {
    static unsigned char dl[1024];
    unsigned char* p = dl;
    unsigned int addr = (unsigned int)bitmap_data;
    int i;

    *p++ = 0x70;
    *p++ = 0x70;
    *p++ = 0x70;

    *p++ = 0x4F;
    *p++ = addr & 0xFF;
    *p++ = (addr >> 8) & 0xFF;

    for (i = 0; i < BITMAP_HEIGHT; ++i) {
        *p++ = 0x4F;
        *p++ = (addr + i * SCREEN_BYTES_PER_LINE) & 0xFF;
        *p++ = ((addr + i * SCREEN_BYTES_PER_LINE) >> 8) & 0xFF;
    }

    for (; i < SCREEN_HEIGHT; ++i) {
        *p++ = 0x70;
    }

    *p++ = 0x41;
    *p++ = (unsigned int)dl & 0xFF;
    *p++ = ((unsigned int)dl >> 8) & 0xFF;

    return (char*)dl;
}

void draw_background(unsigned char* buffer_memory, int offset) {
    int y;
    for (y = 0; y < BITMAP_HEIGHT; ++y) {
        const unsigned char* tile_ptr = background_tile + ((64 + offset + y) % 64) * 32;
        mover((int)(buffer_memory + y * SCREEN_BYTES_PER_LINE), tile_ptr, SCREEN_BYTES_PER_LINE, 1, SCREEN_BYTES_PER_LINE);
    }
}

void draw_frog(unsigned char* buffer_memory, const unsigned char* frame_data, int frog_position) {
    int y, x;
    for (y = 0; y < BITMAP_HEIGHT; ++y) {
        for (x = 0; x < BYTES_PER_LINE; ++x) {
            unsigned char pixel = frame_data[y * BYTES_PER_LINE + x];
            if (pixel != 0x00) {
                mover((int)(buffer_memory + y * SCREEN_BYTES_PER_LINE + frog_position + x), &pixel, 1, 1, SCREEN_BYTES_PER_LINE);
            }
        }
    }
}

void copy_buffer_to_screen(unsigned char* buffer_memory, unsigned char* screen_memory) {
    memcpy(screen_memory, buffer_memory, BUFFER_SIZE);
}

void joystick_control(int *frog_position) {
    int s = PEEK(0x278);  // Odczyt stanu joysticka
    if (s < 12) {
        *frog_position += (8 - s > 0) ? 2 : -2;  // Lewo/Prawo
        if (*frog_position < 0) *frog_position = 0;
        if (*frog_position > SCREEN_BYTES_PER_LINE - BYTES_PER_LINE) *frog_position = SCREEN_BYTES_PER_LINE - BYTES_PER_LINE;
    }
}

void main(void) {
    char* dl;
    unsigned char* screen_memory = (unsigned char*)0x4000;
    unsigned char buffer_memory[BUFFER_SIZE];
    int offset = 0;
    int frog_position = SCREEN_BYTES_PER_LINE / 4 - BYTES_PER_LINE / 4;

    _graphics(10);
    clrscr();

    POKE(704, 15);
    POKE(708, 12);
    POKE(706, 23);
    POKE(707, 7);
    POKE(711, 0);
    POKE(712, 50);

    dl = create_display_list(screen_memory);
    activate_display_list(dl);

    while (!kbhit()) {
        wait_for_vblank();

        joystick_control(&frog_position);

        draw_background(buffer_memory, offset);
        draw_frog(buffer_memory, frog1, frog_position);
        copy_buffer_to_screen(buffer_memory, screen_memory);
        offset = (offset + 1) % 64;

        wait_for_vblank();
        draw_background(buffer_memory, offset);
        draw_frog(buffer_memory, frog2, frog_position);
        copy_buffer_to_screen(buffer_memory, screen_memory);
        offset = (offset + 2) % 64;

        wait_for_vblank();
        draw_background(buffer_memory, offset);
        draw_frog(buffer_memory, frog3, frog_position);
        copy_buffer_to_screen(buffer_memory, screen_memory);
        offset = (offset + 2) % 64;

        wait_for_vblank();
        draw_background(buffer_memory, offset);
        draw_frog(buffer_memory, frog4, frog_position);
        copy_buffer_to_screen(buffer_memory, screen_memory);
        offset = (offset + 2) % 64;

        wait_for_vblank();
        draw_background(buffer_memory, offset);
        draw_frog(buffer_memory, frog5, frog_position);
        copy_buffer_to_screen(buffer_memory, screen_memory);
        offset = (offset + 2) % 64;

        wait_for_vblank();
        draw_background(buffer_memory, offset);
        draw_frog(buffer_memory, frog6, frog_position);
        copy_buffer_to_screen(buffer_memory, screen_memory);
        offset = (offset + 2) % 64;
    }

    cgetc();
}
