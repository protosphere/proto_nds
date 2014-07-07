#ifndef DISTORTER_H
#define DISTORTER_H

#include <nds.h>

typedef enum {
	DistorterEngineMain,
	DistorterEngineSub
} DistorterEngine;

typedef struct {
	u8 dma_channel;
	DistorterEngine engine;
	int x_offset;
	int *lut;
} Distorter;

void Distorter_Init(Distorter *distorter, u8 dma_channel, DistorterEngine engine, int x_offset, int period, int amplitude);
void Distorter_Free(Distorter *distorter);
void Distorter_GenerateLut(Distorter *distorter, int period, int amplitude);
void Distorter_Step(Distorter *distorter);
void Distorter_Draw(Distorter *distorter);

#endif // DISTORTER_H
