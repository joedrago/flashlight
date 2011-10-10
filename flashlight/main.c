#include "flashlight.h"

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

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

int main(int argc, char **argv)
{
    Flashlight *fl = flCreate("config.json");
    int i;
    int key;
    int show;
    while(1)
    {
        show = fl->view.count;
        if(show > 10)
            show = 10;
        system("clear");
        printf("[%5d] Search: %s\n", fl->view.count, fl->search);
        for(i=0; i<show; i++)
        {
            ListEntry *e = fl->view.data[i];
            printf(" * %s\n", e->path);
        }
        flKey(fl, getkey());
    }
    flDestroy(fl);
    return 0;
}

