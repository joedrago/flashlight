#include "flexec.h"

#include <windows.h>
#include <stdio.h>

#define EXECBUFSIZE 8192
static char sExecBuffer[EXECBUFSIZE];
static char sVarBuffer[128];

static void flEscape(Flashlight *fl, const char *path, char *buffer, int size)
{
    char *p = buffer;
    char *front = NULL;
    int pathLen = strlen(path);

    for(; *p; p++)
    {
        if(*p == '!')
        {
            if(front)
            {
                int len = p - (front + 1);
                if(len > 0)
                {
                    memcpy(sVarBuffer, front + 1, len);
                    sVarBuffer[len] = 0;

                    if(!strcmp(sVarBuffer, "*")
                       || !strcmp(sVarBuffer, "0")
                       || !strcmp(sVarBuffer, "1"))
                    {
                        memmove(front + pathLen, p + 1, size - ((front + pathLen) - buffer));
                        memcpy(front, path, pathLen);
                    }
                }
                p = front + pathLen - 1;
                front = NULL;
            }
            else
            {
                front = p;
            }
        }
    }
}

static int flExecStart(Flashlight *fl, Action *action, const char *path, flConsoleOutputFunc consoleOutputFunc, void *userData)
{
    memset(sExecBuffer, 0, EXECBUFSIZE);
    strcpy(sExecBuffer, "cmd /c start \"flashlight\" ");
    strcat(sExecBuffer, action->exec);
    flEscape(fl, path, sExecBuffer, EXECBUFSIZE);
    consoleOutputFunc(fl, userData, sExecBuffer);
    system(sExecBuffer);
    return 1;
}

// ---

void DisplayError(Flashlight *fl, char *pszAPI);
//void ReadAndHandleOutput(Flashlight *fl, HANDLE hPipeRead);

static int flExecRedirected(Flashlight *fl, Action *action, const char *path, flConsoleOutputFunc consoleOutputFunc, void *userData, int console)
{
    HANDLE hOutputReadTmp,hOutputRead,hOutputWrite;
    HANDLE hInputWriteTmp,hInputRead,hInputWrite;
    HANDLE hErrorWrite;
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    CHAR lpBuffer[256];
    DWORD nBytesRead;

    memset(sExecBuffer, 0, EXECBUFSIZE);
    if(!console)
        strcpy(sExecBuffer, "cmd /c start \"flashlight\" ");
    strcat(sExecBuffer, action->exec);
    flEscape(fl, path, sExecBuffer, EXECBUFSIZE);
    consoleOutputFunc(fl, userData, sExecBuffer);

    // Set up the security attributes struct.
    sa.nLength= sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    // Create the child output pipe.
    if (!CreatePipe(&hOutputReadTmp,&hOutputWrite,&sa,0))
        DisplayError(fl, "CreatePipe");

    // Create a duplicate of the output write handle for the std error
    // write handle. This is necessary in case the child application
    // closes one of its std output handles.
    if (!DuplicateHandle(GetCurrentProcess(),hOutputWrite,
        GetCurrentProcess(),&hErrorWrite,0,
        TRUE,DUPLICATE_SAME_ACCESS))
        DisplayError(fl, "DuplicateHandle");


    // Create the child input pipe.
    if (!CreatePipe(&hInputRead,&hInputWriteTmp,&sa,0))
        DisplayError(fl, "CreatePipe");

    // Create new output read handle and the input write handles. Set
    // the Properties to FALSE. Otherwise, the child inherits the
    // properties and, as a result, non-closeable handles to the pipes
    // are created.
    if (!DuplicateHandle(GetCurrentProcess(),hOutputReadTmp,
        GetCurrentProcess(),
        &hOutputRead, // Address of new handle.
        0,FALSE, // Make it uninheritable.
        DUPLICATE_SAME_ACCESS))
        DisplayError(fl, "DupliateHandle");

    if (!DuplicateHandle(GetCurrentProcess(),hInputWriteTmp,
        GetCurrentProcess(),
        &hInputWrite, // Address of new handle.
        0,FALSE, // Make it uninheritable.
        DUPLICATE_SAME_ACCESS))
        DisplayError(fl, "DupliateHandle");


    // Close inheritable copies of the handles you do not want to be
    // inherited.
    if (!CloseHandle(hOutputReadTmp)) DisplayError(fl, "CloseHandle");
    if (!CloseHandle(hInputWriteTmp)) DisplayError(fl, "CloseHandle");

    // Set up the start up info struct.
    ZeroMemory(&si,sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hOutputWrite;
    si.hStdInput  = hInputRead;
    si.hStdError  = hErrorWrite;
    if(console)
    {
        si.dwFlags |= STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
    }

    // Launch the process that you want to redirect (in this case,
    // Child.exe). Make sure Child.exe is in the same directory as
    // redirect.c launch redirect from a command line to prevent location
    // confusion.
    if (!CreateProcess(NULL,sExecBuffer,NULL,NULL,TRUE,
        DETACHED_PROCESS,NULL,NULL,&si,&pi))
        DisplayError(fl, "CreateProcess");

    // Close any unnecessary handles.
    if (!CloseHandle(pi.hThread)) DisplayError(fl, "CloseHandle");

    // Close pipe handles (do not continue to modify the parent).
    // You need to make sure that no handles to the write end of the
    // output pipe are maintained in this process or else the pipe will
    // not close when the child process exits and the ReadFile will hang.
    if (!CloseHandle(hOutputWrite)) DisplayError(fl, "CloseHandle");
    if (!CloseHandle(hInputRead )) DisplayError(fl, "CloseHandle");
    if (!CloseHandle(hErrorWrite)) DisplayError(fl, "CloseHandle");

    // Read the child's output.

    if(console)
    {
        while(TRUE)
        {
            if (!ReadFile(hOutputRead,lpBuffer,sizeof(lpBuffer),
                &nBytesRead,NULL) || !nBytesRead)
            {
                if (GetLastError() == ERROR_BROKEN_PIPE)
                    break; // pipe done - normal exit path.
                else
                    DisplayError(fl, "ReadFile"); // Something bad happened.
            }

            // Display the character read on the screen.
            flOutputBytes(fl, lpBuffer, nBytesRead);
        }
    }
    // Redirection is complete


    // Force the read on the input to return by closing the stdin handle.
    if (!CloseHandle(hOutputRead)) DisplayError(fl, "CloseHandle");
    if (!CloseHandle(hInputWrite)) DisplayError(fl, "CloseHandle");
    return 1;
}

/////////////////////////////////////////////////////////////////////// 
// DisplayError
// Displays the error number and corresponding message.
/////////////////////////////////////////////////////////////////////// 
void DisplayError(Flashlight *fl, char *pszAPI)
{
    LPVOID lpvMessageBuffer;
    CHAR szPrintBuffer[512];

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpvMessageBuffer, 0, NULL);

    sprintf(szPrintBuffer,
        "ERROR: API [%s] code [%d] message [%s]",
        pszAPI, GetLastError(), (char *)lpvMessageBuffer);

    //flOutput(fl, szPrintBuffer);
    LocalFree(lpvMessageBuffer);
}

// ---


int flExec(Flashlight *fl, Action *action, const char *path, flConsoleOutputFunc consoleOutputFunc, void *userData)
{
    return flExecRedirected(fl, action, path, consoleOutputFunc, userData, action->console);
}
