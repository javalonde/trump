
/*
NOTE: The alternate Apple ][ ROM used by 8bitworkshop
      is not compatible with the hi-res TGI driver.
      So we use the lo-res driver here.
*/
#include <stdio.h>
#include <stdlib.h>
#include <cc65.h>
#include <conio.h>
#include <ctype.h>
#include <modload.h>
#include <tgi.h>
#include <string.h>
#include <peekpoke.h>

//#resource "atari-tgi.cfg"
#define CFGFILE atari-tgi.cfg

typedef unsigned char byte;
typedef signed char sbyte;
typedef unsigned short word;
typedef enum { false, true } bool;

// include mode $F bitmap (40 * 192 bytes)
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


///// RANDOM NUMBERS

unsigned long rnd = 1;
word rand16(void) {
  unsigned long x = rnd;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return rnd = x;
}

///// MUSIC

const byte music1[] = {
0xbf,0xbf,0x30,0x30,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x88,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x28,0x90,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x90,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x39,0x39,0x2d,0x91,0x39,0x39,0x2d,0x88,0x39,0x39,0x2d,0x89,0x37,0x37,0x2d,0x88,0x35,0x35,0x89,0x34,0x34,0x2d,0x91,0x34,0x34,0x2d,0x88,0x34,0x34,0x2d,0x89,0x32,0x32,0x2d,0x88,0x30,0x30,0x89,0x32,0x32,0x2d,0x91,0x32,0x32,0x2d,0x88,0x32,0x32,0x2d,0x88,0x34,0x34,0x2d,0x89,0x32,0x32,0x88,0x2d,0x2d,0x2d,0x91,0x2d,0x2d,0x2d,0x89,0x2d,0x2d,0x2d,0x88,0x2d,0x2d,0x2d,0x89,0x2d,0x2d,0x88,0x28,0x28,0x28,0x89,0x2a,0x2a,0x88,0x2c,0x2c,0x89,0x2d,0x2d,0x88,0x2f,0x2f,0x89,0x30,0x30,0x88,0x32,0x32,0x2a,0x89,0x34,0x34,0x88,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x28,0x28,0x28,0x88,0x2a,0x2a,0x89,0x2c,0x2c,0x88,0x2d,0x2d,0x89,0x2f,0x2f,0x88,0x30,0x30,0x89,0x32,0x32,0x2a,0x88,0x34,0x34,0x89,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x30,0x30,0x28,0x90,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x90,0x30,0x30,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x90,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x90,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x39,0x39,0x2d,0x91,0x39,0x39,0x2d,0x89,0x39,0x39,0x2d,0x88,0x37,0x37,0x2d,0x89,0x35,0x35,0x88,0x34,0x34,0x2d,0x91,0x34,0x34,0x2d,0x89,0x34,0x34,0x2d,0x88,0x32,0x32,0x2d,0x88,0x30,0x30,0x89,0x32,0x32,0x2d,0x91,0x32,0x32,0x2d,0x88,0x32,0x32,0x2d,0x89,0x34,0x34,0x2d,0x88,0x32,0x32,0x89,0x2d,0x2d,0x2d,0x91,0x2d,0x2d,0x2d,0x88,0x2d,0x2d,0x2d,0x89,0x2d,0x2d,0x2d,0x88,0x2d,0x2d,0x89,0x28,0x28,0x28,0x88,0x2a,0x2a,0x89,0x2c,0x2c,0x88,0x2d,0x2d,0x88,0x2f,0x2f,0x89,0x30,0x30,0x88,0x32,0x32,0x2a,0x89,0x34,0x34,0x88,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x28,0x28,0x28,0x89,0x2a,0x2a,0x88,0x2c,0x2c,0x89,0x2d,0x2d,0x88,0x2f,0x2f,0x89,0x30,0x30,0x88,0x32,0x32,0x2a,0x89,0x34,0x34,0x88,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x90,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x90,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x88,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x90,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x91,0x39,0x39,0x2d,0x91,0x39,0x39,0x2d,0x88,0x39,0x39,0x2d,0x89,0x37,0x37,0x2d,0x88,0x35,0x35,0x88,0x34,0x34,0x2d,0x91,0x34,0x34,0x2d,0x89,0x34,0x34,0x2d,0x88,0x32,0x32,0x2d,0x89,0x30,0x30,0x88,0x32,0x32,0x2d,0x91,0x32,0x32,0x2d,0x89,0x32,0x32,0x2d,0x88,0x34,0x34,0x2d,0x89,0x32,0x32,0x88,0x2d,0x2d,0x2d,0x91,0x2d,0x2d,0x2d,0x89,0x2d,0x2d,0x2d,0x88,0x2d,0x2d,0x2d,0x88,0x2d,0x2d,0x89,0x28,0x28,0x28,0x88,0x2a,0x2a,0x89,0x2c,0x2c,0x88,0x2d,0x2d,0x89,0x2f,0x2f,0x88,0x30,0x30,0x89,0x32,0x32,0x2a,0x88,0x34,0x34,0x89,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x28,0x28,0x28,0x88,0x2a,0x2a,0x89,0x2c,0x2c,0x88,0x2d,0x2d,0x88,0x2f,0x2f,0x89,0x30,0x30,0x88,0x32,0x32,0x2a,0x89,0x34,0x34,0x88,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x2d,0x2d,0x28,0xb2,0x2d,0x2d,0x28,0xb3,0x2d,0x2d,0x28,0xb3,0x34,0x34,0x24,0x91,0x32,0x32,0x23,0x88,0x34,0x34,0x24,0x89,0x2d,0x2d,0x21,0xff
};

//#link "music.c"
extern const byte* music_ptr;
void start_music(const byte* music);
void play_music();

void music_update() {
    if (!music_ptr) start_music(music1);
    play_music();
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
        if (!readflake(newpos+1) && rand16() > STICK_PROB) {
          newpos++;
        } else if (!readflake(newpos-1) && rand16() > STICK_PROB) {
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
      oldpos = 1 + rand16() % (MAX_X-1);
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
      // update music
      music_update();
      music_update();
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
    tgi_install (atr10_tgi);
    CheckError ("tgi_install");

    tgi_init ();
    CheckError ("tgi_init");
    tgi_clear ();

    /* Get stuff from the driver */
    MaxX = tgi_getmaxx ();
    MaxY = tgi_getmaxy ();
    AspectRatio = tgi_getaspectratio ();

    while (DoSnow()) {
    }

    /* Uninstall the driver */
    tgi_uninstall ();

    /* Done */
    printf ("Done\n");
    return EXIT_SUCCESS;
}
