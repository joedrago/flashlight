#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>

#include "flashlight.h"
#include "image.h"
#include "jpath.h"
#include "theme.h"
#include "util.h"

#define FL_WINDOW_TITLE "Flashlight"
#define FL_WINDOW_CLASS "FlashlightWindow"

#define RESIZE_MARGIN 20

static Flashlight *sFlashlight = NULL;
static HWND sWindow = INVALID_HANDLE_VALUE;
static Theme *sTheme = NULL;

static void flashlightStartup()
{
    sFlashlight = flCreate(flashlightPath("config.json", NULL, NULL), 55);
    sTheme = themeCreate(jpathGetString(sFlashlight->jsonData, "theme", "default"));
}

static void flashlightShutdown()
{
    themeDestroy(sTheme);
    flDestroy(sFlashlight);
    sFlashlight = NULL;
}

static void keyPressed(KeyType type, int key)
{
    flKey(sFlashlight, type, key);
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

    for(i = 0; i < sTheme->images.count; i++)
    {
        ThemeImage *themeImage = sTheme->images.data[i];
        switch(themeImage->type)
        {
        case TIT_BORDER:
            imageDrawBackground(themeImage->image, dc, clientRect.right, clientRect.bottom);
        }
    }

    show = fl->view.count;
    if(show > fl->viewHeight)
        show = fl->viewHeight;

    SelectObject(dc, sTheme->font);
    SetBkMode(dc, TRANSPARENT);
    textRect.left = sTheme->searchMargins.left;
    textRect.top = sTheme->searchMargins.top;
    textRect.right = clientRect.right - sTheme->searchMargins.right;
    textRect.bottom = textRect.top + sTheme->textHeight;
    SetDCBrushColor(dc, sTheme->searchBackgroundColor);
    SelectObject(dc, GetStockObject(DC_BRUSH));
    Rectangle(dc, textRect.left, textRect.top, textRect.right, textRect.bottom);
    //if(fl->searchLen)
    {
        char searchBox[1024];
        sprintf(searchBox, "[%5d/%5d] Search: %s\n", fl->viewIndex + 1, fl->view.count, fl->search);
        textRect.left += sTheme->searchPadding.left;
        textRect.top += sTheme->searchPadding.top;
        SetTextColor(dc, sTheme->searchTextColor);
        DrawText(dc, searchBox, strlen(searchBox), &textRect, DT_SINGLELINE);
    }

    for(i = 0; i < show; i++)
    {
        int currIndex = i + fl->viewOffset;
        ListEntry *e = fl->view.data[currIndex];
        if(currIndex == fl->viewIndex)
            SetTextColor(dc, sTheme->listTextActiveColor);
        else
            SetTextColor(dc, sTheme->listTextInactiveColor);
        textRect.left = sTheme->listMargins.left;
        textRect.top = sTheme->listMargins.top + (i * sTheme->textHeight);
        textRect.right = clientRect.right - sTheme->listMargins.right;
        textRect.bottom = clientRect.bottom - sTheme->listMargins.bottom;
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
    case WM_KEYDOWN:
        switch(wparam)
        {
        case VK_UP:
            keyPressed(KT_SPECIAL, SK_UP);
            break;
        case VK_DOWN:
            keyPressed(KT_SPECIAL, SK_DOWN);
            break;
        case VK_LEFT:
            keyPressed(KT_SPECIAL, SK_LEFT);
            break;
        case VK_RIGHT:
            keyPressed(KT_SPECIAL, SK_RIGHT);
            break;
        };
        break;
    case WM_CHAR:
        if(GetKeyState(VK_CONTROL) & 0x8000)
        {
            keyPressed(KT_CONTROL, (int)wparam);
        }
        else if(wparam == VK_ESCAPE)
            PostQuitMessage(0);
        else
            keyPressed(KT_NORMAL, (int)wparam);
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
    SetWindowPos(sWindow, 0, sTheme->initialX, sTheme->initialY, sTheme->initialWidth, sTheme->initialHeight, SWP_NOZORDER | SWP_FRAMECHANGED);
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

