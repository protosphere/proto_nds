#include <nds.h>
#include <stdio.h>
#include <stdlib.h>

#include "stage1.h"
#include "plasma_cube.h"
#include "title_console.h"
#include "distorter.h"
#include "grid.h"
#include "font.h"

void Stage1_ConfigureHardware();
void Stage1_LoadResources(int grid_bg_id);
void Stage1_Draw(PlasmaCube *cube, Distorter *distorter);

void Stage1_Run()
{
	Stage1_ConfigureHardware();

	/* Configure background engines */
	int bg0_sub = bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 1, 0);

	/* Enable background blending */
	REG_BLDCNT = BLEND_SRC_BG0 | BLEND_FADE_BLACK;
	REG_BLDCNT_SUB = BLEND_SRC_BG0 | BLEND_SRC_BG3 | BLEND_FADE_BLACK;

	/* Set sub background 0 to display below sub background 1 */
	bgSetPriority(bg0_sub, 1);

	Stage1_LoadResources(bg0_sub);

	/* Set up distortion effects */
	Distorter *distorter = malloc(sizeof(Distorter));
	Distorter_Init(distorter, 0, DistorterEngineSub, -(8 << 8), 96, 4 << 8);

	/* Create a plasma cube for use on the main display */
	PlasmaCube *cube = malloc(sizeof(PlasmaCube));
	PlasmaCube_Init(cube);

	/* Create a console for the stage title and print the title to it */
	PrintConsole title_console;	
	TitleConsole_Init(&title_console, 3, false, 2, 1);

	bgSetScroll(title_console.bgId, 0, -16);

	iprintf("PLASMA\n CUBE");

	/* Fade in */
	for (int blend_factor = 0xf; blend_factor >= 0x0; blend_factor--) {
		Stage1_Draw(cube, distorter);

		REG_BLDY = blend_factor;
		REG_BLDY_SUB = blend_factor;
	}

	/* Main stage loop */
	for (int frame = 0; frame < 600; frame++) {
		Stage1_Draw(cube, distorter);
	}

	/* Fade out */
	for (int blend_factor = 0x1; blend_factor <= 0x10; blend_factor++) {
		Stage1_Draw(cube, distorter);

		REG_BLDY = blend_factor;
		REG_BLDY_SUB = blend_factor;
	}

	glDisable(GL_TEXTURE_2D);

	Distorter_Free(distorter);	
	free(distorter);

	PlasmaCube_Free(cube);
	free(cube);
}

void Stage1_ConfigureHardware()
{
	/* Configure video hardware */
	videoSetMode(MODE_0_3D);

	vramSetBankD(VRAM_D_TEXTURE);
	vramSetBankG(VRAM_G_TEX_PALETTE);

	/* Configure the 3D engine */
	glInit();
	glEnable(GL_TEXTURE_2D);
	
	glClearColor(0, 0, 0, 31);
	glClearPolyID(63);
	glClearDepth(0x7FFF);

	glViewport(0, 0, 255, 191);
}

void Stage1_LoadResources(int grid_bg_id)
{
	dmaCopy(gridTiles, bgGetGfxPtr(grid_bg_id), gridTilesLen);
	dmaCopy(gridMap, bgGetMapPtr(grid_bg_id), gridMapLen);
	dmaCopy(gridPal, BG_PALETTE_SUB + 0x2, gridPalLen);
}

void Stage1_Draw(PlasmaCube *cube, Distorter *distorter)
{
	PlasmaCube_Step(cube);
	PlasmaCube_Draw(cube, floattof32(1.5));

	glFlush(0);
	swiWaitForVBlank();

	Distorter_Step(distorter);
	Distorter_Draw(distorter);

	bgUpdate();	
}
