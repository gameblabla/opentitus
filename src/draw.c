/*   
 * Copyright (C) 2008 - 2012 The OpenTitus team
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

/* draw.c
 * Draw functions
 *
 * Global functions:
 * int TFR_SCREENM(TITUS_level *level): Draw tiles on the backbuffer (copy from the tile screen)
 * int viewstatus(TITUS_level *level, bool countbonus): View status screen (F4)
 * int flip_screen(bool slow): Flips the screen and a short delay
 * int INIT_SCREENM(TITUS_level *level): Initialize backbuffer
 * int DISPLAY_COUNT(TITUS_level *level): Draw energy
 * int fadeout(): Fade the screen to black
 * int view_password(TITUS_level *level, uint8 level_index): Display the password
 */

#include <stdio.h>
#include <stdlib.h>
#include "SDL/SDL.h"
#include "globals.h"
#include "backbuffer.h"
#include "sprites.h"
#include "draw.h"
#include "settings.h"
#include "common.h"
#include "tituserror.h"
#include "original.h"
#include "draw.h"

SDL_Surface* rl_screen;

SDL_Surface *sprite_from_cache(TITUS_level *level, TITUS_sprite *spr);

int TFR_SCREENM(TITUS_level *level) { //Draw tiles on the backbuffer (copy from the tile screen)
    SDL_Rect src, dest;
	int r_t, g_t, b_t;
	int i, j, r, g, b;
	int x, y;
    //SDL_Surface *surface = NULL;

	//First of all: make the screen black, at least the lower part of the screen
    dest.x = 0;
    dest.y = screen_height * 16 ;
    dest.w = screen_width * 19 ;
    dest.h = resheight - screen_height * 16 ;
    SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 0, 0));

	#ifndef NOAMIGA
		//Testing: Amiga lines
	
		r_t = -128;
		g_t = -128;
		b_t = 0;
		for (i = 0; i < screen_height * 16; i++) {
			dest.x = 0;
			dest.y = i ;
			dest.w = screen_width * 19 ;
			dest.h = 1;
			r_t++;
			g_t++;
			b_t++;
			r = r_t;
			g = g_t;
			b = b_t;
			if (r < 0) {
				r = 0;
			} else if (r > 255) {
				r = 255;
			}
			if (g < 0) {
				g = 0;
			} else if (g > 255) {
				g = 255;
			}
			if (b < 0) {
				b = 0;
			} else if (b > 255) {
				b = 255;
			}
		
			SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, (uint8)r, (uint8)g, (uint8)b));

		}
	#endif
	
    // Tile screen:  | Output screen:
    //               |
    // D | C         | A | B
    // -   -         | -   -
    // B | A         | C | D
    //
    // The screens are splitted in 4 parts by BITMAP_XM and BITMAP_YM
    // The code below will move the 4 rectangles with tiles to their right place on the output screen
    
/*    
    //Upper left on screen (A)
    src.x = BITMAP_XM * 16 ;
    src.y = BITMAP_YM * 16 ;
    src.w = (screen_width - BITMAP_XM) * 16 ;
    src.h = (screen_height - BITMAP_YM) * 16 ;
    dest.x = 0;
    dest.y = 0;
    SDL_BlitSurface(tilescreen, &src, screen, &dest);

    //Upper right on screen (B)
    src.x = 0;
    src.y = BITMAP_YM * 16 ;
    src.w = BITMAP_XM * 16 ;
    src.h = (screen_height - BITMAP_YM) * 16 ;
    dest.x = (screen_width - BITMAP_XM) * 16 ;
    dest.y = 0;
    SDL_BlitSurface(tilescreen, &src, screen, &dest);

    //Lower left on screen (C)
    src.x = BITMAP_XM * 16 ;
    src.y = 0;
    src.w = (screen_width - BITMAP_XM) * 16 ;
    src.h = BITMAP_YM * 16 ;
    dest.x = 0;
    dest.y = (screen_height - BITMAP_YM) * 16 ;
    SDL_BlitSurface(tilescreen, &src, screen, &dest);

    //Lower right on screen (D)
    src.x = 0;
    src.y = 0;
    src.w = BITMAP_XM * 16 ;
    src.h = BITMAP_YM * 16 ;
    dest.x = (screen_width - BITMAP_XM) * 16 ;
    dest.y = (screen_height - BITMAP_YM) * 16 ;
    SDL_BlitSurface(tilescreen, &src, screen, &dest);
*/


    src.x = 0;
    src.y = 0;
    src.w = 16;
    src.h = 16;
    dest.w = src.w;
    dest.h = src.h;

    for (x = -1; x < 21; x++) {
        int tileX = BITMAP_X + x;
        if(tileX < 0) {
            continue;
        }
        if(tileX >= level->width) {
            continue;
        }
        for (y = 0; y < 12; y++) {
            dest.x = 16 + x * 16;
            dest.y = y * 16;
            int tile = level->tilemap[BITMAP_Y + y][tileX];
            SDL_BlitSurface(level->tile[level->tile[tile].animation[tile_anim]].tiledata, &src, screen, &dest);
        }
    }
    return 0;
}


//Loop through all sprites, and draw the sprites that should be visible on the screen (NOT by using the visible flag, it uses the coordinates)
//If the flash bit is set, the first 3 planes will be 0, the last plane will be normal (colour & 0x01, odd colors gets white, even colours gets black)


void DISPLAY_SPRITES(TITUS_level *level) {
    int16 i;
    char buffer[7]; //xxx ms

    for (i = level->elevatorcount - 1; i >= 0; i--) {
        display_sprite(level, &(level->elevator[i].sprite));
    }

    for (i = level->trashcount - 1; i >= 0; i--) {
        display_sprite(level, &(level->trash[i]));
    }

    for (i = level->enemycount - 1; i >= 0; i--) {
        display_sprite(level, &(level->enemy[i].sprite));
    }

    for (i = level->objectcount - 1; i >= 0; i--) {
        display_sprite(level, &(level->object[i].sprite));
    }

#ifdef __vita__
    int tick_ = SDL_GetTicks();
    if (GODMODE && ((tick_ - godtick) < 2000)) { // Invincible during 2 seconds after a hit
        if ((tick_ % 100) > 25) {
            display_sprite(level, &(level->player.sprite3));
            display_sprite(level, &(level->player.sprite2));
            display_sprite(level, &(level->player.sprite));
        }
    } else {
        GODMODE = false;
        display_sprite(level, &(level->player.sprite3));
        display_sprite(level, &(level->player.sprite2));
        display_sprite(level, &(level->player.sprite));
    }
#else
    display_sprite(level, &(level->player.sprite3));
    display_sprite(level, &(level->player.sprite2));
    display_sprite(level, &(level->player.sprite));
#endif

#ifndef __vita__
    if (GODMODE) {
        SDL_Print_Text("GODMODE", 30 * 8, 0 * 12);
    }
#endif
    if (NOCLIP) {
        SDL_Print_Text("NOCLIP", 30 * 8, 1 * 12);
    }

#ifdef DEBUG_VERSION
    if (DISPLAYLOOPTIME) {
        sprintf(buffer, "%3u ms", LOOPTIME);
        SDL_Print_Text(buffer, 30 * 8, 2 * 12); //Loop time in ms

        sprintf(buffer, "FPS %u", FPS_LAST);
        SDL_Print_Text(buffer, 30 * 8, 4 * 12); //Last second's FPS count

        sprintf(buffer, "CL %d", LAST_CLOCK);
        SDL_Print_Text(buffer, 30 * 8, 6 * 12); //Clock

        sprintf(buffer, "CORR %d", LAST_CLOCK_CORR);
        SDL_Print_Text(buffer, 30 * 8, 8 * 12); //Correction to the clock


		for (i = 0; i <= 15; i++) {
			sprintf(buffer, "%d %3u", i, SUBTIME[i]);
			SDL_Print_Text(buffer, 0 * 8, i * 12); //Sub times from main loop in ms
		}

		sprintf(buffer, "%d %3u", i, SUBTIME[i]);
		SDL_Print_Text(buffer, 0 * 8, i * 12);
	}
	
#endif

/*
//dump tile screen
    SDL_Rect src, dest;
	src.x = 0;
	src.y = 0;
	src.w = 320;
	src.h = 200;
	dest.x = 320;
	dest.y = 0;
    SDL_BlitSurface(tilescreen, &src, screen, &dest);
*/

}

int display_sprite(TITUS_level *level, TITUS_sprite *spr) {
    SDL_Surface *image;
    SDL_Rect src, dest;
    int screen_limit;
    
    if (!spr->enabled) {
        return 0;
    }
    if (spr->invisible) {
        return 0;
    }
    spr->visible = false;
    //At this point, the buffer should be the correct size

    if (!spr->flipped) {
        dest.x = spr->x - spr->spritedata->refwidth - (BITMAP_X << 4) + 16;
    } else {
        dest.x = spr->x + spr->spritedata->refwidth - spr->spritedata->data->w - (BITMAP_X << 4) + 16;
    }
    dest.y = spr->y + spr->spritedata->refheight - spr->spritedata->height + 1 - (BITMAP_Y << 4);
    
	screen_limit = screen_width + 2;

    if ((dest.x >= screen_limit * 16) || //Right for the screen
      (dest.x + spr->spritedata->width < 0) || //Left for the screen
      (dest.y + spr->spritedata->height < 0) || //Above the screen
      (dest.y >= screen_height * 16)) { //Below the screen
        return 0;
    }

	image = sprite_from_cache(level, spr);
/*	
    if ((spr->flipped != spr->flipped_last) ||
      (spr->flash != spr->flash_last)) {
        SDL_FreeSurface(spr->buffer);
        spr->buffer = copysurface(spr->spritedata->data, spr->flipped, spr->flash);
    }
    
    spr->flipped_last = spr->flipped;
    spr->flash_last = spr->flash;
    
    SDL_Surface *image = spr->buffer;
*/
    src.x = 0;
    src.y = 0;
    src.w = image->w;
    src.h = image->h;

    if (dest.x < 0) {
        src.x = 0 - dest.x;
        src.w -= src.x;
        dest.x = 0;
    }
    if (dest.y < 0) {
        src.y = 0 - dest.y;
        src.h -= src.y;
        dest.y = 0;
    }
    if (dest.x + src.w > screen_limit * 16) {
        src.w = screen_limit * 16 - dest.x;
    }
    if (dest.y + src.h > screen_height * 16 ) {
        src.h = screen_height * 16  - dest.y;
    }

    SDL_BlitSurface(image, &src, screen, &dest);

    spr->visible = true;
    spr->flash = false;
	return 0;
}

SDL_Surface *sprite_from_cache(TITUS_level *level, TITUS_sprite *spr) {
	TITUS_spritecache *cache = level->spritecache;
	TITUS_spritedata *spritedata = level->spritedata[spr->number];
	TITUS_spritebuffer *spritebuffer;
	uint8 index;
	int16 i;

	if (spr->flipped) {index = 1;} else {index = 0;};

    if (spr->flash) {
		for (i = cache->count - cache->tmpcount; i < cache->count; i++) {
			spritebuffer = cache->spritebuffer[i];
			if (spritebuffer != NULL) {
				if ((spritebuffer->spritedata == spritedata) &&
				  (spritebuffer->index == index + 2)) {
				    return spritebuffer->data; //Already in buffer
				}
			}
		}
		//Not found, load into buffer
		cache->cycle2++;
		if (cache->cycle2 >= cache->count) { //The last 3 buffer surfaces is temporary (reserved for flash)
			cache->cycle2 = cache->count - cache->tmpcount;
		}
		spritebuffer = cache->spritebuffer[cache->cycle2];
		SDL_FreeSurface(spritebuffer->data); //Free old surface
		spritebuffer->data = copysurface(spritedata->data, spr->flipped, spr->flash);
		spritebuffer->spritedata = spritedata;
		spritebuffer->index = index + 2;
        return spritebuffer->data;
    } else {
        if (spritedata->spritebuffer[index] == NULL) {
		    cache->cycle++;
			if (cache->cycle + cache->tmpcount >= cache->count) { //The last 3 buffer surfaces is temporary (reserved for flash)
			    cache->cycle = 0;
		    }
			spritebuffer = cache->spritebuffer[cache->cycle];
			if (spritebuffer->spritedata != NULL) {
				spritebuffer->spritedata->spritebuffer[spritebuffer->index] = NULL; //Remove old link
			}
			SDL_FreeSurface(spritebuffer->data); //Free old surface
			spritebuffer->data = copysurface(spritedata->data, spr->flipped, spr->flash);
			spritebuffer->spritedata = spritedata;
			spritebuffer->index = index;
			spritedata->spritebuffer[index] = spritebuffer;
		}
		return spritedata->spritebuffer[index]->data;
	}
}

/*
NO_FAST_CPU(bool slow) {
    int tick, duration, delay, tick2;
    tick = SDL_GetTicks();
    if (slow) {
        delay = 29; //28.53612, fps: 70.09Hz/2
    } else {
        delay = 10;
    }
    LOOPTIME = (tick - LAST_CLOCK);
    delay = delay - (tick - LAST_CLOCK);
    if ((delay < 0) || (delay > 40)) {
        delay = 1;
    }
    SDL_Delay(delay);
    //do {
        //SDL_Delay(1);
    //    tick = SDL_GetTicks();
    //    duration = abs(LAST_CLOCK - tick);
    //} while (duration < delay);
    tick2 = SDL_GetTicks();
	if ((tick2 / 1000) != (LAST_CLOCK / 1000)) {
		FPS_LAST = FPS;
		FPS = 0;
    }
	FPS++;
	LAST_CLOCK_CORR = tick2 - tick - delay;
	
	LAST_CLOCK = tick2;
    SUBTIME[15] = LAST_CLOCK - tick;
}
*/

void NO_FAST_CPU(bool slow) {
    int tick, duration, delay, tick2;
    tick = SDL_GetTicks();
    if (slow) {
        delay = 29; //28.53612, fps: 70.09Hz/2
    } else {
        delay = 10;
    }
    LOOPTIME = (tick - LAST_CLOCK);
    delay = delay - (tick - LAST_CLOCK) - LAST_CLOCK_CORR;
    LAST_CLOCK_CORR = 0;
    if (delay > 40) {
        delay = 1;
    } else if (delay < 0) {
        LAST_CLOCK_CORR = (0 - delay) / 2; // To reduce LAST_CLOCK_CORR
        delay = 0;
    }



    tick2 = SDL_GetTicks();
    duration = abs(tick - tick2);
    while (duration < delay) {
		titus_sleep(); 
		//SDL_Delay(1);
        tick2 = SDL_GetTicks();
        duration = abs(tick - tick2);
    }



    //SDL_Delay(delay);




    //do {
        //SDL_Delay(1);
    //    tick = SDL_GetTicks();
    //    duration = abs(LAST_CLOCK - tick);
    //} while (duration < delay);
    tick2 = SDL_GetTicks();
	if ((tick2 / 1000) != (LAST_CLOCK / 1000)) {
		FPS_LAST = FPS;
		FPS = 0;
    }
	FPS++;


	LAST_CLOCK_CORR += tick2 - tick - delay;
        if (LAST_CLOCK_CORR > 25) {
            LAST_CLOCK_CORR = 25;
        }

	LAST_CLOCK = tick2;

    SUBTIME[15] = LAST_CLOCK - tick;
}

int flip_screen(bool slow) {
    int tick = SDL_GetTicks();
    Flip_Titus();
    int oldtick = tick;
    tick = SDL_GetTicks();
    SUBTIME[14] = tick - oldtick;
    
#if defined(_DINGUX) || defined(__vita__)
    if (slow) {
        NO_FAST_CPU(slow);
    }
#else
    NO_FAST_CPU(slow);
#endif
	return 0;
}


int viewstatus(TITUS_level *level, bool countbonus){
    int retval, i, j;
    char tmpchars[10];
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

    if (game == 0) { //Titus
        SDL_Print_Text("LEVEL", 13 * 8, 12 * 5);
        SDL_Print_Text("EXTRA BONUS", 10 * 8, 10 * 12);
        SDL_Print_Text("LIVES", 10 * 8, 11 * 12);
    } else if (game == 1) { //Moktar
        SDL_Print_Text("ETAPE", 13 * 8, 12 * 5);
        SDL_Print_Text("EXTRA BONUS", 10 * 8, 10 * 12);
        SDL_Print_Text("VIE", 10 * 8, 11 * 12);
    }


    sprintf(tmpchars, "%d", level->levelnumber + 1);
    SDL_Print_Text(tmpchars, 25 * 8 - strlen(tmpchars) * 8, 12 * 5);

    SDL_Print_Text(leveltitle[level->levelnumber], 0, 12 * 5 + 16);
    sprintf(tmpchars, "%d", level->extrabonus);
    SDL_Print_Text(tmpchars, 28 * 8 - strlen(tmpchars) * 8, 10 * 12);

    sprintf(tmpchars, "%d", level->lives);
    SDL_Print_Text(tmpchars, 28 * 8 - strlen(tmpchars) * 8, 11 * 12);

    Flip_Titus();

    if (countbonus && (level->extrabonus >= 10)) {
        retval = waitforbutton();
        if (retval < 0) {
            return retval;
        }
        while (level->extrabonus >= 10) {
            for (i = 0; i < 10; i++) {
                level->extrabonus--;
                sprintf(tmpchars, "%2d", level->extrabonus);
                SDL_Print_Text(tmpchars, 28 * 8 - strlen(tmpchars) * 8, 10 * 12);
                Flip_Titus();
                for (j = 0; j < 15; j++) {
                    NO_FAST_CPU(false);
                }
            }
            level->lives++;
            sprintf(tmpchars, "%d", level->lives);
            SDL_Print_Text(tmpchars, 28 * 8 - strlen(tmpchars) * 8, 11 * 12);
            Flip_Titus();
            for (j = 0; j < 10; j++) {
                NO_FAST_CPU(false);
            }
        }
    }

    retval = waitforbutton();
    if (retval < 0)
        return retval;

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    Flip_Titus();

    return (0);
}

int INIT_SCREENM(TITUS_level *level) {
    CLOSE_SCREEN();
    BITMAP_X = 0;
    BITMAP_Y = 0;
    BITMAP_XM = 0;
    BITMAP_YM = 0;
    uint8 i, j;
    for (i = 0; i < 20; i++) {
        for (j = 0; j < 12; j++) {
            DISPLAY_CHAR(level, level->tilemap[BITMAP_Y + j][BITMAP_X + i], j, i);
        }
    }
    do {
        scroll(level);
    } while (g_scroll_y || g_scroll_x);
    OPEN_SCREEN(level);
    
    return 0;
}


int DISPLAY_COUNT(TITUS_level *level) {
    subto0(&(BAR_FLAG));
    if (BAR_FLAG != 0) {
        DISPLAY_ENERGY(level);
    }
    
    return 0;
}


int DISPLAY_ENERGY(TITUS_level *level) {
    int offset = (96 + 16) - g_scroll_px_offset;
    int i;
    SDL_Rect dest;
    for (i = 0; i < level->player.hp; i++) { //Draw big bars (4px*16px, spacing 4px)
        dest.x = offset ;
        dest.y = 9 ;
        dest.w = 4 ;
        dest.h = 16 ;
        SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 255, 255, 255));
        offset += 8;
    }
    for (i = 0; i < MAXIMUM_ENERGY - level->player.hp; i++) { //Draw small bars (4px*4px, spacing 4px)
        dest.x = offset ;
        dest.y = 15 ;
        dest.w = 4 ;
        dest.h = 4 ;
        SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 255, 255, 255));
        offset += 8;
    }
    return 0;
}

int fadeout() {
    SDL_Surface *image;
    int activedelay = 1;
    SDL_Event event;
    unsigned int fade_time = 1000;
    unsigned int tick_start = 0;
    unsigned int image_alpha = 0;
    SDL_Rect src, dest;

    src.x = 0;
    src.y = 0;
    src.w = screen->w;
    src.h = screen->h;
	
    dest.x = 0;
    dest.y = 0;
    dest.w = screen->w;
    dest.h = screen->h;

    image = SDL_ConvertSurface(screen, screen->format, SDL_SWSURFACE);

    SDL_BlitSurface(screen, &src, image, &dest);
    tick_start = SDL_GetTicks();
    while (image_alpha < 255) //Fade to black
    {
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_FreeSurface(image);
                return (-1);
            }

#ifdef __vita__
            if (event.type == SDL_JOYBUTTONDOWN) {
                if (event.jbutton.button == SDLK_ESCAPE) {
#else
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
#endif
                    SDL_FreeSurface(image);
                    return (-1);
                }
#ifdef AUDIO_ENABLED
#ifdef __vita__
                if (event.jbutton.button == KEY_MUSIC) {
#else
                if (event.key.keysym.sym == KEY_MUSIC) {
#endif
					AUDIOMODE++;
					if (AUDIOMODE > 1) {
						AUDIOMODE = 0;
					}
					if (AUDIOMODE == 1) {
						startmusic();
					}
                }
#endif
            }
        }

        image_alpha = (SDL_GetTicks() - tick_start) * 256 / fade_time;

        if (image_alpha > 255)
            image_alpha = 255;
            
        SDL_SetAlpha(image, SDL_SRCALPHA, 255 - image_alpha);
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
        SDL_BlitSurface(image, &src, screen, &dest);
        Flip_Titus();

#ifdef AUDIO_MIKMOD_SINGLETHREAD
        checkmodule();
#endif

#ifdef AUDIO_SDL_MIXER
        checkaudio();
#endif

        titus_sleep();
    }
    SDL_FreeSurface(image);
    
    return 0;

}

int view_password(TITUS_level *level, uint8 level_index) {
    //Display the password !
    char tmpchars[10];
    int retval;
	int saved_value;
    CLOSE_SCREEN();
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    Flip_Titus();
    
    saved_value = g_scroll_px_offset;
    g_scroll_px_offset = 0;

    if (game == 0) { //Titus
        SDL_Print_Text("LEVEL", 13 * 8, 13 * 8);
    } else if (game == 1) { //Moktar
        SDL_Print_Text("ETAPE", 13 * 8, 13 * 8);
    }
    sprintf(tmpchars, "%d", level_index + 1);
    SDL_Print_Text(tmpchars, 25 * 8 - strlen(tmpchars) * 8, 13 * 8);

    SDL_Print_Text("CODE", 14 * 8, 10 * 8);
    SDL_Print_Text(levelcode[level_index], 20 * 8, 10 * 8);

    Flip_Titus();
    retval = waitforbutton();
    g_scroll_px_offset = saved_value;
    if (retval < 0)
        return retval;

    //Flip_Titus();
    OPEN_SCREEN(level);
    return (0);
}

int loadpixelformat(SDL_PixelFormat **pixelformat){
    int i;

    *pixelformat = (SDL_PixelFormat *)SDL_malloc(sizeof(SDL_PixelFormat));
    if (*pixelformat == NULL) {
        sprintf(lasterror, "Error: Not enough memory to initialize palette!\n");
        return (TITUS_ERROR_NOT_ENOUGH_MEMORY);
    }

    (*pixelformat)->palette = (SDL_Palette *)SDL_malloc(sizeof(SDL_Palette));
    if ((*pixelformat)->palette == NULL) {
        sprintf(lasterror, "Error: Not enough memory to initialize palette!\n");
        return (TITUS_ERROR_NOT_ENOUGH_MEMORY);
    }

    (*pixelformat)->palette->ncolors = 16;

    (*pixelformat)->palette->colors = (SDL_Color *)SDL_malloc(sizeof(SDL_Color) * (*pixelformat)->palette->ncolors);
    if ((*pixelformat)->palette->colors == NULL) {
        sprintf(lasterror, "Error: Not enough memory to initialize palette!\n");
        return (TITUS_ERROR_NOT_ENOUGH_MEMORY);
    }

    for (i = 0; i < (*pixelformat)->palette->ncolors; i++) {
        (*pixelformat)->palette->colors[i].r = orig_palette_colour[i].r;
        (*pixelformat)->palette->colors[i].g = orig_palette_colour[i].g;
        (*pixelformat)->palette->colors[i].b = orig_palette_colour[i].b;
        (*pixelformat)->palette->colors[i].unused = orig_palette_colour[i].unused;
    }

    (*pixelformat)->BitsPerPixel = 8;
    (*pixelformat)->BytesPerPixel = 1;

    (*pixelformat)->Rloss = 0;
    (*pixelformat)->Gloss = 0;
    (*pixelformat)->Bloss = 0;
    (*pixelformat)->Aloss = 0;

    (*pixelformat)->Rshift = 0;
    (*pixelformat)->Gshift = 0;
    (*pixelformat)->Bshift = 0;
    (*pixelformat)->Ashift = 0;

    (*pixelformat)->Rmask = 0;
    (*pixelformat)->Gmask = 0;
    (*pixelformat)->Bmask = 0;
    (*pixelformat)->Amask = 0;

    (*pixelformat)->colorkey = 0;
    (*pixelformat)->alpha = 255;

    return (0);
}

int loadpixelformat_font(SDL_PixelFormat **pixelformat){
    int i;

    *pixelformat = (SDL_PixelFormat *)SDL_malloc(sizeof(SDL_PixelFormat));
    if (*pixelformat == NULL) {
        sprintf(lasterror, "Error: Not enough memory to initialize palette!\n");
        return (TITUS_ERROR_NOT_ENOUGH_MEMORY);
    }

    (*pixelformat)->palette = (SDL_Palette *)SDL_malloc(sizeof(SDL_Palette));
    if ((*pixelformat)->palette == NULL) {
        sprintf(lasterror, "Error: Not enough memory to initialize palette!\n");
        return (TITUS_ERROR_NOT_ENOUGH_MEMORY);
    }

    (*pixelformat)->palette->ncolors = 16;

    (*pixelformat)->palette->colors = (SDL_Color *)SDL_malloc(sizeof(SDL_Color) * (*pixelformat)->palette->ncolors);
    if ((*pixelformat)->palette->colors == NULL) {
        sprintf(lasterror, "Error: Not enough memory to initialize palette!\n");
        return (TITUS_ERROR_NOT_ENOUGH_MEMORY);
    }

    for (i = 0; i < (*pixelformat)->palette->ncolors; i++) {
        (*pixelformat)->palette->colors[i].r = orig_palette_font_colour[i].r;
        (*pixelformat)->palette->colors[i].g = orig_palette_font_colour[i].g;
        (*pixelformat)->palette->colors[i].b = orig_palette_font_colour[i].b;
        (*pixelformat)->palette->colors[i].unused = orig_palette_font_colour[i].unused;
    }

    (*pixelformat)->BitsPerPixel = 8;
    (*pixelformat)->BytesPerPixel = 1;

    (*pixelformat)->Rloss = 0;
    (*pixelformat)->Gloss = 0;
    (*pixelformat)->Bloss = 0;
    (*pixelformat)->Aloss = 0;

    (*pixelformat)->Rshift = 0;
    (*pixelformat)->Gshift = 0;
    (*pixelformat)->Bshift = 0;
    (*pixelformat)->Ashift = 0;

    (*pixelformat)->Rmask = 0;
    (*pixelformat)->Gmask = 0;
    (*pixelformat)->Bmask = 0;
    (*pixelformat)->Amask = 0;

    (*pixelformat)->colorkey = 0;
    (*pixelformat)->alpha = 255;

    return (0);
}

int freepixelformat(SDL_PixelFormat **pixelformat){
    free ((*pixelformat)->palette->colors);
    free ((*pixelformat)->palette);
    free (*pixelformat);
    return (0);
}

void Flip_Titus()
{
	int i;
    SDL_Rect src, dst;
    #ifdef NORMAL_SDL
    src.x = 16 - g_scroll_px_offset;
    dst.y = src.y = 0;
    dst.w = src.w = reswidth;
    dst.h = src.h = resheight;
    dst.x = 0;
    dst.y = 20;
	SDL_BlitSurface(screen, &src, rl_screen, &dst);
	SDL_Flip(rl_screen);
    #else
    /* This kludgery is required on Dreamcast otherwise... it will just crash...*/
    SDL_LockSurface(rl_screen);
	uint32_t *s = (uint32_t*)screen->pixels + ((16 - g_scroll_px_offset) >> 1);
	uint32_t *d = (uint32_t*)rl_screen->pixels;
	for(i = 0; i < 191; i++, s += (352/2), d += 160)
	{
		#ifdef DREAMCAST
		memcpy
		#else
		memmove
		#endif
		(d, s, ((352)*2));
	}
	SDL_UnlockSurface(rl_screen);
	SDL_Flip(rl_screen);
	#endif	
}
