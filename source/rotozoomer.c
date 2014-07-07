#include <nds.h>

#include "rotozoomer.h"

void Rotozoomer_Init(Rotozoomer *roto, int bg_id, int min_scale_factor, int max_scale_factor)
{
	bgWrapOn(bg_id);
	bgSetCenter(bg_id, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	roto->bg_id = bg_id;
	roto->min_scale_factor = min_scale_factor;
	roto->max_scale_factor = max_scale_factor;

	roto->scale_factor = min_scale_factor;
	roto->scale_mode = RotozoomerScaleModeEnlarge;
	roto->angle = 0;
}

void Rotozoomer_Step(Rotozoomer *roto, int d_scale, int d_angle)
{
	if (roto->scale_mode == RotozoomerScaleModeEnlarge) {
		if (roto->scale_factor > roto->max_scale_factor) 
			 roto->scale_factor -= d_scale;
		else
			roto->scale_mode = RotozoomerScaleModeReduce;
	} else {
		if (roto->scale_factor < roto->min_scale_factor) 
			 roto->scale_factor += d_scale;
		else
			roto->scale_mode = RotozoomerScaleModeEnlarge;
	}

	roto->angle += d_angle;

	bgSetRotateScale(roto->bg_id, roto->angle, roto->scale_factor, roto->scale_factor);
}
