#include <nds.h>
#include <stdio.h>
#include <stdlib.h>

#include "stage2.h"
#include "rotozoomer.h"
#include "title_console.h"
#include "distorter.h"
#include "roto.h"
#include "grid.h"

void Stage2_ConfigureHardware(); 
void Stage2_LoadResources(int roto_bg_id, int grid_bg_id);
void Stage2_Draw(Rotozoomer *rotozoomer, Distorter *distorter);

void Stage2_Run() {
	Stage2_ConfigureHardware();

	/* Configure background engines */
	int bg3_main = bgInit(3, BgType_Rotation, BgSize_R_128x128, 1, 0);
	int bg0_sub = bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 1, 0);

	/* Enable background blending */
	REG_BLDCNT = BLEND_SRC_BG3 | BLEND_FADE_BLACK;
	REG_BLDCNT_SUB = BLEND_SRC_BG0 | BLEND_SRC_BG3 | BLEND_FADE_BLACK;

	/* Set sub background 0 to display below sub background 1 */
	bgSetPriority(bg0_sub, 1);

	Stage2_LoadResources(bg3_main, bg0_sub);

	/* Set up distortion effects */
	Distorter *distorter = malloc(sizeof(Distorter));
	Distorter_Init(distorter, 0, DistorterEngineSub, -(8 << 8), 96, 4 << 8);

	/* Create a rotozoomer for use on the main display */
	Rotozoomer *rotozoomer = malloc(sizeof(Rotozoomer));
	Rotozoomer_Init(rotozoomer, bg3_main, (2 << 8), (1 << 5));

	/* Create a console for the stage title and print the title to it */
	PrintConsole title_console;	
	TitleConsole_Init(&title_console, 3, false, 2, 1);

	bgSetScroll(title_console.bgId, 0, -16);

	iprintf(" ROTO\nZOOMER");

	/* Fade in */
	for (int blend_factor = 0xf; blend_factor >= 0x0; blend_factor--) {
		Stage2_Draw(rotozoomer, distorter);

		REG_BLDY = blend_factor;
		REG_BLDY_SUB = blend_factor;
	}
	
	/* Main stage loop */
	for (int frame = 0; frame < 600; frame++) {
		Stage2_Draw(rotozoomer, distorter);
	}

	/* Fade out */
	for (int blend_factor = 0x1; blend_factor <= 0x10; blend_factor++) {
		Stage2_Draw(rotozoomer, distorter);

		REG_BLDY = blend_factor;
		REG_BLDY_SUB = blend_factor;
	}

	Distorter_Free(distorter);
	free(distorter);

	free(rotozoomer);
}

void Stage2_ConfigureHardware()
{
	/* Configure video hardware */
	videoSetMode(MODE_1_2D);

	vramSetBankF(VRAM_F_MAIN_BG_0x06000000);
}

void Stage2_LoadResources(int roto_bg_id, int grid_bg_id)
{
	/* Copy tile, map and palette data into VRAM */
	dmaCopy(rotoTiles, bgGetGfxPtr(roto_bg_id), rotoTilesLen);
	dmaCopy(rotoMap, bgGetMapPtr(roto_bg_id), rotoMapLen);
	dmaCopy(rotoPal, BG_PALETTE, rotoPalLen);

	dmaCopy(gridTiles, bgGetGfxPtr(grid_bg_id), gridTilesLen);
	dmaCopy(gridMap, bgGetMapPtr(grid_bg_id), gridMapLen);
	dmaCopy(gridPal, BG_PALETTE_SUB + 0x2, gridPalLen);
}

void Stage2_Draw(Rotozoomer *rotozoomer, Distorter *distorter)
{
	Rotozoomer_Step(rotozoomer, 4, 546);

	swiWaitForVBlank();

	Distorter_Step(distorter);	
	Distorter_Draw(distorter);

	bgUpdate();
}
