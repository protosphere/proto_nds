#include <nds.h>
#include <math.h>

#include "plasma_cube.h"

static const int plasma_texture_size = 16384;
static const int plasma_palette_size = 128;
static const int plasma_palette_slice_size = 64;

static PlasmaCubeVertex plasma_cube_verticies[] = {
	{inttov16(0), inttov16(0), inttov16(0)},
	{inttov16(0), inttov16(0), inttov16(1)},
	{inttov16(0), inttov16(1), inttov16(0)},
	{inttov16(0), inttov16(1), inttov16(1)},
	{inttov16(1), inttov16(0), inttov16(0)},
	{inttov16(1), inttov16(0), inttov16(1)},
	{inttov16(1), inttov16(1), inttov16(0)},
	{inttov16(1), inttov16(1), inttov16(1)}
};

static PlasmaCubeQuad plasma_cube_quads[] = {
	{&plasma_cube_verticies[2], &plasma_cube_verticies[0], &plasma_cube_verticies[4], &plasma_cube_verticies[6]},
	{&plasma_cube_verticies[6], &plasma_cube_verticies[4], &plasma_cube_verticies[5], &plasma_cube_verticies[7]},
	{&plasma_cube_verticies[7], &plasma_cube_verticies[5], &plasma_cube_verticies[1], &plasma_cube_verticies[3]},
	{&plasma_cube_verticies[3], &plasma_cube_verticies[1], &plasma_cube_verticies[0], &plasma_cube_verticies[2]},
	{&plasma_cube_verticies[1], &plasma_cube_verticies[5], &plasma_cube_verticies[4], &plasma_cube_verticies[0]},
	{&plasma_cube_verticies[2], &plasma_cube_verticies[6], &plasma_cube_verticies[7], &plasma_cube_verticies[3]}
};

void PlasmaCube_DrawQuad(const PlasmaCubeQuad *quad, int palette_id);
void PlasmaCube_LoadContinuousPalette(u16 *palette, u16 start_color, u16 end_color, int base_index, int size);
void PlasmaCube_CyclePalette(u16 *palette, int size);

static inline u8 PlasmaCube_PlasmaFunction(int x, int y)
{
	int32 period_48 = 682; /* DEGREES_IN_CIRCLE / 48 */
	int32 period_64 = 512; /* DEGREES_IN_CIRCLE / 64 */
	int32 period_96 = 341; /* DEGREES_IN_CIRCLE / 96 */
	int32 period_192 = 171; /* DEGREES_IN_CIRCLE / 192 */

	int point1_dx = x - 48;
	int point1_dy = y - 92;

	int point2_dx = x - 16;
	int point2_dy = y - 8;

	int32 distance1 = sqrtf32(inttof32(point1_dx*point1_dx + point1_dy*point1_dy));
	int32 distance2 = sqrtf32(inttof32(point2_dx*point2_dx + point2_dy*point2_dy));

	int32 a = sinLerp(mulf32(distance1, inttof32(period_48)) >> 12);
	int32 b = cosLerp(mulf32(distance2, inttof32(period_64)) >> 12);
	int32 c = cosLerp(x * period_96);
	int32 d = sinLerp(y * period_192);

	u8 f = 64 + (mulf32(floattof32(15.75), (a + b + c + d)) >> 12);

	return f;
}

void PlasmaCube_Init(PlasmaCube *cube)
{
	cube->angle = 0;

	u8 *texture = malloc(plasma_texture_size);
	
	/* Allocate memory for each palette */
	for (int i = 0; i < 3; i++)
		cube->palettes[i] = malloc(plasma_palette_size * sizeof(u16));

	/* Generate the plasma texture */
	for(int x = 0; x < 128; x++) {
		for(int y = 0; y < 128; y++) {    
			texture[y*128 + x] = PlasmaCube_PlasmaFunction(x, y);
		}
	}

	/* Load the desired colors into each palette */
	PlasmaCube_LoadContinuousPalette(cube->palettes[0], 0x001f, 0x7c1f, 0, plasma_palette_slice_size);
	PlasmaCube_LoadContinuousPalette(cube->palettes[0], 0x001f, 0x03ff, 64, plasma_palette_slice_size);

	PlasmaCube_LoadContinuousPalette(cube->palettes[1], 0x700f, 0x2ebf, 0, plasma_palette_slice_size);
	PlasmaCube_LoadContinuousPalette(cube->palettes[1], 0x700f, 0x7f00, 64, plasma_palette_slice_size);

	PlasmaCube_LoadContinuousPalette(cube->palettes[2], 0x7c1f, 0x7fe0, 0, plasma_palette_slice_size);
	PlasmaCube_LoadContinuousPalette(cube->palettes[2], 0x7c1f, 0x03e0, 64, plasma_palette_slice_size);

	/* Allocate memory for the texture and palettes and set it to the active texture */
	glGenTextures(1, cube->texture_ids);
	glGenTextures(3, cube->palette_ids);
	glBindTexture(0, cube->texture_ids[0]);

	/* Copy the texture into video memory */
	glTexImage2D(0, 0, GL_RGB256, TEXTURE_SIZE_128, TEXTURE_SIZE_128, 0, TEXGEN_TEXCOORD, (u8 *)texture);
	free(texture);
}

void PlasmaCube_Free(PlasmaCube *cube)
{
	for (int i = 0; i < 3; i++)
		free(cube->palettes[i]);

	glDeleteTextures(1, cube->texture_ids);
	glDeleteTextures(3, cube->palette_ids);
}

void PlasmaCube_Step(PlasmaCube *cube)
{
	/* Cycle each of the 3 palettes */
	for (int i = 0; i < 3; i++) {
		PlasmaCube_CyclePalette(cube->palettes[i], 128);

		glBindTexture(0, cube->palette_ids[i]);
		glColorTableEXT(0, 0, plasma_palette_size, 0, 0, cube->palettes[i]);
	}

	cube->angle += 90;
}

void PlasmaCube_Draw(PlasmaCube *cube, int32 scale)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof32(0, inttof32(4), 0, inttof32(3), 0, inttof32(40));

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	gluLookAtf32(0, 0, inttof32(1),
				 0, 0, 0,
				 0, inttof32(1), 0);

	/* Move the cube to the center of the screen and rotate it */
	glTranslatef32(floattof32(2), floattof32(1.5), 0);
	glScalef32(scale, scale, scale);
	glRotatef32i(cube->angle, inttof32(1), floattof32(0.5), 0);
	glTranslatef32(floattof32(-0.5), floattof32(-0.5), floattof32(-0.5));

	glColor(0xffff);
	glBindTexture(0, cube->texture_ids[0]);

	glPolyFmt(POLY_ALPHA(0x1f) | POLY_CULL_BACK | POLY_MODULATION);

	for(int i = 0; i < 6; i++) {
		/* Choose the palette based on the side of the cube being drawn */
		int palette_index;

		if (i == 0 || i == 2)
			palette_index = 0;
		else if (i == 1 || i == 3)
			palette_index = 1;
		else
			palette_index = 2;

		int palette_id = cube->palette_ids[palette_index];
		const PlasmaCubeQuad *quad = &plasma_cube_quads[i];

		PlasmaCube_DrawQuad(quad, palette_id);
	}

	glPopMatrix(1);
}

void PlasmaCube_DrawQuad(const PlasmaCubeQuad *quad, int palette_id)
{
	glAssignColorTable(0, palette_id);

	glBegin(GL_QUAD);
	{
		PlasmaCubeVertex *vertex_1 = quad->v1;
		PlasmaCubeVertex *vertex_2 = quad->v2;
		PlasmaCubeVertex *vertex_3 = quad->v3;
		PlasmaCubeVertex *vertex_4 = quad->v4;

		/* Draw the quad */
		glTexCoord2t16(0, 0);
		glVertex3v16(vertex_1->x, vertex_1->y, vertex_1->z);
		
		glTexCoord2t16(0, inttot16(128));
		glVertex3v16(vertex_2->x, vertex_2->y, vertex_2->z);

		glTexCoord2t16(inttot16(128), inttot16(128));
		glVertex3v16(vertex_3->x, vertex_3->y, vertex_3->z);

		glTexCoord2t16(inttot16(128), inttot16(0));
		glVertex3v16(vertex_4->x, vertex_4->y, vertex_4->z);
	}
	glEnd();
}

void PlasmaCube_LoadContinuousPalette(u16 *palette, u16 start_color, u16 end_color, int base_index, int size)
{
	/* Extract the individual colour components from the RGB555 triplet. */
	u8 start_red = (start_color & 0x001f);
	u8 start_green = (start_color & 0x03e0) >> 5;
	u8 start_blue = (start_color & 0x7c00) >> 10;

	u8 end_red = (end_color & 0x001f);
	u8 end_green = (end_color & 0x03e0) >> 5;
	u8 end_blue = (end_color & 0x7c00) >> 10;

	/* Get the change in each colour component in 20.12 fixed point format */
	int32 delta_red = (end_red - start_red) << 12;
	int32 delta_green = (end_green - start_green) << 12;
	int32 delta_blue = (end_blue - start_blue) << 12;

	int end_index = size - 1;
	int ramp_size = size >> 1;
	int ramp_end_index = ramp_size - 1;

	for(int i = 0; i < ramp_size; i++) {
		int32 percentage = divf32((i << 12), (ramp_end_index << 12));

		/* Interpolate the color components and convert back into integer form */
		u8 red = start_red + (mulf32(delta_red, percentage) >> 12);
		u8 green = start_green + (mulf32(delta_green, percentage) >> 12);
		u8 blue = start_blue + (mulf32(delta_blue, percentage) >> 12);

		/* Add the color to opposite sides of the palette to ensure there are no discontinuities */
		u16 color = RGB15(red, green, blue);

		palette[base_index + i] = color;
		palette[base_index + end_index - i] = color;		
	}
}

void PlasmaCube_CyclePalette(u16 *palette, int size)
{
	int end_index = size - 1;

	u16 temp = palette[0];

	for(int i = 0; i <= end_index; i++) {
		palette[i] = palette[i + 1];
	}

	palette[end_index] = temp;
}
