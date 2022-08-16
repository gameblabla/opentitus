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

/* scroll.c
 * Scroll functions
 */

#include <stdio.h>
#include <stdlib.h>
#include "SDL/SDL.h"
#include "level.h"
#include "globals.h"
#include "definitions.h"
#include "backbuffer.h"
#include "scroll.h"

static uint8 BARRYCENTRE(TITUS_level *level);
static int REFRESH_COLUMNS(TITUS_level *level, int8 column);
static int REFRESH_LINE(TITUS_level *level, int8 line);
bool L_SCROLL(TITUS_level *level);
bool R_SCROLL(TITUS_level *level);
bool U_SCROLL(TITUS_level *level);
bool D_SCROLL(TITUS_level *level);

void X_ADJUST(TITUS_level *level);
void Y_ADJUST(TITUS_level *level);

void X_ADJUST(TITUS_level *level);
void Y_ADJUST(TITUS_level *level);

float clamp(float x, float lowerlimit, float upperlimit) {
    if (x < lowerlimit)
        x = lowerlimit;
    if (x > upperlimit)
        x = upperlimit;
    return x;
}

float smootherstep(float edge0, float edge1, float x) {
    // Scale, and clamp x to 0..1 range
    x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    // Evaluate polynomial
    return x * x * x * (x * (x * 6 - 15) + 10);
}


float ideal_camera_position(TITUS_level *level) {
    if (!level->player.sprite.flipped) {
        return 60.0f;
    }
    else {
        return -60.0f;
    }
}



int scroll(TITUS_level *level) {
    //Scroll screen and update tile animation
    loop_cycle++; //Cycle from 0 to 3
    if (loop_cycle > 3) {
        loop_cycle = 0;
    }
    if (loop_cycle == 0) { //Every 4th call
        tile_anim++; //Cycle tile animation (0-1-2)
        if (tile_anim > 2) {
            tile_anim = 0;
        }
    }
    //Scroll
    if (!NOSCROLL_FLAG) {
        X_ADJUST(level);
        Y_ADJUST(level);
    }
    
    return 0;
}

void X_ADJUST(TITUS_level *level) {
    TITUS_player *player = &(level->player);
    g_scroll_x = true;

    static float camera_offset = 0.0f;
    int16 target_camera_offset = ideal_camera_position(level);
    if(camera_offset < target_camera_offset) {
        camera_offset += 3.0;
    }
    else if(camera_offset > target_camera_offset) {
        camera_offset -= 3.0;
    }
    int real_camera_offset = smootherstep(-60, 60, camera_offset) * 120 - 60;

    // clamp player position to level bounds
    int16 player_position = player->sprite.x;
    int16 camera_position = player_position + real_camera_offset;

    // left side of the map
    if(camera_position < 160) {
        camera_position = 160;
    }

    // right side of the map, or a weird arbitrary divide on the right
    int16 rlimit;
    if(player_position > XLIMIT * 16) {
        rlimit = (level->width * 16 - 160);
    }
    else {
        rlimit = (XLIMIT * 16 - 160);
    }
    if(camera_position > rlimit || player_position > rlimit) {
        camera_position = rlimit;
    }

    int16 camera_screen_px = camera_position - BITMAP_X * 16;
    int16 scroll_px_target = 160;
    int16 scroll_offset_x = scroll_px_target - camera_screen_px;
    int16 tile_offset_x = scroll_offset_x / 16;
    int16 px_offset_x = scroll_offset_x % 16;
    if(tile_offset_x < 0) {
        BITMAP_X ++;
        g_scroll_px_offset = px_offset_x;
        g_scroll_x = true;
    }
    else if (tile_offset_x > 0) {
        BITMAP_X --;
        g_scroll_px_offset = px_offset_x;
        g_scroll_x = true;
    }
    else {
        g_scroll_px_offset = scroll_offset_x;
        g_scroll_x = false;
    }
}

void Y_ADJUST(TITUS_level *level) {
    TITUS_player *player = &(level->player);
    if (player->sprite.speedY == 0) {
        g_scroll_y = false;
    }
    int16 pstileY = (player->sprite.y >> 4) - BITMAP_Y; //Player screen tile Y (0 to 11)
    if (!g_scroll_y) {
        if ((player->sprite.speedY == 0) &&
          (LADDER_FLAG == 0)) {
            if (pstileY >= screen_height - 1) {
                g_scroll_y_target = screen_height - 2;
                g_scroll_y = true;
            } else if (pstileY <= 2) {
                g_scroll_y_target = screen_height - 3;
                g_scroll_y = true;
            }
        } else {
            if (pstileY >= screen_height - 2) { //The player is at the bottom of the screen, scroll down!
                g_scroll_y_target = 3;
                g_scroll_y = true;
            } else if (pstileY <= 2) { //The player is at the top of the screen, scroll up!
                g_scroll_y_target = screen_height - 3;
                g_scroll_y = true;
            }
        }
    }

    if ((player->sprite.y <= ((ALTITUDE_ZERO + screen_height) << 4)) && //If the player is above the horizontal limit
      (BITMAP_Y > ALTITUDE_ZERO + 1)) { //... and the screen have scrolled below the the horizontal limit
        if (U_SCROLL(level)) { //Scroll up
            g_scroll_y = false;
        }
    } else if ((BITMAP_Y > ALTITUDE_ZERO - 5) && //If the screen is less than 5 tiles above the horizontal limit
      (BITMAP_Y <= ALTITUDE_ZERO) && //... and still above the horizontal limit
      (player->sprite.y + (7 * 16) > ((ALTITUDE_ZERO + screen_height) << 4))) {
        if (D_SCROLL(level)) { //Scroll down
            g_scroll_y = false;
        }
    } else if (g_scroll_y) {
        if (g_scroll_y_target == pstileY) {
            g_scroll_y = false;
        } else if (g_scroll_y_target > pstileY) {
            if (U_SCROLL(level)) {
                g_scroll_y = false;
            }
        } else if ((player->sprite.y <= ((ALTITUDE_ZERO + screen_height) << 4)) && //If the player is above the horizontal limit
          (BITMAP_Y > ALTITUDE_ZERO)) { //... and the screen is below the horizontal limit
            g_scroll_y = false; //Stop scrolling
        } else {
            if (D_SCROLL(level)) { //Scroll down
                g_scroll_y = false;
            }
        }
    }
}



static uint8 BARRYCENTRE(TITUS_level *level) {
    //If an enemy is behind the player, max. 12.5 tiles away horizontally, scroll until player is in the middle
    //If not, scroll until player is in the 3rd screen tile
    int16 enemy_left, i;
    for (i = 0; i < level->enemycount; i++) {
        if (!level->enemy[i].sprite.enabled || !level->enemy[i].visible) {
            continue;
        }
        enemy_left = (level->enemy[i].sprite.x < level->player.sprite.x); //True if enemy is left for the player
        if ((enemy_left != level->player.sprite.flipped) && //Enemy is behind the player
          (abs(level->enemy[i].sprite.x - level->player.sprite.x) < 200)) { //Enemy is max. 12.5 tiles away
            return (screen_width / 2);
        }
    }
    if (!level->player.sprite.flipped) {
        return 3;
    } else {
        return (screen_width - 3);
    }
}


bool L_SCROLL(TITUS_level *level) {
    //Scroll left
    if (BITMAP_X == 0) {
        return true; //Stop scrolling
    }
    BITMAP_X--; //Scroll 1 tile left
    if (BITMAP_XM <= 0) {
        BITMAP_XM = screen_width;
    }
    BITMAP_XM--; //BITMAP_XM range: 0 to 19
    REFRESH_COLUMNS(level, 0);
    return false; //Continue scrolling
}


bool R_SCROLL(TITUS_level *level) {
    //Scroll right
    uint8 maxX;
    if (((level->player.sprite.x >> 4) - screen_width) > XLIMIT) { //Scroll limit
        maxX = level->width - screen_width; //256 - 20
    } else {
        maxX = XLIMIT;
    }
    if (BITMAP_X >= maxX) {
        return true; //Stop scrolling
    }
    BITMAP_X++; //Increase pointer
    BITMAP_XM++; //BITMAP_XM range: 0 to 19
    if (BITMAP_XM >= screen_width) {
        BITMAP_XM = 0;
    }
    REFRESH_COLUMNS(level, screen_width - 1);
    return false;
}


bool U_SCROLL(TITUS_level *level) {
    //Scroll up
    if (BITMAP_Y == 0) {
        return true;
    }
    BITMAP_Y--; //Scroll 1 tile up
    if (BITMAP_YM <= 0) {
        BITMAP_YM = screen_height;
    }
    BITMAP_YM--; //BITMAP_YM range: 0 to 11
    REFRESH_LINE(level, 0);
    return false;
}


bool D_SCROLL(TITUS_level *level) {
    //Scroll down
    if (BITMAP_Y >= (level->height - screen_height)) { //The screen is already at the bottom
        return true; //Stop scrolling
    }
    BITMAP_Y++; //Increase pointer
    BITMAP_YM++; //BITMAP_YM range: 0 to 11
    if (BITMAP_YM >= screen_height) {
        BITMAP_YM = 0;
    }
    REFRESH_LINE(level, screen_height - 1);
    return false;
}


static int REFRESH_COLUMNS(TITUS_level *level, int8 column) {
    //The screen is scrolled left or right, redraw one column on the hidden tile screen (located in OFS_SCREENM)
    //screen_offset: 0 if scroll left; 19 (screen_width - 1) if scroll right
    uint8 tmpX = BITMAP_X + column; //left column (+0) or right column (+19)
    uint8 tmpY = BITMAP_Y;
    uint8 tmpYM = BITMAP_YM;
    uint8 tmpXM = BITMAP_XM + column;
    uint8 i, cur_tile;
    if (tmpXM >= screen_width) {
        tmpXM -= screen_width;
    }
    for (i = 0; i < screen_height; i++) {
        cur_tile = level->tilemap[tmpY][tmpX];
        PERMUT_FLAG = PERMUT_FLAG | level->tile[cur_tile].animated;
        DISPLAY_CHAR (level, cur_tile, tmpYM, tmpXM);
        tmpY++;
        tmpYM++;
        if (tmpYM >= screen_height) {
            tmpYM -= screen_height;
        }
    }
    return 0;
}

static int REFRESH_LINE(TITUS_level *level, int8 line) {
    //The screen is scrolled up or down, redraw one line on the hidden tile screen
    //screen_offset: 0 if scroll up; 11 (screen_height - 1) if scroll down
    uint8 tmpX = BITMAP_X;
    uint8 tmpY = BITMAP_Y + line; //top column (+0) or bottom column (+11)
    uint8 tmpYM = BITMAP_YM + line;
    uint8 tmpXM = BITMAP_XM;
    uint8 i, cur_tile;
    if (tmpYM >= screen_height) {
        tmpYM -= screen_height;
    }
    for (i = 0; i < screen_width; i++) {
        cur_tile = level->tilemap[tmpY][tmpX];
        PERMUT_FLAG = PERMUT_FLAG | level->tile[cur_tile].animated;
        DISPLAY_CHAR (level, cur_tile, tmpYM, tmpXM);
        tmpX++;
        tmpXM++;
        if (tmpXM >= screen_width) {
            tmpXM -= screen_width;
        }
    }
    return 0;
}

void DISPLAY_CHAR(TITUS_level *level, uint8 tile, uint8 y, uint8 x) {
    //Update the tile surface
    SDL_Rect src, dest;
    src.x = 0;
    src.y = 0;
    src.w = 16;
    src.h = 16;
    dest.x = x * 16;
    dest.y = y * 16;
    dest.w = src.w;
    dest.h = src.h;
    SDL_BlitSurface(level->tile[level->tile[tile].animation[tile_anim]].tiledata, &src, tilescreen, &dest);
}
