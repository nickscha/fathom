#ifndef WIN32_FATHOM_API_H
#define WIN32_FATHOM_API_H

#include "fathom_types.h"

/* #############################################################################
 * # [SECTION] Win32 64 bit types
 * #############################################################################
 */
#if __STDC_VERSION__ >= 199901L
typedef long long i64;
typedef unsigned long long u64;
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlong-long"
typedef long long i64;
typedef unsigned long long u64;
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
typedef __int64 i64;
typedef unsigned __int64 u64;
#else
typedef long i64;
typedef unsigned long u64;
#endif

#define WIN32_FATHOM_API_TYPES_STATIC_ASSERT(c, m) typedef char win32_fathom_api_types_assert_##m[(c) ? 1 : -1]
WIN32_FATHOM_API_TYPES_STATIC_ASSERT(sizeof(u64) == 8, u64_size_must_be_8);
WIN32_FATHOM_API_TYPES_STATIC_ASSERT(sizeof(i64) == 8, i64_size_must_be_8);
#undef WIN32_FATHOM_API_TYPES_STATIC_ASSERT

/* #############################################################################
 * # [SECTION] Win32 "windows.h" substitution for fast compilation
 * #############################################################################
 */
#define WIN32_API(r) __declspec(dllimport) r __stdcall

#define INVALID_FILE_SIZE ((u32)0xFFFFFFFF)
#define INVALID_HANDLE ((void *)-1)
#define GENERIC_READ (0x80000000L)
#define GENERIC_WRITE (0x40000000L)
#define CREATE_ALWAYS 2
#define FILE_SHARE_READ 0x00000001
#define FILE_SHARE_WRITE 0x00000002
#define FILE_SHARE_DELETE 0x00000004
#define OPEN_EXISTING 3
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000

#define MEM_COMMIT 0x00001000
#define MEM_RESERVE 0x00002000
#define MEM_RELEASE 0x00008000
#define PAGE_READWRITE 0x04

#define WM_ERASEBKGND 0x0014
#define WM_CREATE 0x0001
#define WM_CLOSE 0x0010
#define WM_QUIT 0x0012
#define WM_SIZE 0x0005
#define WM_INPUT 0x00FF
#define WM_DEVICECHANGE 0x0219

#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_RBUTTONDOWN 0x0204
#define WM_RBUTTONUP 0x0205

#define DBT_DEVICEARRIVAL 0x8000
#define DBT_DEVICEREMOVECOMPLETE 0x8004
#define DBT_DEVNODES_CHANGED 0x0007

#define SIZE_MINIMIZED 1

#define CS_OWNDC 0x0020

#define HWND_TOPMOST ((void *)-1)
#define HWND_TOP ((void *)0)

#define WS_CLIPSIBLINGS 0x04000000
#define WS_CLIPCHILDREN 0x02000000
#define WS_THICKFRAME 0x00040000L
#define WS_MINIMIZEBOX 0x00020000L
#define WS_MAXIMIZEBOX 0x00010000L
#define WS_CAPTION 0x00C00000L
#define WS_SYSMENU 0x00080000L
#define WS_OVERLAPPED 0x00000000L
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)

#define SWP_NOSIZE 0x0001
#define SWP_NOMOVE 0x0002
#define SWP_NOZORDER 0x0004
#define SWP_SHOWWINDOW 0x0040
#define SWP_NOOWNERZORDER 0x0200
#define SWP_FRAMECHANGED 0x0020
#define SW_SHOW 5

#define GWL_STYLE -16
#define GWLP_USERDATA -21

#define MONITOR_DEFAULTTONEAREST 0x00000002

#define PM_REMOVE 0x0001

#define PFD_DOUBLEBUFFER 0x00000001
#define PFD_SUPPORT_OPENGL 0x00000020
#define PFD_DRAW_TO_WINDOW 0x00000004
#define PFD_TYPE_RGBA 0

#define MAKEINTRESOURCEA(i) ((s8 *)((u32)((u16)(i))))
#define IDC_ARROW MAKEINTRESOURCEA(32512)

#define RIDEV_INPUTSINK 0x00000100
#define RID_INPUT 0x10000003
#define RIM_TYPEMOUSE 0
#define RIM_TYPEKEYBOARD 1
#define RI_KEY_BREAK 1
#define RI_MOUSE_WHEEL 0x0400

#define WHEEL_DELTA 120

#define HIGH_PRIORITY_CLASS 0x80
#define THREAD_PRIORITY_HIGHEST 2
#define ES_SYSTEM_REQUIRED ((u32)0x00000001)
#define ES_DISPLAY_REQUIRED ((u32)0x00000002)
#define ES_CONTINUOUS ((u32)0x80000000)

#define TH32CS_SNAPTHREAD 0x00000004

typedef void *(*PROC)(void);
typedef i64 (*WNDPROC)(void *, u32, u64, i64);

typedef struct CREATESTRUCTA
{
    void *lpCreateParams;
    void *hInstance;
    void *hMenu;
    void *hwndParent;
    i32 cy;
    i32 cx;
    i32 y;
    i32 x;
    i32 style;
    s8 *lpszName;
    s8 *lpszClass;
    u32 dwExStyle;
} CREATESTRUCTA;

typedef struct WNDCLASSA
{
    u32 style;
    WNDPROC lpfnWndProc;
    i32 cbClsExtra;
    i32 cbWndExtra;
    void *hInstance;
    void *hIcon;
    void *hCursor;
    void *hbrBackground;
    s8 *lpszMenuName;
    s8 *lpszClassName;
} WNDCLASSA;

typedef struct POINT
{
    i32 x;
    i32 y;
} POINT;

typedef struct RECT
{
    i32 left;
    i32 top;
    i32 right;
    i32 bottom;
} RECT;

typedef struct MSG
{
    void *hwnd;
    u32 message;
    u64 wParam;
    i64 lParam;
    u32 time;
    POINT pt;
    u32 lPrivate;
} MSG;

typedef struct PIXELFORMATDESCRIPTOR
{
    u16 nSize;
    u16 nVersion;
    u32 dwFlags;
    u8 iPixelType;
    u8 cColorBits;
    u8 cRedBits;
    u8 cRedShift;
    u8 cGreenBits;
    u8 cGreenShift;
    u8 cBlueBits;
    u8 cBlueShift;
    u8 cAlphaBits;
    u8 cAlphaShift;
    u8 cAccumBits;
    u8 cAccumRedBits;
    u8 cAccumGreenBits;
    u8 cAccumBlueBits;
    u8 cAccumAlphaBits;
    u8 cDepthBits;
    u8 cStencilBits;
    u8 cAuxBuffers;
    u8 iLayerType;
    u8 bReserved;
    u32 dwLayerMask;
    u32 dwVisibleMask;
    u32 dwDamageMask;
} PIXELFORMATDESCRIPTOR;

typedef struct FILETIME
{
    u32 dwLowDateTime;
    u32 dwHighDateTime;
} FILETIME;

typedef struct WIN32_FILE_ATTRIBUTE_DATA
{
    u32 dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    u32 nFileSizeHigh;
    u32 nFileSizeLow;
} WIN32_FILE_ATTRIBUTE_DATA;

typedef struct RAWINPUTDEVICE
{
    u16 usUsagePage;
    u16 usUsage;
    u32 dwFlags;
    void *hwndTarget;
} RAWINPUTDEVICE;

typedef struct RAWINPUTHEADER
{
    u32 dwType;
    u32 dwSize;
    void *hDevice;
    u64 wParam;
} RAWINPUTHEADER;

typedef struct RAWKEYBOARD
{
    u16 MakeCode;
    u16 Flags;
    u16 Reserved;
    u16 VKey;
    u32 Message;
    u32 ExtraInformation;
} RAWKEYBOARD;

typedef struct RAWMOUSE
{
    u16 usFlags;
    u16 usButtonFlags;
    u16 usButtonData;
    u32 ulRawButtons;
    i32 lLastX;
    i32 lLastY;
    u32 ulExtraInformation;
} RAWMOUSE;

typedef struct RAWHID
{
    u32 dwSizeHid;
    u32 dwCount;
    u8 bRawData[1];
} RAWHID;

typedef struct RAWINPUT
{
    RAWINPUTHEADER header;
    union
    {
        RAWMOUSE mouse;
        RAWKEYBOARD keyboard;
        RAWHID hid;
    } data;
} RAWINPUT;

typedef struct PROCESS_MEMORY_COUNTERS_EX
{
    u32 cb;
    u32 PageFaultCount;
    u64 PeakWorkingSetSize;
    u64 WorkingSetSize;
    u64 QuotaPeakPagedPoolUsage;
    u64 QuotaPagedPoolUsage;
    u64 QuotaPeakNonPagedPoolUsage;
    u64 QuotaNonPagedPoolUsage;
    u64 PagefileUsage;
    u64 PeakPagefileUsage;
    u64 PrivateUsage;
} PROCESS_MEMORY_COUNTERS_EX;

typedef struct THREADENTRY32
{
    u32 dwSize;
    u32 cntUsage;
    u32 th32ThreadID;
    u32 th32OwnerProcessID;
    i32 tpBasePri;
    i32 tpDeltaPri;
    u32 dwFlags;
} THREADENTRY32;

typedef struct WINDOWPLACEMENT
{
    u32 length;
    u32 flags;
    u32 showCmd;
    POINT ptMinPosition;
    POINT ptMaxPosition;
    RECT rcNormalPosition;
    RECT rcDevice;
} WINDOWPLACEMENT;

typedef struct MONITORINFO
{
    u32 cbSize;
    RECT rcMonitor;
    RECT rcWork;
    u32 dwFlags;
} MONITORINFO;

/* clang-format off */
WIN32_API(void *) GetStdHandle(u32 nStdHandle);
WIN32_API(i32)    CloseHandle(void *hObject);
WIN32_API(void *) LoadLibraryA(s8 *lpLibFileName);
WIN32_API(i32)    FreeLibrary(void *hLibModule);
WIN32_API(void *) GetProcAddress(void *hModule, char *lpProcName);
WIN32_API(i32)    SetProcessDPIAware(void);
WIN32_API(void *) VirtualAlloc(void *lpAddress, u32 dwSize, u32 flAllocationType, u32 flProtect);
WIN32_API(i32)    VirtualFree(void *lpAddress, u32 dwSize, u32 dwFreeType);
WIN32_API(void *) CreateFileA(s8 *lpFileName, u32 dwDesiredAccess, u32 dwShareMode, void *, u32 dwCreationDisposition, u32 dwFlagsAndAttributes, void *hTemplateFile);
WIN32_API(u32)    GetFileSize(void *hFile, u32 *lpFileSizeHigh);
WIN32_API(i32)    ReadFile(void *hFile, void *lpBuffer, u32 nNumberOfBytesToRead, u32 *lpNumberOfBytesRead, void *lpOverlapped);
WIN32_API(i32)    WriteFile(void *hFile, void *lpBuffer, u32 nNumberOfBytesToWrite, u32 *lpNumberOfBytesWritten, void *lpOverlapped);
WIN32_API(i32)    CompareFileTime(FILETIME *lpFileTime1, FILETIME *lpFileTime2);
WIN32_API(i32)    GetFileAttributesExA(s8 *lpFileName, u32 fInfoLevelId, void *lpFileInformation);
WIN32_API(void)   Sleep(u32 dwMilliseconds);
WIN32_API(void)   ExitProcess(u32 uExitCode);
WIN32_API(i32)    PeekMessageA(MSG* lpMsg, void *hWnd, u32 wMsgFilterMin, u32 wMsgFilterMax, u32 wRemoveMsg);
WIN32_API(i32)    GetMessageA(MSG* lpMsg, void *hWnd, u32 wMsgFilterMin, u32 wMsgFilterMax);
WIN32_API(i64)    DispatchMessageA(MSG *lpMsg);
WIN32_API(i64)    DefWindowProcA(void *hWnd, u32 Msg, u64 wParam, i64 lParam);
WIN32_API(i64)    SetWindowLongPtrA(void *hWnd, i32 nIndex, i64 dwNewLong);
WIN32_API(i64)    GetWindowLongPtrA(void *hWnd, i32 nIndex);
WIN32_API(void *) GetModuleHandleA(s8 *lpModuleName);
WIN32_API(void *) LoadCursorA(void *hInstance, s8 *lpCursorName);
WIN32_API(void *) LoadIconA(void *hInstance, s8 *lpIconName);
WIN32_API(u16)    RegisterClassA(WNDCLASSA *lpWndClass);
WIN32_API(void *) CreateWindowExA(u32 dwExStyle, s8 *lpClassName, s8 *lpWindowName, u32 dwStyle, i32 X, i32 Y, i32 nWidth, i32 nHeight, void *hWndParent, void *hMenu, void *hInstance, void *lpParam);
WIN32_API(void *) GetDC(void *hWnd);
WIN32_API(i32)    ReleaseDC(void *hWnd, void *hDC);
WIN32_API(i32)    SwapBuffers(void *unnamedParam1);
WIN32_API(i32)    ChoosePixelFormat(void *hdc, PIXELFORMATDESCRIPTOR *ppfd);
WIN32_API(i32)    SetPixelFormat(void *hdc, i32 format, PIXELFORMATDESCRIPTOR *ppfd);
WIN32_API(i32)    DescribePixelFormat(void *hdc, i32 iPixelFormat, u32 nBytes, PIXELFORMATDESCRIPTOR* ppfd);
WIN32_API(i32)    ShowWindow(void *hWnd, i32 nCmdShow);
WIN32_API(i32)    DestroyWindow(void *hWnd);
WIN32_API(i32)    AdjustWindowRect(RECT* lpRect, u32 dwStyle, i32 bMenu);
WIN32_API(i32)    QueryPerformanceCounter(i64 *lpPerformanceCount);
WIN32_API(i32)    QueryPerformanceFrequency(i64 *lpFrequency);
WIN32_API(s8 *)   GetCommandLineA(void);
WIN32_API(i32)    RegisterRawInputDevices(RAWINPUTDEVICE* pRawInputDevices, u32 uiNumDevices, u32 cbSize);
WIN32_API(u32)    GetRawInputData(void *hRawInput, u32 uiCommand, void *pData, u32 *pcbSize, u32 cbSizeHeader);
WIN32_API(i32)    GetCursorPos(POINT *lpPoint);
WIN32_API(i32)    ScreenToClient(void *hWnd, POINT *lpPoint);

WIN32_API(i32)    GetWindowLongA(void *hWnd, i32 nIndex);
WIN32_API(i32)    GetWindowPlacement(void *hWnd, WINDOWPLACEMENT *lpwndpl);
WIN32_API(i32)    GetMonitorInfoA(void *hMonitor, MONITORINFO* lpmi);
WIN32_API(void *) MonitorFromWindow(void *hwnd, u32 dwFlags);
WIN32_API(i32)    SetWindowLongA(void *hWnd, i32 nIndex, i32 dwNewLong);
WIN32_API(i32)    SetWindowPos(void *hWnd, void *hWndInsertAfter, i32 X, i32 Y, i32 cx, i32 cy, u32 uFlags);
WIN32_API(i32)    SetWindowPlacement(void *hWnd, WINDOWPLACEMENT *lpwndpl);
WIN32_API(i32)    GetClientRect(void *hWnd, RECT* lpRect);

WIN32_API(void *) GetCurrentProcess(void);
WIN32_API(u32)    GetCurrentProcessId(void);
WIN32_API(i32)    SetPriorityClass(void *hProcess, u32 dwPriorityClass);
WIN32_API(void *) GetCurrentThread(void);
WIN32_API(i32)    SetThreadPriority(void *hThread, i32 nPriority);
WIN32_API(u32)    SetThreadExecutionState(u32 esFlags);
WIN32_API(i32)    GetProcessHandleCount(void* hProcess, u32* pdwHandleCount);
WIN32_API(void *) CreateToolhelp32Snapshot(u32 dwFlags, u32 th32ProcessID);
WIN32_API(i32)    Thread32First(void* hSnapshot, THREADENTRY32* lpte);
WIN32_API(i32)    Thread32Next(void* hSnapshot, THREADENTRY32* lpte);

/* WGL */
WIN32_API(void *) wglCreateContext(void *unnamedParam1);
WIN32_API(void *) wglGetCurrentContext(void);
WIN32_API(void *) wglGetCurrentDC(void);
WIN32_API(i32)    wglDeleteContext(void *unnamedParam1);
WIN32_API(i32)    wglMakeCurrent(void *unnamedParam1, void *unnamedParam2);
WIN32_API(PROC)   wglGetProcAddress(s8 *unnamedParam1);
/* clang-format on */

/* #############################################################################
 * # OpenGL Functions not automatically provided by win32 opengl32
 * # Needs to be loaded during runtime
 * #############################################################################
 */
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_TYPE_RGBA_ARB 0x202B
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_ALPHA_BITS_ARB 0x201B
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002

typedef i32 (*PFNWGLCHOOSEPIXELFORMATARBPROC)(void *hdc, i32 *piAttribIList, f32 *pfAttribFList, u32 nMaxFormats, i32 *piFormats, u32 *nNumFormats);
static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;

typedef void *(*PFNWGLCREATECONTEXTATTRIBSARBPROC)(void *hDC, void *hShareContext, i32 *attribList);
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

typedef i32 (*PFNWGLSWAPINTERVALEXTPROC)(i32 interval);
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

#endif /* WIN32_FATHOM_API_H */