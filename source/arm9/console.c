/*---------------------------------------------------------------------------------

Copyright (C) 2005
Michael Noland (joat)
Jason Rogers (dovoto)
Dave Murphy (WinterMute)

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.	The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.
2.	Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.
3.	This notice may not be removed or altered from any source
distribution.

---------------------------------------------------------------------------------*/

#include <nds/ndstypes.h>
#include <nds/memory.h>
#include <nds/arm9/console.h>
#include <nds/arm9/video.h>
#include <nds/arm9/background.h>
#include <nds/debug.h>
#include <default_font_bin.h>

#include <stdio.h>
#include <stdarg.h>
#ifndef NO_DEVOPTAB
#include <sys/iosupport.h>
#endif


PrintConsole defaultConsole =
{
	//Font:
	{
		(u16*)default_font_bin, //font gfx
		0, //font palette
		0, //font color count
		1, //bpp
		32, //first ascii character in the set
		95, //number of characters in the font set
		true //convert single color
	},
	0, //font background map
	0, //font background gfx
	22, //map base
	3, //char base
	0, //bg layer in use
	-1, //bg id
	0,0, //cursorX cursorY
	0,0, //prevcursorX prevcursorY
	32, //console width
	24, //console height
	0,  //window x
	0,  //window y
	32, //window width
	24, //window height
	3, //tab size
	0, //font character offset
	0, //selected palette
	0,  //print callback
	false, //console initialized
	true, //load graphics
};

PrintConsole currentCopy;

PrintConsole* currentConsole = &currentCopy;

PrintConsole* consoleGetDefault(void){return &defaultConsole;}

void consolePrintChar(char c);


//---------------------------------------------------------------------------------
static void consoleCls(char mode) {
//---------------------------------------------------------------------------------

	int i = 0;
	int colTemp,rowTemp;

	switch (mode)
	{
	case '[':
	case '0':
		{
			colTemp = currentConsole->cursorX ;
			rowTemp = currentConsole->cursorY ;

			while(i++ < ((currentConsole->windowHeight * currentConsole->windowWidth) - (rowTemp * currentConsole->consoleWidth + colTemp)))
				consolePrintChar(' ');

			currentConsole->cursorX  = colTemp;
			currentConsole->cursorY  = rowTemp;
			break;
		}
	case '1':
		{
			colTemp = currentConsole->cursorX ;
			rowTemp = currentConsole->cursorY ;

			currentConsole->cursorY  = 0;
			currentConsole->cursorX  = 0;

			while (i++ < (rowTemp * currentConsole->windowWidth + colTemp))
				consolePrintChar(' ');

			currentConsole->cursorX  = colTemp;
			currentConsole->cursorY  = rowTemp;
			break;
		}
	case '2':
		{
			currentConsole->cursorY  = 0;
			currentConsole->cursorX  = 0;

			while(i++ < currentConsole->windowHeight * currentConsole->windowWidth)
				consolePrintChar(' ');

			currentConsole->cursorY  = 0;
			currentConsole->cursorX  = 0;
			break;
		}
	}
}
//---------------------------------------------------------------------------------
static void consoleClearLine(char mode) {
//---------------------------------------------------------------------------------

	int i = 0;
	int colTemp;

	switch (mode)
	{
	case '[':
	case '0':
		{
			colTemp = currentConsole->cursorX ;

			while(i++ < (currentConsole->windowWidth - colTemp)) {
				consolePrintChar(' ');
			}

			currentConsole->cursorX  = colTemp;

			break;
		}
	case '1':
		{
			colTemp = currentConsole->cursorX ;

			currentConsole->cursorX  = 0;

			while(i++ < ((currentConsole->windowWidth - colTemp)-2)) {
				consolePrintChar(' ');
			}

			currentConsole->cursorX  = colTemp;

			break;
		}
	case '2':
		{
			colTemp = currentConsole->cursorX ;

			currentConsole->cursorX  = 0;

			while(i++ < currentConsole->windowWidth) {
				consolePrintChar(' ');
			}

			currentConsole->cursorX  = colTemp;

			break;
		}
	default:
		{
			colTemp = currentConsole->cursorX ;

			while(i++ < (currentConsole->windowWidth - colTemp)) {
				consolePrintChar(' ');
			}

			currentConsole->cursorX  = colTemp;

			break;
		}
	}
}

//---------------------------------------------------------------------------------
#ifndef NO_DEVOPTAB
ssize_t nocash_write(struct _reent *r, void *fd, const char *ptr, size_t len) {
#else
ssize_t nocash_write(const char *ptr, size_t len) {
#endif
//---------------------------------------------------------------------------------
	nocashWrite(ptr,len);
	return len;
}


//---------------------------------------------------------------------------------
#ifndef NO_DEVOPTAB
ssize_t con_write(struct _reent *r,void *fd,const char *ptr, size_t len) {
#else
ssize_t con_write(const char *ptr, size_t len) {
#endif
//---------------------------------------------------------------------------------

	char chr;

	size_t i, count = 0;
	char *tmp = (char*)ptr;
	int intensity = 0;

	if(!tmp || len<=0) return -1;

	i = 0;

	while(i<len) {

		chr = *(tmp++);
		i++; count++;

		if ( chr == 0x1b && *tmp == '[' ) {
			bool escaping = true;
			char *escapeseq	= tmp;
			int escapelen = 0;

			do {
				chr = *(tmp++);
				i++; count++; escapelen++;
				int parameter;

				switch (chr) {
					/////////////////////////////////////////
					// Cursor directional movement
					/////////////////////////////////////////
					case 'A':
						sscanf(escapeseq,"[%dA", &parameter);
						currentConsole->cursorY  =  (currentConsole->cursorY  - parameter) < 0 ? 0 : currentConsole->cursorY  - parameter;
						escaping = false;
						break;
					case 'B':
						sscanf(escapeseq,"[%dB", &parameter);
						currentConsole->cursorY  =  (currentConsole->cursorY  + parameter) > currentConsole->windowHeight - 1 ? currentConsole->windowHeight - 1 : currentConsole->cursorY  + parameter;
						escaping = false;
						break;
					case 'C':
						sscanf(escapeseq,"[%dC", &parameter);
						currentConsole->cursorX  =  (currentConsole->cursorX  + parameter) > currentConsole->windowWidth - 1 ? currentConsole->windowWidth - 1 : currentConsole->cursorX  + parameter;
						escaping = false;
						break;
					case 'D':
						sscanf(escapeseq,"[%dD", &parameter);
						currentConsole->cursorX  =  (currentConsole->cursorX  - parameter) < 0 ? 0 : currentConsole->cursorX  - parameter;
						escaping = false;
						break;
						/////////////////////////////////////////
						// Cursor position movement
						/////////////////////////////////////////
					case 'H':
					case 'f':
						sscanf(escapeseq,"[%d;%df", &currentConsole->cursorY , &currentConsole->cursorX );
						escaping = false;
						break;
						/////////////////////////////////////////
						// Screen clear
						/////////////////////////////////////////
					case 'J':
						consoleCls(escapeseq[escapelen-2]);
						escaping = false;
						break;
						/////////////////////////////////////////
						// Line clear
						/////////////////////////////////////////
					case 'K':
						consoleClearLine(escapeseq[escapelen-2]);
						escaping = false;
						break;
						/////////////////////////////////////////
						// Save cursor position
						/////////////////////////////////////////
					case 's':
						currentConsole->prevCursorX  = currentConsole->cursorX ;
						currentConsole->prevCursorY  = currentConsole->cursorY ;
						escaping = false;
						break;
						/////////////////////////////////////////
						// Load cursor position
						/////////////////////////////////////////
					case 'u':
						currentConsole->cursorX  = currentConsole->prevCursorX ;
						currentConsole->cursorY  = currentConsole->prevCursorY ;
						escaping = false;
						break;
						/////////////////////////////////////////
						// Color scan codes
						/////////////////////////////////////////
					case 'm':
						sscanf(escapeseq,"[%d;%dm", &parameter, &intensity);

						//only handle 30-37,39 and intensity for the color changes
						parameter -= 30;

						//39 is the reset code
						if(parameter == 9){
							parameter = 15;
						}
						else if(parameter > 8){
							parameter -= 2;
						}
						else if(intensity){
							parameter += 8;
						}
						if(parameter < 16 && parameter >= 0){
							currentConsole->fontCurPal = parameter << 12;
						}

						escaping = false;
						break;
				}
			} while (escaping);
			continue;
		}

		consolePrintChar(chr);
	}

	return count;
}

#ifndef NO_DEVOPTAB

static const devoptab_t dotab_stdout = {
	"con",
	0,
	NULL,
	NULL,
	con_write,
	NULL,
	NULL,
	NULL
};


static const devoptab_t dotab_nocash = {
	"nocash",
	0,
	NULL,
	NULL,
	nocash_write,
	NULL,
	NULL,
	NULL
};

static const devoptab_t dotab_null = {
	"null",
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

#else

typedef ssize_t (* fn_write_ptr)(const char *, size_t);

fn_write_ptr libnds_stdout_write = NULL;
fn_write_ptr libnds_stderr_write = NULL;

#endif

//---------------------------------------------------------------------------------
void consoleLoadFont(PrintConsole* console) {
//---------------------------------------------------------------------------------
	int i;

	u16* palette = BG_PALETTE_SUB;

	//check which display is being utilized
	if(console->fontBgGfx < BG_GFX_SUB){
			palette = BG_PALETTE;
	}


	if (console->font.bpp == 1 || console->font.bpp == 4) {
		if(console->font.bpp == 4 && !console->font.convertSingleColor) {

			if(console->font.gfx)
				dmaCopy(console->font.gfx, console->fontBgGfx, console->font.numChars * 64 / 2);
			if(console->font.pal)
				dmaCopy(console->font.pal, palette + console->fontCurPal * 16, console->font.numColors*2);

			console->fontCurPal <<= 12;
		} else {
			console->fontCurPal = 15 << 12;

			if (console->font.bpp == 1) {
				for (i = 0; i < console->font.numChars * 8; i++) {
					u8 row = ((const u8*) console->font.gfx)[i];
					u32 temp = 0;
					if(row & 0x80) temp |= 0xF;
					if(row & 0x40) temp |= 0xF0;
					if(row & 0x20) temp |= 0xF00;
					if(row & 0x10) temp |= 0xF000;
					if(row & 0x08) temp |= 0xF0000;
					if(row & 0x04) temp |= 0xF00000;
					if(row & 0x02) temp |= 0xF000000;
					if(row & 0x01) temp |= 0xF0000000;
					((u32*) console->fontBgGfx)[i] = temp;
				}
			} else {
				for (i = 0; i < console->font.numChars * 16; i++) {
					u16 temp = 0;

					if(console->font.gfx[i] & 0xF)
						temp |= 0xF;
					if(console->font.gfx[i] & 0xF0)
						temp |= 0xF0;
					if(console->font.gfx[i] & 0xF00)
						temp |= 0xF00;
					if(console->font.gfx[i] & 0xF000)
						temp |= 0xF000;

					console->fontBgGfx[i] = temp;
				}
			}

			//set up the palette for color printing
			palette[1 * 16 - 1] = RGB15(0,0,0); //30 normal black
			palette[2 * 16 - 1] = RGB15(15,0,0); //31 normal red
			palette[3 * 16 - 1] = RGB15(0,15,0); //32 normal green
			palette[4 * 16 - 1] = RGB15(15,15,0); //33 normal yellow

			palette[5 * 16 - 1] = RGB15(0,0,15); //34 normal blue
			palette[6 * 16 - 1] = RGB15(15,0,15); //35 normal magenta
			palette[7 * 16 - 1] = RGB15(0,15,15); //36 normal cyan
			palette[8 * 16 - 1] = RGB15(24,24,24); //37 normal white

			palette[9 * 16 - 1 ] = RGB15(15,15,15); //40 bright black
			palette[10 * 16 - 1] = RGB15(31,0,0); //41 bright red
			palette[11 * 16 - 1] = RGB15(0,31,0); //42 bright green
			palette[12 * 16 - 1] = RGB15(31,31,0);	//43 bright yellow

			palette[13 * 16 - 1] = RGB15(0,0,31); //44 bright blue
			palette[14 * 16 - 1] = RGB15(31,0,31);	//45 bright magenta
			palette[15 * 16 - 1] = RGB15(0,31,31);	//46 bright cyan
			palette[16 * 16 - 1] = RGB15(31,31,31); //47 & 39 bright white
		}

	} else if(console->font.bpp == 8) {

		console->fontCurPal = 0;

		if(!console->font.convertSingleColor) {

			if(console->font.gfx)
				dmaCopy(console->font.gfx, console->fontBgGfx, console->font.numChars * 64);
			if(console->font.pal)
				dmaCopy(console->font.pal, palette, console->font.numColors*2);
		} else {

			for(i = 0; i < console->font.numChars * 16; i++) {
				u32 temp = 0;

				if(console->font.gfx[i] & 0xF)
					temp = 255;
				if(console->font.gfx[i] & 0xF0)
					temp |= 255 << 8;
				if(console->font.gfx[i] & 0xF00)
					temp |= 255 << 16;
				if(console->font.gfx[i] & 0xF000)
					temp |= 255 << 24;

				((u32*)console->fontBgGfx)[i] = temp;

			}

			palette[255] = RGB15(31,31,31);
		}

	}

	palette[0] = RGB15(0,0,0);
	PrintConsole* tmp = consoleSelect(console);
	consoleCls('2');
	consoleSelect(tmp);

}

//---------------------------------------------------------------------------------
PrintConsole* consoleInit(PrintConsole* console, int layer,
				BgType type, BgSize size,
				int mapBase, int tileBase,
				bool mainDisplay, bool loadGraphics){
//---------------------------------------------------------------------------------

	static bool firstConsoleInit = true;

	if(firstConsoleInit) {
#ifndef NO_DEVOPTAB
		devoptab_list[STD_OUT] = &dotab_stdout;
		devoptab_list[STD_ERR] = &dotab_stdout;
#else
		libnds_stdout_write = con_write;
		libnds_stderr_write = con_write;
#endif
		setvbuf(stdout, NULL , _IONBF, 0);
		setvbuf(stderr, NULL , _IONBF, 0);

		firstConsoleInit = false;
	}

	if(console) {
		currentConsole = console;
	} else {
		console = currentConsole;
	}

	*currentConsole = defaultConsole;

	if(mainDisplay) {
		console->bgId = bgInit(layer, type, size, mapBase, tileBase);
	} else {
		console->bgId = bgInitSub(layer, type, size, mapBase, tileBase);
	}

	console->fontBgGfx = (u16*)bgGetGfxPtr(console->bgId);
	console->fontBgMap = (u16*)bgGetMapPtr(console->bgId);

	console->consoleInitialised = 1;

	consoleCls('2');

	if(loadGraphics)
		consoleLoadFont(console);

	return currentConsole;

}
//---------------------------------------------------------------------------------
PrintConsole *consoleSelect(PrintConsole* console){
//---------------------------------------------------------------------------------
	// Make sure that the buffers for the current console are flushed before
	// switching consoles.
	fflush(stdout);
	fflush(stderr);

	PrintConsole *tmp = currentConsole;
	currentConsole = console;
	return tmp;
}

//---------------------------------------------------------------------------------
void consoleSetFont(PrintConsole* console, ConsoleFont* font){
//---------------------------------------------------------------------------------

	if(!console) console = currentConsole;

	console->font = *font;

	consoleLoadFont(console);

}

//---------------------------------------------------------------------------------
void consoleDebugInit(DebugDevice device){
//---------------------------------------------------------------------------------

	int buffertype = _IONBF;

	switch(device) {

	case DebugDevice_NOCASH:
#ifndef NO_DEVOPTAB
		devoptab_list[STD_ERR] = &dotab_nocash;
		buffertype = _IOLBF;
#else
		libnds_stderr_write = nocash_write;
#endif
		break;
	case DebugDevice_CONSOLE:
#ifndef NO_DEVOPTAB
		devoptab_list[STD_ERR] = &dotab_stdout;
#else
		libnds_stderr_write = con_write;
#endif
		break;
	case DebugDevice_NULL:
#ifndef NO_DEVOPTAB
		devoptab_list[STD_ERR] = &dotab_null;
#else
		libnds_stderr_write = NULL;
#endif
		break;
	}
	setvbuf(stderr, NULL , buffertype, 0);

}

//---------------------------------------------------------------------------------
// Places the console in a default mode using bg0 of the sub display, and vram c for
// font and map..this is provided for rapid prototyping and nothing more
PrintConsole* consoleDemoInit(void) {
//---------------------------------------------------------------------------------
	videoSetModeSub(MODE_0_2D);
	vramSetBankC(VRAM_C_SUB_BG);
	setBrightness(2, 0);

	return consoleInit(NULL, defaultConsole.bgLayer, BgType_Text4bpp, BgSize_T_256x256, defaultConsole.mapBase, defaultConsole.gfxBase, false, true);
}

//---------------------------------------------------------------------------------
static void newRow() {
//---------------------------------------------------------------------------------


	currentConsole->cursorY ++;

	if(currentConsole->cursorY  >= currentConsole->windowHeight)  {
		int rowCount;
		int colCount;

		currentConsole->cursorY --;

		for(rowCount = 0; rowCount < currentConsole->windowHeight - 1; rowCount++)
			for(colCount = 0; colCount < currentConsole->windowWidth; colCount++)
				currentConsole->fontBgMap[(colCount + currentConsole->windowX) + (rowCount + currentConsole->windowY) * currentConsole->consoleWidth] =
					currentConsole->fontBgMap[(colCount + currentConsole->windowX) + (rowCount + currentConsole->windowY + 1) * currentConsole->consoleWidth];

		for(colCount = 0; colCount < currentConsole->windowWidth; colCount++)
			currentConsole->fontBgMap[(colCount + currentConsole->windowX) + (rowCount + currentConsole->windowY) * currentConsole->consoleWidth] =
				(' ' + currentConsole->fontCharOffset - currentConsole->font.asciiOffset);

	}
}


//---------------------------------------------------------------------------------
void consolePrintChar(char c) {
//---------------------------------------------------------------------------------
	if (c==0) return;
	if(currentConsole->fontBgMap == 0) return;

	if(currentConsole->PrintChar)
		if(currentConsole->PrintChar(currentConsole, c))
			return;

	if(currentConsole->cursorX  >= currentConsole->windowWidth) {
		currentConsole->cursorX  = 0;

		newRow();
	}

	switch(c) {
		/*
		The only special characters we will handle are tab (\t), carriage return (\r), line feed (\n)
		and backspace (\b).
		Carriage return & line feed will function the same: go to next line and put cursor at the beginning.
		For everything else, use VT sequences.

		Reason: VT sequences are more specific to the task of cursor placement.
		The special escape sequences \b \f & \v are archaic and non-portable.
		*/
		case 8:
			currentConsole->cursorX--;

			if(currentConsole->cursorX < 0) {
				if(currentConsole->cursorY > 0) {
					currentConsole->cursorX = currentConsole->windowX - 1;
					currentConsole->cursorY--;
				} else {
					currentConsole->cursorX = 0;
				}
			}

			currentConsole->fontBgMap[currentConsole->cursorX + currentConsole->windowX + (currentConsole->cursorY + currentConsole->windowY) * currentConsole->consoleWidth] = currentConsole->fontCurPal | (u16)(' ' + currentConsole->fontCharOffset - currentConsole->font.asciiOffset);

			break;

		case 9:
			currentConsole->cursorX  += currentConsole->tabSize - ((currentConsole->cursorX)%(currentConsole->tabSize));
			break;
		case 10:
			newRow();
		case 13:
			currentConsole->cursorX  = 0;
			break;
		default:
			currentConsole->fontBgMap[currentConsole->cursorX + currentConsole->windowX + (currentConsole->cursorY + currentConsole->windowY) * currentConsole->consoleWidth] = currentConsole->fontCurPal | (u16)(c + currentConsole->fontCharOffset - currentConsole->font.asciiOffset);
			++currentConsole->cursorX ;
			break;
	}
}

//---------------------------------------------------------------------------------
void consoleClear(void) {
//---------------------------------------------------------------------------------
	printf("\x1b[2J");
}

//---------------------------------------------------------------------------------
void consoleSetWindow(PrintConsole* console, int x, int y, int width, int height){
//---------------------------------------------------------------------------------

	if(!console) console = currentConsole;

	console->windowWidth = width;
	console->windowHeight = height;
	console->windowX = x;
	console->windowY = y;

	console->cursorX = 0;
	console->cursorY = 0;

}


