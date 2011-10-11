#include "flashlight.h"

#include <stdio.h>

#ifdef FL_UNIX
// ---------------------------------------------------------------------------
// Unix
#include <termios.h>
#include <unistd.h>
#define CONFIG_FILENAME "unix.json"
#define CLEARSCREEN_COMMAND "clear"
int getkey()
{
    struct termios oldt,
                   newt;
    int            ch;
    tcgetattr( STDIN_FILENO, &oldt );
    newt = oldt;
    newt.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newt );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
    return ch;
}
// ---------------------------------------------------------------------------
#else
// ---------------------------------------------------------------------------
// Windows
#include <stdlib.h>
#include <conio.h>
#include <crtdbg.h>
#define CONFIG_FILENAME "win.json"
#define CLEARSCREEN_COMMAND "cls"
int getkey()
{
    return getch();
}
// ---------------------------------------------------------------------------
#endif

int main(int argc, char **argv)
{
#ifdef FL_WINDOWS
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
    {
        Flashlight *fl = flCreate(CONFIG_FILENAME);
        int i;
        int show;
        int key;
        while(1)
        {
            show = fl->view.count;
            if(show > 10)
                show = 10;
            system(CLEARSCREEN_COMMAND);
            printf("[%5d] Search: %s\n", fl->view.count, fl->search);
            for(i=0; i<show; i++)
            {
                ListEntry *e = fl->view.data[i];
                printf(" * %s\n", e->path);
            }
            key = getkey();
            flKey(fl, key);
            if(key == ' ')
                break;
        }
        flDestroy(fl);
    }
    return 0;
}
