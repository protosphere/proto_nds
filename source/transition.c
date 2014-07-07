#include <nds.h>
#include <stdlib.h>

#include "transition.h"
#include "font.h"
#include "dycp_scroller.h"
#include "starfield.h"

void Transition_ConfigureHardware();
void Transition_DrawStarfield(Starfield *field);

void Transition_Run(const char *scroller_text)
{
	Transition_ConfigureHardware();

	/* Set main background 0 to display below main background 1 */
	bgSetPriority(0, 1);
	
	/* Disable backgrounds 0 and 3 on sub display */
	REG_DISPCNT_SUB &= ~(0x9 << 8);
	
	/* Enable background blending */
	REG_BLDCNT = BLEND_SRC_BG0 | BLEND_FADE_BLACK;
	REG_BLDCNT_SUB = BLEND_SRC_BG0 | BLEND_FADE_BLACK;

	/* Create the starfield */
	Starfield *field = malloc(sizeof(Starfield));
	Starfield_Init(field);

	/* Create a font for the DYCP scroller */
	DycpScrollerFont font_future;

	font_future.tiles = (u8 *)fontTiles;
	font_future.num_chars = 59;
	font_future.ascii_offset = 32;

	/* Copy the font palette into VRAM */
	dmaCopy(fontPal, SPRITE_PALETTE, fontPalLen);

	/* Create a DYCP scroller for the scroller text */
	DycpScroller *scroller = malloc(sizeof(DycpScroller));
	DycpScroller_Init(scroller, &font_future, 0, scroller_text, 128, 16);

	/* Fade in */
	for (int blend_factor = 0xf; blend_factor >= 0x0; blend_factor--) {
		Transition_DrawStarfield(field);

		swiWaitForVBlank();		

		REG_BLDY = blend_factor;
		REG_BLDY_SUB = blend_factor;
	}
	
	/* Main stage loop */
	while (!scroller->complete) {
		DycpScroller_Step(scroller, 1);
		Transition_DrawStarfield(field);

		swiWaitForVBlank();
		
		oamUpdate(&oamMain);
	}

	/* Fade out */
	for (int blend_factor = 0x1; blend_factor <= 0x10; blend_factor++) {
		Transition_DrawStarfield(field);

		swiWaitForVBlank();		

		REG_BLDY = blend_factor;
		REG_BLDY_SUB = blend_factor;
	}

	/* Disable blending */
	REG_BLDCNT = 0;

	/* (Re-)enable backgrounds 0 and 3 on sub display */
	REG_DISPCNT_SUB |= (0x9 << 8);
	
	Starfield_Free(field);
	free(field);

	DycpScroller_Free(scroller);
	free(scroller);
}

void Transition_ConfigureHardware()
{
	/* Configure video hardware */
	videoSetMode(MODE_0_3D);

	oamEnable(&oamMain);
	oamInit(&oamMain, SpriteMapping_1D_32, false);
	vramSetBankE(VRAM_E_MAIN_SPRITE);	

	/* Configure the 3D engine */
	glInit();
	
	glClearColor(0, 0, 0, 31);
	glClearPolyID(63);
	glClearDepth(0x7FFF);

	glViewport(0, 0, 255, 191);
}

void Transition_DrawStarfield(Starfield *field)
{
	Starfield_Step(field);
	Starfield_Draw(field);

	glFlush(0);
}
