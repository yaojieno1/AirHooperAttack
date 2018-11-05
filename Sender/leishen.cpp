/*
  tempest for Leishen code
  Copyright (C) 2001  Erik Thiele
  Copyright (C) 2018  ITB Yaojie

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <math.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <SDL.h>
#include <string.h>

double carrier;
int resx;
int resy;
int horizontalspan;
double pixelclock;

SDL_Surface *screen;
Uint32 white, black;

class zeit : public timeval {
public:
  zeit() { setzero(); };
  void getcurrenttime() { gettimeofday (this,0); };
  void subtract (const zeit &subme)
    {
      tv_sec-=subme.tv_sec;
      tv_usec-=subme.tv_usec;
      normalize(); 
    };
  void add (const zeit &addme)
    {
      tv_sec+=addme.tv_sec;
      tv_usec+=addme.tv_usec;
      normalize(); 
    };
  void normalize()
    {
      while (tv_usec<0) {
	tv_usec+=1000000;
	tv_sec--;
      };
      while (tv_usec>=1000000) {
	tv_usec-=1000000;
	tv_sec++;
      };
    };
  bool zero()
    {
      return (tv_sec==0)&&(tv_usec==0);
    };
  void setzero()
    {
      tv_sec=tv_usec=0;
    };
  void print(const char * prefix)
    {
      printf("%s():%8d:%8d\n",prefix,int(tv_sec),int(tv_usec));
    };
  void setdouble(const double val)
    {
      long long i=(long long)(val*1000000);
      tv_usec=i%1000000;
      tv_sec=i/1000000;
      normalize();
    };
};

inline void pixelchen (const int x, const int y, const Uint32 pixel)
{
  Uint8 *bits=((Uint8*)screen->pixels)+y*screen->pitch+x*screen->format->BytesPerPixel;
  switch(screen->format->BytesPerPixel) {
    case 1:
      *((Uint8 *)(bits)) = (Uint8)pixel;
      break;
    case 2:
      *((Uint16 *)(bits)) = (Uint16)pixel;
      break;
    case 3:
      { /* Format/endian independent */
        Uint8 r, g, b;
      
        r = (pixel>>screen->format->Rshift)&0xFF;
        g = (pixel>>screen->format->Gshift)&0xFF;
        b = (pixel>>screen->format->Bshift)&0xFF;
        *((bits)+screen->format->Rshift/8) = r; 
        *((bits)+screen->format->Gshift/8) = g;
        *((bits)+screen->format->Bshift/8) = b;
      }
      break;
    case 4:
      *((Uint32 *)(bits)) = (Uint32)pixel;
      break;
  };
};

void waituntil (const zeit &desttime)
{
  zeit tm(desttime);
  zeit cur;
  cur.getcurrenttime();
  tm.subtract(cur);
  //tm.print("waituntil");

  select(1,0,0,0,&tm);
};

struct DTMF {
  double low;
  double high;
};

void mkrealsound(struct DTMF freq)
{
  SDL_Rect myrect;
  myrect.x=myrect.y=0;
  myrect.w=resx;
  myrect.h=resy;
  SDL_FillRect(screen,&myrect,black);

  //printf("mkrealsound(): freq=%f, %f\n", freq.low, freq.high);

  int x, y;
  double t;
  
  double lk = 2 * freq.low / pixelclock;
  double hk = 2 * freq.high / pixelclock;

  double k2 = 2 * carrier / pixelclock;
  t = 0;
  for (y = 0; y < resy; y ++) {
    if (y < resy / 2) {
      if (int(t * lk) % 2) {
        for (x = 0; x < resx; x ++) {
          if (int(t * k2) % 2) {
            pixelchen(x,y,white);
          }
          t ++;
        }
      } else {
        t = t + resx;
      }
    } else {
      if (int(t * hk) % 2) {
        for (x = 0; x < resx; x ++) {
          if (int(t * k2) % 2) {
            pixelchen(x,y,white);
          }
          t ++;
        }
      } else {
        t = t + resx;
      }
    }
  }
}

void mkrealsound(int nr) {
  SDL_Rect myrect;
  myrect.x=myrect.y=0;
  myrect.w=resx;
  myrect.h=resy;
  SDL_FillRect(screen,&myrect,black);

  int x, y;
  double t;
  double k1, k2;
  printf(" %d ", nr);
  
  if (nr == 0) {
    k1 = 697.0f;
  } else {
    k1 = 1633.0f;
  }

  k1 = 2 * k1 / pixelclock;
  k2 = 2 * carrier / pixelclock;
  t = 0;
  for (y = 0; y < resy; y ++) {
    if (int(t * k1) % 2) {
      for (x = 0; x < resx; x ++) {
        if (int(t * k2) % 2) {
          pixelchen(x,y,white);
        }
        t ++;
      }
    } else {
      t = t + resx;
    }
  }
}

/*
DTMF keypad frequencies (with sound clips) 	
        1209 Hz 	1336 Hz 	1477 Hz 	1633 Hz
697 Hz 	  1       	2       	3         A
770 Hz 	  4        	5         6         B
852 Hz   	7        	8       	9         C
941 Hz  	*       	0         #         D
*/


struct DTMF note(int nr)
{
  struct DTMF dtmf = {0.0f, 0.0f};
  switch (nr) {
    case 0:
      dtmf.low  = 697.0f;
      dtmf.high = 1209.0f;
      break;
    case 1:
      dtmf.low = 697.0f;
      dtmf.high = 1336.0f;
      break;
    case 2:
      dtmf.low = 697.0f;
      dtmf.high = 1477.0f;
      break;
    case 3:
      dtmf.low = 697.0f;
      dtmf.high = 1633.0f;
      break;
    case 4:
      dtmf.low = 770.0f;
      dtmf.high = 1209.0f;
      break;
    case 5:
      dtmf.low = 770.0f;
      dtmf.high = 1336.0f;
      break;
    case 6:
      dtmf.low = 770.0f;
      dtmf.high = 1477.0f;
      break;
    case 7:
      dtmf.low = 770.0f;
      dtmf.high = 1633.0f;
      break;
    case 8:
      dtmf.low = 852.0f;
      dtmf.high = 1209.0f;
      break;
    case 9:
      dtmf.low = 852.0f;
      dtmf.high = 1336.0f;
      break;
    case 10:
      dtmf.low = 852.0f;
      dtmf.high = 1477.0f;
      break;
    case 11:
      dtmf.low = 852.0f;
      dtmf.high = 1633.0f;
      break;
    case 12:
      dtmf.low = 941.0f;
      dtmf.high = 1209.0f;
      break;
    case 13:
      dtmf.low = 941.0f;
      dtmf.high = 1336.0f;
      break;
    case 14:
      dtmf.low = 941.0f;
      dtmf.high = 1477.0f;
      break;
    case 15:
      dtmf.low = 941.0f;
      dtmf.high = 1633.0f;
      break;
    default:
      printf("NOTE(): unknown nr %d!!!", nr);
      dtmf.low = 941.0f;
      dtmf.high = 1633.0f;
      break;
  }
  return dtmf;
}

/*
double note (int nr)
{
  double n = pow(2.0,nr/12.0)*440.0*2;
  printf("note(): nr=%d, n=%f\n", nr, n);
  return n;
};
*/

//int curoct, curnot;
double curlen=10.0f;
//bool loaded;
zeit timefornextsound;

void resetplay()
{
  //curoct=0;
  //curnot=0;
  //curlen=80.0f;
  //loaded=false;
  timefornextsound.setzero();
};

void flushread() 
{
  SDL_FillRect(screen,0,black);
}

void finishplay ()
{
  flushread();
  waituntil(timefornextsound);
  SDL_UpdateRect(screen,0,0,resx,resy);
};


void setDelayTime() {
  zeit length;
  length.setdouble(curlen/40.0);
  timefornextsound.add(length);
}

void showBlackScreen() {
  flushread();
  SDL_UpdateRect(screen,0,0,0,0);
  setDelayTime();
  waituntil(timefornextsound);
}

void waitawhile() {
  if (timefornextsound.zero()) {
    timefornextsound.getcurrenttime();
  }

  setDelayTime();
  waituntil(timefornextsound);
  showBlackScreen();
}

void read (const char *const book)
{
  int pos=0;
  while (pos<int(strlen(book))) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) if (event.type == SDL_MOUSEBUTTONDOWN) exit(0);
    char x = book[pos];
    printf("\nx=%c : ", x);
    for (int i = 0; i < 8; i ++) {
      mkrealsound((x >> (7-i)) & 0x1);
      SDL_UpdateRect(screen,0,0,resx,resy);
      waitawhile();
    }
    pos++;
  }
  flushread();
};

void read_prefix() {
  const char x[] = {0x11, 0x11, 0x11, 0x11, 0x0};
  read(x);
}

void usage()
{
  printf("\nwrong parameters ! read readme file!\n\n");
  exit(1);
};

int main(int argc, char *argv[])
{
  carrier=10000000.0;
  resx=1024;
  resy=768;
  horizontalspan=1400;
  pixelclock=105.0 * 1e6;
  char *filename;

  atexit(SDL_Quit);

  printf(
	 "\n"
	 "Tempest for LeiShen - by itb !\n"
	 "--------------------------------\n"
	 "\n"
	 );

  if (argc!=7) usage();
  pixelclock=atof(argv[1]);
  resx=atol(argv[2]);
  resy=atol(argv[3]);
  horizontalspan=atol(argv[4]);
  carrier=atof(argv[5]);
  filename=argv[6];

  printf("\n"
	 "Pixel Clock %.0f Hz\n"
	 "X Resolution %d Pixels\n"
	 "Y Resolution %d Pixels\n"
	 "Horizontal Total %d Pixels\n"
	 "AM Carrier Frequency %.0f Hz\n"
	 "\n\n",
	 pixelclock,resx,resy,horizontalspan,carrier);

  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
    exit(1);
  };
  
  /* Have a preference for 8-bit, but accept any depth */
  screen = SDL_SetVideoMode(resx, resy, 8, SDL_SWSURFACE|SDL_ANYFORMAT|SDL_FULLSCREEN);
  if ( screen == NULL ) {
    fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
    exit(1);
  }
  printf("Have Set %d bits-per-pixel mode\n",
	 screen->format->BitsPerPixel);

  SDL_Color col[2];
  col[0].r=col[0].g=col[0].b=0xff; // white
  col[1].r=col[1].g=col[1].b=0x00; // black
  
  SDL_SetColors(screen,col,0,2);
  white = SDL_MapRGB(screen->format,0xff,0xff,0xff);
  black = SDL_MapRGB(screen->format,0x00,0x00,0x00);
  printf("main(): black=%d, white=%d\n", black, white);
  
  char book[100000];
  FILE *input=fopen(filename,"r");
  if (!input) { printf("cannot open file\n"); return 1; };
  resetplay();
  char *ret;
  do {
    ret=fgets(book,100000,input);
    if (ret) {
      read_prefix();
      read (book);
    }
//    if (ret) {
//      read_prefix();
//      read (book);
//    }
  } while(ret);
  finishplay();
  fclose(input);

  return 0;
};
