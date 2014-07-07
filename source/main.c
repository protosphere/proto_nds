#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "transition.h"
#include "intro.h"
#include "stage1.h"
#include "stage2.h"
#include "outro.h"

void Main_ConfigureHardware();

int main(void)
{
	/* Seed the RNG */
	srand(time(NULL));

	Main_ConfigureHardware();

	Intro_Run();
	Transition_Run("WELCOME TO A SHORT NDS DEMO BY PROTOSPHERE...FIRST UP IS A PLASMA CUBE!");
	Stage1_Run();
	Transition_Run("NEXT UP WE HAVE A BORING OLD ROTO-ZOOMER...");
	Stage2_Run();
	Transition_Run("GREETINGS TO: WINOCM - PLUS-CHAN - DIALEXIO - SHUFFLEJOY - TRIF - MISPY - TALEN LEE - SQUIFFY - DIRKG");
	Outro_Run();

	return 0;
}

void Main_ConfigureHardware()
{
	videoSetModeSub(MODE_3_2D);
	vramSetBankI(VRAM_I_SUB_BG_0x06208000);

	/* Hide any backgrounds we aren't using */
	REG_DISPCNT &= ~(0x6 << 8);
	REG_DISPCNT_SUB &= ~(0x6 << 8);
}
