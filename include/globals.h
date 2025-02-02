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

/* globals.h
 * Global variables
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#ifdef DREAMCAST
#include <kos.h>
#include <dc/sd.h>
#include <kos/blockdev.h>
#include <fat/fs_fat.h>
#include <ext2/fs_ext2.h>
#include <SDL/SDL.h>
#include <SDL/SDL_dreamcast.h>
#endif
#include <SDL/SDL.h>
#include "definitions.h"
#include "level.h"
#include "dingoo.h"

// Global controls

#ifdef DREAMCAST
extern maple_device_t *cont;
extern cont_state_t *state;
extern uint8 partition_type;
extern kos_blockdev_t sd_dev;
extern int is_sdcard;
#define JUMP_BUTTON (state->buttons & CONT_DPAD_UP || state->buttons & CONT_A)
#define UP_BUTTON (state->buttons & CONT_DPAD_UP || state->joyy < 0)
#define DOWN_BUTTON (state->buttons & CONT_DPAD_DOWN || state->joyy > 0)
#define SPACE_BUTTON (state->buttons & CONT_A)
#define LEFT_BUTTON (state->buttons & CONT_DPAD_LEFT || state->joyx < 0)
#define RIGHT_BUTTON (state->buttons & CONT_DPAD_RIGHT || state->joyx > 0)

#define THROW_BUTTON (state->buttons & CONT_B)
#define PAUSE_BUTTON (state->buttons & CONT_START)

#define POLL_CONTROLS cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER); state = (cont_state_t *) maple_dev_status(cont); 

extern int buttonPressed_DC(int key);
#else
#define JUMP_BUTTON (keystate[KEY_UP] || buttonPressed(KEY_UP)) || (keystate[KEY_JUMP] || buttonPressed(KEY_JUMP))
#define UP_BUTTON (keystate[KEY_UP] || buttonPressed(KEY_UP))
#define DOWN_BUTTON (keystate[KEY_DOWN] || buttonPressed(KEY_DOWN))
#define SPACE_BUTTON (keystate[KEY_SPACE] || buttonPressed(KEY_SPACE))
#define LEFT_BUTTON (keystate[KEY_LEFT] || buttonPressed(KEY_LEFT))
#define RIGHT_BUTTON (keystate[KEY_RIGHT] || buttonPressed(KEY_RIGHT))
#define THROW_BUTTON (keystate[KEY_SPACE])
#define PAUSE_BUTTON (keystate[KEY_RETURN])
#define POLL_CONTROLS 
#endif

//To simplify porting:
#ifdef DINGUX

/*
#define DINGOO_BUTTON_UP            SDLK_UP
#define DINGOO_BUTTON_DOWN          SDLK_DOWN
#define DINGOO_BUTTON_RIGHT         SDLK_RIGHT
#define DINGOO_BUTTON_LEFT          SDLK_LEFT
#define DINGOO_BUTTON_R             SDLK_BACKSPACE
#define DINGOO_BUTTON_L             SDLK_TAB
#define DINGOO_BUTTON_A             SDLK_LCTRL
#define DINGOO_BUTTON_B             SDLK_LALT
#define DINGOO_BUTTON_X             SDLK_SPACE
#define DINGOO_BUTTON_Y             SDLK_LSHIFT
#define DINGOO_BUTTON_SELECT        SDLK_ESCAPE
#define DINGOO_BUTTON_START         SDLK_RETURN
#define DINGOO_BUTTON_END           SDLK_UNKNOWN
*/

#define KEY_F1          DINGOO_BUTTON_R //Loose a life
#define KEY_F2          SDLK_F2 //Game over, not in use


//#define KEY_STATUS      DINGOO_BUTTON_A //Energy + status page
#define KEY_STATUS 		SDLK_F12

#define KEY_LEFT        DINGOO_BUTTON_LEFT //Left
#define KEY_RIGHT       DINGOO_BUTTON_RIGHT //Right
#define KEY_UP          DINGOO_BUTTON_UP //Up
#define KEY_DOWN        DINGOO_BUTTON_DOWN //Down
#define KEY_JUMP        DINGOO_BUTTON_A //Jump
#define KEY_SPACE       DINGOO_BUTTON_B //Space
#define KEY_ENTER       DINGOO_BUTTON_START //Enter
#define KEY_RETURN      DINGOO_BUTTON_START //Return
#define KEY_ESC         DINGOO_BUTTON_SELECT //Quit
#define KEY_P           DINGOO_BUTTON_START //Toggle pause (A button)
#define KEY_CHEAT       DINGOO_BUTTON_L //Toggle cheat noclip/godmode
#define KEY_SKIPLEVEL   DINGOO_BUTTON_R //Skip level
#define KEY_DEBUG       DINGOO_BUTTON_Y //Toggle debug mode
#define KEY_MUSIC       DINGOO_BUTTON_Y //Toggle music

#define KEY_GODMODE SDLK_F1 //Toggle godmode
#define KEY_E SDLK_F3 //Display energy
#define KEY_F4 SDLK_F4 //Status page
#define KEY_NOCLIP SDLK_n //Toggle noclip


#elif __vita__

#define KEY_F1          SDLK_F1 //Loose a life, not in use
#define KEY_F2          SDLK_F2 //Game over, not in use
#define KEY_STATUS      0 //Energy + status page (TRIANGLE)
#define KEY_LEFT        7 //Left
#define KEY_RIGHT       9 //Right
#define KEY_UP          8 //Up
#define KEY_DOWN        6 //Down
#define KEY_JUMP        1 //Jump (CIRCLE)
#define KEY_SPACE       2 //Space (CROSS)
#define KEY_ENTER       11 //Enter (START)
#define KEY_RETURN      11 //Return (START)
#define KEY_ESC         10 //Quit (SELECT)
#define KEY_P           11 //Toggle pause (START)
#define KEY_CHEAT       4 //Toggle cheat noclip/godmode (LTRIGGER)
#define KEY_SKIPLEVEL   5 //Skip level (RTRIGGER)
#define KEY_DEBUG       3 //Toggle debug mode (SQUARE)
#define KEY_MUSIC       3 //Toggle music (SQUARE)

#else

#define KEY_F1 SDLK_F1 //Loose a life
#define KEY_F2 SDLK_F2 //Game over
#define KEY_E SDLK_e //Display energy
#define KEY_F4 SDLK_F4 //Status page
#define KEY_LEFT SDLK_LEFT //Left
#define KEY_RIGHT SDLK_RIGHT //Right
#define KEY_UP SDLK_UP //Up
#define KEY_DOWN SDLK_DOWN //Down
#define KEY_JUMP SDLK_UP //Up
#define KEY_SPACE SDLK_SPACE //Space
#define KEY_ENTER SDLK_KP_ENTER //Enter
#define KEY_RETURN SDLK_RETURN //Return
#define KEY_ESC SDLK_ESCAPE //Quit
#define KEY_P SDLK_p //Toggle pause
#define KEY_NOCLIP SDLK_n //Toggle noclip
#define KEY_GODMODE SDLK_g //Toggle godmode
#define KEY_DEBUG SDLK_d //Toggle debug mode
#define KEY_MUSIC SDLK_F3 //Toggle music

#endif


#define TEST_ZONE 4
#define MAX_X 4
#define MAX_Y 12
#define MAP_LIMIT_Y -1
#define S_LINES 12
#define S_COLUMNS 20
#define LIMIT_TIMER 22
#define ALERT_X 32
#define screen_width 20
#define screen_height 12
#define FIRST_OBJET 30
#define FIRST_NMI 101
#define MAXIMUM_BONUS 100
#define MAXIMUM_ENERGY 16
#define MAXIMUM_DTRP 4
#define GESTION_X 40
#define GESTION_Y 20
#define MAX_SPEED_DEAD 20

#define HFLAG_NOWALL 0
#define HFLAG_WALL 1
#define HFLAG_BONUS 2
#define HFLAG_DEADLY 3
#define HFLAG_CODE 4
#define HFLAG_PADLOCK 5
#define HFLAG_LEVEL14 6

#define FFLAG_NOFLOOR 0
#define FFLAG_FLOOR 1
#define FFLAG_SSFLOOR 2
#define FFLAG_SFLOOR 3
#define FFLAG_VSFLOOR 4
#define FFLAG_DROP 5
#define FFLAG_LADDER 6
#define FFLAG_BONUS 7
#define FFLAG_WATER 8
#define FFLAG_FIRE 9
#define FFLAG_SPIKES 10
#define FFLAG_CODE 11
#define FFLAG_PADLOCK 12
#define FFLAG_LEVEL14 13

#define CFLAG_NOCEILING 0
#define CFLAG_CEILING 1
#define CFLAG_LADDER 2
#define CFLAG_PADLOCK 3
#define CFLAG_DEADLY 4

#define uint8 unsigned char
#define uint16 unsigned short int
#define int8 signed char
#define int16 signed short int


extern uint8 RESETLEVEL_FLAG;
extern bool GAMEOVER_FLAG; //triggers a game over
extern uint8 BAR_FLAG; //timer for health bar
extern bool X_FLAG; //true if left or right key is pressed
extern bool Y_FLAG; //true if up or down key is pressed
extern uint8 CHOC_FLAG; //headache timer
extern uint8 action; //player sprite array
extern uint8 KICK_FLAG; //hit/burn timer
extern bool GRANDBRULE_FLAG; //If set, player will be "burned" when hit (fireballs)
extern bool LADDER_FLAG; //True if in a ladder
extern bool PRIER_FLAG; //True if player is forced into kneestanding because of low ceiling
extern uint8 SAUT_FLAG; //6 if free fall or in the middle of a jump, decremented if on solid surface. Must be 0 to initiate a jump.
extern uint8 LAST_ORDER; //Last action (kneestand + jump = silent walk)
extern uint8 FURTIF_FLAG; //Silent walk timer
extern bool DROP_FLAG; //True if an object is throwed forward
extern bool DROPREADY_FLAG;
extern bool CARRY_FLAG; //true if carrying something (add 16 to player sprite)
extern bool POSEREADY_FLAG;
extern uint8 ACTION_TIMER; //Frames since last action change

extern uint8 INVULNERABLE_FLAG; //When non-zero, boss is invulnerable
extern uint8 TAPISFLY_FLAG; //When non-zero, the flying carpet is flying
extern uint8 CROSS_FLAG; //When non-zero, fall through certain floors (after key down)
extern uint8 GRAVITY_FLAG; //When zero, skip object gravity function
extern uint8 FUME_FLAG; //Smoke when object hits the floor
extern Uint8 *keystate; //Keyboard state
extern uint8 LIFE; //Lives
extern uint8 YFALL;
extern bool POCKET_FLAG;
extern bool PERMUT_FLAG; //If false, there are no animated tiles on the screen?
extern uint8 loop_cycle; //Increased every loop in game loop
extern uint8 tile_anim; //Current tile animation (0-1-2), changed every 4th game loop cycle
extern uint8 BITMAP_X; //Screen offset (X) in tiles
extern uint8 BITMAP_XM; //Point to the left tile in the tile screen (0 to 19)
extern uint8 BITMAP_Y; //Screen offset (Y) in tiles
extern uint8 BITMAP_YM; //Point to the top tile in the tile screen (0 to 11)

extern int16 XLIMIT; //The engine will not scroll past this tile before the player have crossed the line (X)

extern bool g_scroll_x; //If true, the screen will scroll in X
extern int16 g_scroll_x_target; //If scrolling: scroll until player is in this tile (X)
extern int16 g_scroll_px_offset;
extern bool g_scroll_y; //If true, the screen will scroll in Y
extern uint8 g_scroll_y_target; //If scrolling: scroll until player is in this tile (Y)


extern uint8 ALTITUDE_ZERO; //The engine will not scroll below this tile before the player have gone below (Y)
extern int LAST_CLOCK; //Used for fixed framerate
extern uint16 IMAGE_COUNTER; //Increased every loop in game loop (0 to 0x0FFF)
extern int8 SENSX; //1: walk right, 0: stand still, -1: walk left, triggers the ACTION_TIMER if it changes
extern uint8 SAUT_COUNT; //Incremented from 0 to 3 when accelerating while jumping, stop acceleration upwards if >= 3
extern bool NOSCROLL_FLAG;
extern bool NEWLEVEL_FLAG; //Finish a level
extern uint8 BIGNMI_NBR; //Number of bosses that needs to be killed to finish
extern uint8 TAUPE_FLAG; //Used for enemies walking and popping up
extern uint8 TAPISWAIT_FLAG; //Flying carpet state
extern uint8 SEECHOC_FLAG; //Counter when hit
extern bool NFC_FLAG; //Skip NO_FAST_CPU
extern uint8 BIGNMI_POWER; //Lives of the boss
extern bool boss_alive; //True if the boss is alive
extern uint8 AUDIOMODE;
extern int scaling; //Scale the output, default: 1 (2: 2x scale (640x480))
extern Uint32 sky_colour;

extern bool AMIGA_LINES; //If true, draw amiga lines on the background
extern bool GODMODE; //If true, the player will not interfere with the enemies
extern bool NOCLIP; //If true, the player will move noclip
extern bool DISPLAYLOOPTIME; //If true, display loop time in milliseconds
extern uint8 LOOPTIME; //Loop time
extern uint8 SUBTIME[16]; //Sub time
extern uint16 FPS; //Frames pr second
extern uint16 FPS_LAST; //Frames pr second
extern uint16 LAST_CLOCK_CORR; //Correction to LAST_CLOCK

typedef struct {
    bool enabled;
    uint16 NUM;
} SPRITE;

extern SPRITE sprites[256]; 

typedef struct {
    bool enabled;
    uint16 NUM;
} SPRITEDATA;

extern SPRITEDATA spritedata[256]; 

extern uint16 level_code[16];

#ifdef __vita__
extern SDL_Joystick* joystick;
extern int godtick;
#endif

extern void Flip_Titus();

extern SDL_Surface* rl_screen;

#include "enemies.h"
#include "fonts.h"
#include "backbuffer.h"
#include "sprites.h"
#include "objects.h"
#include "audio.h"
#include "player.h"
#include "level.h"
#include "engine.h"
#include "definitions.h"
#include "keyboard.h"
#include "draw.h"
#include "gates.h"
#include "scroll.h"

#endif

