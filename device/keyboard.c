#include <kernel/io.h>
#include <kernel/print.h>
#include <kernel/interrupt.h>

#define KBD_BUF_PORT 0x60               // port of keyboard buffer register

#define ESC         '\033'
#define BACKSPACE   '\b'
#define TAB         '\t'
#define ENTER       '\r'
#define DELETE      '\177'

// invisible characters
#define CHAR_INVISIBLE      0
#define CTRL_L_CHAR         CHAR_INVISIBLE
#define CTRL_R_CHAR         CHAR_INVISIBLE
#define SHIFT_L_CHAR        CHAR_INVISIBLE
#define SHIFT_R_CHAR        CHAR_INVISIBLE
#define ALT_L_CHAR          CHAR_INVISIBLE
#define ALT_R_CHAR          CHAR_INVISIBLE
#define CAPS_LOCK_CHAR      CHAR_INVISIBLE

// make code of control characters.
#define SHIFT_L_MAKE        0x2a
#define SHIFT_R_MAKE        0x36
#define ALT_L_MAKE          0x38
#define ALT_R_MAKE          0xe038
#define CTRL_L_MAKE         0x1d
#define CTRL_R_MAKE         0xe01d
#define CAPS_LOCK_MAKE      0x3a

static bool ctrlStatus, shiftStatus, altStatus, capslockStatus, extScancode;

// indexed by make code of characters.
// scancode [0]:  combined with shift [1]:  non-combined with shift.
// only process these characters currently.
static char keymap[][2] = {
/* 0x00 */	{0,	0},		
/* 0x01 */	{ESC, ESC},		
/* 0x02 */	{'1', '!'},		
/* 0x03 */	{'2', '@'},		
/* 0x04 */	{'3', '#'},		
/* 0x05 */	{'4', '$'},		
/* 0x06 */	{'5', '%'},		
/* 0x07 */	{'6', '^'},		
/* 0x08 */	{'7', '&'},		
/* 0x09 */	{'8', '*'},		
/* 0x0A */	{'9', '('},		
/* 0x0B */	{'0', ')'},		
/* 0x0C */	{'-', '_'},		
/* 0x0D */	{'=', '+'},		
/* 0x0E */	{BACKSPACE, BACKSPACE},	
/* 0x0F */	{TAB, TAB},		
/* 0x10 */	{'q', 'Q'},		
/* 0x11 */	{'w', 'W'},		
/* 0x12 */	{'e', 'E'},		
/* 0x13 */	{'r', 'R'},		
/* 0x14 */	{'t', 'T'},		
/* 0x15 */	{'y', 'Y'},		
/* 0x16 */	{'u', 'U'},		
/* 0x17 */	{'i', 'I'},		
/* 0x18 */	{'o', 'O'},		
/* 0x19 */	{'p', 'P'},		
/* 0x1A */	{'[', '{'},		
/* 0x1B */	{']', '}'},		
/* 0x1C */	{ENTER, ENTER},
/* 0x1D */	{CTRL_L_CHAR, CTRL_L_CHAR},
/* 0x1E */	{'a', 'A'},		
/* 0x1F */	{'s', 'S'},		
/* 0x20 */	{'d', 'D'},		
/* 0x21 */	{'f', 'F'},		
/* 0x22 */	{'g', 'G'},		
/* 0x23 */	{'h', 'H'},		
/* 0x24 */	{'j', 'J'},		
/* 0x25 */	{'k', 'K'},		
/* 0x26 */	{'l', 'L'},		
/* 0x27 */	{';', ':'},		
/* 0x28 */	{'\'', '"'},		
/* 0x29 */	{'`', '~'},		
/* 0x2A */	{SHIFT_L_CHAR, SHIFT_L_CHAR},	
/* 0x2B */	{'\\', '|'},		
/* 0x2C */	{'z', 'Z'},		
/* 0x2D */	{'x', 'X'},		
/* 0x2E */	{'c', 'C'},		
/* 0x2F */	{'v', 'V'},		
/* 0x30 */	{'b', 'B'},		
/* 0x31 */	{'n', 'N'},		
/* 0x32 */	{'m', 'M'},		
/* 0x33 */	{',', '<'},		
/* 0x34 */	{'.', '>'},		
/* 0x35 */	{'/', '?'},
/* 0x36	*/	{SHIFT_R_CHAR, SHIFT_R_CHAR},	
/* 0x37 */	{'*', '*'},    	
/* 0x38 */	{ALT_L_CHAR, ALT_L_CHAR},
/* 0x39 */	{' ', ' '},		
/* 0x3A */	{CAPS_LOCK_CHAR, CAPS_LOCK_CHAR}
};

// handler of keyboard interrupt.
static void intrKeyboardHandler() {
    uint16 scancode = (uint16) inb(KBD_BUF_PORT);

    // extend code
    if (scancode == 0xe0) {
        extScancode = true;
        return;
    }

    if (extScancode) {
        scancode |= 0xe000;
        extScancode = false;
    }

    bool breakcode = ((scancode & 0x0080) != 0);
    if (breakcode) {
        uint16 makecode = (scancode &= 0xff7f);
        switch (makecode) {
            case CTRL_L_MAKE:
            case CTRL_R_MAKE:
                ctrlStatus = false; break;

            case SHIFT_L_MAKE:
            case SHIFT_R_MAKE:
                shiftStatus = false; break;

            case ALT_L_MAKE:
            case ALT_R_MAKE:
                altStatus = false; break;
        }

        return;
    }

    if (scancode >= 0x01 && scancode <= 0x3a) {
        bool shift = false;
        if (scancode <= 0x0d || scancode == 0x29 || scancode == 0x1a ||
            scancode == 0x1b || scancode == 0x2b || scancode == 0x27 ||
            scancode == 0x28 || scancode == 0x33 || scancode == 0x34 ||
            scancode == 0x35) {
            shift = shiftStatus;
        } else {
            if (shiftStatus && capslockStatus) { shift = false; }
            else if (shiftStatus || capslockStatus) { shift = true; }
            else { shift = false; }
        }

        uint8 idx = (scancode &= 0x00ff);
        char ch = keymap[idx][shift];
        if (ch) {
            putchar(ch);
            return;
        }

        switch (scancode) {
            case CTRL_L_MAKE:
            case CTRL_R_MAKE:
                ctrlStatus = true; break;

            case SHIFT_L_MAKE:
            case SHIFT_R_MAKE:
                shiftStatus = true; break;

            case ALT_L_MAKE:
            case ALT_R_MAKE:
                altStatus = true; break;

            case CAPS_LOCK_MAKE:
                capslockStatus = !capslockStatus; break;
        }

        return;
    }

    puts("\nunknown key.");
}

void keyboardInit() {
    puts("keyboard init starts.\n");
    registerHandler(0x21, intrKeyboardHandler);
    puts("keyboard init done!\n");
}
