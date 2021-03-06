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
#define SELECT_PADDING 2

static Flashlight *sFlashlight = NULL;
static HWND sWindow = INVALID_HANDLE_VALUE;
static Theme *sTheme = NULL;

void onFlashlightEvent(struct Flashlight *fl, FlashlightEvent e, void *data);

static void flashlightStartup()
{
    sFlashlight = flCreate(flPath("config.json", NULL, NULL, NULL));
    sTheme = themeCreate(jpathGetString(sFlashlight->jsonData, "theme", "default"));
    flSetEventFunc(sFlashlight, onFlashlightEvent);
}

static void flashlightShutdown()
{
    themeDestroy(sTheme);
    flDestroy(sFlashlight);
    sFlashlight = NULL;
}

static void flashlightHide()
{
    ShowWindow(sWindow, SW_HIDE);
}

static void flashlightShow()
{
    flClearSearch(sFlashlight);
    ShowWindow(sWindow, SW_HIDE);
    ShowWindow(sWindow, SW_SHOW);
    SetForegroundWindow(sWindow);
}

static void keyPressed(KeyType type, int key)
{
    flKey(sFlashlight, type, key);
    InvalidateRect(sWindow, NULL, TRUE);
}

static void getListRect(RECT *outputRect, RECT *marginsRect, RECT *clientRect, int i)
{
    outputRect->left = marginsRect->left;
    if(marginsRect->top < 0)
        outputRect->top = (clientRect->bottom + marginsRect->top) + (i * sTheme->textHeight);
    else
        outputRect->top = marginsRect->top + (i * sTheme->textHeight);
    outputRect->right = clientRect->right - marginsRect->right;
    outputRect->bottom = clientRect->bottom - marginsRect->bottom;
}

static void flashlightDraw()
{
    PAINTSTRUCT ps;
    HDC dc;
    HDC realDC;
    HDC memDC;
    HBITMAP memBMP;
    RECT textRect;
    RECT clientRect;
    int i;
    int show;
    Flashlight *fl = sFlashlight;

    realDC = BeginPaint(sWindow, &ps);
    GetClientRect(sWindow, &clientRect);
    memDC = CreateCompatibleDC(realDC);
    memBMP = CreateCompatibleBitmap(realDC, clientRect.right, clientRect.bottom);
    SelectObject(memDC, memBMP);
    dc = memDC;

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

    SelectObject(dc, sTheme->font);
    SetBkMode(dc, TRANSPARENT);
    textRect.left = sTheme->searchMargins.left;
    textRect.top = sTheme->searchMargins.top;
    textRect.right = clientRect.right - sTheme->searchMargins.right + sTheme->searchPadding.right;
    textRect.bottom = textRect.top + sTheme->textHeight + sTheme->searchPadding.bottom;
    SetDCBrushColor(dc, sTheme->searchBackgroundColor);
    SelectObject(dc, GetStockObject(DC_BRUSH));
    Rectangle(dc, textRect.left, textRect.top, textRect.right, textRect.bottom);
    //if(fl->searchLen)
    {
        char searchBox[1024];
        sprintf(searchBox, "[%5d/%5d] Search [%s]: %s\n", fl->viewIndex + 1, fl->view.count, fl->currentListName, fl->search);
        textRect.left += sTheme->searchPadding.left;
        textRect.top += sTheme->searchPadding.top;
        //textRect.right -= sTheme->searchPadding.right;
        //textRect.bottom -= sTheme->searchPadding.bottom;
        SetTextColor(dc, sTheme->searchTextColor);
        DrawText(dc, searchBox, strlen(searchBox), &textRect, DT_SINGLELINE);
    }

    show = fl->view.count;
    if(show > fl->viewHeight)
        show = fl->viewHeight;
    for(i = 0; i < show; i++)
    {
        int currIndex = i + fl->viewOffset;
        ListEntry *e = fl->view.data[currIndex];
        if(currIndex == fl->viewIndex)
            SetTextColor(dc, sTheme->listTextActiveColor);
        else
            SetTextColor(dc, sTheme->listTextInactiveColor);
        getListRect(&textRect, &sTheme->listMargins, &clientRect, i);
        DrawText(dc, e->path, strlen(e->path), &textRect, DT_SINGLELINE);
        //    printf(" * ");
        //printf(" %s\n", e->path);
    }

    show = fl->scrollback.count;
    if(show > fl->scrollbackHeight)
        show = fl->scrollbackHeight;
    for(i = 0; i < show; i++)
    {
        int currIndex = i;// + fl->viewOffset;
        const char *t = fl->scrollback.data[(show - 1) - currIndex];
        SetTextColor(dc, sTheme->scrollbackColor);
        getListRect(&textRect, &sTheme->scrollbackMargins, &clientRect, i);
        DrawText(dc, t, strlen(t), &textRect, DT_SINGLELINE);
        //    printf(" * ");
        //printf(" %s\n", e->path);
    }

    for(i = 0; i < fl->viewActions.count; i++)
    {
        Action *action = fl->viewActions.data[i];
        if(action->image)
        {
            if(i == fl->viewActionIndex)
            {
                RECT r;
                r.left = sTheme->actionMargins.left + (i * sTheme->actionMargins.right) + (i * sTheme->actionSpacing);
                r.top = sTheme->actionMargins.top;
                r.right = r.left + sTheme->actionMargins.right;
                r.bottom = r.top + sTheme->actionMargins.bottom;
                r.left -= SELECT_PADDING;
                r.top -= SELECT_PADDING;
                r.right += SELECT_PADDING;
                r.bottom += SELECT_PADDING;
                SetDCBrushColor(dc, sTheme->selectBackgroundColor);
                Rectangle(dc, r.left, r.top, r.right, r.bottom);
            }
            imageDrawTrans(action->image, dc, sTheme->actionMargins.left + (i * sTheme->actionMargins.right) + (i * sTheme->actionSpacing), sTheme->actionMargins.top, sTheme->actionMargins.right, sTheme->actionMargins.bottom, RGB(255, 255, 255));
        }
    }

    BitBlt(realDC, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY);
    EndPaint(sWindow, &ps);
    DeleteDC(memDC);
    DeleteBitmap(memBMP);
}

void flashlightResize()
{
    RECT clientRect;
    RECT listRect;
    int height;
    if(sWindow == INVALID_HANDLE_VALUE)
        return;
    if(sTheme->textHeight == 0)
        return;
    GetClientRect(sWindow, &clientRect);

    // Recalc List Height
    getListRect(&listRect, &sTheme->listMargins, &clientRect, 0);
    height = ((listRect.bottom - listRect.top) / sTheme->textHeight);
    if(height < 1)
        height = 1;
    flSetViewHeight(sFlashlight, height);

    // Recalc Output Height
    getListRect(&listRect, &sTheme->scrollbackMargins, &clientRect, 0);
    height = ((listRect.bottom - listRect.top) / sTheme->textHeight);
    if(height < 1)
        height = 1;
    flSetScrollbackHeight(sFlashlight, height);
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
        case VK_F1:
            keyPressed(KT_SPECIAL, SK_F1);
            break;
        case VK_F2:
            keyPressed(KT_SPECIAL, SK_F2);
            break;
        case VK_F3:
            keyPressed(KT_SPECIAL, SK_F3);
            break;
        case VK_F4:
            keyPressed(KT_SPECIAL, SK_F4);
            break;
        case VK_F5:
            keyPressed(KT_SPECIAL, SK_F5);
            break;
        case VK_F6:
            keyPressed(KT_SPECIAL, SK_F6);
            break;
        case VK_F7:
            keyPressed(KT_SPECIAL, SK_F7);
            break;
        case VK_F8:
            keyPressed(KT_SPECIAL, SK_F8);
            break;
        case VK_F9:
            keyPressed(KT_SPECIAL, SK_F9);
            break;
        case VK_F10:
            keyPressed(KT_SPECIAL, SK_F10);
            break;
        case VK_F11:
            keyPressed(KT_SPECIAL, SK_F11);
            break;
        case VK_F12:
            keyPressed(KT_SPECIAL, SK_F12);
            break;
        case VK_HOME:
            keyPressed(KT_SPECIAL, SK_HOME);
            break;
        case VK_END:
            keyPressed(KT_SPECIAL, SK_END);
            break;
        case VK_PRIOR:
            keyPressed(KT_SPECIAL, SK_PAGEUP);
            break;
        case VK_NEXT:
            keyPressed(KT_SPECIAL, SK_PAGEDOWN);
            break;
        };
        break;
    case WM_HOTKEY:
        flashlightShow();
        break;
    case WM_CHAR:
        if(GetAsyncKeyState(VK_CONTROL) & 0x8000)
        {
            keyPressed(KT_CONTROL, (int)wparam);
        }
        else if(wparam == VK_ESCAPE)
            flashlightHide();
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
    case WM_SIZE:
        flashlightResize();
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

    if(!RegisterHotKey(sWindow, 1, sFlashlight->hotkeyModifiers, sFlashlight->hotkey))
    {
        MessageBox(NULL, "Cant get hotkey", "flashlight", MB_OK);
    }

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

void onFlashlightEvent(struct Flashlight *fl, FlashlightEvent e, void *data)
{
    if(e == FE_CONSOLE)
    {
        InvalidateRect(sWindow, NULL, TRUE);
    }
    else if(e == FE_ACTION)
    {
        Action *action = data;
        if(action->autoClose)
            flashlightHide();
    }
}
