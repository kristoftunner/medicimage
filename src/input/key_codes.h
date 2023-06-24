#pragma once
#include <stdint.h>
//#include "SDL_keycode.h"
namespace medicimage 
{
	using KeyCode = uint16_t;
	
	namespace Key
	{
		enum : KeyCode
		{
			// keycode mapping from SDL keycodes
    MDIK_UNKNOWN = 0,

    MDIK_RETURN = '\r',
    MDIK_ESCAPE = '\x1B',
    MDIK_BACKSPACE = '\b',
    MDIK_TAB = '\t',
    MDIK_SPACE = ' ',
    MDIK_EXCLAIM = '!',
    MDIK_QUOTEDBL = '"',
    MDIK_HASH = '#',
    MDIK_PERCENT = '%',
    MDIK_DOLLAR = '$',
    MDIK_AMPERSAND = '&',
    MDIK_QUOTE = '\'',
    MDIK_LEFTPAREN = '(',
    MDIK_RIGHTPAREN = ')',
    MDIK_ASTERISK = '*',
    MDIK_PLUS = '+',
    MDIK_COMMA = ',',
    MDIK_MINUS = '-',
    MDIK_PERIOD = '.',
    MDIK_SLASH = '/',
    MDIK_0 = '0',
    MDIK_1 = '1',
    MDIK_2 = '2',
    MDIK_3 = '3',
    MDIK_4 = '4',
    MDIK_5 = '5',
    MDIK_6 = '6',
    MDIK_7 = '7',
    MDIK_8 = '8',
    MDIK_9 = '9',
    MDIK_COLON = ':',
    MDIK_SEMICOLON = ';',
    MDIK_LESS = '<',
    MDIK_EQUALS = '=',
    MDIK_GREATER = '>',
    MDIK_QUESTION = '?',
    MDIK_AT = '@',

    /*
       Skip uppercase letters
     */

    MDIK_LEFTBRACKET = '[',
    MDIK_BACKSLASH = '\\',
    MDIK_RIGHTBRACKET = ']',
    MDIK_CARET = '^',
    MDIK_UNDERSCORE = '_',
    MDIK_BACKQUOTE = '`',
    MDIK_a = 'a',
    MDIK_b = 'b',
    MDIK_c = 'c',
    MDIK_d = 'd',
    MDIK_e = 'e',
    MDIK_f = 'f',
    MDIK_g = 'g',
    MDIK_h = 'h',
    MDIK_i = 'i',
    MDIK_j = 'j',
    MDIK_k = 'k',
    MDIK_l = 'l',
    MDIK_m = 'm',
    MDIK_n = 'n',
    MDIK_o = 'o',
    MDIK_p = 'p',
    MDIK_q = 'q',
    MDIK_r = 'r',
    MDIK_s = 's',
    MDIK_t = 't',
    MDIK_u = 'u',
    MDIK_v = 'v',
    MDIK_w = 'w',
    MDIK_x = 'x',
    MDIK_y = 'y',
    MDIK_z = 'z',
    
    MDIK_RIGHT = 0,
    MDIK_LEFT = 1,
    MDIK_DOWN = 2,
    MDIK_UP = 3,
    MDIK_DELETE
    //MDIK_RIGHT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT),
    //MDIK_LEFT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT),
    //MDIK_DOWN = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN),
    //MDIK_UP = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP)
		};
	}
}