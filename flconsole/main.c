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
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
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
    // _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
    {
        Flashlight *fl = flCreate(CONFIG_FILENAME);
        int i;
        int show;
        int key;
        while(1)
        {
            show = fl->view.count;
            if(show > fl->viewHeight)
                show = fl->viewHeight;
            system(CLEARSCREEN_COMMAND);
            printf("[%5d/%5d] Search: %s\n", fl->viewIndex + 1, fl->view.count, fl->search);
            for(i = 0; i < show; i++)
            {
                int currIndex = i + fl->viewOffset;
                ListEntry *e = fl->view.data[currIndex];
                if(currIndex == fl->viewIndex)
                    printf(" * ");
                printf(" %s\n", e->path);
            }
            key = getkey();
            flKey(fl, KT_NORMAL, key);
            if(key == ' ')
                break;
        }
        flDestroy(fl);
    }
    return 0;
}
