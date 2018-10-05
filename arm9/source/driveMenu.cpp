/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2013
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy
	Claudio "sverx"

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/

#include <nds.h>

#include "main.h"
#include "date.h"
#include "driveOperations.h"

#define SCREEN_COLS 32
#define ENTRIES_PER_SCREEN 22
#define ENTRIES_START_ROW 2
#define ENTRY_PAGE_LENGTH 10

using namespace std;

static int dmCursorPosition = 0;
static bool sdMounted = false;

void driveMenu (void) {
	int pressed = 0;
	int held = 0;

	if (isDSiMode()) {
		sdMounted = sdFound();
	}

	while (true) {
		consoleInit(NULL, 1, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);
		if (dmCursorPosition == 0 && isDSiMode()) {
			printf ("[sd:] SDCARD\n");
			printf ("(SD FAT)");
		} else {
			printf ("[fat:] GAMECART\n");
			printf ("(Flashcart FAT)");
		}
		iprintf ("\x1b[%i;0H", 23-isDSiMode());
		printf (titleName);
		if (isDSiMode()) {
			printf ("\x1b[23;0H");
			if (sdMounted) {
				printf ("R+B - Unmount SD card");
			} else {
				printf ("R+B - Remount SD card");
			}
		}

		consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, true, true);

		printf ("[root]");

		// Move to 2nd row
		printf ("\x1b[1;0H");
		// Print line of dashes
		printf ("--------------------------------");

		// Show cursor
		printf ("\x1b[%d;0H*", dmCursorPosition + ENTRIES_START_ROW);

		printf ("\x1b[2;1H");
		if (isDSiMode()){
			printf ("[sd:] SDCARD");
			if (!sdMounted) {
				printf ("\x1b[2;29H");
				printf ("[x]");
			}
			printf ("\x1b[3;1H");
		}
		printf ("[fat:] GAMECART");
		if (!flashcardFound()) {
			iprintf ("\x1b[%i;29H", 2+isDSiMode());
			printf ("[x]");
		}

		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
			// Move to right side of screen
			printf ("\x1b[0;27H");
			// Print time
			printf (RetTime().c_str());
	
			scanKeys();
			pressed = keysDownRepeat();
			held = keysHeld();
			swiWaitForVBlank();
		} while (!(pressed & KEY_UP) && !(pressed & KEY_DOWN) && !(pressed & KEY_A) && !(held & KEY_R));
	
		if ((pressed & KEY_UP) && isDSiMode()) 		dmCursorPosition -= 1;
		if ((pressed & KEY_DOWN) && isDSiMode()) 		dmCursorPosition += 1;
		
		if (dmCursorPosition < 0) 	dmCursorPosition = 1;		// Wrap around to bottom of list
		if (dmCursorPosition > 1)	dmCursorPosition = 0;		// Wrap around to top of list

		if (pressed & KEY_A) {
			if (dmCursorPosition == 0 && isDSiMode()) {
				if (sdMounted) {
					chdir("sd:/");
					screenMode = 1;
					break;
				}
			} else {
				if (isDSiMode()) {
					flashcardMount();
				}
				if (flashcardFound()) {
					chdir("fat:/");
					screenMode = 1;
					break;
				}
			}
		}

		// Unmount/Remount SD card
		if ((held & KEY_R) && (pressed & KEY_B) && isDSiMode()) {
			if (sdMounted) {
				sdUnmount();
				sdMounted = false;
			} else {
				sdMounted = sdMount();
			}
		}
	}
}