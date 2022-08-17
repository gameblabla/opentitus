#include <stdint.h>
#include "SDL/SDL.h"
#include "definitions.h"
#include "level.h"
#include "dingoo.h"

#ifdef DREAMCAST
#include <kos.h>
#include <dc/sd.h>
#include <kos/blockdev.h>
#include <fat/fs_fat.h>
#include <ext2/fs_ext2.h>
maple_device_t *cont;
cont_state_t *state;
uint8 partition_type;
kos_blockdev_t sd_dev;
int is_sdcard = 0;
static uint32_t p_buttons = 0;   //previous buttons
//This simple function will check if a button has been pressed
int buttonPressed_DC(int key) {
  if (state->buttons & key && !(p_buttons & key)) { //Compares to the current state and the previous buttons states.
      p_buttons |= state->buttons; // If true, it will "add" the buttons to the pressed buttons
      return(1);
  }
  p_buttons &= state->buttons;
  return(0);
}
#endif

uint8 RESETLEVEL_FLAG;
bool GAMEOVER_FLAG; //triggers a game over
uint8 BAR_FLAG; //timer for health bar
bool X_FLAG; //true if left or right key is pressed
bool Y_FLAG; //true if up or down key is pressed
uint8 CHOC_FLAG; //headache timer
uint8 action; //player sprite array
uint8 KICK_FLAG; //hit/burn timer
bool GRANDBRULE_FLAG; //If set, player will be "burned" when hit (fireballs)
bool LADDER_FLAG; //True if in a ladder
bool PRIER_FLAG; //True if player is forced into kneestanding because of low ceiling
uint8 SAUT_FLAG; //6 if free fall or in the middle of a jump, decremented if on solid surface. Must be 0 to initiate a jump.
uint8 LAST_ORDER; //Last action (kneestand + jump = silent walk)
uint8 FURTIF_FLAG; //Silent walk timer
bool DROP_FLAG; //True if an object is throwed forward
bool DROPREADY_FLAG;
bool CARRY_FLAG; //true if carrying something (add 16 to player sprite)
bool POSEREADY_FLAG;
uint8 ACTION_TIMER; //Frames since last action change
//TITUS_sprite sprite; //Player sprite
//TITUS_sprite sprite2; //Secondary player sprite (throwed objects, "hit" when object hits an enemy, smoke when object hits the floor)
uint8 INVULNERABLE_FLAG; //When non-zero, boss is invulnerable
uint8 TAPISFLY_FLAG; //When non-zero, the flying carpet is flying
uint8 CROSS_FLAG; //When non-zero, fall through certain floors (after key down)
uint8 GRAVITY_FLAG; //When zero, skip object gravity function
uint8 FUME_FLAG; //Smoke when object hits the floor
Uint8 *keystate; //Keyboard state
uint8 LIFE; //Lives
uint8 YFALL;
bool POCKET_FLAG;
bool PERMUT_FLAG; //If false, there are no animated tiles on the screen?
uint8 loop_cycle; //Increased every loop in game loop
uint8 tile_anim; //Current tile animation (0-1-2), changed every 4th game loop cycle
uint8 BITMAP_X; //Screen offset (X) in tiles
uint8 BITMAP_XM; //Point to the left tile in the tile screen (0 to 19)
uint8 BITMAP_Y; //Screen offset (Y) in tiles
uint8 BITMAP_YM; //Point to the top tile in the tile screen (0 to 11)
int16 XLIMIT; //The engine will not scroll past this tile before the player have crossed the line (X)

bool g_scroll_x; //If true, the screen will scroll in X
int16 g_scroll_x_target; //If scrolling: scroll until player is in this tile (X)
int16 g_scroll_px_offset;
bool g_scroll_y; //If true, the screen will scroll in Y
uint8 g_scroll_y_target; //If scrolling: scroll until player is in this tile (Y)

uint8 ALTITUDE_ZERO; //The engine will not scroll below this tile before the player have gone below (Y)
int LAST_CLOCK; //Used for fixed framerate
uint16 IMAGE_COUNTER; //Increased every loop in game loop (0 to 0x0FFF)
int8 SENSX; //1: walk right, 0: stand still, -1: walk left, triggers the ACTION_TIMER if it changes
uint8 SAUT_COUNT; //Incremented from 0 to 3 when accelerating while jumping, stop acceleration upwards if >= 3
bool NOSCROLL_FLAG;
bool NEWLEVEL_FLAG; //Finish a level
uint8 BIGNMI_NBR; //Number of bosses that needs to be killed to finish
uint8 TAUPE_FLAG; //Used for enemies walking and popping up
uint8 TAPISWAIT_FLAG; //Flying carpet state
uint8 SEECHOC_FLAG; //Counter when hit
bool NFC_FLAG; //Skip NO_FAST_CPU
uint8 BIGNMI_POWER; //Lives of the boss
bool boss_alive; //True if the boss is alive
uint8 AUDIOMODE;
Uint32 sky_colour;

bool GODMODE; //If true, the player will not interfere with the enemies
bool NOCLIP; //If true, the player will move noclip
bool DISPLAYLOOPTIME; //If true, display loop time in milliseconds
uint8 LOOPTIME; //Loop time
uint8 SUBTIME[16]; //Sub time
uint16 FPS; //Frames pr second
uint16 FPS_LAST; //Frames pr second
uint16 LAST_CLOCK_CORR; //Correction to LAST_CLOCK

typedef struct {
    bool enabled;
    uint16 NUM;
} SPRITE;

SPRITE sprites[256]; 

typedef struct {
    bool enabled;
    uint16 NUM;
} SPRITEDATA;

SPRITEDATA spritedata[256]; 

uint16 level_code[16];

#ifdef __vita__
SDL_Joystick* joystick;
int godtick;
#endif



/* original.h */

#include "original.h"

SDL_Color orig_palette_colour[16];
SDL_Color orig_palette_level_colour[16];
SDL_Color orig_palette_font_colour[16];

uint8 orig_sky_colour[16];

uint8 spritewidth[SPRITECOUNT];
uint8 spriteheight[SPRITECOUNT];
uint8 spritecollwidth[SPRITECOUNT];
uint8 spritecollheight[SPRITECOUNT];
uint8 spriterefwidth[SPRITECOUNT];
uint8 spriterefheight[SPRITECOUNT];

int16 anim_player[ANIM_PLAYER_COUNT][ANIM_PLAYER_MAX];
int16 anim_enemy[NMI_ANIM_TABLE_COUNT];
uint8 NMI_POWER[ORIG_LEVEL_COUNT];
uint8 LEVEL_MUSIC[ORIG_LEVEL_COUNT];

int16 anim_zoubida[ORIG_ANIM_MAX];
int16 anim_moktar[ORIG_ANIM_MAX];
int16 anim_smoke[ORIG_ANIM_MAX];
int16 COEUR_POS[ORIG_ANIM_MAX * 2];

uint8 object_maxspeedY[ORIG_OBJECT_COUNT];
bool object_support[ORIG_OBJECT_COUNT]; //not support/support
bool object_bounce[ORIG_OBJECT_COUNT]; //not bounce/bounce against floor + player bounces (ball, all spring, yellow stone, squeezed ball, skateboard)
bool object_gravity[ORIG_OBJECT_COUNT]; //no gravity on throw/gravity (ball, all carpet, trolley, squeezed ball, garbage, grey stone, scooter, yellow bricks between the statues, skateboard, cage)
bool object_droptobottom[ORIG_OBJECT_COUNT]; //on drop, lands on ground/continue below ground(cave spikes, rolling rock, ambolt, safe, dead man with helicopter)
bool object_no_damage[ORIG_OBJECT_COUNT]; //weapon/not weapon(cage)

/* end of original.h */

/* settings.h */

char audiofile[256];
char spritefile[256];
char levelfiles[16][256]; //16 levels in moktar, 15 levels in titus
char tituslogofile[256];
int tituslogoformat;
char titusintrofile[256];
int titusintroformat;
char titusmenufile[256];
int titusmenuformat;
char titusfinishfile[256];
int titusfinishformat;
char fontfile[256];
int levelcount;
int devmode;
int reswidth;
int resheight;
int bitdepth;
int ingamewidth;
int ingameheight;
int videomode;
int game;

char levelcode[16][5];
char leveltitle[16][41];

char moduleintrofile[256]; //.mod file
int  moduleintrofileloop; //loop info
char moduleprelevelfile[256];
int  moduleprelevelfileloop;
char modulelevelfile[6][256]; //6 different level files
int  modulelevelfileloop[6];
char modulegameoverfile[256];
int  modulegameoverfileloop;
char modulelevel[16]; //Link to modulelevelfiles

/* end of settings.h */

/* backbuffer.c */

SDL_Surface *screen; //Backbuffer
//SDL_Surface *tilescreen; //Tile screen
uint8 tile_screen[12][20];

/* end of backbuffer.c */

/* tituserror.c */

char lasterror[200];
int lasterrornr; //Only to be used when needed, f.ex. when return value is not int (f.ex. in function SDL_Text) (maybe this always should be used?)

/* end of tituserror.c */

/* fonts.h */

typedef struct _TITUS_font TITUS_font;

struct _TITUS_font {
    uint8 type[256]; //0: not in use, 1: malloced sub, 2: malloced surface, 3: surface pointer, 4: invalid UTF-8
    void *sub[256]; //May be malloced
};

TITUS_font *font; //Malloced
SDL_Surface *font_undefined; //Pointer

/* end of fonts.h */
