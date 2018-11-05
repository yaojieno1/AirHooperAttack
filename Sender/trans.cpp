/*
  tempest for eliza code
  Copyright (C) 2001  Erik Thiele

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
  void print()
    {
      printf("%8d:%8d\n",int(tv_sec),int(tv_usec));
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

  select(1,0,0,0,&tm);
};

void mkrealsound(double freq)
{
  SDL_Rect myrect;
  myrect.x=myrect.y=0;
  myrect.w=resx;
  myrect.h=resy;
  SDL_FillRect(screen,&myrect,black);

  printf("mkrealsound(): freq=%f\n", freq);

  int x, y;
  double t;
  
/*
  double ftfp2 = freq / pixelclock * 2.0;
  double fcfp2 = carrier / pixelclock * 2.0;
  
  t = 0;
  for (y = 0; y < resy; y++) {
    if (int(t*ftfp2)%2) {
      // 1
      t+=resx;
    }else{
      // 0
      for (x = 0; x < resx; x++) {
	if (int(t*fcfp2)%2)
	  pixelchen(x,y,white);
	t++;
      };
    };
    
    t += horizontalspan - resx;
  }
*/
  double k = 2 * freq / pixelclock;
  double k2 = 2 * carrier / pixelclock;
  t = 0;
  for (y = 0; y < resy; y ++) {
    if (int(t * k) % 2) {
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

double note (int nr)
{
  double n = pow(2.0,nr/12.0)*440.0*2;
  printf("note(): nr=%d, n=%f\n", nr, n);
  return n;
};


int curoct, curnot;
double curlen;
bool loaded;
zeit timefornextsound;

void resetplay()
{
  curoct=0;
  curnot=0;
  curlen=128;
  loaded=false;
  timefornextsound.setzero();
};

void finishplay ()
{
  SDL_Rect myrect;
  myrect.x=myrect.y=0;
  myrect.w=resx;
  myrect.h=resy;
  SDL_FillRect(screen,&myrect,black);

  waituntil(timefornextsound);
  SDL_UpdateRect(screen,0,0,resx,resy);
};

void flushplay ()
{
  if (loaded) {
    printf("flushplay(): curnot=%d,curoct=%d,curlen=%f\n",curnot, curoct, curlen);
    if (curnot==-1000) {
      SDL_Rect myrect;
      myrect.x=myrect.y=0;
      myrect.w=resx;
      myrect.h=resy;
      SDL_FillRect(screen,&myrect,black);

      waituntil(timefornextsound);
    }else{
      mkrealsound(note(curnot+12*curoct));
      //mkrealsound(22000.00f+random()%1000);
    };
    if (timefornextsound.zero()) {
      timefornextsound.getcurrenttime();
    }else{
      waituntil(timefornextsound);
    };
    SDL_UpdateRect(screen,0,0,resx,resy);

    zeit length;
    length.setdouble(curlen/40.0);
    timefornextsound.add(length);
    loaded=false;
  };
};

void play (const char *const song)
{
  int pos=0;
  while (pos<int(strlen(song))) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) if (event.type == SDL_MOUSEBUTTONDOWN) exit(0);
    switch(song[pos]) {
    case 'c': printf("c\n");curnot=0;     loaded=true; flushplay();break;
    case 'd': printf("d\n");curnot=2;     loaded=true; flushplay(); break;
    case 'e': printf("e\n");curnot=4;     loaded=true; flushplay(); break;
    case 'f': printf("f\n");curnot=5;     loaded=true; flushplay(); break;
    case 'g': printf("g\n");curnot=7;     loaded=true; flushplay(); break;
    case 'a': printf("a\n");curnot=9;     loaded=true; flushplay(); break;
    case 'b': printf("b\n");curnot=11;    loaded=true; flushplay(); break;
    case 'p': printf("p\n");curnot=-1000; loaded=true; flushplay(); break;
    case '#': printf("#\n");curnot++;     loaded=true; flushplay(); break;
    case '1': printf("1\n");curlen=128; break;
    case '2': printf("2\n");curlen=64;  break;
    case '4': printf("4\n");curlen=32;  break;
    case '5': printf("5\n");curlen=48;  break;
    case '8': printf("8\n");curlen=16;  break;
    case '6': printf("6\n");curlen=8;   break;
    case 'u': printf("u\n");curoct=-2;  break;
    case 'l': printf("l\n");curoct=-1;  break;
    case 'h': printf("h\n");curoct=0;   break;
    case 'x': printf("x\n");curoct=1;   break;
    default:break;
    };
    pos++;
  };
  flushplay();
};

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
	 "Tempest for Binge - by yaojie !\n"
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
  
  char song[100000];
  FILE *input=fopen(filename,"r");
  if (!input) { printf("cannot open file\n"); return 1; };
  resetplay();
  char *ret;
  do {
    ret=fgets(song,100000,input);
    if (ret) {
      for (unsigned i=0;i<strlen(song);i++)
	      if (song[i]=='*') song[i]=0;
      play (song);
    }
  } while(ret);
  finishplay();
  fclose(input);

  return 0;
};
