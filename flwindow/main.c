#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "flashlight.h"

#define FL_WINDOW_TITLE "Flashlight"
#define FL_WINDOW_CLASS "FlashlightWindow"

static Flashlight *sFlashlight = NULL;
static HWND sWindow = INVALID_HANDLE_VALUE;

static void flashlightStartup()
{
    sFlashlight = flCreate("win.json", 3);
}

static void flashlightShutdown()
{
    flDestroy(sFlashlight);
    sFlashlight = NULL;
}

static void keyPressed(int key)
{
    flKey(sFlashlight, KT_NORMAL, key);
    InvalidateRect(sWindow, NULL, TRUE);
}

static void flashlightDraw()
{
    PAINTSTRUCT ps;
    HDC dc;
    RECT clientRect;
    int i;
    int show;
    Flashlight *fl = sFlashlight;

    dc = BeginPaint(sWindow, &ps);
    GetClientRect(sWindow, &clientRect);
    BitBlt(dc, 0, 0, clientRect.right, clientRect.bottom, NULL, 0, 0, BLACKNESS);

    show = fl->view.count;
    if(show > fl->viewHeight)
        show = fl->viewHeight;
    //    TextOut(dc, 0, 0, fl->search, fl->searchLen);
    //    printf("[%5d/%5d] Search: %s\n", fl->viewIndex + 1, fl->view.count, fl->search);
    SetBkMode(dc, TRANSPARENT);
    for(i = 0; i < show; i++)
    {
        int currIndex = i + fl->viewOffset;
        ListEntry *e = fl->view.data[currIndex];
        if(currIndex == fl->viewIndex)
            SetTextColor(dc, RGB(192, 255, 192));
        else
            SetTextColor(dc, RGB(64, 96, 64));
        TextOut(dc, 0, i * 20, e->path, strlen(e->path));
        //    printf(" * ");
        //printf(" %s\n", e->path);
    }


    EndPaint(sWindow, &ps);
}

static LRESULT CALLBACK wndProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch(message)
    {
    case WM_CHAR:
        keyPressed((int)wparam);
        return 0;
    case WM_ERASEBKGND:
        return TRUE;
    case WM_PAINT:
        flashlightDraw();
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(window, message, wparam, lparam);
}

static int prepareWindow(HINSTANCE inst)
{
    WNDCLASSEX wcex;
    memset(&wcex, 0, sizeof(wcex));
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = wndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = inst;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = NULL;
    wcex.lpszClassName = FL_WINDOW_CLASS;
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassEx(&wcex);

    sWindow = CreateWindow(FL_WINDOW_CLASS, FL_WINDOW_TITLE, WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, inst, NULL);
    if(!sWindow)
        return 0;

    ShowWindow(sWindow, SW_SHOW);
    UpdateWindow(sWindow);
    return 1;
}

static int mainLoop()
{
    MSG msg;

    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int) msg.wParam;
}

int APIENTRY WinMain(HINSTANCE inst, HINSTANCE prevInst, LPTSTR cmdline, int showCmd)
{
    int ret;
    flashlightStartup();
    if(!prepareWindow(inst))
        return 0;
    ret = mainLoop();
    flashlightShutdown();
    return ret;
}

