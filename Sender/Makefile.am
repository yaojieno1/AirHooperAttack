SUBDIRS=songs

EXTRA_DIST=mksong.sh

INCLUDES=@SDL_CFLAGS@

bin_PROGRAMS=tempest_for_eliza tempest_for_mp3 tempest_for_binge

tempest_for_eliza_SOURCES=player.cpp

tempest_for_eliza_LDADD=@SDL_LIBS@


tempest_for_mp3_SOURCES=mp3player.cpp

tempest_for_mp3_LDADD=@SDL_LIBS@


tempest_for_binge_SOURCES=trans.cpp
tempest_for_binge_LDADD=@SDL_LIBS@