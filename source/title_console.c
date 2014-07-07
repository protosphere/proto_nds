#include <nds.h>

#include "title_console.h"
#include "fontAffine.h"

void TitleConsole_Init(PrintConsole *console, int bg_layer, bool main_engine, int map_base, int tile_base)
{
	consoleInit(console, bg_layer, BgType_ExRotation, BgSize_ER_256x256, map_base, tile_base, main_engine, false);

	ConsoleFont font_future;

	font_future.gfx = (u16 *)fontAffineTiles;
	font_future.pal = (u16 *)fontAffinePal;
	font_future.numChars = 59;
	font_future.numColors = 2;
	font_future.bpp = 8;
	font_future.asciiOffset = 32;
	font_future.convertSingleColor = false;
	
	consoleSetFont(console, &font_future);
	consoleSelect(console);

	bgSetScale(console->bgId, (1 << 6), (1 << 6));
}

