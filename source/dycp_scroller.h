#ifndef DYCP_SCROLLER_H
#define DYCP_SCROLLER_H

#include <nds.h>

typedef struct {
	u8 ascii_offset;
	u8 *tiles;
	u16 num_chars;
} DycpScrollerFont;

typedef struct {
	bool loaded;
	u16 *gfx;
} DycpScrollerSprite;

typedef struct {
	bool complete;

	DycpScrollerFont *font;
	const char *text;
	int text_length;

	int scroll_base_x;
	int min_char_index;
	u8 max_rendered_chars;

	int oam_offset;
	DycpScrollerSprite *characters;

	int amplitude;
	int period;
} DycpScroller;

void DycpScroller_Init(DycpScroller *scroller, DycpScrollerFont *font, int oam_offset, const char *text, int period, int amplitude);
void DycpScroller_Free(DycpScroller *scroller);
void DycpScroller_Step(DycpScroller *scroller, u8 step_size);

#endif // DYCP_SCROLLER_H
