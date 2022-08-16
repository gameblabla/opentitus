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

/* menu.c
 * Handles the menu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DREAMCAST
	#include <kos.h>
#endif

#include "SDL/SDL.h"
#include "sqz.h"
#include "backbuffer.h"
#include "menu.h"
#include "fonts.h"
#include "settings.h"
#include "audio.h"
#include "globals.h"
#include "common.h"
#include "keyboard.h"

//Probably not the best way, but it works...
#define HAVE_CONFIG_H 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef AUDIO_ENABLED
#include "audio.h"
#endif

int enterpassword();

int viewmenu(char * menufile, int menuformat) {
#ifdef DREAMCAST
	maple_device_t *cont;
    cont_state_t *state;
	struct button_dc
	{
		unsigned char time;
		unsigned char state;
	} buttons[3];
	unsigned char z = 0;
	long button_to_press = 0;
	
	for (z=0;z<3;z++)
	{	
		buttons[z].state = 2;
		buttons[z].time = -5;
	}
#endif
    SDL_Surface *surface;
    SDL_Palette *palette;
    char *tmpchar;
    SDL_Surface *image;
    unsigned char *menudata;
    int retval;
    int i;
    int j;
    int menuloop = 1;
    int selection = 0;
    SDL_Event event;
    int curlevel = 1;

    unsigned int fade_time = 1000;
    unsigned int tick_start = 0;
    unsigned int image_alpha = 0;

    SDL_Rect src, dest;
    SDL_Rect sel[2];
    SDL_Rect sel_dest[2];

    retval = unSQZ(menufile, &menudata);

    if (retval < 0) {
        free (menudata);
        return (retval);
    }

    switch (menuformat) {
    case 1: //Planar 16-color

        break;

    case 2: //256 color
        surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 200, 8, 0, 0, 0, 0);
        palette = (surface->format)->palette;
        if (palette) {
            for (i = 0; i < 256; i++) {
                palette->colors[i].r = (menudata[i * 3] & 0xFF) * 4;
                palette->colors[i].g = (menudata[i * 3 + 1] & 0xFF) * 4;
                palette->colors[i].b = (menudata[i * 3 + 2] & 0xFF) * 4;
                palette->colors[i].unused = 0; 
            }
            palette->ncolors = 256;
        }

        tmpchar = (char *)surface->pixels;
        for (i = 256 * 3; i < 256 * 3 + 320*200; i++) {
            *tmpchar = menudata[i];
            tmpchar++;
        }

        image = SDL_DisplayFormat(surface);
        palette = NULL;

        SDL_FreeSurface(surface);

        break;
    }

    free (menudata);

    src.x = 0;
    src.y = 0;
    src.w = image->w;
    src.h = image->h;
	
    dest.x = SCROLL_OFFSET;
    dest.y = 0;
    dest.w = image->w;
    dest.h = image->h;

    if (game == 0) { //Titus

        sel[0].x = 120;
        sel[0].y = 160;
        sel[0].w = 8;
        sel[0].h = 8;
	
        sel[1].x = 120;
        sel[1].y = 173;
        sel[1].w = 8;
        sel[1].h = 8;

    } else if (game == 1) { //Moktar

        sel[0].x = 130;
        sel[0].y = 167;
        sel[0].w = 8;
        sel[0].h = 8;
	
        sel[1].x = 130;
        sel[1].y = 180;
        sel[1].w = 8;
        sel[1].h = 8;

    }

    sel_dest[0] = sel[0];
    sel_dest[0].x += SCROLL_OFFSET;
    sel_dest[1] = sel[1];
    sel_dest[1].x += SCROLL_OFFSET;

    tick_start = SDL_GetTicks();

    while (image_alpha < 255) { //Fade in

        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_FreeSurface(image);
                return (-1);
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    SDL_FreeSurface(image);
                    return (-1);
                }
#ifdef AUDIO_ENABLED
                if (event.key.keysym.sym == KEY_MUSIC) {
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

        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
        SDL_SetAlpha(image, SDL_SRCALPHA, image_alpha);
        SDL_BlitSurface(image, &src, screen, &dest);
        SDL_BlitSurface(image, &sel[1], screen, &sel_dest[0]);
        SDL_BlitSurface(image, &sel[0], screen, &sel_dest[selection]);
        Flip_Titus();
        titus_sleep();

#ifdef AUDIO_MIKMOD_SINGLETHREAD
        checkmodule();
#endif

#ifdef AUDIO_SDL_MIXER
        checkaudio();
#endif

    }

    beforemenuloop:

    while (menuloop) { //View the menu

#ifdef DREAMCAST
		cont  = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
		state = (cont_state_t *) maple_dev_status(cont);
#endif

        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_FreeSurface(image);
                return (-1);
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    SDL_FreeSurface(image);
                    return (-1);
                }
                if (event.key.keysym.sym == SDLK_UP)
                    selection = 0;
                if (event.key.keysym.sym == SDLK_DOWN)
                    selection = 1;
                if (event.key.keysym.sym == KEY_RETURN || event.key.keysym.sym == KEY_ENTER || event.key.keysym.sym == KEY_SPACE)
                    menuloop = 0;
#ifdef AUDIO_ENABLED
                if (event.key.keysym.sym == KEY_MUSIC) {
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

#ifdef DREAMCAST
		for (z=0;z<3;z++)
		{	
			if (z==0) button_to_press = CONT_DPAD_UP;
			else if (z==1) button_to_press = CONT_DPAD_DOWN;
			else button_to_press = CONT_A;
					
			switch (buttons[z].state)
			{
				case 0:
					if (state->buttons & button_to_press)
					{
						buttons[z].state = 1;
						buttons[z].time = 0;
					}
				break;
				
				case 1:
					buttons[z].time++;
					
					if (buttons[z].time > 0)
					{
						buttons[z].state = 2;
						buttons[z].time = 0;
					}
				break;
				
				case 2:
					if (!(state->buttons & button_to_press))
					{
						buttons[z].state = 3;
						buttons[z].time = 0;
					}
				break;
				
				case 3:
					buttons[z].time++;
					
					if (buttons[z].time > 0)
					{
						buttons[z].state = 0;
						buttons[z].time = 0;
					}
				break;
			}   
		}

		if ((buttons[0].state == 1)) 
		{
			selection = 0;
			buttons[0].state = 2;
		}
		else if ((buttons[1].state == 1)) 
		{
			selection = 1;
			buttons[1].state = 2;
		}
		else if ((buttons[2].state == 1)) 
		{
			menuloop = 0;
			buttons[2].state = 2;
		}
#endif

        SDL_FillRect(screen, NULL, 0);
        SDL_BlitSurface(image, &src, screen, &dest);
        SDL_BlitSurface(image, &sel[1], screen, &sel_dest[0]);
        SDL_BlitSurface(image, &sel[0], screen, &sel_dest[selection]);
        Flip_Titus();
        titus_sleep();

#ifdef AUDIO_MIKMOD_SINGLETHREAD
        checkmodule();
#endif

#ifdef AUDIO_SDL_MIXER
        checkaudio();
#endif

    }

    switch (selection) {
    case 0: //Start

        break;

    case 1: //Password
        retval = enterpassword();

        if (retval < 0)
            return retval;

        if (retval > 0) {
            if (retval <= levelcount)
                curlevel = retval;
        }
        selection = 0;
        menuloop = 1;
        goto beforemenuloop;
        break;

    default:
        return (-1);
        break;
    }

    tick_start = SDL_GetTicks();
    image_alpha = 0;
    while (image_alpha < 255) { //Fade out

        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_FreeSurface(image);
                return (-1);
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    SDL_FreeSurface(image);
                    return (-1);
                }
#ifdef AUDIO_ENABLED
                if (event.key.keysym.sym == KEY_MUSIC) {
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

        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
        SDL_SetAlpha(image, SDL_SRCALPHA, 255 - image_alpha);
        SDL_BlitSurface(image, &src, screen, &dest);
        SDL_FillRect(screen, &sel_dest[0], 0); //SDL_MapRGB(surface->format, 0, 0, 0));
        SDL_BlitSurface(image, &sel[0], screen, &sel_dest[selection]);
        Flip_Titus();
        titus_sleep();

#ifdef AUDIO_MIKMOD_SINGLETHREAD
        checkmodule();
#endif

#ifdef AUDIO_SDL_MIXER
        checkaudio();
#endif

    }

    return (curlevel);

}

int enterpassword()
{
	
#ifdef DREAMCAST
	maple_device_t *cont;
    cont_state_t *state;
	struct button_dc
	{
		unsigned char time;
		unsigned char state;
	} buttons[6];
	unsigned char j = 0;
	long button_to_press = 0;
	
	for (j=0;j<6;j++)
	{	
		buttons[j].state = 2;
		buttons[j].time = -5;
	}
#endif
    int retval;
    char code[] = "____";
    int i;
    SDL_Event event;
    char tmpchar;

    SDL_FillRect(screen, NULL, 0);
    Flip_Titus();

    SDL_Print_Text("CODE", 111, 80);

#if defined(_DINGUX) || defined(DREAMCAST) || defined(NSPIRE)
    int index = 0;
    int counter = 0;
#endif

    for (i = 0; i < 4; ) {
		
#ifdef DREAMCAST
		cont  = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
		state = (cont_state_t *) maple_dev_status(cont);

		for (j=0;j<5;j++)
		{	
			if (j==0) button_to_press = CONT_DPAD_UP;
			else if (j==1) button_to_press = CONT_DPAD_DOWN;
			else if (j==2) button_to_press = CONT_DPAD_LEFT;
			else if (j==3) button_to_press = CONT_DPAD_RIGHT;
			else button_to_press = CONT_A;
					
			switch (buttons[j].state)
			{
				case 0:
					if (state->buttons & button_to_press)
					{
						buttons[j].state = 1;
						buttons[j].time = 0;
					}
				break;
				
				case 1:
					buttons[j].time++;
					
					if (buttons[j].time > 0)
					{
						buttons[j].state = 2;
						buttons[j].time = 0;
					}
				break;
				
				case 2:
					if (!(state->buttons & button_to_press))
					{
						buttons[j].state = 3;
						buttons[j].time = 0;
					}
				break;
				
				case 3:
					buttons[j].time++;
					
					if (buttons[j].time > 0)
					{
						buttons[j].state = 0;
						buttons[j].time = 0;
					}
				break;
			}   

		}

		if ((buttons[0].state == 1) || (buttons[2].state == 1))
		{
            index++;
            if (index > 15)
                index = 0;
                
			buttons[0].state = 2;
			buttons[2].state = 2;
		}       
		else if ((buttons[1].state == 1) || (buttons[3].state == 1))
		{
			index--;
			if (index < 0)
				index = 15;
			buttons[1].state = 2;
			buttons[3].state = 2;
		}   
		else if (buttons[4].state == 1)
		{
			if (index < 10)
				code[i] = index + CHAR_0;
			else
				code[i] = index - 10 + CHAR_A;
				
			i++;
			index = 0;
			buttons[4].state = 2;
		} 
#endif
		
        while(SDL_PollEvent(&event)) { //Check all events
            if (event.type == SDL_QUIT) {
                return (-1);
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    return (-1);
                }
#if defined(_DINGUX) || defined(NSPIRE)
                if (event.key.keysym.sym == KEY_UP) {
                    index++;
                    if (index > 15) {
                        index = 0;
                    }
                } else if (event.key.keysym.sym == KEY_DOWN) {
                    index--;
                    if (index < 0) {
                        index = 15;
                    }
                } else if (event.key.keysym.sym == KEY_SPACE) {
                    if (index < 10) {
                        code[i] = index + CHAR_0;
                    } else {
                        code[i] = index - 10 + CHAR_A;
                    }
                    i++;
                    index = 0;
                }
                else if (event.key.keysym.sym == SDLK_RETURN) {
                    if (index < 10) {
                        code[i] = index + CHAR_0;
                    } else {
                        code[i] = index - 10 + CHAR_A;
                    }
                    i++;
                    index = 0;
                }
                else if (event.key.keysym.sym == SDLK_LCTRL) {
                    if (index < 10) {
                        code[i] = index + CHAR_0;
                    } else {
                        code[i] = index - 10 + CHAR_A;
                    }
                    i++;
                    index = 0;
                }
#else

                if ((event.key.keysym.unicode & 0xFF80) == 0) {
                    tmpchar = (char)(event.key.keysym.unicode & 0x007F);

                    if ((tmpchar >= CHAR_0) && (tmpchar <= CHAR_9))
                        code[i++] = tmpchar;

                    if ((tmpchar >= CHAR_a) && (tmpchar <= CHAR_f))
                        tmpchar -= (CHAR_a - CHAR_A);

                    if ((tmpchar >= CHAR_A) && (tmpchar <= CHAR_F))
                        code[i++] = tmpchar;

                }
#endif

#ifdef AUDIO_ENABLED
                if (event.key.keysym.sym == KEY_MUSIC) {
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
        
#if defined(_DINGUX) || defined(DREAMCAST) || defined(NSPIRE)
        if (i < 4) {
            counter++;
            if (counter > 10) {
                counter = 0;
            } else if (counter > 5) {
                code[i] = *"_";
            } else {
                if (index < 10) {
                    code[i] = index + CHAR_0;
                } else {
                    code[i] = index - 10 + CHAR_A;
                }
            }
        }
#endif
        SDL_Print_Text(code, 159, 80);
        Flip_Titus();
        titus_sleep();

#ifdef AUDIO_MIKMOD_SINGLETHREAD
        checkmodule();
#endif

#ifdef AUDIO_SDL_MIXER
        checkaudio();
#endif

    }

    for (i = 0; i < levelcount; i++) {
        if (strcmp (code, levelcode[i]) == 0) {
            SDL_Print_Text("LEVEL", 103, 104);
            sprintf(code, "%d", i + 1);
            SDL_Print_Text(code, 199 - 8 * strlen(code), 104);
            Flip_Titus();
            retval = waitforbutton();

            if (retval < 0)
                return retval;

            SDL_FillRect(screen, NULL, 0);
            Flip_Titus();

            return (i + 1);
        }
    }
  
    SDL_Print_Text("!  WRONG CODE  !", 87, 104);
    Flip_Titus();
    retval = waitforbutton();

    SDL_FillRect(screen, NULL, 0);
    Flip_Titus();
    return (retval);
}
