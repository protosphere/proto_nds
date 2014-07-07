#include <nds.h>
#include <stdlib.h>

#include "dycp_scroller.h"
#include "font.h"

static const u8 sprite_size = 8;
static const u8 sprite_mem_size = (8*8 >> 1);
static const int max_onscreen_chars = 33;

void DycpScroller_BuildLut(DycpScroller *scroller);
void DycpScroller_LoadCharacter(DycpScroller *scroller, DycpScrollerSprite *sprite, char character);

static inline s16 DycpScroller_PathFunction(DycpScroller *scroller, int x)
{
	int32 a = div32(DEGREES_IN_CIRCLE, scroller->period);
	int32 sine = sinLerp(a * x);
	int32 amplitude_fixed = scroller->amplitude << 12;

	s16 y_pos = 92 + (mulf32(amplitude_fixed, sine) >> 12);

	return y_pos;
}

void DycpScroller_Init(DycpScroller *scroller, DycpScrollerFont *font, int oam_offset, const char *text, int period, int amplitude)
{
	scroller->complete = false;
	scroller->font = font;
	scroller->text = text;
	scroller->text_length = strlen(text);

	scroller->scroll_base_x = SCREEN_WIDTH;	
	scroller->min_char_index = 0;	
	scroller->max_rendered_chars = ((scroller->text_length < max_onscreen_chars) ? scroller->text_length : max_onscreen_chars);

	scroller->oam_offset = oam_offset;

	/* Allocate enough memory to hold the characters */
	scroller->characters = malloc(sizeof(DycpScrollerSprite) * scroller->max_rendered_chars);

	for(int i = 0; i < scroller->max_rendered_chars; i++) {
		/* Allocate VRAM memory for each sprite) that will be displayed on-screen */
		scroller->characters[i].gfx = oamAllocateGfx(&oamMain, SpriteSize_8x8, SpriteColorFormat_16Color);

		/* Load the initial set of characters */
		char character = scroller->text[i];		
		DycpScrollerSprite *sprite = &scroller->characters[i];

		DycpScroller_LoadCharacter(scroller, sprite, character);
	}

	scroller->period = period;
	scroller->amplitude = amplitude;
}

void DycpScroller_Free(DycpScroller *scroller)
{
	for(int i = 0; i < scroller->max_rendered_chars; i++)
		oamFreeGfx(&oamMain, scroller->characters[i].gfx);

	free(scroller->characters);
}

void DycpScroller_LoadCharacter(DycpScroller *scroller, DycpScrollerSprite *sprite, char character)
{
	u8 ascii_offset = character - scroller->font->ascii_offset;
	u8 *sprite_offset = scroller->font->tiles + sprite_mem_size * ascii_offset;

	dmaCopy(sprite_offset, sprite->gfx, sprite_mem_size);

	sprite->loaded = true;
}

void DycpScroller_Step(DycpScroller *scroller, u8 step_size)
{
	scroller->scroll_base_x -= 1;

	int render_index = 0;
	int character_index = scroller->min_char_index;
	char character = scroller->text[scroller->min_char_index];

	if (!scroller->complete)
		scroller->complete = (scroller->min_char_index >= scroller->text_length);

	while (!scroller->complete && render_index < scroller->max_rendered_chars) {
		character_index = scroller->min_char_index + render_index;

		if (character_index >= scroller->text_length)
			break;

		int sprite_index = mod32(character_index, scroller->max_rendered_chars);
		
		character = scroller->text[character_index];
		DycpScrollerSprite *sprite = &scroller->characters[sprite_index];

        /* Check if character is loaded and load it if neccessary */
		if (!sprite->loaded) {
			DycpScroller_LoadCharacter(scroller, sprite, character);
		}

		int sprite_id = scroller->oam_offset + sprite_index;
		int sprite_x = scroller->scroll_base_x + (render_index * sprite_size);
		int sprite_y = DycpScroller_PathFunction(scroller, sprite_x);

		/* Display the character if it is onscreen */
		if (sprite_x < SCREEN_WIDTH) {
			oamSet(&oamMain, sprite_id, sprite_x, sprite_y,
				   0, 0, SpriteSize_8x8, SpriteColorFormat_16Color,
				   sprite->gfx, -1, false, false, false, false, false);
		}

        /* Check if character is offscreen and unload it if neccessary */
		if (scroller->scroll_base_x <= -sprite_size) {
			scroller->scroll_base_x = 0;
			scroller->min_char_index++;

			sprite->loaded = false;
		}

		render_index++;
	}
}
