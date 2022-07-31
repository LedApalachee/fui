#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include "fuistrwin.h"


StringWin* strwin_new(int sx, int sy, char* str, chtype attr, uint8_t flags)
{
	StringWin* strwin = (StringWin*)malloc(sizeof(StringWin));
	if (flags & STRWIN_BORDERED)
	{
		strwin->sx_b = sx;
		strwin->sy_b = sy;
		strwin->sx = sx-2;
		strwin->sy = sy-2;
	}
	else
	{
		strwin->sx = strwin->sx_b = sx;
		strwin->sy = strwin->sy_b = sy;
	}
	int len = strlen(str);
	strwin->str = (char*)malloc(sizeof(char) * (len + 1));
	strwin->attr_str = (chtype*)malloc(sizeof(chtype) * (len+1));

	for (int i = 0; i < len; ++i)
	{
		strwin->str[i] = str[i];
		strwin->attr_str[i] = str[i] | attr;
	}

	strwin->str[len] = '\0';
	strwin->attr_str[len] = '\0';

	strwin->len = len;
	strwin->cur_page = 0;

	strwin_recount(strwin);
	
	strwin->flags = flags;
	return strwin;
}


void strwin_recount(StringWin* strwin)
{
	strwin->rows = 1;
	for (int i = 0, j = 1; i < strwin->len; ++i, ++j)
	{
		if (j > strwin->sx || strwin->str[i] == '\n')
		{
			++strwin->rows;
			j = (strwin->str[i] == '\n' ? 0 : 1);
		}
	}
	strwin->pages = (strwin->rows / strwin->sy) + (strwin->rows % strwin->sy ? 1 : 0);
}


void strwin_newstr(StringWin* strwin, char* str, chtype attr, int recount_pages)
{
	free(strwin->str);
	free(strwin->attr_str);
	int len = strlen(str);
	strwin->str = (char*)malloc(sizeof(char) * (len + 1));
	strwin->attr_str = (chtype*)malloc(sizeof(chtype) * (len+1));

	for (int i = 0; i < len; ++i)
	{
		strwin->str[i] = str[i];
		strwin->attr_str[i] = str[i] | attr;
	}

	strwin->str[len] = '\0';
	strwin->attr_str[len] = '\0';

	strwin->len = len;
	strwin->cur_page = 0;
	if (recount_pages)
		strwin_recount(strwin);
}


void strwin_append(StringWin* strwin, char* str, chtype attr, int recount_pages)
{
	int len = strlen(str);
	strwin->str = realloc(strwin->str, sizeof(char) * (strwin->len + len + 1));
	strwin->attr_str = realloc(strwin->attr_str, sizeof(chtype) * (strwin->len + len + 1));

	for (int i = 0; i < len; ++i)
	{
		strwin->str[strwin->len + i] = str[i];
		strwin->attr_str[strwin->len + i] = str[i] | attr;
	}

	strwin->len += len;
	strwin->str[strwin->len] = '\0';
	strwin->attr_str[strwin->len] = '\0';
	if (recount_pages)
		strwin_recount(strwin);
}


void strwin_insert(StringWin* strwin, char* str, chtype attr, int where, int recount_pages)
{
	int len2 = 0;
	for (int i = 0; str[i] != '\0'; ++i)
		++len2;

	int newlen = strwin->len + len2;
	char* newstr = (char*)malloc(sizeof(char) * (newlen + 1));
	chtype* newattrstr = (chtype*)malloc(sizeof(chtype) * (newlen + 1));

	for (int i = 0, j = 0; i < newlen; ++i, ++j)
	{
		if (i == where)
			i += len2;
		newstr[i] = strwin->str[j];
		newattrstr[i] = strwin->attr_str[j];
	}

	for (int i = 0; i < len2; ++i)
	{
		newstr[where+i] = str[i];
		newattrstr[where+i] = str[i] | attr;
	}

	newstr[newlen] = '\0';
	newattrstr[newlen] = '\0';

	strwin->len = newlen;
	free(strwin->str);
	free(strwin->attr_str);
	strwin->str = newstr;
	strwin->attr_str = newattrstr;

	strwin->cur_page = 0;

	if (recount_pages)
		strwin_recount(strwin);
}


void strwin_cut(StringWin* strwin, int from, int to, int recount_pages)
{
	if (to >= strwin->len) to = strwin->len-1;
	for (int i = 1; to + i < strwin->len; ++i)
	{
		strwin->str[from + i - 1] = strwin->str[to + i];
		strwin->attr_str[from + i - 1] = strwin->attr_str[to + i];
	}

	strwin->str = realloc(strwin->str, sizeof(char) * (strwin->len - to + from));
	strwin->attr_str = realloc(strwin->attr_str, sizeof(chtype) * (strwin->len - to + from));

	strwin->len += from - to - 1;
	strwin->str[strwin->len] = '\0';
	strwin->attr_str[strwin->len] = '\0';
	strwin->cur_page = 0;
	if (recount_pages)
		strwin_recount(strwin);
}


void strwin_wordwrap(StringWin* strwin, int recount_pages)
{
	int where;
	int wordlen;
	for (int i = 0, j = 1; i < strwin->len; ++i, ++j)
	{
		if (j > strwin->sx || strwin->str[i] == '\n')
			j = (strwin->str[i] == '\n' ? 0 : 1);

		// encountered a word
		if (strwin->str[i] >= 'A' && strwin->str[i] <= 'z' || strwin->str[i] == '\'')
		{
			where = i;
			for (wordlen = 1; i < strwin->len-1 && strwin->str[i+1] >= 'A' && strwin->str[i+1] <= 'z' ||
			     strwin->str[i+1] == '\''; ++i, ++j, ++wordlen)
				 ;
			if (j > strwin->sx && wordlen <= strwin->sx)
			{
				strwin_insert(strwin, "\n", 0, where, 0);
				++i;
				j = wordlen;
			}
		}

		// encountered a number
		if (strwin->str[i] >= '0' && strwin->str[i] <= '9' || strwin->str[i] == '.')
		{
			where = i;
			for (wordlen = 1; i < strwin->len-1 && strwin->str[i+1] >= '0' && strwin->str[i+1] <= '9' ||
			     strwin->str[i+1] == '.'; ++i, ++j, ++wordlen)
				 ;
			if (j > strwin->sx && wordlen <= strwin->sx)
			{
				strwin_insert(strwin, "\n", 0, where, 0);
				++i;
				j = wordlen;
			}
		}
	}
	if (recount_pages)
		strwin_recount(strwin);
}


void strwin_nextpage(StringWin* strwin, int n)
{
	if (n == 1)
	{
		if (strwin->cur_page < strwin->pages - 1)
			++strwin->cur_page;
	}
	else if (n == -1)
	{
		if (strwin->cur_page > 0)
			--strwin->cur_page;
	}
}


int strwin_draw(StringWin* strwin, int x, int y)
{
	WINDOW* strwinwin;
	WINDOW* strwinwin_b;
	if (strwin->flags & STRWIN_BORDERED)
	{
		strwinwin_b =  newwin(strwin->sy_b, strwin->sx_b, y, x);
		strwinwin = newwin(strwin->sy, strwin->sx, y+1, x+1);
		refresh();
		wborder(strwinwin_b, 0, 0, 0, 0, 0, 0, 0, 0);
	}
	else
	{
		strwinwin = newwin(strwin->sy, strwin->sx, y, x);
		refresh();
	}

	int cur_rows = 1;
	int cur_page = 0;
	int print = 1;
	for (int i = 0, j = 1; i < strwin->len; ++i, ++j, print = 1)
	{
		if (j > strwin->sx || strwin->str[i] == '\n')
		{
			print = !(j > strwin->sx && strwin->str[i] == '\n' || cur_rows == strwin->sy && strwin->str[i] == '\n');
			j = (strwin->str[i] == '\n' ? 0 : 1);
			++cur_rows;
			if (cur_rows > strwin->sy)
			{
				cur_rows = 1;
				++cur_page;
				if (cur_page > strwin->cur_page) break;
			}
		}
		if (cur_page == strwin->cur_page && print)
			waddch(strwinwin, strwin->attr_str[i]);
	}
	
	if (strwin->flags & STRWIN_BORDERED)
		wrefresh(strwinwin_b);
	wrefresh(strwinwin);
}


void strwin_free(StringWin* strwin)
{
	free(strwin->str);
	free(strwin->attr_str);
	strwin->len = 0;
}
