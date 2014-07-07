#include <nds.h>
#include <stdio.h>
#include <stdlib.h>

#include "intro.h"
#include "logo.h"

void Intro_ConfigureHardware();
void Intro_LoadResources(int logo_bg_main_id, int logo_bg_sub_id);
void Intro_TransitionMosaicIn();
void Intro_TransitionMosaicOut();

void Intro_Run()
{
	Intro_ConfigureHardware();

	/* Configure background engines */
	int bg0_main = bgInit(1, BgType_Text4bpp, BgSize_T_256x256, 2, 0);
	int bg0_sub = bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 2, 0);

	/* Ensure background 1 on the main engine is drawn above all other backgrounds */
	bgSetPriority(bg0_main, 0);
	bgSetPriority(0, 1);

	/* Enable background blending */
	REG_BLDCNT = BLEND_SRC_BG1 | BLEND_FADE_BLACK;
	REG_BLDCNT_SUB = BLEND_SRC_BG0 | BLEND_FADE_BLACK;

	/* Enable background mosaic */
	bgMosaicEnable(bg0_main);
	bgMosaicEnable(bg0_sub);

	Intro_LoadResources(bg0_main, bg0_sub);
	
	/* Transition the splash screen in */
	Intro_TransitionMosaicIn();

	/* Wait for 60 frames (1 sec) */
	for (int i = 0; i < 60; i++)
		swiWaitForVBlank();

	/* Pixelate the splash out */
	Intro_TransitionMosaicOut();

	/* Disable mosaic effects */
	bgMosaicDisable(bg0_main);
	bgMosaicDisable(bg0_sub);
}

void Intro_ConfigureHardware()
{
	/* Configure video hardware */
	videoSetMode(MODE_0_2D);

	vramSetBankF(VRAM_F_MAIN_BG_0x06000000);
}

void Intro_LoadResources(int logo_bg_main_id, int logo_bg_sub_id)
{
	/* Copy tile, map and palette data into VRAM */
	dmaCopy(logoTiles, bgGetGfxPtr(logo_bg_main_id), logoTilesLen);
	dmaCopy(logoMap, bgGetMapPtr(logo_bg_main_id), logoMapLen);
	dmaCopy(logoPal, BG_PALETTE, logoPalLen);

	dmaCopy(logoTiles, bgGetGfxPtr(logo_bg_sub_id), logoTilesLen);
	dmaCopy(logoMap, bgGetMapPtr(logo_bg_sub_id), logoMapLen);
	dmaCopy(logoPal, BG_PALETTE_SUB, logoPalLen);
}

void Intro_TransitionMosaicIn()
{
	int frame = 0;
	int mosaic_size = 0xf;
	int blend_factor = 0xf;

	while (blend_factor >= 0) {
		/* Decrease the mosaic size and blend factor every 2 frames
		 * Over 0xf steps, this means the animation will take approx. 0.5 seconds */
		if ((frame & 1) == 0) {
			REG_BLDY = blend_factor;
			REG_BLDY_SUB = blend_factor;

			bgSetMosaic(mosaic_size, mosaic_size);
			bgSetMosaicSub(mosaic_size, mosaic_size);

			blend_factor--;
			mosaic_size--;
		}

		swiWaitForVBlank();
		frame++;
	}
}

void Intro_TransitionMosaicOut()
{
	int frame = 0;
	int mosaic_size = 0x0;
	int blend_factor = 0x1;

	while (blend_factor <= 0x10) {
		if ((frame & 1) == 0) {
			REG_BLDY = blend_factor;
			REG_BLDY_SUB = blend_factor;

			bgSetMosaic(mosaic_size, mosaic_size);
			bgSetMosaicSub(mosaic_size, mosaic_size);

			blend_factor++;
			mosaic_size++;
		}
		
		swiWaitForVBlank();
		frame++;
	}
}
