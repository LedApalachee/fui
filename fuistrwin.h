#ifndef FUISTRWIN_H
#define FUISTRWIN_H

#include <stdint.h>
#include <curses.h>


typedef struct StringWin
{
	int sx_b, sy_b; // size of borders
	int sx, sy;
	int len;
	char* str;
	chtype* attr_str;
	int cur_page;
	int pages;
	int rows;
	uint8_t flags;
} StringWin;


// flags
#define STRWIN_BORDERED 1


StringWin* strwin_new(int sx, int sy, char* str, chtype attr, uint8_t flags);
void       strwin_recount(StringWin* strwin);
void       strwin_newstr(StringWin* strwin, char* str, chtype attr, int recount_pages);
void       strwin_append(StringWin* strwin, char* str, chtype attr, int recount_pages);
void       strwin_insert(StringWin* strwin, char* str, chtype attr, int where, int recount_pages);
void       strwin_cut(StringWin* strwin, int from, int to, int recount_pages);
void       strwin_wordwrap(StringWin* strwin, int recount_pages);
void       strwin_nextpage(StringWin* strwin, int n);
int        strwin_draw(StringWin* strwin, int x, int y);
void       strwin_free(StringWin* strwin);


/*
	You can create a StringWin object with "strwin_new" which "malloc"s memory for it.
	Remember to free the structure with "strwin_free".

	"attr" is curses lib attributes for text. This is a set of flags like "A_BOLD", "A_REVERSE", etc.
	If you don't want your string to have any special attributes, pass this value 0.

	"strwin_recount" is used to count how many pages are there for this StringWin object.

	"strwin_newstr" is used to allocate a new string held in StringWin object.
	Old string is "free"ed.

	"strwin_nextpage" is used to change current page:
	if "n" is 1, then next page becomes current,
	if "n" is -1, the previous one.

	"strwin_draw" is used to draw this StringWin object on the screen.

	"recount_pages" parameter is a bool value:
	if it's "true" (1), then the function calls "strwin_recount" to recount pages,
	if it's "false" (0), then not.
	Normally, you may want pages number to relate whith the current string, so you should pass "recount_pages" 1.
	I added this parameter in technical means.

	I advice you to call "strwin_wordwrap" after a string changing, so the words will not split between lines.

	Notice: if "STRWIN_BORDERED" flag is not set, then sx_b = sx and sy_b = sy;
*/


#endif
