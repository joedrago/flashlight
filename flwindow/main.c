#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>

#include "flashlight.h"
#include "image.h"

#define FL_WINDOW_TITLE "Flashlight"
#define FL_WINDOW_CLASS "FlashlightWindow"

#define RESIZE_MARGIN 20

static Flashlight *sFlashlight = NULL;
static HWND sWindow = INVALID_HANDLE_VALUE;
static HFONT sFont = INVALID_HANDLE_VALUE;

static Image *sImage = NULL;

static void flashlightStartup()
{
    sFlashlight = flCreate("config.json", 55);
    sImage = imageCreate("border.png");

    //AddFontResourceEx("04b25.ttf", FR_PRIVATE|FR_NOT_ENUM, 0);
    sFont = CreateFont(18, 0, 0, 0, /*FW_DONTCARE*/ FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                       CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Courier New"));
}

static void flashlightShutdown()
{
    DeleteObject(sFont);
    sFont = INVALID_HANDLE_VALUE;
    imageDestroy(sImage);
    sImage = NULL;
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
    RECT textRect;
    RECT clientRect;
    int i;
    int show;
    Flashlight *fl = sFlashlight;

    dc = BeginPaint(sWindow, &ps);
    GetClientRect(sWindow, &clientRect);
    BitBlt(dc, 0, 0, clientRect.right, clientRect.bottom, NULL, 0, 0, BLACKNESS);
    //imageDraw(sImage, dc, 0, 0);
    imageDrawBackground(sImage, dc, clientRect.right, clientRect.bottom);

    show = fl->view.count;
    if(show > fl->viewHeight)
        show = fl->viewHeight;

    SelectObject(dc, sFont);
    SetBkMode(dc, TRANSPARENT);
    textRect.left = 10;
    textRect.top = 10;
    textRect.right = clientRect.right - 10;
    textRect.bottom = textRect.top + 20;
    Rectangle(dc, textRect.left, textRect.top, textRect.right, textRect.bottom);
    //if(fl->searchLen)
    {
        char searchBox[1024];
        sprintf(searchBox, "[%5d/%5d] Search: %s\n", fl->viewIndex + 1, fl->view.count, fl->search);
        textRect.left += 2;
        textRect.top += 2;
        DrawText(dc, searchBox, strlen(searchBox), &textRect, DT_SINGLELINE);
    }

    for(i = 0; i < show; i++)
    {
        int currIndex = i + fl->viewOffset;
        ListEntry *e = fl->view.data[currIndex];
        if(currIndex == fl->viewIndex)
            SetTextColor(dc, RGB(0, 0, 0));
        else
            SetTextColor(dc, RGB(0, 0, 96));
        textRect.left = 10;
        textRect.top = 40 + (i * 20);
        textRect.right = clientRect.right - 10;
        textRect.bottom = clientRect.bottom - 10;
        DrawText(dc, e->path, strlen(e->path), &textRect, DT_SINGLELINE);
        //    printf(" * ");
        //printf(" %s\n", e->path);
    }

    EndPaint(sWindow, &ps);
}

static LRESULT CALLBACK wndProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    static int resizing = 0;
    static POINT dragPoint;
    static RECT dragClientRect;
    POINT p;
    RECT clientRect;
    UINT uHitTest;

    switch(message)
    {
    case WM_CHAR:
        keyPressed((int)wparam);
        return 0;
    case WM_NCHITTEST:
        uHitTest = DefWindowProc(window, WM_NCHITTEST, wparam, lparam);
        if(uHitTest == HTCLIENT)
        {
            p.x = GET_X_LPARAM(lparam);
            p.y = GET_Y_LPARAM(lparam);
            ScreenToClient(window, &p);
            GetClientRect(window, &clientRect);
            if((p.x > clientRect.right - RESIZE_MARGIN)
               && (p.y > clientRect.bottom - RESIZE_MARGIN))
                return uHitTest;
            return HTCAPTION;
        }
    case WM_LBUTTONDOWN:
        dragPoint.x = GET_X_LPARAM(lparam);
        dragPoint.y = GET_Y_LPARAM(lparam);
        GetClientRect(window, &dragClientRect);
        resizing = true;
        SetCapture(window);
        return TRUE;
    case WM_MOUSEMOVE:
        if(resizing)
        {
            p.x = GET_X_LPARAM(lparam) - dragPoint.x;
            p.y = GET_Y_LPARAM(lparam) - dragPoint.y;
            SetWindowPos(sWindow, 0, 0, 0, dragClientRect.right + p.x, dragClientRect.bottom + p.y, SWP_NOMOVE | SWP_NOZORDER);
        }
        return TRUE;
    case WM_LBUTTONUP:
        p.x = GET_X_LPARAM(lparam);
        p.y = GET_Y_LPARAM(lparam);
        resizing = false;
        ReleaseCapture();
        return TRUE;
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

    SetWindowLong(sWindow, GWL_STYLE, GetWindowLong(sWindow, GWL_STYLE) & ~(WS_CAPTION | WS_BORDER | WS_THICKFRAME));
    SetWindowPos(sWindow, 0, 0, 0, 800, 500, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
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

