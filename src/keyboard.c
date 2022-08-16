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

/* keyboard.c
 * Keyboard functions
 */

#include "SDL/SDL.h"
#include "keyboard.h"
#include "globals.h"
#include "common.h"

//Probably not the best way, but it works...
#define HAVE_CONFIG_H 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef AUDIO_ENABLED
#include "audio.h"
#endif

int waitforbutton() {
#ifdef DREAMCAST
	struct button_dc
	{
		unsigned char time;
		unsigned char state;
	} buttons[3];
	unsigned char j = 0;
	long button_to_press = 0;
	
	for (j=0;j<3;j++)
	{	
		buttons[j].state = 2;
		buttons[j].time = -5;
	}
#endif
    SDL_Event event;
    int waiting = 1;
    while (waiting > 0)
    {

#ifdef DREAMCAST
		POLL_CONTROLS
		for (j=0;j<3;j++)
		{	
			if (j==0) button_to_press = CONT_A;
			else button_to_press = CONT_START;

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
		if (buttons[0].state == 1 || buttons[1].state == 1)
		{
			waiting = 0;
		}
#endif
		
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                waiting = -1;

#ifdef __vita__
            if (event.type == SDL_JOYBUTTONDOWN) {
                if (event.jbutton.button == KEY_RETURN || event.jbutton.button == KEY_ENTER || event.jbutton.button == KEY_SPACE)
#else
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == KEY_RETURN || event.key.keysym.sym == KEY_ENTER || event.key.keysym.sym == KEY_SPACE)
#endif
                    waiting = 0;

#ifdef __vita__
                if (event.jbutton.button == KEY_ESC)
#else
                if (event.key.keysym.sym == SDLK_ESCAPE)
#endif
                    waiting = -1;

#ifdef AUDIO_ENABLED
#if __vita__
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
		titus_sleep();
#ifdef AUDIO_MIKMOD_SINGLETHREAD
        checkmodule();
#endif

#ifdef AUDIO_SDL_MIXER
        checkaudio();
#endif

    }
    return (waiting);
}

