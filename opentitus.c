/*   
 * Copyright (C) 2008 - 2011 The OpenTitus team
 *
 * Authors:
 * Eirik Stople
 *
 * "Titus the Fox: To Marrakech and Back" (1992) and
 * "Lagaf': Les Aventures de Moktar - Vol 1: La Zoubida" (1991)
 * was developed by, and is probably copyrighted by Titus Software,
 * which, according to Wikipedia, stopped buisness in 2005.
 *
 * OpenTitus is not affiliated with Titus Software.
 *
 * OpenTitus is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 3  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

/* opentitus.c
 * Main source file
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL/SDL.h"
#include "globals.h"
//#include "opentitus.h"

//Probably not the best way, but it works...
#define HAVE_CONFIG_H 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef AUDIO_ENABLED
#include "audio.h"
#endif

#include "tituserror.h"
#include "sqz.h"
#include "settings.h"
#include "malloc.h"
#include "sprites.h"
#include "backbuffer.h"
#include "viewimage.h"
#include "fonts.h"
#include "menu.h"
#include "engine.h"
#include "original.h"
#include "objects.h"

int init_opentitus();

int main(int argc, char *argv[]) 
{
	char retval;
    unsigned char state = 1; //View the menu when the main loop starts
    
#ifdef NSPIRE
	enable_relative_paths(argv);
#endif

#ifdef HOME_PATH
	char home_path[256];
	snprintf(home_path, sizeof(home_path), "%s/.opentitus", getenv("HOME"));
	if (access( home_path, F_OK ) == -1)
	{
		mkdir(home_path, 0755);
		nogame_data();
        state = 0;
        goto exitgame;
	}
#endif
  
    retval = init_opentitus();
    
    if (retval < 0)
    {
		nogame_data();
        state = 0;
        goto exitgame;
    } 

/*
    if (state) 
    {
        retval = viewintrotext();
        if (retval < 0)
            state = 0;
    }
*/
    if (state) 
    {
        retval = viewimage(tituslogofile, tituslogoformat, 0, 4000);
        if (retval < 0)
            state = 0;
    }

#ifdef AUDIO_ENABLED
    SELECT_MUSIC(15);
#endif

    if (state) {
        retval = viewimage(titusintrofile, titusintroformat, 0, 6500);
        if (retval < 0)
            state = 0;
    }

    while (state) {
        retval = viewmenu(titusmenufile, titusmenuformat);

        if (retval <= 0)
            state = 0;

        if (state && (retval <= levelcount)) {
            retval = playtitus(retval - 1);
            if (retval < 0)
                state = 0;
        }
    }
    
exitgame:
    freefonts();

#ifdef AUDIO_ENABLED
    freeaudio();
#endif

    SDL_Quit();

    checkerror();

    if (retval == -1)
        retval = 0;

    return retval;
}

int init_opentitus() {
	int flags;
    int retval;
	char path_folder[512];

#ifdef HOME_SUPPORT 
	sprintf(path_folder, "%s/.opentitus/%s", getenv("HOME"), OPENTITUS_CONFIG_FILE);
#else
	#ifdef NSPIRE
		sprintf(path_folder, "/documents/opentitus/%s.tns", OPENTITUS_CONFIG_FILE);
	#elif defined(DREAMCAST)
		#ifdef SDCARD_DREAMCAST
		if(sd_init()) 
		{
			printf("No SD card detected. Make sure to have SD card !\n");
		}
		else
		{
			is_sdcard = 1;
			sd_blockdev_for_partition(0, &sd_dev, &partition_type);
			fs_fat_init();
			fs_fat_mount("/sd", &sd_dev, FS_FAT_MOUNT_READONLY);
		}
		#endif
		sprintf(path_folder, "/cd/%s", OPENTITUS_CONFIG_FILE);
		SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
	#else
		sprintf(path_folder, "./%s", OPENTITUS_CONFIG_FILE);
	#endif
#endif

    retval = readconfig(path_folder);
    if (retval < 0)
    {
		// Make sure to init at least something
		SDL_Init(SDL_INIT_VIDEO);
		screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320 + 32, 200, 16, 0, 0, 0, 0);
		rl_screen = SDL_SetVideoMode(320, 200, 0, SDL_SWSURFACE | SDL_DOUBLEBUF);
        return retval;
	}

#ifdef AUDIO_ENABLED
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return TITUS_ERROR_SDL_ERROR;
    }
#else
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return TITUS_ERROR_SDL_ERROR;
    }
#endif

	SDL_ShowCursor(SDL_DISABLE);
#if defined(_DINGUX) 
	flags = SDL_SWSURFACE;
#elif defined(DREAMCAST)
	flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
	SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
#elif defined(_TINSPIRE)
	flags = SDL_SWSURFACE;
#elif defined(GCW)
	flags = SDL_HWSURFACE | SDL_TRIPLEBUF;
#else
    switch (videomode) {
    case 0: //window mode
		SDL_ShowCursor(SDL_ENABLE);
        flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
        SDL_WM_SetCaption(OPENTITUS_WINDOW_TEXT, 0);
        break;
    case 1: //fullscreen
        SDL_ShowCursor(SDL_DISABLE);
        flags = SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN;
        break;
    }
#endif

	screen = SDL_CreateRGBSurface(SDL_SWSURFACE, reswidth + 32, resheight, bitdepth, 0, 0, 0, 0);
	rl_screen = SDL_SetVideoMode(reswidth, resheight, bitdepth, flags);
	 
    if (rl_screen == NULL) {
        printf("Unable to set video mode: %s\n", SDL_GetError());
        return TITUS_ERROR_SDL_ERROR;
    }

    /*SDL_EnableUNICODE (1);*/

#ifdef AUDIO_ENABLED
	initaudio();
#endif

    initoriginal();
    initcodes();
    initleveltitles();
    loadfonts();
    return 0;

}

void checkerror(void) {
    printf("%s\n", lasterror);
}

