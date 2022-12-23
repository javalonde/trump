
#include <stdio.h>
#include <stdlib.h>
#include <cc65.h>
#include <conio.h>
#include <ctype.h>
#include <modload.h>
#include <tgi.h>
#include <string.h>
#include <peekpoke.h>
#include <6502.h>

// This .cfg file tells CC65:
// * output a 16 KB cartridge
// * reserve $1E00 bytes for TGI frame buffer

//#resource "atari-tgi.cfg"
#define CFGFILE atari-tgi.cfg

// helpful type definitions
typedef unsigned char byte;
typedef signed char sbyte;
typedef unsigned short word;
typedef enum { false, true } bool;

// include Mode F bitmap data (40 * 192 bytes)
#include "bitmap.c"

// palette colors
const byte PALETTE[9] = {
  0xf0, 0x42, 0x84, 0x46,
  0xa6, 0x3a, 0xaa, 0x1e, 0x0e,
};

// probability (out of 65536) that snow will stick
#define STICK_PROB 1000

// screen dimensions
#define MAX_X 79
#define MAX_Y 191

// maximum # of snowflakes
#define MAX_FLAKES 100


///// MUSIC

const byte music1[] = {
//0x30,0x30,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x88,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x28,0x90,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x90,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x39,0x39,0x2d,0x91,0x39,0x39,0x2d,0x88,0x39,0x39,0x2d,0x89,0x37,0x37,0x2d,0x88,0x35,0x35,0x89,0x34,0x34,0x2d,0x91,0x34,0x34,0x2d,0x88,0x34,0x34,0x2d,0x89,0x32,0x32,0x2d,0x88,0x30,0x30,0x89,0x32,0x32,0x2d,0x91,0x32,0x32,0x2d,0x88,0x32,0x32,0x2d,0x88,0x34,0x34,0x2d,0x89,0x32,0x32,0x88,0x2d,0x2d,0x2d,0x91,0x2d,0x2d,0x2d,0x89,0x2d,0x2d,0x2d,0x88,0x2d,0x2d,0x2d,0x89,0x2d,0x2d,0x88,0x28,0x28,0x28,0x89,0x2a,0x2a,0x88,0x2c,0x2c,0x89,0x2d,0x2d,0x88,0x2f,0x2f,0x89,0x30,0x30,0x88,0x32,0x32,0x2a,0x89,0x34,0x34,0x88,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x28,0x28,0x28,0x88,0x2a,0x2a,0x89,0x2c,0x2c,0x88,0x2d,0x2d,0x89,0x2f,0x2f,0x88,0x30,0x30,0x89,0x32,0x32,0x2a,0x88,0x34,0x34,0x89,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x30,0x30,0x28,0x90,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x90,0x30,0x30,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x90,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x90,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x39,0x39,0x2d,0x91,0x39,0x39,0x2d,0x89,0x39,0x39,0x2d,0x88,0x37,0x37,0x2d,0x89,0x35,0x35,0x88,0x34,0x34,0x2d,0x91,0x34,0x34,0x2d,0x89,0x34,0x34,0x2d,0x88,0x32,0x32,0x2d,0x88,0x30,0x30,0x89,0x32,0x32,0x2d,0x91,0x32,0x32,0x2d,0x88,0x32,0x32,0x2d,0x89,0x34,0x34,0x2d,0x88,0x32,0x32,0x89,0x2d,0x2d,0x2d,0x91,0x2d,0x2d,0x2d,0x88,0x2d,0x2d,0x2d,0x89,0x2d,0x2d,0x2d,0x88,0x2d,0x2d,0x89,0x28,0x28,0x28,0x88,0x2a,0x2a,0x89,0x2c,0x2c,0x88,0x2d,0x2d,0x88,0x2f,0x2f,0x89,0x30,0x30,0x88,0x32,0x32,0x2a,0x89,0x34,0x34,0x88,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x28,0x28,0x28,0x89,0x2a,0x2a,0x88,0x2c,0x2c,0x89,0x2d,0x2d,0x88,0x2f,0x2f,0x89,0x30,0x30,0x88,0x32,0x32,0x2a,0x89,0x34,0x34,0x88,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x90,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x90,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x88,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x90,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x91,0x39,0x39,0x2d,0x91,0x39,0x39,0x2d,0x88,0x39,0x39,0x2d,0x89,0x37,0x37,0x2d,0x88,0x35,0x35,0x88,0x34,0x34,0x2d,0x91,0x34,0x34,0x2d,0x89,0x34,0x34,0x2d,0x88,0x32,0x32,0x2d,0x89,0x30,0x30,0x88,0x32,0x32,0x2d,0x91,0x32,0x32,0x2d,0x89,0x32,0x32,0x2d,0x88,0x34,0x34,0x2d,0x89,0x32,0x32,0x88,0x2d,0x2d,0x2d,0x91,0x2d,0x2d,0x2d,0x89,0x2d,0x2d,0x2d,0x88,0x2d,0x2d,0x2d,0x88,0x2d,0x2d,0x89,0x28,0x28,0x28,0x88,0x2a,0x2a,0x89,0x2c,0x2c,0x88,0x2d,0x2d,0x89,0x2f,0x2f,0x88,0x30,0x30,0x89,0x32,0x32,0x2a,0x88,0x34,0x34,0x89,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x28,0x28,0x28,0x88,0x2a,0x2a,0x89,0x2c,0x2c,0x88,0x2d,0x2d,0x88,0x2f,0x2f,0x89,0x30,0x30,0x88,0x32,0x32,0x2a,0x89,0x34,0x34,0x88,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x2d,0x2d,0x28,0xb2,0x2d,0x2d,0x28,0xb3,0x2d,0x2d,0x28,0xb3,0x34,0x34,0x24,0x91,0x32,0x32,0x23,0x88,0x34,0x34,0x24,0x89,0x2d,0x2d,0x21,0xff
//0x0d,0x0d,0x91,0x19,0x19,0x88,0x0d,0x0d,0x91,0x19,0x19,0x90,0x0d,0x0d,0x91,0x19,0x19,0x91,0x0d,0x0d,0x88,0x0d,0x0d,0x91,0x19,0x19,0x90,0x0d,0x0d,0x91,0x19,0x19,0x88,0x0d,0x0d,0x91,0x19,0x19,0x91,0x0d,0x0d,0x90,0x19,0x19,0x91,0x27,0x2f,0x90,0x2a,0x19,0x89,0x2a,0x2f,0x90,0x2e,0x19,0x91,0x25,0x2a,0x91,0x19,0x19,0x88,0x0d,0x0d,0x91,0x19,0x19,0x90,0x0d,0x0d,0x91,0x0d,0x2a,0x91,0x27,0x2f,0x88,0x27,0x2a,0x88,0x0d,0x0d,0x89,0x2e,0x25,0x90,0x25,0x2a,0x89,0x0d,0x0d,0x88,0x19,0x19,0x88,0x0d,0x0d,0x91,0x0d,0x19,0x91,0x0d,0x0d,0x90,0x27,0x19,0x91,0x27,0x2a,0x88,0x27,0x0d,0x91,0x18,0x27,0x90,0x2e,0x89,0x2e,0x88,0x2c,0x17,0x88,0x2c,0x91,0x16,0x25,0xa1,0x2a,0x24,0xa2,0x27,0x88,0x14,0x23,0x91,0x2a,0x88,0x27,0x14,0x88,0x2a,0x23,0x89,0x2e,0x90,0x29,0x19,0xa2,0x2f,0x27,0x90,0x2f,0x27,0x89,0x27,0x2a,0x90,0x19,0x25,0x91,0x2e,0x2a,0x90,0x19,0x19,0x89,0x0d,0x0d,0x90,0x19,0x19,0x91,0x0d,0x0d,0x91,0x0d,0x2f,0x90,0x27,0x2f,0x89,0x2f,0x2a,0x88,0x0d,0x0d,0x88,0x0d,0x2e,0x91,0x2e,0x2a,0x88,0x0d,0x0d,0x89,0x19,0x19,0x88,0x0d,0x0d,0x91,0x0d,0x19,0x90,0x0d,0x0d,0x91,0x19,0x27,0x91,0x27,0x2a,0x88,0x2a,0x27,0x91,0x27,0x2e,0x90,0x2e,0x88,0x2e,0x89,0x17,0x26,0x88,0x2c,0x91,0x16,0x2a,0xa1,0x24,0x2a,0xa1,0x27,0x89,0x14,0x23,0x90,0x2a,0x89,0x14,0x27,0x88,0x19,0x23,0x88,0x2e,0x91,0x2c,0x29,0xa1,0x19,0x19,0x89,0x19,0x19,0x90,0x12,0x12,0x91,0x2b,0x0f,0x99,0x16,0x16,0x88,0x16,0x16,0x88,0x2e,0x91,0x33,0x88,0x21,0x21,0x89,0x33,0x90,0x27,0x2a,0x91,0x21,0x21,0x99,0x0f,0x2b,0x91,0x2e,0x88,0x16,0x16,0x88,0x33,0x16,0x91,0x35,0x88,0x33,0x89,0x21,0x21,0x88,0x33,0x91,0x27,0x90,0x2a,0x99,0x2b,0x0f,0x91,0x2e,0x88,0x16,0x16,0x89,0x16,0x33,0x90,0x35,0x89,0x33,0x88,0x21,0x21,0x88,0x33,0x91,0x27,0x90,0x2a,0x99,0x23,0x14,0x91,0x27,0x17,0x91,0x1b,0x2a,0x90,0x2e,0x1e,0x91,0x19,0x25,0x99,0x27,0x2a,0x91,0x27,0x2a,0x88,0x2f,0x27,0x88,0x19,0x19,0x89,0x2a,0x25,0x90,0x25,0x2e,0x91,0x19,0x19,0x88,0x0d,0x0d,0x91,0x19,0x19,0x91,0x0d,0x0d,0x90,0x27,0x0d,0x91,0x2f,0x27,0x88,0x2a,0x2f,0x88,0x0d,0x0d,0x89,0x0d,0x2e,0x90,0x25,0x2e,0x89,0x0d,0x0d,0x88,0x19,0x19,0x88,0x0d,0x0d,0x91,0x0d,0x19,0x91,0x0d,0x0d,0x90,0x2f,0x0d,0x91,0x2f,0x27,0x88,0x2a,0x27,0x91,0x27,0x2e,0x91,0x2e,0x88,0x2e,0x88,0x26,0x17,0x89,0x2c,0x90,0x25,0x2a,0xa2,0x24,0x2a,0xb4,0x2e,0x27,0x8b,0x27,0x18,0x8b,0x17,0x2c,0x89,0x26,0x2c,0x90,0x2a,0x25,0xa2,0x15,0x24,0xa9,0x27,0x2e,0x91,0x27,0x2e,0x91,0x2c,0x17,0x88,0x26,0x17,0x91,0x25,0x2a,0xa1,0x15,0x2a,0xa1,0x27,0x88,0x23,0x14,0x91,0x2a,0x88,0x14,0x27,0x89,0x2a,0x19,0x88,0x2f,0x91,0x19,0x25,0x90,0x19,0x19,0x91,0x0d,0x0d,0x91,0x19,0x19,0x90,0x0d,0x0d,0x91,0x19,0x19,0x91,0x2a,0x23,0x90,0x20,0x0d,0x89,0x20,0x2a,0x88,0x20,0x2a,0x88,0x22,0x2c,0x91,0x25,0x27,0x88,0x0d,0x0d,0x91,0x19,0x19,0x91,0x0d,0x0d,0x88,0x22,0x88,0x19,0x19,0x91,0x2a,0x0d,0x90,0x2a,0x23,0x89,0x20,0x23,0x88,0x0d,0x0d,0x88,0x22,0x25,0x91,0x29,0x27,0x88,0x0d,0x0d,0x91,0x19,0x19,0x91,0x0d,0x0d,0x90,0x19,0x19,0x91,0x2a,0x20,0x91,0x0d,0x23,0x88,0x2a,0x20,0x88,0x23,0x27,0x89,0x2c,0x0d,0x90,0x19,0x19,0x89,0x0d,0x0d,0x90,0x19,0x19,0x91,0x0d,0x0d,0x91,0x19,0x19,0x90,0x0d,0x0d,0x91,0x19,0x19,0x88,0x0d,0x0d,0x91,0x0f,0x2b,0x99,0x16,0x16,0x88,0x16,0x16,0x88,0x2e,0x91,0x33,0x88,0x21,0x21,0x89,0x33,0x90,0x27,0x2a,0x91,0x21,0x21,0x99,0x0f,0x2b,0x91,0x2e,0x88,0x16,0x16,0x88,0x16,0x33,0x91,0x35,0x88,0x33,0x89,0x21,0x21,0x88,0x33,0x91,0x27,0x90,0x2a,0x99,0x0f,0x2b,0x91,0x2e,0x88,0x16,0x16,0x88,0x33,0x16,0x91,0x35,0x88,0x33,0x89,0x21,0x21,0x88,0x33,0x91,0x27,0x90,0x2a,0x99,0x23,0x14,0x91,0x27,0x17,0x91,0x2a,0x1b,0x90,0x2e,0x1e,0x91,0x25,0x31,0x99,0x2f,0x27,0x91,0x27,0x2a,0x88,0x2a,0x27,0x88,0x19,0x19,0x89,0x2e,0x25,0x90,0x2a,0x25,0x91,0x19,0x19,0x88,0x0d,0x0d,0x91,0x19,0x19,0x90,0x0d,0x0d,0x91,0x0d,0x27,0x91,0x27,0x2a,0x88,0x27,0x2f,0x88,0x0d,0x0d,0x89,0x0d,0x2e,0x90,0x25,0x2a,0x89,0x0d,0x0d,0x88,0x19,0x19,0x88,0x0d,0x0d,0x91,0x0d,0x19,0x91,0x0d,0x0d,0x90,0x19,0x0d,0x91,0x2f,0x27,0x88,0x27,0x2a,0x91,0x27,0x2e,0x91,0x2e,0x88,0x2e,0x88,0x26,0x17,0x88,0x2c,0x91,0x25,0x2a,0xa1,0x2a,0x24,0xb5,0x27,0x18,0x8b,0x27,0x18,0x8b,0x2c,0x26,0x89,0x26,0x17,0x90,0x16,0x25,0xa2,0x24,0x15,0xa9,0x2e,0x27,0x91,0x2e,0x27,0x91,0x26,0x17,0x88,0x17,0x26,0x90,0x2a,0x16,0xa2,0x15,0x24,0xa1,0x27,0x88,0x2a,0x23,0x91,0x2a,0x88,0x27,0x14,0x89,0x19,0x2a,0x88,0x2f,0x91,0x0d,0x27,0x90,0x19,0x19,0x91,0x0d,0x0d,0x91,0x19,0x19,0x90,0x0d,0x0d,0x91,0x19,0x19,0x91,0x27,0x20,0x90,0x20,0x23,0x89,0x27,0x19,0x88,0x0d,0x20,0x88,0x2a,0x25,0x91,0x19,0x19,0x88,0x31,0x0d,0x84,0x2d,0x84,0x2e,0x89,0x19,0x31,0x88,0x31,0x88,0x0d,0x2e,0x89,0x33,0x88,0x19,0x19,0x88,0x31,0x89,0x0d,0x0d,0x88,0x27,0x23,0x91,0x2a,0x27,0x88,0x19,0x19,0x88,0x2a,0x0d,0x89,0x19,0x19,0x90,0x2c,0x0d,0x84,0x2d,0x85,0x2e,0x88,0x31,0x19,0x88,0x31,0x89,0x0d,0x2e,0x88,0x31,0x88,0x19,0x19,0x89,0x2e,0x88,0x0d,0x0d,0x88,0x23,0x0d,0x89,0x2a,0x0d,0x90,0x2a,0x25,0x89,0x0d,0x0d,0x88,0x19,0x19,0x91,0x31,0x0d,0x84,0x2d,0x84,0x2e,0x88,0x31,0x19,0x88,0x31,0x89,0x0d,0x2e,0x88,0x2e,0x33,0x88,0x19,0x19,0x89,0x31,0x88,0x0d,0x0d,0x88,0x27,0x23,0x89,0x2a,0x27,0x90,0x19,0x2a,0xa2,0x31,0x88,0x0d,0x0d,0x88,0x19,0x19,0x89,0x31,0x88,0x0d,0x0d,0x88,0x0d,0x2c,0x82,0x2d,0x97,0x19,0x19,0x89,0x27,0x23,0x88,0x27,0x0d,0x91,0x2a,0x0d,0x99,0x2f,0x34,0x88,0x31,0x19,0x88,0x34,0x89,0x2f,0x0d,0x88,0x31,0x88,0x34,0x19,0x88,0x2f,0x89,0x0d,0x31,0x88,0x34,0x88,0x2f,0x19,0x81,0x31,0x90,0x2a,0x27,0x88,0x19,0x23,0x89,0x23,0x19,0x88,0x0d,0x25,0x99,0x34,0x19,0x88,0x31,0x89,0x34,0x0d,0x88,0x2f,0x88,0x19,0x31,0x89,0x34,0x88,0x2f,0x0d,0x88,0x33,0x91,0x2a,0x19,0x88,0x19,0x27,0x89,0x19,0x2a,0x90,0x22,0x0d,0x89,0x19,0x19,0x90,0x34,0x2f,0x89,0x31,0x88,0x34,0x19,0x88,0x2f,0x89,0x31,0x0d,0x88,0x34,0x88,0x2f,0x19,0x88,0x31,0x89,0x0d,0x0d,0x88,0x2a,0x27,0x88,0x27,0x2a,0x91,0x25,0x0d,0xa1,0x2f,0x34,0x89,0x31,0x88,0x34,0x19,0x88,0x2f,0x89,0x31,0x0d,0x88,0x34,0x0d,0x88,0x19,0x2f,0x89,0x31,0x88,0x0d,0x0d,0x88,0x27,0x23,0x89,0x23,0x2a,0x90,0x25,0x22,0x91,0x19,0x19,0x91,0x0d,0x0d,0x88,0x0d,0x27,0x88,0x19,0x19,0x89,0x23,0x2a,0x88,0x19,0x19,0x88,0x2a,0x22,0x89,0x19,0x19,0x90,0x0d,0x23,0x88,0x27,0x23,0x89,0x19,0x27,0x88,0x0d,0x0d,0x88,0x19,0x25,0x89,0x0d,0x0d,0x88,0x19,0x19,0x91,0x27,0x23,0x88,0x0d,0x2a,0x88,0x19,0x19,0x91,0x19,0x27,0x88,0x22,0x2a,0x89,0x19,0x19,0x90,0x23,0x2a,0x89,0x2a,0x27,0x88,0x23,0x19,0x88,0x0d,0x0d,0x89,0x22,0x19,0x88,0x0d,0x0d,0x88,0x19,0x19,0x91,0x23,0x0d,0x88,0x27,0x23,0x89,0x19,0x19,0x88,0x23,0x0d,0x88,0x19,0x19,0x89,0x22,0x25,0x88,0x19,0x19,0x91,0x2a,0x27,0x88,0x2a,0x27,0x88,0x2a,0x19,0x88,0x0d,0x0d,0x89,0x19,0x25,0x88,0x0d,0x0d,0x88,0x19,0x19,0x91,0x27,0x23,0x88,0x27,0x2a,0x89,0x19,0x19,0x88,0x0d,0x23,0x88,0x19,0x19,0x89,0x25,0x22,0x99,0x2a,0x23,0x90,0x0d,0x0d,0x99,0x2c,0x25,0xa1,0x2c,0x19,0xff
0x33,0x99,0x33,0x24,0x8c,0x35,0x8c,0x33,0x24,0x98,0x30,0x24,0x97,0x2c,0x99,0x30,0x24,0x8c,0x31,0x8c,0x33,0x24,0x98,0x24,0x27,0x97,0x35,0x99,0x35,0x25,0x98,0x33,0x8c,0x31,0x8c,0x30,0x24,0x97,0x2e,0x8d,0x31,0x8c,0x33,0x25,0x98,0x33,0x25,0x98,0x25,0x27,0x97,0x33,0x8d,0x27,0x8c,0x33,0x2c,0x8c,0x35,0x30,0x8c,0x33,0x2c,0x8c,0x30,0x8c,0x30,0x2c,0x8b,0x27,0x8c,0x2c,0x8d,0x27,0x8c,0x30,0x2c,0x8c,0x2e,0x30,0x8c,0x33,0x2c,0x8c,0x30,0x8c,0x2c,0x8b,0x27,0x8c,0x35,0x8d,0x25,0x8c,0x31,0x29,0x8c,0x33,0x2c,0x8c,0x35,0x27,0x8c,0x33,0x2c,0x8c,0x30,0x27,0x8b,0x24,0x8c,0x2e,0x8d,0x25,0x8c,0x33,0x27,0x8c,0x2b,0x8c,0x2c,0x24,0x8c,0x30,0x86,0x2e,0x86,0x2c,0x8b,0x27,0x8c,0x31,0x29,0x99,0x31,0x29,0x98,0x35,0x29,0xaf,0x33,0x27,0x99,0x33,0x27,0x8c,0x33,0x8c,0x30,0x27,0xaf,0x2e,0x25,0x99,0x2e,0x22,0x98,0x31,0x22,0x94,0x27,0x83,0x2b,0x82,0x2e,0x83,0x31,0x93,0x30,0x27,0x99,0x33,0x27,0x8c,0x33,0x88,0x27,0x83,0x2c,0x81,0x2c,0x82,0x30,0x82,0x33,0xab,0x33,0x8d,0x27,0x2c,0x8c,0x33,0x27,0x8c,0x35,0x27,0x8c,0x33,0x27,0x8c,0x27,0x2c,0x8c,0x30,0x24,0x8b,0x24,0x2a,0x8c,0x31,0x8d,0x20,0x25,0x8c,0x33,0x20,0x8c,0x20,0x25,0x8c,0x35,0x25,0x8c,0x25,0x2b,0x8c,0x29,0x2c,0x8b,0x27,0x2e,0x8c,0x33,0x8d,0x27,0x2c,0x8c,0x33,0x27,0x8c,0x35,0x29,0x8c,0x33,0x27,0x8c,0x33,0x27,0x8c,0x30,0x27,0x8b,0x27,0x2c,0x8c,0x2e,0x8d,0x25,0x2b,0x8c,0x33,0x25,0x8c,0x25,0x2b,0x8c,0x2c,0x24,0xaf,0x33,0x99,0x33,0x24,0x8c,0x35,0x8c,0x33,0x24,0x98,0x30,0x24,0x97,0x2c,0x99,0x30,0x24,0x8c,0x31,0x8c,0x33,0x24,0x98,0x24,0x27,0x97,0x35,0x99,0x35,0x25,0x98,0x33,0x8c,0x31,0x8c,0x30,0x24,0x97,0x2e,0x8d,0x31,0x8c,0x33,0x25,0x98,0x33,0x25,0x98,0x25,0x27,0x97,0x33,0x8d,0x27,0x8c,0x33,0x2c,0x8c,0x35,0x30,0x8c,0x33,0x2c,0x8c,0x30,0x8c,0x30,0x2c,0x8b,0x27,0x8c,0x2c,0x8d,0x27,0x8c,0x30,0x2c,0x8c,0x2e,0x30,0x8c,0x33,0x2c,0x8c,0x30,0x8c,0x2c,0x8b,0x27,0x8c,0x35,0x8d,0x25,0x8c,0x31,0x29,0x8c,0x33,0x2c,0x8c,0x35,0x27,0x8c,0x33,0x2c,0x8c,0x30,0x27,0x8b,0x24,0x8c,0x2e,0x8d,0x25,0x8c,0x33,0x27,0x8c,0x2b,0x8c,0x2c,0x24,0x8c,0x30,0x86,0x2e,0x86,0x2c,0x8b,0x27,0x8c,0x31,0x29,0x99,0x31,0x29,0x98,0x35,0x29,0xaf,0x33,0x27,0x99,0x33,0x27,0x8c,0x33,0x8c,0x30,0x27,0xaf,0x2e,0x25,0x99,0x2e,0x22,0x98,0x31,0x22,0x94,0x27,0x83,0x2b,0x82,0x2e,0x83,0x31,0x93,0x30,0x27,0x99,0x33,0x27,0x8c,0x33,0x88,0x27,0x83,0x2c,0x81,0x2c,0x82,0x30,0x82,0x33,0xab,0x33,0x8d,0x27,0x2c,0x8c,0x33,0x27,0x8c,0x35,0x27,0x8c,0x33,0x27,0x8c,0x27,0x2c,0x8c,0x30,0x24,0x8b,0x24,0x2a,0x8c,0x31,0x8d,0x20,0x25,0x8c,0x33,0x20,0x8c,0x20,0x25,0x8c,0x35,0x25,0x8c,0x25,0x2b,0x8c,0x29,0x2c,0x8b,0x27,0x2e,0x8c,0x33,0x8d,0x27,0x2c,0x8c,0x33,0x27,0x8c,0x35,0x29,0x8c,0x33,0x27,0x8c,0x33,0x27,0x8c,0x30,0x27,0x8b,0x27,0x2c,0x8c,0x2e,0x8d,0x25,0x2b,0x8c,0x33,0x25,0x8c,0x25,0x2b,0x8c,0x2c,0x24,0xaf,0x33,0x99,0x33,0x24,0x8c,0x35,0x8c,0x33,0x24,0x98,0x30,0x24,0x97,0x2c,0x99,0x30,0x24,0x8c,0x31,0x8c,0x33,0x24,0x98,0x24,0x27,0x97,0x35,0x99,0x35,0x25,0x98,0x33,0x8c,0x31,0x8c,0x30,0x24,0x97,0x2e,0x8d,0x31,0x8c,0x33,0x25,0x98,0x33,0x25,0x98,0x25,0x27,0x97,0x33,0x8d,0x27,0x8c,0x33,0x2c,0x8c,0x35,0x30,0x8c,0x33,0x2c,0x8c,0x30,0x8c,0x30,0x2c,0x8b,0x27,0x8c,0x2c,0x8d,0x27,0x8c,0x30,0x2c,0x8c,0x2e,0x30,0x8c,0x33,0x2c,0x8c,0x30,0x8c,0x2c,0x8b,0x27,0x8c,0x35,0x8d,0x25,0x8c,0x31,0x29,0x8c,0x33,0x2c,0x8c,0x35,0x27,0x8c,0x33,0x2c,0x8c,0x30,0x27,0x8b,0x24,0x8c,0x2e,0x8d,0x25,0x8c,0x33,0x27,0x8c,0x2b,0x8c,0x2c,0x24,0x8c,0x30,0x86,0x2e,0x86,0x2c,0x8b,0x27,0x8c,0x31,0x29,0x99,0x31,0x29,0x98,0x35,0x29,0xaf,0x33,0x27,0x99,0x33,0x27,0x8c,0x33,0x8c,0x30,0x27,0xaf,0x2e,0x25,0x99,0x2e,0x22,0x98,0x31,0x22,0x94,0x27,0x83,0x2b,0x82,0x2e,0x83,0x31,0x93,0x30,0x27,0x99,0x33,0x27,0x8c,0x33,0x88,0x27,0x83,0x2c,0x81,0x2c,0x82,0x30,0x82,0x33,0xab,0x33,0x8d,0x27,0x2c,0x8c,0x33,0x27,0x8c,0x35,0x27,0x8c,0x33,0x27,0x8c,0x27,0x2c,0x8c,0x30,0x24,0x8b,0x24,0x2a,0x8c,0x31,0x8d,0x20,0x25,0x8c,0x33,0x20,0x8c,0x20,0x25,0x8c,0x35,0x25,0x8c,0x25,0x2b,0x8c,0x29,0x2c,0x8b,0x27,0x2e,0x8c,0x33,0x8d,0x27,0x2c,0x8c,0x33,0x27,0x8c,0x35,0x29,0x8c,0x33,0x27,0x8c,0x33,0x27,0x8c,0x30,0x27,0x8b,0x27,0x2c,0x8c,0x2e,0x8d,0x25,0x2b,0x8c,0x33,0x25,0x8c,0x25,0x2b,0x8c,0x2c,0x24,0xff
};

//#link "music.c"
extern const byte* music_ptr;
void start_music(const byte* music);
void play_music();

unsigned char music_update() {
  if (!music_ptr) start_music(music1);
  play_music();
  return IRQ_NOT_HANDLED;
}

/////

/* Driver stuff */
static unsigned MaxX;
static unsigned MaxY;
static unsigned AspectRatio;

static void CheckError (const char* S)
{
    unsigned char Error = tgi_geterror ();
    if (Error != TGI_ERR_OK) {
        printf ("%s: %d\n", S, Error);
        if (doesclrscrafterexit ()) {
            cgetc ();
        }
        exit (EXIT_FAILURE);
    }
}


///// SNOWFLAKES

// snowflake array
word flakes[MAX_FLAKES];

// hard-coded frame buffer (TODO: get from TGI)
byte* fb = (byte*) 0x6150;

// clear the pixel
void clearflake(word pos) {
  if (pos & 1)
    fb[pos>>1] &= ~0x0f;
  else
    fb[pos>>1] &= ~0xf0;
}

// xor a snowflake pixel
void drawflake(word pos) {
  if (pos & 1)
    fb[pos>>1] ^= 0x08;
  else
    fb[pos>>1] ^= 0x80;
}

// read the pixel color at a screen position
// return true if it is snow-colored
bool readflake(word pos) {
  if (pos & 1)
    return (fb[pos>>1] & 0x0f) == 0x08;
  else
    return (fb[pos>>1] & 0xf0) == 0x80;
//  return fb[i] & READMASK[pos&1];
}

// animate snow one pixel downward
bool animate(bool created) {
  byte i;
  for (i=0; i<MAX_FLAKES; i++) {
    // get old position in array
    word oldpos = flakes[i];
    // is there a valid snowflake?
    if (oldpos) {
      byte bg;
      // move down one pixel
      word newpos = oldpos + (MAX_X + 1);
      // read pixel at new pos
      bg = readflake(newpos);
      if (bg) {
        // cold mode, slide pixel left or right to empty space
        if (!readflake(newpos+1) && rand() > STICK_PROB) {
          newpos++;
        } else if (!readflake(newpos-1) && rand() > STICK_PROB) {
          newpos--;
        } else {
          newpos = 0;	// get rid of snowflake
        }
      }
      // if we didn't get rid of it, erase and redraw
      if (newpos) {
        drawflake(newpos);
        drawflake(oldpos);
      } else {
        clearflake(oldpos);
        drawflake(oldpos);
      }
      // set new position in array
      flakes[i] = newpos;
    }
    // no valid snowflake in this slot, create one?
    else if (!created) {
      // create a new random snowfake at top (only once per loop)
      oldpos = 1 + rand() % (MAX_X-1);
      // make sure snowdrift didn't pile to top of screen
      if (readflake(oldpos) && readflake(oldpos+(MAX_X+1))) {
        // screen is full, exit
        return false;
      } else {
        // create new snowflake, draw it
        flakes[i] = oldpos;
        created = 1;
        drawflake(oldpos);
      }
    }
  }
  return true; // true if screen isn't full
}

// main snowflake routine
bool DoSnow(void) {

  // draw bitmap
  tgi_clear();
  tgi_setpalette(PALETTE);
  memcpy(fb, BITMAP, 40*192);
  // draw line on bottom
  memset(fb+40*191, 0x88, 40); // color of snow
  
  // animate snowflakes until screen is full
  {
    byte i;
    // initialize snowflake array  
    memset(flakes, 0, sizeof(flakes));
    // animate snowflakes until they pile up too high
    while (animate(false)) {
      // press ESC to exit
      if (kbhit() && cgetc() == 0x1b) return false;
    }
    // finish falling snowflakes, don't create any
    for (i=0; i<200; i++) animate(true);
  }
  return true;
}

////////////

int main (void)
{
    /* Install the driver */
    tgi_install (atr10_tgi); // 80 x 192, 9 colors
    CheckError ("tgi_install");

    tgi_init ();
    CheckError ("tgi_init");
    tgi_clear ();

    /* Get stuff from the driver */
    MaxX = tgi_getmaxx ();
    MaxY = tgi_getmaxy ();
    AspectRatio = tgi_getaspectratio ();
  
    set_irq(music_update, (void*)0x7000, 0x100);

    while (DoSnow()) {
    }

    /* Uninstall the driver */
    tgi_uninstall ();

    /* Done */
    printf ("Done\n");
    return EXIT_SUCCESS;
}
