#include <nds.h>
#include <stdio.h>
#include <stdlib.h>

#include "starfield.h"

static const int num_stars = 200;
static const int max_depth = 3;
static const u16 star_palette[] = {0x03ff, 0x2ebf, 0x7def};

void Starfield_InitStar(StarfieldStar *star);
void Starfield_DrawStar(StarfieldStar *star);

void Starfield_Init(Starfield *starfield)
{
	starfield->stars = malloc(num_stars * sizeof(StarfieldStar));

	for (int i = 0; i < num_stars; i++) {
		StarfieldStar *star = &starfield->stars[i];

		Starfield_InitStar(star);
	}
}

void Starfield_Free(Starfield *starfield)
{
	free(starfield->stars);
}

void Starfield_Step(Starfield *starfield)
{
	for (int i = 0; i < num_stars; i++) {
		StarfieldStar *star = &starfield->stars[i];

		star->x += (max_depth - star->depth + 1);

		if (star->x > SCREEN_WIDTH) {
			star->x = 0;
			star->y = mod32(rand(), 192);
			star->depth = mod32(rand(), max_depth);
		}
	}
}

void Starfield_Draw(Starfield *starfield)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof32(0, inttof32(4), 0, inttof32(3), 0, inttof32(40));

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	gluLookAtf32(0, 0, inttof32(1),
				 0, 0, 0,
				 0, inttof32(1), 0);

	glPolyFmt(POLY_ALPHA(0x1f) | POLY_CULL_NONE);

	for (int i = 0; i < num_stars; i++) {
		StarfieldStar *star = &starfield->stars[i];
		Starfield_DrawStar(star);
	}

	glPopMatrix(1);
}

void Starfield_InitStar(StarfieldStar *star)
{
	star->x = rand() & 255;
	star->y = mod32(rand(), 192);
	star->depth = mod32(rand(), max_depth);
}

void Starfield_DrawStar(StarfieldStar *star)
{
	v16 startX = star->x * 64;
	v16 endX = startX + 64;

	v16 startY = star->y * 64;
	v16 endY = startY + 64;

	u16 star_color = star_palette[star->depth];
	glColor(star_color);

	glBegin(GL_QUAD);
	{
		glVertex3v16(startX, startY, 0);
		glVertex3v16(startX, endY, 0);
		glVertex3v16(endX, endY, 0);
		glVertex3v16(endX, startY, 0);
	}
	glEnd();
}
