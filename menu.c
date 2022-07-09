#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include "menu.h"


Menu* menu_new(int sx, int sy, uint8_t flags)
{
	Menu* menu = (Menu*)malloc(sizeof(Menu));
	if (flags & MENU_BORDERED)
	{
		menu->sx_b = sx;
		menu->sy_b = sy;
		menu->sx = sx-2;
		menu->sy = sy-2;
	}
	else
	{
		menu->sx = menu->sx_b = sx;
		menu->sy = menu->sy_b = sy;
	}
	menu->header = 0;
	menu->header_rows = 0;
	menu->pages = 1;
	menu->buttons = 0;
	menu->buttons_num = 0;
	menu->selected = 0;
	menu->flags = flags;
	return menu;
}


void menu_add_header(Menu* menu, char* str)
{
	int len = strlen(str);
	free(menu->header);
	menu->header = (char*)malloc(sizeof(char) * (len + 1));
	strcpy(menu->header, str);
	menu->header[len] = '\0';
	menu->header_rows = 1;
	for (int i = 0, j = 1; i < len; ++i, ++j)
		if (j > menu->sx || str[i] == '\n')
		{
			++menu->header_rows;
			j = (str[i] == '\n' ? 0 : 1);
		}
	if (menu->flags & MENU_LINE) ++menu->header_rows;
}


void menu_add_button(Menu* menu, char* button_text, int button_value)
{
	++menu->buttons_num;
	menu->buttons = realloc(menu->buttons, sizeof(MenuButton) * menu->buttons_num);
	MenuButton* new_butt = &menu->buttons[menu->buttons_num-1];
	int len = strlen(button_text);

	// assigning fields to a new button
	new_butt->value = button_value;
	new_butt->text = (char*)malloc(sizeof(char) * (len + 1));
	strcpy(new_butt->text, button_text);
	new_butt->text[len] = '\0';

	// counting how much rows in this menu a new button takes
	new_butt->rows = 1;
	for (int i = 0, j = 1; i < len; ++i, ++j)
		if (j > menu->sx || button_text[i] == '\n')
		{
			++new_butt->rows;
			j = (button_text[i] == '\n' ? 0 : 1);
		}

	// deciding on which page a new button is
	int cur_page_buttons_rows = 0;
	for (int i = 0; i < menu->buttons_num-1; ++i)
		if (menu->buttons[i].page == menu->pages-1)
			cur_page_buttons_rows += menu->buttons[i].rows;
	if (menu->sy - menu->header_rows - cur_page_buttons_rows < new_butt->rows)
	{
		new_butt->page = menu->pages;
		++menu->pages;
	}
	else
		new_butt->page = menu->pages-1;
}


void menu_select(Menu* menu, int which_one)
{
	if (which_one >= 0 && which_one < menu->buttons_num)
		menu->selected = which_one;
	else
		if (menu->flags & MENU_IS_RING)
			if (which_one > 0)
				menu_select(menu, which_one - menu->buttons_num);
			else
				menu_select(menu, menu->buttons_num + which_one);
		else
			if (which_one > 0)
				menu_select(menu, menu->buttons_num-1);
			else
				menu_select(menu, 0);
}


void menu_next(Menu* menu, int n)
{
	if (n == 1)
	{
		if (menu->selected < menu->buttons_num-1)
			++menu->selected;
		else if (menu->flags & MENU_IS_RING)
				menu->selected = 0;
	}
	else if (n == -1)
	{
		if (menu->selected > 0)
			--menu->selected;
		else if (menu->flags & MENU_IS_RING)
			menu->selected = menu->buttons_num-1;
	}
}


void menu_del_button(Menu* menu, int which_one)
{
	if (which_one < 0 || which_one >= menu->buttons_num) return;
	free(menu->buttons[which_one].text);
	for (int i = which_one; i < menu->buttons_num-1; ++i)
	{
		menu->buttons[i].text = menu->buttons[i+1].text;
		menu->buttons[i].value = menu->buttons[i+1].value;
		menu->buttons[i].rows = menu->buttons[i+1].rows;
		menu->buttons[i].page = menu->buttons[i+1].page;
	}
	--menu->buttons_num;
	menu->buttons = realloc(menu->buttons, sizeof(MenuButton) * menu->buttons_num);
	menu->selected = 0;

	menu->pages = 1;
	int cur_page_buttons_rows = 0;
	for (int i = 0; i < menu->buttons_num; ++i)
	{
		if (menu->sy - menu->header_rows - cur_page_buttons_rows < menu->buttons[i].rows)
		{
			++menu->pages;
			cur_page_buttons_rows = 0;
		}
		menu->buttons[i].page = menu->pages-1;
		cur_page_buttons_rows += menu->buttons[i].rows;
	}
}


int menu_draw(Menu* menu, int x, int y, int term_sx, int term_sy)
{
	if (x + menu->sx_b > term_sx || y + menu->sy_b > term_sy)
		return WIN_CROSSES_TERMINAL_BOUNDS;

	WINDOW* menuwin_b;
	WINDOW* menuwin;
	if (menu->flags & MENU_BORDERED)
	{
		menuwin_b =  newwin(menu->sy_b, menu->sx_b, y, x);
		menuwin = newwin(menu->sy, menu->sx, y+1, x+1);
		refresh();
		wborder(menuwin_b, 0, 0, 0, 0, 0, 0, 0, 0);
	}
	else
	{
		menuwin = newwin(menu->sy, menu->sx, y, x);
		refresh();
	}

	if (menu->header != 0)
		wprintw(menuwin, menu->header);

	if (menu->flags & MENU_LINE)
		for (int i = 0; i < menu->sx; ++i)
			mvwaddch(menuwin, menu->header_rows-1, i, '-');
	
	int cur_page = menu->buttons[menu->selected].page;

	int cur_rows = 0;
	for (int i = 0; i < menu->buttons_num; ++i)
		if (menu->buttons[i].page == cur_page)
		{
			if (i == menu->selected)
			{
				wattron(menuwin, A_STANDOUT);
				wprintw(menuwin, menu->buttons[i].text);
				wattroff(menuwin, A_STANDOUT);
			}
			else wprintw(menuwin, menu->buttons[i].text);
			cur_rows += menu->buttons[i].rows;
			wmove(menuwin, menu->header_rows + cur_rows, 0);
		}

	if (menu->flags & MENU_BORDERED)
		wrefresh(menuwin_b);
	wrefresh(menuwin);
}


void menu_free(Menu* menu)
{
	for (int i = 0; i < menu->buttons_num; ++i)
		free(menu->buttons[i].text);
	free(menu->buttons);
	free(menu->header);
	free(menu);
}
