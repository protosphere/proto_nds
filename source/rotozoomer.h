#ifndef ROTOZOOMER_H
#define ROTOZOOMER_H

#include <nds.h>

typedef enum {
	RotozoomerScaleModeEnlarge,
	RotozoomerScaleModeReduce
} RotozoomerScaleMode;

typedef struct
{
	int bg_id;
	int min_scale_factor;
	int max_scale_factor;

	int scale_factor;
	RotozoomerScaleMode scale_mode;
	int angle;
} Rotozoomer;

void Rotozoomer_Init(Rotozoomer *roto, int bg_id, int min_scale_factor, int max_scale_factor);
void Rotozoomer_Step(Rotozoomer *roto, int d_scale, int d_rotation);

#endif // ROTOZOOMER_H
