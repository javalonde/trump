#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atari.h>
#include <conio.h>
#include <peekpoke.h>
#include "bitmap.c"

#define BITMAP_WIDTH 35
#define BITMAP_HEIGHT 32
#define SCREEN_WIDTH 160 // Szerokość ekranu w trybie Antic Mode F (160 pikseli)
#define SCREEN_HEIGHT 80
#define BYTES_PER_LINE (BITMAP_WIDTH / 4)
#define SCREEN_BYTES_PER_LINE (SCREEN_WIDTH / 4)
#define BITMAP_SIZE (BITMAP_WIDTH * BITMAP_HEIGHT / 4)
#define BUFFER_SIZE (SCREEN_BYTES_PER_LINE * BITMAP_HEIGHT)

#define PMG_BASE 0xC000
#define SPRITE_WIDTH 8
#define SPRITE_HEIGHT 8
#define SPRITE_POS_X 0x64   // Pozycja X sprite'a w heksadecymalnej
#define SPRITE_POS_Y 0x32   // Pozycja Y sprite'a w heksadecymalnej

// Makra do zapisu do pamięci
#define poke(addr, val) (*(unsigned char*) (addr) = (val))
#define pokew(addr, val) (*(unsigned int*) (addr) = (val))

// Definicja danych duszka - napis ATARI (8x8 pikseli)
const unsigned char atari_sprite[] = {
    0xFF, // AAAAAAAA
    0x81, // A......A
    0xBD, // A.XXXX.X
    0xBD, // A.XXXX.X
    0xBD, // A.XXXX.X
    0xBD, // A.XXXX.X
    0x81, // A......A
    0xFF  // AAAAAAAA
};

// Asemblerowa funkcja kopiująca blok pamięci (używana do rysowania)
void mover(int n1, const unsigned char *n2, int n3, int n4, int n5)
{
    pokew(0xCB, n1);                 // Adres docelowy w pamięci ekranu
    pokew(0xCD, (unsigned int)n2);   // Rzutowanie wskaźnika na typ całkowity
    poke(0xCF, n3);                  // Liczba bajtów na wiersz
    poke(0xD2, n4);                  // Liczba wierszy
    poke(0xD0, n5);                  // Liczba bajtów na wiersz w pamięci ekranu

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

// Funkcja aktywująca display list
void activate_display_list(char* dl) {
    POKE(0x230, (unsigned char)((unsigned int)dl & 0xFF));
    POKE(0x231, (unsigned char)(((unsigned int)dl >> 8) & 0xFF));
}

// Funkcja czekająca na VBlank
void wait_for_vblank() {
    while (PEEK(0xD40B) != 0) {}  // Wait for VCOUNT to reset to 0
}

// Funkcja tworząca display list dla bitmapy 35x32
char* create_display_list(unsigned char* bitmap_data) {
    static unsigned char dl[1024];
    unsigned char* p = dl;
    unsigned int addr = (unsigned int)bitmap_data;
    int i;

    // Display List Setup for Antic Mode F (4-color mode)
    *p++ = 0x70; // 8 pustych linii
    *p++ = 0x70;
    *p++ = 0x70;

    // Load Memory Scan (LMS) with Antic Mode F
    *p++ = 0x4F; // LMS + Antic Mode F
    *p++ = (unsigned char)(addr & 0xFF);
    *p++ = (unsigned char)((addr >> 8) & 0xFF);

    // Pozostałe linie ekranu w Antic Mode F
    for (i = 0; i < BITMAP_HEIGHT; ++i) {
        *p++ = 0x4F; // Antic Mode F
        *p++ = (unsigned char)((addr + i * SCREEN_BYTES_PER_LINE) & 0xFF);
        *p++ = (unsigned char)(((addr + i * SCREEN_BYTES_PER_LINE) >> 8) & 0xFF);
    }

    // Wypełnienie reszty ekranu pustymi liniami
    for (; i < SCREEN_HEIGHT; ++i) {
        *p++ = 0x70; // Pusta linia
    }

    // Koniec Display List
    *p++ = 0x41;  // JVB
    *p++ = (unsigned char)((unsigned int)dl & 0xFF);
    *p++ = (unsigned char)(((unsigned int)dl >> 8) & 0xFF);

    return (char*)dl;
}

// Funkcja rysująca tło z przesunięciem w poziomie
void draw_background(unsigned char* buffer_memory, int offset) {
    int y;
    for (y = 0; y < BITMAP_HEIGHT; ++y) {
        const unsigned char* tile_ptr = background_tile + ((64 - offset + y) % 64) * 32; // 64 rows, each row 32 bytes
        mover((int)(buffer_memory + y * SCREEN_BYTES_PER_LINE), tile_ptr, SCREEN_BYTES_PER_LINE, 1, SCREEN_BYTES_PER_LINE);
    }
}

// Funkcja rysująca żabkę na ekranie z uwzględnieniem zerowych danych
void draw_frog(unsigned char* buffer_memory, const unsigned char* frame_data) {
    int y, x;
    for (y = 0; y < BITMAP_HEIGHT; ++y) {
        for (x = 0; x < BYTES_PER_LINE; ++x) {
            unsigned char pixel = frame_data[y * BYTES_PER_LINE + x];
            if (pixel != 0x00) {
                mover((int)(buffer_memory + y * SCREEN_BYTES_PER_LINE + (SCREEN_BYTES_PER_LINE / 2 - BYTES_PER_LINE / 2) + x), 
                      &pixel, 1, 1, SCREEN_BYTES_PER_LINE);
            }
        }
    }
}

// Funkcja kopiująca bufor do pamięci ekranu
void copy_buffer_to_screen(unsigned char* buffer_memory, unsigned char* screen_memory) {
    memcpy(screen_memory, buffer_memory, BUFFER_SIZE);
}

// Funkcja inicjalizująca PMG (Player/Missile Graphics)
void init_pmg() {
    // Włączenie PMG
    POKE(0xD00D, 0x03);  // GRACTL - Włączenie PMG dla graczy i pocisków
    POKE(0xD400, 0x22);  // DMACTL - Włączenie DMA dla PMG

    // Ustawienie bazowej pamięci PMG na 0xC000
    POKE(0xD407, 0xC0); // PMBASE ustawiony na 0xC000
    memset((void*)PMG_BASE, 0, 512); // Wyczyszczenie pamięci PMG

    // Skopiowanie danych sprite'a do pamięci PMG
    memcpy((void*)(PMG_BASE + 0x380), atari_sprite, SPRITE_HEIGHT);

    // Ustawienie koloru gracza 0 na różowy
    POKE(0xD012, 0xE0); // Kolor duszka (różowy)

    // Ustawienie początkowej pozycji sprite'a
    POKE(0xD000, SPRITE_POS_X); // Pozycja X gracza 0
    POKE(0xD001, SPRITE_POS_Y); // Pozycja Y gracza 0
}

// Główna funkcja
void main(void) {
    char* dl;
    unsigned char* screen_memory = (unsigned char*)0x4000;
    unsigned char buffer_memory[BUFFER_SIZE]; // Bufor do podwójnego buforowania
    int offset = 0;
    int sprite_x = SPRITE_POS_X, sprite_y = SPRITE_POS_Y;
    int sprite_dx = 1, sprite_dy = 1;

    // Inicjalizacja trybu graficznego
    _graphics(10);
    clrscr();

    // Ustawienia palety kolorów
    POKE(704, 15);  // Kolor tła (biały)
    POKE(708, 12);  // COLOR0
    POKE(706, 23);  // COLOR2
    POKE(707, 7);   // COLOR3
    POKE(711, 0);   // COLOR4
    POKE(712, 50);  // COLOR5

    // Tworzenie i aktywacja display list
    dl = create_display_list(screen_memory);
    activate_display_list(dl);

    // Inicjalizacja PMG (sprite'a)
    init_pmg();

    // Animacja żaby z przesuwającym się tłem i sprite'em
    while (!kbhit()) {
        wait_for_vblank();  // Synchronize with the VBlank
        draw_background(buffer_memory, offset);  // Rysowanie przesuniętego tła w buforze
        draw_frog(buffer_memory, frog1);         // Rysowanie żaby (1 klatka) w buforze
        copy_buffer_to_screen(buffer_memory, screen_memory);  // Kopiowanie bufora na ekran
        offset = (offset + 3) % 64;  // Zwiększanie offsetu, aby tło przesuwało się

        // Aktualizacja pozycji sprite'a
        sprite_x += sprite_dx;
        sprite_y += sprite_dy;

        // Zmiana kierunku, gdy napotka krawędzie ekranu
        if (sprite_x <= 0 || sprite_x >= 152) sprite_dx = -sprite_dx;
        if (sprite_y <= 0 || sprite_y >= 192) sprite_dy = -sprite_dy;

        // Ustawianie pozycji sprite'a (napis "ATARI")
        POKE(0xD000, sprite_x); // Pozycja X gracza 0
        POKE(0xD001, sprite_y); // Pozycja Y gracza 0

        wait_for_vblank();
        draw_background(buffer_memory, offset);
        draw_frog(buffer_memory, frog2);
        copy_buffer_to_screen(buffer_memory, screen_memory);
        offset = (offset + 3) % 64;
    }

    // Czekanie na naciśnięcie klawisza przed zakończeniem
    cgetc();
}
