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
#include <assert.h>

double fT; // Traegerfrequenz des Radiosenders [1/s]
u_int32_t resx; // anzahl sichtbarer pixel in X richtung
u_int32_t resy; // anzahl sichtbarer pixel in Y richtung
u_int32_t horizontalspan; // anzahl virtueller pixel in X richtung
u_int32_t verticalspan; // anzahl virtueller pixel in Y richtung
double fP; // Pixelfrequenz der Grafikkarte [1/s]
int playbackmode;
double amplify;

SDL_Surface *screen;

void error (const char *const msg)
{
  fprintf(stderr, "ERROR: %s\n\n", msg);
  exit(1);
}

void usage()
{
  printf("\nwrong parameters ! read readme file!\n\n");
  exit(1);
}

int main(int argc, char *argv[])
{
  char *filename;

  atexit(SDL_Quit);

  printf(
	 "\n"
	 "Tempest for MP3 - by erikyyy !\n"
	 "------------------------------\n"
	 "\n"
	 "Read the README file to understand what's happening\n"
	 "if you do not read it, you will NOT know what to do\n"
	 );

  if (argc!=10) usage();
  fP=atof(argv[1]);
  resx=atol(argv[2]);
  resy=atol(argv[3]);
  horizontalspan=atol(argv[4]);
  verticalspan=atol(argv[5]);
  fT=atof(argv[6]);
  playbackmode=atoi(argv[7]);
  amplify=atof(argv[8]);
  filename=argv[9];

  double npp=horizontalspan * verticalspan; // number of pixels pro screen update periode
  double gZ=npp/fP; // gesamtZeit pro screen update periode
  double cT=gZ*fT; // anzahl Traegerperioden pro update periode
  cT=double(int(cT));
  fT=cT/gZ;

  printf("\n"
	 "Pixel Clock %.5f Hz\n"
	 "X Resolution %d Pixels\n"
	 "Y Resolution %d Pixels\n"
	 "Horizontal Total %d Pixels\n"
	 "Vertical Total %d Pixels\n"
	 "AM Carrier Frequency %.5f Hz\n"
	 "Playbackmode %d\n"
	 "Amplify %.5f\n"
	 "\n\n",
	 fP,resx,resy,horizontalspan,verticalspan,fT,playbackmode,amplify);

  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }
  
  screen = SDL_SetVideoMode(resx, resy, 8, SDL_HWPALETTE | SDL_ANYFORMAT | SDL_FULLSCREEN);
  if ( screen == NULL ) {
    fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
    exit(1);
  }
  if ((screen->flags | SDL_HWPALETTE) == 0) error("i can't get hardware palette support.");
  if ((screen->flags | SDL_FULLSCREEN) == 0) error("cannot set fullscreen mode.");
  if (screen->format->BitsPerPixel != 8) error("cannot set 8bpp mode.");

  SDL_LockSurface (screen);
  u_int8_t *p = (u_int8_t*) screen->pixels;
  u_int32_t pitch=screen->pitch;
  double npcb=npp/double(128); // number of pixels pro screen update periode pro audio sample
  for (u_int32_t y=0;y<resy;++y)
    for (u_int32_t x=0;x<resx;++x) {
      double iP=y*horizontalspan+x; // pixel index
      double tP=iP/fP; // pixel zeit [s]
      if (sin(tP*fT*double(2)*M_PI)>0) 
	p[pitch*y+x]=int(iP/npcb)*2;
      else
	p[pitch*y+x]=int(iP/npcb)*2+1;
    }
  SDL_UnlockSurface(screen);
  SDL_Flip(screen);

  double aF=fP/npcb; // audio frequenz [1/s]
  printf("audio file frequency should be %f Hz\n",aF);

  u_int32_t audiolength;
  u_int8_t *audiobuf;
  {
    FILE *input;
    input = fopen(filename,"rb");
    assert (input != NULL);
    assert (fseek(input,0,SEEK_END) != -1);
    audiolength=ftell(input);
    assert (audiolength>=0);
    assert (fseek(input,0,SEEK_SET) != -1);
    audiobuf = (u_int8_t*) malloc (audiolength);
    assert (audiobuf != 0);
    assert (fread(audiobuf,audiolength,1,input)==1);
    assert (fclose(input)==0);
  }
  printf("audiolength is %d\n",audiolength);

  SDL_Event event;

  SDL_Color colors[256];
  u_int32_t curpos=0;
  for (;;) {
    if(SDL_PollEvent(&event)) if (event.type==SDL_KEYDOWN) exit(0);

    for(int i=0;i<128;i++){
      double value=audiobuf[curpos];
      value-=double(128);
      value*=amplify;
      value+=double(128);
      int ivalue=int(value);
      if (ivalue<0) ivalue=0;
      if (ivalue>255) ivalue=255;
      
      if (playbackmode==0) {
	colors[i*2].r=ivalue;
	colors[i*2].g=ivalue;
	colors[i*2].b=ivalue;
	
	colors[i*2+1].r=0;
	colors[i*2+1].g=0;
	colors[i*2+1].b=0;
      }else if (playbackmode==1) {
	int col = (ivalue * 20) / 45;
	
	colors[i*2].r=col;
	colors[i*2].g=col;
	colors[i*2].b=col;
	
	colors[i*2+1].r=255-col;
	colors[i*2+1].g=255-col;
	colors[i*2+1].b=255-col;
      }
      curpos ++;
      if (curpos>=audiolength) exit(0);
    }
    SDL_SetPalette(screen, SDL_PHYSPAL, colors, 0, 256);
  }

  return 0;
}
