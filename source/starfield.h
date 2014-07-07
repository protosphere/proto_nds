#ifndef STARFIELD_H
#define STARFIELD_H

#include <nds.h>

typedef struct
{
	int x;
	int y;
	int depth;
} StarfieldStar;

typedef struct
{
	StarfieldStar *stars;
} Starfield;

void Starfield_Init(Starfield *starfield);
void Starfield_Free(Starfield *starfield);
void Starfield_Step(Starfield *starfield);
void Starfield_Draw(Starfield *starfield);

#endif // STARFIELD_H
