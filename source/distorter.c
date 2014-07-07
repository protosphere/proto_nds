#include <nds.h>
#include <stdio.h>
#include <stdlib.h>

#include "distorter.h"

static const uint32 lut_length = SCREEN_HEIGHT;

void Distorter_Init(Distorter *distorter, u8 dma_channel, DistorterEngine engine, int x_offset, int period, int amplitude)
{
	distorter->dma_channel = dma_channel;
	distorter->engine = engine;
	distorter->x_offset = x_offset;
	
	distorter->lut = malloc(lut_length * sizeof(int));
	Distorter_GenerateLut(distorter, period, amplitude);
}

void Distorter_Free(Distorter *distorter)
{
	DMA_CR(distorter->dma_channel) = 0;	
	free(distorter->lut);
}

void Distorter_GenerateLut(Distorter *distorter, int period, int32 amplitude)
{
	int32 x = div32(DEGREES_IN_CIRCLE, period);
	
	for (int i = 0; i < lut_length; i++) {
		int32 amplitude_20_12 = amplitude << 4; /* Shift left 4 to convert from 24.8 to 20.12 fixed point */

		int32 sine = sinLerp(x * (i + 1));
		int32 sine_offset = mulf32(amplitude_20_12, sine) >> 4; /* Shift right 4 to convert from 20.12 to 24.8 */

		distorter->lut[i] = distorter->x_offset + sine_offset;
	}
}

void Distorter_Step(Distorter *distorter)
{
	int end_index = lut_length - 1;

	int temp = distorter->lut[0];	

	for(int i = 0; i < lut_length; i++) {
		distorter->lut[i] = distorter->lut[i + 1];
	}

	distorter->lut[end_index] = temp;

	/* Flush the CPU cache */
	DC_FlushRange(distorter->lut, lut_length * sizeof(int));
}

void Distorter_Draw(Distorter *distorter)
{
	/*
	 * Transfer the lookup table values to the scroll register every HBlank interval via DMA
	 */
	u8 channel = distorter->dma_channel;
	uint32 destination;

	if (distorter->engine == DistorterEngineMain)
		destination = (uint32)&REG_BG3X;
	else
		destination = (uint32)&REG_BG3X_SUB;

	DMA_CR(channel) = 0;

	DMA_SRC(channel) = (uint32)distorter->lut;
	DMA_DEST(channel) = destination;
	DMA_CR(channel) = (DMA_ENABLE | DMA_32_BIT | DMA_DST_RESET | DMA_REPEAT | DMA_START_HBL | 0x1);
}
