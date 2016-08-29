#include "tcmalloc-test.h"

#include <string.h>

#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>

void TCMallocTestMenu::printStats(char* message) {
     std::istringstream input_buffer;
     std::vector<std::string> lines;
     delwin(win_stats);
     lines.empty();
     int rows = 1, row = 1;

     if(strlen(message) > 0) {
         input_buffer.str(message);
         std::string line;
         for( ; std::getline(input_buffer, line); rows++) {
             lines.push_back(line);
         }
     } else {
         lines.push_back("No stats");
     }
     free(input_buffer);

     int cols = COLS / 2, startx = (COLS - cols) / 2;
     win_stats = newwin(lines.size() + 2, cols, 1, startx);
     box(win_stats, 0, 0);

     for(std::vector<std::string>::iterator it = lines.begin() ; it != lines.end(); ++it) {
         mvwprintw(win_stats, row, 2, (*it).c_str());
         row++;
     }
     wrefresh(win_stats);
}

void TCMallocTestMenu::refreshScreen() {
     refresh();
     endwin();
}

int TCMallocTestMenu::printMenu() {
     delwin(win_menu);
     char* list[4];
     list[TCMallocTestMenu::MENU_LOAD_OBJECTS] = const_cast<char*>("Load 50 objects");
     list[TCMallocTestMenu::MENU_UPDATE_OBJECTS] = const_cast<char*>("Update 50 objects");
     list[TCMallocTestMenu::MENU_DELETE_OBJECTS] = const_cast<char*>("Delete 50 objects");
     list[TCMallocTestMenu::MENU_QUIT] = const_cast<char*>("Quit");
     char item[15];
     int ch, i = 0, list_size = sizeof(list)/sizeof(list[0]);

     int cols = COLS / 2;
     int startx = (COLS - cols) / 2;
     win_menu = newwin(list_size + 2, cols, 20, startx);
     box(win_menu, 0, 0 );

     // now print all the menu items and highlight the first one
     for(i = 0; i < list_size; i++) {
        if(i == 0)
            wattron(win_menu, A_STANDOUT); // highlights the first item.
        else
            wattroff(win_menu, A_STANDOUT);
        sprintf(item, "%-15s", list[i]);
        mvwprintw(win_menu, i+1, 2, "%s", item);
     }

     wrefresh(win_menu); // update the terminal screen

     i = 0;
     keypad(win_menu, TRUE ); // enable keyboard input for the window.

     // get the input
     while((ch = wgetch(win_menu)) != 'q') {
          // right pad with spaces to make the items appear with even width.
          sprintf(item, "%-15s",  list[i]);
          mvwprintw(win_menu, i+1, 2, "%s", item);
          // use a variable to increment or decrement the value based on the input.
          switch(ch) {
               case KEY_UP:
                     i--;
                     i = (i < 0) ? list_size : i;
                     break;
               case KEY_DOWN:
                     i++;
                     i = (i > list_size) ? 0 : i;
                     break;
          }

          if(ch == '\n') {
               break;
          }
          wattron(win_menu, A_STANDOUT );
          sprintf(item, "%-15s",  list[i]);
          mvwprintw(win_menu, i+1, 2, "%s", item);
          wattroff(win_menu, A_STANDOUT );
     }
     return i;
}
