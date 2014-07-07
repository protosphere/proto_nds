#ifndef PLASMA_CUBE_H
#define PLASMA_CUBE_H

#include <nds.h>

typedef struct {
	v16 x;
	v16 y;
	v16 z;
} PlasmaCubeVertex;

typedef struct {
	PlasmaCubeVertex *v1;
	PlasmaCubeVertex *v2;
	PlasmaCubeVertex *v3;
	PlasmaCubeVertex *v4;
} PlasmaCubeQuad;

typedef struct {
	int angle;
	
	int texture_ids[1];
	int palette_ids[3];

	u16 *palettes[3];
} PlasmaCube;

void PlasmaCube_Init(PlasmaCube *cube);
void PlasmaCube_Free(PlasmaCube *cube);
void PlasmaCube_Step(PlasmaCube *cube);
void PlasmaCube_Draw(PlasmaCube *cube, int32 scale);

#endif // PLASMA_CUBE_H
