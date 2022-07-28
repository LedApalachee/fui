#ifndef FUIMENU_H
#define FUIMENU_H

#include <stdint.h>

#define WIN_CROSSES_TERMINAL_BOUNDS 1


typedef struct MenuButton
{
	char* text;
	int value;
	int rows; // how many rows it occupies
	int page; // on which page this button is shown
} MenuButton;

/*
	"value" is a number that is unique for every button.
	It's intended to say which button has been pressed.
	This number is not from a certain list of preserved values, but a value defined in outside context.
*/


typedef struct Menu
{
	int sx_b, sy_b; // borders
	int sx, sy;
	char* header;
	int header_rows;
	int pages;
	MenuButton* buttons;
	int buttons_num;
	int selected; // an index for "buttons" array
	uint8_t flags;
} Menu;


// flags
#define MENU_IS_RING 1 // when you press "down" at the lowest button selected, the uppest button will be selected, and vice versa
#define MENU_LINE 2 // a line between header and buttons (must it be or not)
#define MENU_BORDERED 4


Menu* menu_new(int sx, int sy, uint8_t flags);
void  menu_add_header(Menu* menu, char* str);
void  menu_add_button(Menu* menu, char* button_text, int button_value);
void  menu_select(Menu* menu, int which_button);
void  menu_next(Menu* menu, int n);
void  menu_del_button(Menu* menu, int which_one);
int   menu_draw(Menu* menu, int where_x, int where_y, int term_sx, int term_sy);
void  menu_free(Menu* menu);

// returns a selected button's value
// this define can be used instead of typing "menu->buttons[menu->selected].value"
#define MENU_RETURN(menu_ptr) (menu_ptr->buttons[menu_ptr->selected].value)

/*
	The process of creating a menu:
	1) call "menu_new" to dynamically allocate a "Menu" struct
	2) add header (if you want) by calling "menu_add_header"
	3) add buttons by calling "menu_add_button" for each one

	The other funcs:
	"menu_select" - a button from the given menu with index "which_one" is marked as "selected"
	"menu_next" - an upper (if "n" equals -1) or lower (if "n" equals 1) button is makred as "selected"
	"menu_del_button" - deletes the given button
	"menu_draw" - draws menu at the given position
				  returns "WIN_CROSSES_TERMINAL_BOUNDS" if you understood what
	"menu_free" - frees the given "Menu" struct

	Notice: if MENU_BORDERED is not set then sx = sx_b and sy = sy_b
*/

#endif
