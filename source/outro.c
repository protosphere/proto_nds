#include <nds.h>
#include <stdio.h>

#include "outro.h"
#include "title_console.h"
#include "distorter.h"
#include "grid.h"

void Outro_ConfigureHardware(); 
void Outro_LoadResources(int grid_bg_main_id, int grid_bg_sub_id);
void Outro_Draw(Distorter *distorter_main, Distorter *distorter_sub);

void Outro_Run() {
	Outro_ConfigureHardware();

	/* Enable background blending */
	REG_BLDCNT = BLEND_SRC_BG0 | BLEND_SRC_BG3 | BLEND_FADE_BLACK;
	REG_BLDCNT_SUB = BLEND_SRC_BG0 | BLEND_SRC_BG3 | BLEND_FADE_BLACK;

	/* Configure background engines */	
	int bg0_main = bgInit(0, BgType_Text4bpp, BgSize_T_256x256, 1, 0);
	int bg0_sub = bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 1, 0);

	/* Set background 0 to display below background 1 */
	bgSetPriority(bg0_main, 1);
	bgSetPriority(bg0_sub, 1);

	Outro_LoadResources(bg0_main, bg0_sub);

	/* Set up distortion effects */
	Distorter *distorter_main = malloc(sizeof(Distorter));
	Distorter_Init(distorter_main, 0, DistorterEngineMain, -(20 << 8), 96, 4 << 8);

	Distorter *distorter_sub = malloc(sizeof(Distorter));
	Distorter_Init(distorter_sub, 3, DistorterEngineSub, -(16 << 8), 96, 4 << 8);

	/* Create a console for the main display and print the message to it */
	PrintConsole display_console_main;	
	TitleConsole_Init(&display_console_main, 3, true, 2, 1);

	consoleSelect(&display_console_main);
	iprintf("THE\nEND");

	/* Create a console for the sub display and print the message to it */
	PrintConsole display_console_sub;	
	TitleConsole_Init(&display_console_sub, 3, false, 2, 1);

	consoleSelect(&display_console_sub);
	iprintf("MORE\nSOON");

	/* Offset each console so they are vertically centered */
	bgSetScroll(display_console_main.bgId, 0, -16);
	bgSetScroll(display_console_sub.bgId, 0, -16);
	bgUpdate();

	/* Fade in */
	for (int blend_factor = 0xf; blend_factor >= 0x0; blend_factor--) {
		Outro_Draw(distorter_main, distorter_sub);

		REG_BLDY = blend_factor;
		REG_BLDY_SUB = blend_factor;
	}

	/* Main stage loop. Exit when a key is pressed */
	while (!(keysDown() & KEY_START)) {
		Outro_Draw(distorter_main, distorter_sub);

		scanKeys();
	}

	Distorter_Free(distorter_main);
	free(distorter_main);

	Distorter_Free(distorter_sub);
	free(distorter_sub);
}

void Outro_ConfigureHardware()
{
	/* Configure video hardware */
	videoSetMode(MODE_3_2D);

	vramSetBankF(VRAM_F_MAIN_BG_0x06000000);
}

void Outro_LoadResources(int grid_bg_main_id, int grid_bg_sub_id)
{
	/* Copy tile, map and palette data into VRAM */
	dmaCopy(gridTiles, bgGetGfxPtr(grid_bg_main_id), gridTilesLen);
	dmaCopy(gridMap, bgGetMapPtr(grid_bg_main_id), gridMapLen);
	dmaCopy(gridPal, BG_PALETTE + 0x2, gridPalLen);

	dmaCopy(gridTiles, bgGetGfxPtr(grid_bg_sub_id), gridTilesLen);
	dmaCopy(gridMap, bgGetMapPtr(grid_bg_sub_id), gridMapLen);
	dmaCopy(gridPal, BG_PALETTE_SUB + 0x2, gridPalLen);
}

void Outro_Draw(Distorter *distorter_main, Distorter *distorter_sub)
{
	swiWaitForVBlank();

	Distorter_Step(distorter_main);
	Distorter_Draw(distorter_main);

	Distorter_Step(distorter_sub);
	Distorter_Draw(distorter_sub);
}
