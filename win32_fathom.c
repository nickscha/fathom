/* win32_fathom.c - v0.1 - public domain data structures - nickscha 2026

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#include "fathom_types.h"
#include "fathom_font.h"
#include "fathom_string_builder.h"
#include "fathom_opengl.h"
#include "win32_fathom_api.h"
#include "win32_fathom_xinput.h"

/* Unfortunaly "modern" compilers sometimes inject memset intrinsics in the generated code
 * even if the application does not call memset and even with -fno-builtin, ... set.
 * Therefore we have to provide our own memset function.
 */
#ifdef _MSC_VER
#pragma function(memset)
#endif
void *memset(void *dest, i32 c, u32 count)
{
  s8 *bytes = (s8 *)dest;
  while (count--)
  {
    *bytes++ = (s8)c;
  }
  return dest;
}

/* #############################################################################
 * # [SECTION] Force Discrete GPU
 * #############################################################################
 */
__declspec(dllexport) u32 NvOptimusEnablement = 0x00000001;         /* NVIDIA Force discrete GPU */
__declspec(dllexport) i32 AmdPowerXpressRequestHighPerformance = 1; /* AMD Force discrete GPU    */

/* #############################################################################
 * # [SECTION] WIN32 specifiy functions
 * #############################################################################
 */
FATHOM_API void win32_print(s8 *str)
{
  static u32 written;
  static void *log_file;

  if (!log_file)
  {
    log_file = CreateFileA("fathom.log", GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
  }

  {
    s8 *p = str;
    u32 len = 0;

    while (*p++)
    {
      len++;
    }

    WriteFile(log_file, str, len, &written, 0);
  }
}

FATHOM_API u8 *win32_file_read(s8 *filename, u32 *file_size_out)
{
  void *hFile = INVALID_HANDLE;
  u32 fileSize = 0;
  u32 bytesRead = 0;

  u8 *buffer = 0;
  i32 attempt;

  /* Retry loop for hot-reload: file might be locked or partially written */
  for (attempt = 0; attempt < 4; ++attempt)
  {
    hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);

    if (hFile != INVALID_HANDLE)
    {
      break;
    }

    Sleep(5); /* Small delay, adjust as needed */
  }

  if (hFile == INVALID_HANDLE)
  {
    return FATHOM_NULL;
  }

  fileSize = GetFileSize(hFile, 0);

  if (fileSize == INVALID_FILE_SIZE || fileSize == 0)
  {
    CloseHandle(hFile);
    return FATHOM_NULL;
  }

  buffer = (u8 *)VirtualAlloc(0, fileSize + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

  if (!buffer)
  {
    CloseHandle(hFile);
    return FATHOM_NULL;
  }

  if (!ReadFile(hFile, buffer, fileSize, &bytesRead, 0) || bytesRead != fileSize)
  {
    VirtualFree(buffer, 0, MEM_RELEASE);
    CloseHandle(hFile);
    return FATHOM_NULL;
  }

  buffer[fileSize] = '\0';

  *file_size_out = fileSize;
  CloseHandle(hFile);
  return buffer;
}

FATHOM_API FATHOM_INLINE FILETIME win32_file_mod_time(s8 *file)
{
  static FILETIME empty = {0, 0};
  WIN32_FILE_ATTRIBUTE_DATA fad;
  return GetFileAttributesExA(file, 0, &fad) ? fad.ftLastWriteTime : empty;
}

FATHOM_API FATHOM_INLINE u8 win32_enable_high_priority(void)
{
  /* TODO(nickscha): Check integrated plus discrete GPU
   *
   * It was noticed that on a device with a integrated GPU (intel)
   * and a discrete GPU (NVIDIA) the fans were spinning high even
   * though there was only 1.5% total CPU and 2.4% GPU usage.
   * Check carefully when these priority functions can be set safely.
   *
  if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
  {
    return 1;
  }

  if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST))
  {
    return 1;
  }

  if (!SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED))
  {
    return 1;
  }
  */
  (void)HIGH_PRIORITY_CLASS;
  (void)THREAD_PRIORITY_HIGHEST;
  (void)ES_CONTINUOUS;
  (void)ES_DISPLAY_REQUIRED;
  (void)ES_SYSTEM_REQUIRED;

  return 1;
}

FATHOM_API FATHOM_INLINE u8 win32_enable_dpi_awareness(void)
{
  /* Try Windows 10 / 11 (Per-Monitor V2) */
  void *user32 = GetModuleHandleA("user32.dll");

  void *shcore;

  if (user32)
  {
    typedef i32(__stdcall * SetProcessDpiAwarenessContextProc)(void *);
    SetProcessDpiAwarenessContextProc setProcessDpiAwarenessContext;

    *(void **)(&setProcessDpiAwarenessContext) = GetProcAddress(user32, "SetProcessDpiAwarenessContext");

    if (setProcessDpiAwarenessContext && setProcessDpiAwarenessContext((void *)-4)) /* DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2  */
    {
      return 1;
    }
  }

  shcore = LoadLibraryA("Shcore.dll");

  if (shcore)
  {
    typedef i32(__stdcall * SetProcessDpiAwarenessProc)(i32);
    SetProcessDpiAwarenessProc setDpiAwareness;

    *(void **)(&setDpiAwareness) = GetProcAddress(shcore, "SetProcessDpiAwareness");

    if (setDpiAwareness)
    {
      setDpiAwareness(2); /* PROCESS_PER_MONITOR_DPI_AWARE */
    }

    FreeLibrary(shcore);
  }
  else
  {
    SetProcessDPIAware();
  }

  return 1;
}

FATHOM_API FATHOM_INLINE u8 win32_enable_high_resolution_timer(void)
{
  void *winmm = LoadLibraryA("Winmm.dll");

  if (winmm)
  {
    typedef u32(__stdcall * timeBeginPeriodProc)(u32);
    timeBeginPeriodProc timeBeginPeriod;

    u32 res = 0;

    *(void **)(&timeBeginPeriod) = GetProcAddress(winmm, "timeBeginPeriod");

    if (!timeBeginPeriod)
    {
      return 0;
    }

    res = timeBeginPeriod(1);

    FreeLibrary(winmm);

    /* TIMERR_NOCANDO */
    if (res == 97)
    {
      return 0;
    }
  }

  return 1;
}

FATHOM_API i32 win32_process_thread_count(void)
{
  u32 pid = GetCurrentProcessId();
  void *snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  THREADENTRY32 te;
  i32 count = 0;

  if (snapshot == INVALID_HANDLE)
  {
    return 0;
  }

  te.dwSize = sizeof(te);

  if (Thread32First(snapshot, &te))
  {
    do
    {
      if (te.th32OwnerProcessID == pid)
      {
        count++;
      }
    } while (Thread32Next(snapshot, &te));
  }

  CloseHandle(snapshot);

  return count;
}

typedef struct win32_controller_state
{

  u8 button_a;
  u8 button_b;
  u8 button_x;
  u8 button_y;

  u8 shoulder_left;
  u8 shoulder_right;

  u8 trigger_left;
  u8 trigger_right;

  u8 dpad_up;
  u8 dpad_down;
  u8 dpad_left;
  u8 dpad_right;

  u8 stick_left;
  u8 stick_right;

  u8 start;
  u8 back;

  f32 stick_left_x;
  f32 stick_left_y;
  f32 stick_right_x;
  f32 stick_right_y;

  f32 trigger_left_value;
  f32 trigger_right_value;

  u8 id; /* The XInput id associated with this controller */
  u8 connected;
  u8 check_needed; /* If a device is plugged in or disconnected we should check XInput controller state again */

} win32_controller_state;

typedef struct process_memory_info
{
  u64 private_bytes; /* Commit charge (what you asked for) */
  u64 working_set;   /* RAM currently used */
  u64 peak_working_set;
} process_memory_info;

u8 win32_process_memory(process_memory_info *out)
{
  typedef i32(__stdcall * GetProcessMemoryInfo_Fn)(void *, PROCESS_MEMORY_COUNTERS_EX *, u32);
  static GetProcessMemoryInfo_Fn pGetMemInfo = 0;
  static i32 initialized = 0;

  PROCESS_MEMORY_COUNTERS_EX pmc;

  if (!initialized)
  {
    /* Win7+ */
    void *kernel32 = LoadLibraryA("kernel32.dll");

    if (kernel32)
    {
      *(void **)(&pGetMemInfo) = GetProcAddress(kernel32, "K32GetProcessMemoryInfo");
    }

    /* Vista and older */
    if (!pGetMemInfo)
    {
      void *psapi = LoadLibraryA("psapi.dll");

      if (psapi)
      {
        *(void **)(&pGetMemInfo) = GetProcAddress(psapi, "GetProcessMemoryInfo");
      }
    }

    initialized = 1;
  }

  if (!pGetMemInfo)
  {
    return 0;
  }

  pmc.cb = sizeof(pmc);

  if (!pGetMemInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
  {
    return 0;
  }

  out->private_bytes = (u64)pmc.PrivateUsage;
  out->working_set = (u64)pmc.WorkingSetSize;
  out->peak_working_set = (u64)pmc.PeakWorkingSetSize;

  return 1;
}

#define KEYS_COUNT 256

typedef struct win32_fathom_state
{

  u32 window_width;
  u32 window_height;

  u32 window_width_pending;
  u32 window_height_pending;

  f32 window_clear_color_g;
  f32 window_clear_color_b;
  f32 window_clear_color_r;
  f32 window_clear_color_a;

  i32 iFrame;        /* Frames processed count               */
  f64 iTime;         /* Total elapsed time in seconds        */
  f64 iTimeDelta;    /* Current render frame time in seconds */
  f64 iFrameRate;    /* Frame Rate per second                */
  f64 iFrameRateRaw; /* Frame Rate per second raw (no cap)   */

  u8 running;
  u8 window_minimized;
  u8 window_size_changed;

  u8 shader_paused;
  u8 ui_enabled;
  u8 fullscreen_enabled;
  u8 borderless_enabled;
  u8 screen_recording_enabled;
  u8 screen_recording_initialized;

  u32 target_frames_per_second;

  s8 *window_title;

  void *window_handle;
  void *device_context;

  /* Input state */
  i32 mouse_dx; /* Relative movement delta for x  */
  i32 mouse_dy; /* Relative movement delta for y  */
  i32 mouse_x;  /* Mouse position on screen for x */
  i32 mouse_y;  /* Mouse position on screen for y */
  f32 mouse_scroll;
  u8 mouse_left_is_down;
  u8 mouse_left_was_down;
  u8 mouse_right_is_down;
  u8 mouse_right_was_down;

  /* State Examples:
    Key Pressed:  state.keys_is_down[0x0D] && !state.keys_was_down[0x0D]
    Key Released: !state.keys_is_down[0x0D] && state.keys_was_down[0x0D]

    Example of a Toggle switch (when pressed first toggles on, when pressed second time toggles off):

    static u8 ui_enabled = 0;

    if (state.keys_is_down[0x70] && !state.keys_was_down[0x70])
    {
      ui_enabled = !ui_enabled;
    }
  */
  u8 keys_is_down[KEYS_COUNT];
  u8 keys_was_down[KEYS_COUNT];

  win32_controller_state controller;

  s8 *gl_version;
  s8 *gl_renderer;
  s8 *gl_vendor;
  i32 gl_max_3d_texture_size;

  u32 mem_brick_map_bytes;
  u32 mem_atlas_bytes;
  u32 grid_sdf_invocations;
  u32 grid_active_brick_count;
  u32 grid_atlas_width;
  u32 grid_atlas_height;
  u32 grid_atlas_depth;

  i64 grid_calc_time_start;
  i64 grid_calc_time_end;

} win32_fathom_state;

FATHOM_API FATHOM_INLINE i64 win32_window_callback(void *window, u32 message, u64 wParam, i64 lParam)
{
  win32_fathom_state *state = (win32_fathom_state *)GetWindowLongPtrA(window, GWLP_USERDATA);

  i64 result = 0;

  switch (message)
  {
  case WM_ERASEBKGND:
    return 1;
  case WM_CREATE:
  {
    CREATESTRUCTA *cs = (CREATESTRUCTA *)lParam;
    state = (win32_fathom_state *)cs->lpCreateParams;
    SetWindowLongPtrA(window, GWLP_USERDATA, (i64)state);

    /* Setup raw input for mouse and keyboard */
    {
      RAWINPUTDEVICE rid[2] = {0};

      (void)RIDEV_INPUTSINK; /* Receive input even when not focused */

      rid[0].usUsagePage = 0x01;
      rid[0].usUsage = 0x06; /* Keyboard */
      rid[0].dwFlags = 0;    /* Receive input only when focused */
      rid[0].hwndTarget = window;

      rid[1].usUsagePage = 0x01;
      rid[1].usUsage = 0x02; /* Mouse */
      rid[1].dwFlags = 0;    /* Receive input only when focused */
      rid[1].hwndTarget = window;

      if (!RegisterRawInputDevices(rid, 2, sizeof(rid[0])))
      {
        win32_print("[win32] Failed to register RAWINPUT device\n");
      }
    }
  }
  break;
  case WM_CLOSE:
  case WM_QUIT:
  {
    if (!state)
    {
      break;
    }

    state->running = 0;
  }
  break;
  case WM_SIZE:
  {
    if (!state)
    {
      break;
    }

    if (wParam == SIZE_MINIMIZED)
    {
      state->window_minimized = 1;
    }
    else
    {
      state->window_minimized = 0;
      state->window_size_changed = 1;
      state->window_width_pending = (u16)(((u64)(lParam)) & 0xffff);          /* Low Word  */
      state->window_height_pending = (u16)((((u64)(lParam)) >> 16) & 0xffff); /* High Word */
    }
  }
  break;
  case WM_INPUT:
  {
    static u8 rawBuffer[128];
    RAWINPUT *raw = (RAWINPUT *)rawBuffer;

    u32 dwSize = 0;

    if (!state)
    {
      break;
    }

    GetRawInputData((RAWINPUT *)lParam, RID_INPUT, FATHOM_NULL, &dwSize, sizeof(RAWINPUTHEADER));

    if (dwSize > sizeof(rawBuffer) ||
        GetRawInputData((RAWINPUT *)lParam, RID_INPUT, raw, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
    {
      return result;
    }

    if (raw->header.dwType == RIM_TYPEKEYBOARD)
    {
      RAWKEYBOARD *keyboard = &raw->data.keyboard;

      u16 vKey = keyboard->VKey;

      if (vKey < KEYS_COUNT)
      {
        /*key->was_down = key->is_down;*/
        state->keys_is_down[vKey] = !(keyboard->Flags & RI_KEY_BREAK); /* 1 if pressed, 0 if released */
      }
    }
    else if (raw->header.dwType == RIM_TYPEMOUSE)
    {
      RAWMOUSE *mouse = &raw->data.mouse;

      i32 dx = mouse->lLastX;
      i32 dy = mouse->lLastY;

      state->mouse_dx += dx;
      state->mouse_dy -= dy;

      /* Scroll wheel */
      if (mouse->usButtonFlags & RI_MOUSE_WHEEL)
      {
        i16 wheelDelta = (i16)mouse->usButtonData;
        state->mouse_scroll += (f32)wheelDelta / (f32)WHEEL_DELTA;
      }
    }
  }
  break;
  case WM_DEVICECHANGE:
  {
    /* Check if a device notification arrived in order to find out when to requery XInput controller states */
    /* DBT_DEVNODES_CHANGED is the most reliable for USB plugging/unplugging */
    if (wParam == DBT_DEVNODES_CHANGED || wParam == DBT_DEVICEARRIVAL || wParam == DBT_DEVICEREMOVECOMPLETE)
    {
      state->controller.check_needed = 1;
    }
  }
  break;
  case WM_LBUTTONDOWN:
    state->mouse_left_is_down = 1;
    break;
  case WM_LBUTTONUP:
    state->mouse_left_is_down = 0;
    break;
  case WM_RBUTTONDOWN:
    state->mouse_right_is_down = 1;
    break;
  case WM_RBUTTONUP:
    state->mouse_right_is_down = 0;
    break;
  default:
  {
    result = DefWindowProcA(window, message, wParam, lParam);
  }
  break;
  }

  return (result);
}

static WINDOWPLACEMENT g_wpPrev = {0};

FATHOM_API void win32_window_enter_fullscreen(win32_fathom_state *state)
{
  i32 dwStyle = GetWindowLongA(state->window_handle, GWL_STYLE);

  if (dwStyle & WS_OVERLAPPEDWINDOW)
  {
    if (GetWindowPlacement(state->window_handle, &g_wpPrev))
    {
      MONITORINFO mi;
      mi.cbSize = sizeof(mi);

      GetMonitorInfoA(MonitorFromWindow(state->window_handle, MONITOR_DEFAULTTONEAREST), &mi);
      SetWindowLongA(state->window_handle, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

      SetWindowPos(
          state->window_handle, HWND_TOP,
          mi.rcMonitor.left, mi.rcMonitor.top,
          mi.rcMonitor.right - mi.rcMonitor.left,
          mi.rcMonitor.bottom - mi.rcMonitor.top,
          SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

      state->window_width_pending = (u32)(mi.rcMonitor.right - mi.rcMonitor.left);
      state->window_height_pending = (u32)(mi.rcMonitor.bottom - mi.rcMonitor.top);
      state->window_size_changed = 1;
    }
  }
}

FATHOM_API void win32_window_enter_borderless(win32_fathom_state *state)
{
  if (GetWindowPlacement(state->window_handle, &g_wpPrev))
  {
    i32 dwStyle = GetWindowLongA(state->window_handle, GWL_STYLE);

    MONITORINFO mi;
    mi.cbSize = sizeof(mi);

    GetMonitorInfoA(MonitorFromWindow(state->window_handle, MONITOR_DEFAULTTONEAREST), &mi);
    SetWindowLongA(state->window_handle, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

    SetWindowPos(
        state->window_handle, HWND_TOP,
        mi.rcMonitor.left, mi.rcMonitor.top,
        mi.rcMonitor.right - mi.rcMonitor.left,
        mi.rcMonitor.bottom - mi.rcMonitor.top,
        SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

    state->window_width_pending = (u32)(mi.rcMonitor.right - mi.rcMonitor.left);
    state->window_height_pending = (u32)(mi.rcMonitor.bottom - mi.rcMonitor.top);
    state->window_size_changed = 1;
  }
}

FATHOM_API void win32_window_enter_windowed(win32_fathom_state *state)
{
  i32 dwStyle = GetWindowLongA(state->window_handle, GWL_STYLE);

  if (!(dwStyle & WS_OVERLAPPEDWINDOW))
  {
    RECT rect;

    SetWindowLongA(state->window_handle, GWL_STYLE, WS_OVERLAPPEDWINDOW);
    SetWindowPlacement(state->window_handle, &g_wpPrev);
    SetWindowPos(state->window_handle, FATHOM_NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    GetClientRect(state->window_handle, &rect);

    state->window_width_pending = (u32)(rect.right - rect.left);
    state->window_height_pending = (u32)(rect.bottom - rect.top);
    state->window_size_changed = 1;
  }
  else
  {
    /* resize windowed mode */
    RECT rect = {0};
    rect.right = (i32)state->window_width;
    rect.bottom = (i32)state->window_height;

    AdjustWindowRect(&rect, (u32)GetWindowLongA(state->window_handle, GWL_STYLE), 0);
    SetWindowPos(state->window_handle, FATHOM_NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
  }
}

/* #############################################################################
 * # [SECTION] Font Loading and Parsing
 * #############################################################################
 */

FATHOM_API void unpack_1bit_to_8bit(
    u8 *dst, /* width * height bytes */
    u8 *src, /* packed bits */
    u32 width,
    u32 height)
{
  u32 x;
  u32 y;

  for (y = 0; y < height; ++y)
  {
    for (x = 0; x < width; ++x)
    {
      u32 bit_index = y * width + x;
      u32 byte_index = bit_index >> 3;
      u32 bit = 7 - (bit_index & 7);
      u8 b = (src[byte_index] >> bit) & 1;

      /* 1 = black, 0 = white */
      dst[bit_index] = b ? 0xFF : 0x00;
    }
  }
}

typedef enum glyph_state_flag
{
  GLYPH_STATE_NONE = 0,
  GLYPH_STATE_BLINK = 1 << 8,
  GLYPH_STATE_VFLIP = 1 << 9,
  GLYPH_STATE_HFLIP = 1 << 10

} glyph_state_flag;

typedef struct glyph
{
  u16 x;
  u16 y;
  u16 packed_glyph_state; /* packs the glyph_index | glyph_state_flag */
  u16 packed_color;       /* rgb565_color */
} glyph;

FATHOM_API u16 pack_rgb565(u8 r, u8 g, u8 b)
{
  return (u16)(((r >> 3) << 11) |
               ((g >> 2) << 5) |
               ((b >> 3) << 0));
}

FATHOM_API FATHOM_INLINE u16 glyph_advance_x(f32 font_scale)
{
  return (u16)((f32)(FATHOM_FONT_GLYPH_WIDTH + 1) * font_scale);
}

FATHOM_API FATHOM_INLINE u16 glyph_advance_y(f32 font_scale)
{
  return (u16)(((f32)FATHOM_FONT_GLYPH_HEIGHT * font_scale) + (f32)FATHOM_FONT_GLYPH_HEIGHT);
}

FATHOM_API void glyph_add(
    glyph *glyph_buffer,     /* pre allocated glyph array */
    u32 glyph_buffer_size,   /* maximum size */
    u32 *glyph_buffer_count, /* current size */
    s8 *text,                /* text to append */
    u16 *offset_x,           /* current offset x. Note: this is advanced by this function */
    u16 *offset_y,           /* current offset y. Note: this is advanced by this function */
    u16 color_rgb565,        /* rgb565 packed color. Use: pack_rgb565(255, 255, 255); */
    glyph_state_flag state,  /* GLYPH_STATE_BLINK | GLYPH_STATE_VFLIP | GLYPH_STATE_HFLIP */
    f32 font_scale)
{
  u16 start_x = *offset_x;

  u16 advance_x = glyph_advance_x(font_scale);
  u16 advance_y = glyph_advance_y(font_scale);

  while (*text && *glyph_buffer_count < glyph_buffer_size)
  {
    s8 c = *text++;
    i32 glyph_index;

    if (c == '\n')
    {
      *offset_x = start_x;
      *offset_y += advance_y;
      continue;
    }

    /* skip spaces explicitly */
    if (c == ' ')
    {
      *offset_x += advance_x;
      continue;
    }

    glyph_index = fathom_font_s8_to_index(c);

    if (glyph_index < 0)
    {
      continue;
    }

    glyph_buffer[*glyph_buffer_count].x = *offset_x;
    glyph_buffer[*glyph_buffer_count].y = *offset_y;
    glyph_buffer[*glyph_buffer_count].packed_glyph_state = (u16)glyph_index | (u16)state;
    glyph_buffer[(*glyph_buffer_count)++].packed_color = color_rgb565;

    *offset_x += advance_x;
  }
}

/* #############################################################################
 * # [SECTION] OpenGL Context Creation and Shader Management
 * #############################################################################
 */
typedef struct shader_header
{
  u32 created;
  u32 program;
  u8 had_failure;

} shader_header;

typedef struct shader_main
{
  shader_header header;

  i32 loc_iResolution;
  i32 loc_iTime;
  i32 loc_iTimeDelta;
  i32 loc_iFrame;
  i32 loc_iFrameRate;
  i32 loc_iMouse;
  i32 loc_iTextureInfo;
  i32 loc_iTexture;
  i32 loc_iController;

  i32 loc_brick_map_texture;
  i32 loc_atlas_texture;
  i32 loc_material_texture;

  i32 loc_atlas_brick_dim;

  i32 loc_inverse_atlas_size;
  i32 loc_grid_start;
  i32 loc_cell_size;
  i32 loc_cell_diagonal;
  i32 loc_truncation;

  /* Camera */
  i32 loc_camera_position;
  i32 loc_camera_forward;
  i32 loc_camera_right;
  i32 loc_camera_up;
  i32 loc_camera_fov;

} shader_main;

typedef struct shader_font
{
  shader_header header;

  i32 loc_iResolution;
  i32 loc_iTime;
  i32 loc_iTextureInfo;
  i32 loc_iTexture;
  i32 loc_iFontScale;

} shader_font;

typedef struct shader_recording
{
  shader_header header;

  i32 loc_iResolution;
  i32 loc_iTime;

} shader_recording;

static u32 opengl_failed_function_load_count = 0;

FATHOM_API PROC win32_opengl_load_function(s8 *name)
{
  PROC gl_function = wglGetProcAddress(name);

  /* Some GPU drivers do not return a valid null pointer if requested OpenGL function is not available */
  if (gl_function == (PROC)0 || gl_function == (PROC)0x1 || gl_function == (PROC)0x2 || gl_function == (PROC)0x3 || gl_function == (PROC)-1)
  {
    /* Try falling back to opengl32 */
    static void *gl_lib = 0;
    void *object_ptr;

    if (!gl_lib)
    {
      gl_lib = GetModuleHandleA("opengl32.dll");
    }

    object_ptr = (void *)GetProcAddress(gl_lib, name);

    gl_function = *(PROC *)&object_ptr;
  }

  if (gl_function == (PROC)0)
  {
    win32_print("[opengl] \"");
    win32_print(name);
    win32_print("\" not found!\n");

    opengl_failed_function_load_count++;
  }

  return gl_function;
}

FATHOM_API FATHOM_INLINE i32 opengl_create_context(win32_fathom_state *state)
{
  void *window_instance = GetModuleHandleA(0);
  WNDCLASSA window_class = {0};
  u32 window_style = WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;
  RECT rect = {0};

  void *fake_window;
  void *fake_device_context;
  void *fake_rc;
  i32 fake_pixel_format;
  PIXELFORMATDESCRIPTOR fake_pfd = {0};

  window_class.style = CS_OWNDC;
  window_class.lpfnWndProc = win32_window_callback;
  window_class.hInstance = window_instance;
  window_class.hCursor = LoadCursorA(0, IDC_ARROW);
  window_class.hIcon = LoadIconA(window_instance, MAKEINTRESOURCEA(1));
  window_class.hbrBackground = 0;
  window_class.lpszClassName = state->window_title;

  if (!RegisterClassA(&window_class))
  {
    return 0;
  }

  fake_window = CreateWindowExA(
      0,
      window_class.lpszClassName,
      window_class.lpszClassName,
      WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
      0, 0,
      1, 1,
      0, 0,
      window_instance, 0);

  if (!fake_window)
  {
    return 0;
  }

  fake_device_context = GetDC(fake_window);

  fake_pfd.nSize = sizeof(fake_pfd);
  fake_pfd.nVersion = 1;
  fake_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  fake_pfd.iPixelType = PFD_TYPE_RGBA;
  fake_pfd.cColorBits = 32;
  fake_pfd.cAlphaBits = 8;
  fake_pfd.cDepthBits = 24;

  fake_pixel_format = ChoosePixelFormat(fake_device_context, &fake_pfd);

  if (!fake_pixel_format || !SetPixelFormat(fake_device_context, fake_pixel_format, &fake_pfd))
  {
    return 0;
  }

  fake_rc = wglCreateContext(fake_device_context);

  if (!fake_rc || !wglMakeCurrent(fake_device_context, fake_rc))
  {
    return 0;
  }

  rect.right = (i32)state->window_width;
  rect.bottom = (i32)state->window_height;
  AdjustWindowRect(&rect, window_style, 0);

  state->window_handle = CreateWindowExA(
      0,
      window_class.lpszClassName,
      window_class.lpszClassName,
      window_style,
      0, 0,
      rect.right - rect.left,
      rect.bottom - rect.top,
      0, 0,
      window_instance,
      state /* Pass pointer to user data to the window callback */
  );

  /* Modal window */
  SetWindowPos(state->window_handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

  state->device_context = GetDC(state->window_handle);

#ifdef _MSC_VER
#pragma warning(disable : 4068)
#endif
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
  /* Core WGL functions */
  wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)win32_opengl_load_function("wglChoosePixelFormatARB");
  wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)win32_opengl_load_function("wglCreateContextAttribsARB");
  wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)win32_opengl_load_function("wglSwapIntervalEXT");
#pragma GCC diagnostic pop

  fathom_opengl_load_functions(win32_opengl_load_function);

  if (opengl_failed_function_load_count > 0)
  {
    win32_print("[opengl] Some of the required OpenGL functions are not available on your machine (see log file above)!\n");
    return 0;
  }

  /* Set Pixel Format */
  {

    i32 pixel_attributes[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_ALPHA_BITS_ARB, 8,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        0};

    i32 pixel_format_id;
    u32 num_formats;

    if (!wglChoosePixelFormatARB(state->device_context, pixel_attributes, 0, 1, &pixel_format_id, &num_formats) || !num_formats)
    {
      return 0;
    }

    SetPixelFormat(state->device_context, pixel_format_id, 0);
  }

  /* Create the OpenGL context */
  {
    i32 context_attributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0};

    void *rc = wglCreateContextAttribsARB(state->device_context, 0, context_attributes);

    wglMakeCurrent(0, 0);
    wglDeleteContext(fake_rc);
    ReleaseDC(fake_window, fake_device_context);
    DestroyWindow(fake_window);

    if (!rc || !wglMakeCurrent(state->device_context, rc))
    {
      return 0;
    }
  }

  /* Disable VSync */
  if (wglSwapIntervalEXT)
  {
    wglSwapIntervalEXT(0);
  }

  /* Print opengl information */
  state->gl_vendor = (s8 *)glGetString(GL_VENDOR);
  state->gl_renderer = (s8 *)glGetString(GL_RENDERER);
  state->gl_version = (s8 *)glGetString(GL_VERSION);

  win32_print("[opengl] vendor  : ");
  win32_print(state->gl_vendor);
  win32_print("\n");

  win32_print("[opengl] renderer: ");
  win32_print(state->gl_renderer);
  win32_print("\n");

  win32_print("[opengl] version : ");
  win32_print(state->gl_version);
  win32_print("\n");

  return 1;
}

FATHOM_API u32 opengl_shader_load(shader_header *shader, s8 *shader_code_vertex, s8 *shader_code_fragment)
{
  u32 new_program;

  if (!fathom_opengl_shader_create(&new_program, shader_code_vertex, shader_code_fragment, win32_print))
  {
    win32_print("[opengl] compile failed, keeping old shader is present\n");
    shader->had_failure = 1;
    return 0;
  }

  shader->had_failure = 0;

  /* If there has been already a shader created delete the old one */
  if (shader->created)
  {
    glDeleteProgram(shader->program);
  }

  shader->program = new_program;

  glUseProgram(shader->program);

  shader->created = 1;

  return 1;
}

static s8 *shader_code_vertex =
    "#version 330 core\n"
    "vec2 quad[3]=vec2[3]("
    "vec2(-1.0,-1.0),"
    "vec2(3.0,-1.0),"
    "vec2(-1.0,3.0)"
    ");"
    "void main(){gl_Position=vec4(quad[gl_VertexID],0.0,1.0);}";

FATHOM_API void opengl_shader_load_shader_main(shader_main *shader, s8 *shader_file_name)
{

  u32 size = 0;
  u8 *shader_code_fragment = win32_file_read(shader_file_name, &size);

  if (!shader_code_fragment || size < 1)
  {
    return;
  }

  if (opengl_shader_load(&shader->header, shader_code_vertex, (s8 *)shader_code_fragment))
  {
    shader->loc_iResolution = glGetUniformLocation(shader->header.program, "iResolution");
    shader->loc_iTime = glGetUniformLocation(shader->header.program, "iTime");
    shader->loc_iTimeDelta = glGetUniformLocation(shader->header.program, "iTimeDelta");
    shader->loc_iFrame = glGetUniformLocation(shader->header.program, "iFrame");
    shader->loc_iFrameRate = glGetUniformLocation(shader->header.program, "iFrameRate");
    shader->loc_iMouse = glGetUniformLocation(shader->header.program, "iMouse");
    shader->loc_iTextureInfo = glGetUniformLocation(shader->header.program, "iTextureInfo");
    shader->loc_iTexture = glGetUniformLocation(shader->header.program, "iTexture");
    shader->loc_iController = glGetUniformLocation(shader->header.program, "iController");

    shader->loc_brick_map_texture = glGetUniformLocation(shader->header.program, "uBrickMap");
    shader->loc_atlas_texture = glGetUniformLocation(shader->header.program, "uAtlas");
    shader->loc_material_texture = glGetUniformLocation(shader->header.program, "uMaterial");

    shader->loc_atlas_brick_dim = glGetUniformLocation(shader->header.program, "uAtlasBrickDim");
    shader->loc_inverse_atlas_size = glGetUniformLocation(shader->header.program, "uInvAtlasSize");
    shader->loc_grid_start = glGetUniformLocation(shader->header.program, "uGridStart");
    shader->loc_cell_size = glGetUniformLocation(shader->header.program, "uCellSize");
    shader->loc_cell_diagonal = glGetUniformLocation(shader->header.program, "uCellDiagonal");
    shader->loc_truncation = glGetUniformLocation(shader->header.program, "uTruncation");

    /* Camera */
    shader->loc_camera_position = glGetUniformLocation(shader->header.program, "camera_position");
    shader->loc_camera_forward = glGetUniformLocation(shader->header.program, "camera_forward");
    shader->loc_camera_right = glGetUniformLocation(shader->header.program, "camera_right");
    shader->loc_camera_up = glGetUniformLocation(shader->header.program, "camera_up");
    shader->loc_camera_fov = glGetUniformLocation(shader->header.program, "camera_fov");
  }

  VirtualFree(shader_code_fragment, 0, MEM_RELEASE);
}

FATHOM_API void opengl_shader_load_shader_font(shader_font *shader)
{
  static s8 *shader_font_code_vertex =
      "#version 330 core\n"
      "layout(location=0)in vec2 p;"
      "layout(location=1)in uvec4 g;"
      "uniform vec3 r;"
      "uniform vec4 t;"
      "uniform float s;"
      "out vec2 vUV;"
      "out vec3 vC;"
      "flat out uint b;"
      "void main(){"
      "float i=g.z&255u,C=t.x/t.z;"
      "b=(g.z>>8)&1u;"
      "float v=(g.z>>9)&1u,h=(g.z>>10)&1u;"
      "vec2 Q=vec2(p.x*(1.-2.*h)+h,p.y*(1.-2.*v)+v);"
      "vec2 p2=vec2(g.xy)+Q*t.zw*s;"
      "vec2 N=p2/r.xy*2.-1.;"
      "gl_Position=vec4(N.x,-N.y,0,1);"
      "vUV=vec2((mod(i,C)+p.x)*t.z/t.x,(floor(i/C)+p.y)*t.w/t.y);"
      "uint K=g.w;"
      "vC=vec3((K>>11&31u)/31.,(K>>5&63u)/63.,(K&31u)/31.);"
      "}";

  static s8 *shader_font_code_fragment =
      "#version 330 core\n"
      "in vec2 vUV;"
      "in vec3 vC;"
      "flat in uint b;"
      "uniform sampler2D iTexture;"
      "uniform float iTime;"
      "out vec4 F;"
      "void main(){"
      "float a=texture(iTexture,vUV).r;"
      "if(b==1u&&fract(iTime)<.5)a=0.;" /* Blink Support */
      "F=vec4(vC,a);"
      "}";

  if (opengl_shader_load(&shader->header, shader_font_code_vertex, shader_font_code_fragment))
  {
    shader->loc_iResolution = glGetUniformLocation(shader->header.program, "r");
    shader->loc_iTextureInfo = glGetUniformLocation(shader->header.program, "t");
    shader->loc_iFontScale = glGetUniformLocation(shader->header.program, "s");
    shader->loc_iTime = glGetUniformLocation(shader->header.program, "iTime");
    shader->loc_iTexture = glGetUniformLocation(shader->header.program, "iTexture");
  }
}

FATHOM_API void opengl_shader_load_shader_recording(shader_recording *shader)
{
  static s8 *shader_code_fragment =
      "#version 330 core\n"
      "out vec4 FragColor;"
      "uniform float iTime;"
      "uniform vec3 iRes;"
      "void main()"
      "{"
      "float r=10.0;"
      "float m=10.0;"
      "vec2 p=gl_FragCoord.xy;"
      "vec2 c=vec2(iRes.x-r-m, r+m);"
      "float d=length(p-c);"
      "float blink=0.5+0.5*sin(iTime*12.0);"
      "float b=4.0;"
      "if ((d<=r&&blink>0.5)||(p.x<b||p.y<b||p.x>=iRes.x-b||p.y>=iRes.y-b))"
      " FragColor=vec4(1.0,0.0,0.0,blink);"
      "else"
      " discard;"
      "}";

  if (opengl_shader_load(&shader->header, shader_code_vertex, shader_code_fragment))
  {
    shader->loc_iResolution = glGetUniformLocation(shader->header.program, "iRes");
    shader->loc_iTime = glGetUniformLocation(shader->header.program, "iTime");
  }
}

#include "fathom_math_sdf.h"
#include "fathom_sparse_grid.h"

/* Simple Sphere SDF */
FATHOM_API fathom_grid_data sdf_function(fathom_vec3 position, void *user_data)
{
  fathom_grid_data d;

  f32 sphere_radius = 0.5f;
  f32 sphere = fathom_sdf_sphere(position, sphere_radius);

  fathom_vec3 box_pos = fathom_vec3_sub(position, fathom_vec3_init(-0.5f, 0.5f, -0.5f));
  fathom_mat2x2 box_rot = fathom_mat2x2_rot2d(FATHOM_DEG_TO_RAD(45.0f));
  f32 box_scale = 1.0f;

  fathom_vec2_mul_mat2x2(&box_pos.x, &box_pos.y, box_rot); /* rotate around z axis */
  fathom_vec2_mul_mat2x2(&box_pos.y, &box_pos.z, box_rot); /* rotate around x axis */

  {
    f32 box = fathom_sdf_box(fathom_vec3_divf(box_pos, box_scale), fathom_vec3_init(0.25f, 0.25f, 0.25f)) * box_scale;

    f32 ground = position.y - (-0.25f);

    d.distance = fathom_sminf(ground, fathom_sminf(sphere, box, 0.4f), 0.6f);
    d.material = 0;

    if (sphere < box && sphere < ground)
    {
      d.material = 1;
    }
    else if (box < sphere && box < ground)
    {
      d.material = 2;
    }

    ((win32_fathom_state *)user_data)->grid_sdf_invocations++;
  }

  return d;
}

FATHOM_API void fathom_create_grid(win32_fathom_state *state, fathom_sparse_grid *grid, fathom_vec3 grid_center, u32 grid_cell_count, f32 grid_cell_size)
{
  fathom_sparse_grid_initialize(grid, grid_center, grid_cell_count, grid_cell_size);

  grid->brick_map_data = VirtualAlloc(0, grid->brick_map_bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

  fathom_sparse_grid_pass_01_fill_brick_map(grid, sdf_function, state);

  grid->atlas_data = VirtualAlloc(0, grid->atlas_bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  grid->material_data = VirtualAlloc(0, grid->atlas_bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

  state->mem_brick_map_bytes = grid->brick_map_bytes;
  state->mem_atlas_bytes = grid->atlas_bytes;
  state->grid_active_brick_count = grid->brick_map_active_bricks_count;
  state->grid_atlas_width = grid->atlas_width;
  state->grid_atlas_height = grid->atlas_height;
  state->grid_atlas_depth = grid->atlas_depth;

  QueryPerformanceCounter(&state->grid_calc_time_start);
  fathom_sparse_grid_pass_02_fill_atlas(grid, sdf_function, state);
  QueryPerformanceCounter(&state->grid_calc_time_end);
}

FATHOM_API void fathom_render_grid(win32_fathom_state *state, shader_main *main_shader, u32 main_vao)
{
  static u8 grid_initialized = 0;
  static fathom_sparse_grid grid_lod0 = {0};
  static fathom_sparse_grid grid_lod1 = {0};
  static fathom_sparse_grid grid_lod2 = {0};
  static u32 brickMapTex;
  static u32 atlasTex;
  static u32 materialTex;

  /* Camera */
  static fathom_vec3 camera_position;
  static fathom_vec3 camera_forward;
  static fathom_vec3 camera_right;
  static fathom_vec3 camera_up;
  static f32 camera_fov = 1.5f;

  if (!grid_initialized)
  {
    u32 grid_cell_count = 128;
    f32 grid_cell_size = 1.0f / 16.0f;

    fathom_create_grid(state, &grid_lod0, fathom_vec3_zero, grid_cell_count, grid_cell_size);        /* LOD 0 */
    fathom_create_grid(state, &grid_lod1, fathom_vec3_zero, grid_cell_count, grid_cell_size * 2.0f); /* LOD 1 */
    fathom_create_grid(state, &grid_lod2, fathom_vec3_zero, grid_cell_count, grid_cell_size * 4.0f); /* LOD 2 */

    /* Brick Map */
    glGenTextures(1, &brickMapTex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_3D, brickMapTex);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_R16UI,
                 (i32)grid_lod0.brick_map_dimensions,
                 (i32)grid_lod0.brick_map_dimensions,
                 (i32)grid_lod0.brick_map_dimensions,
                 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, grid_lod0.brick_map_data);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    /* Atlas Texture */
    glGenTextures(1, &atlasTex);
    glBindTexture(GL_TEXTURE_3D, atlasTex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

#ifdef FATHOM_SPARSE_GRID_QUANTIZE_U8
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8,
                 (i32)grid_lod0.atlas_width,
                 (i32)grid_lod0.atlas_height,
                 (i32)grid_lod0.atlas_depth,
                 0, GL_RED, GL_UNSIGNED_BYTE, grid_lod0.atlas_data);
#else
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8_SNORM,
                 (i32)grid_lod0.atlas_width,
                 (i32)grid_lod0.atlas_height,
                 (i32)grid_lod0.atlas_depth,
                 0, GL_RED, GL_BYTE, grid_lod0.atlas_data);
#endif

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); /* GL_LINEAR */
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); /* GL_LINEAR */
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);

    /* Material Texture */
    glGenTextures(1, &materialTex);
    glBindTexture(GL_TEXTURE_3D, materialTex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI,
                 (i32)grid_lod0.atlas_width,
                 (i32)grid_lod0.atlas_height,
                 (i32)grid_lod0.atlas_depth,
                 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, grid_lod0.material_data);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    grid_initialized = 1;
  }

  /* Camera Setup */
  {
    fathom_vec3 world_up = fathom_vec3_init(0.0f, 1.0f, 0.0f);
    fathom_vec3 camera_look_at = fathom_vec3_zero;

    camera_position = fathom_vec3_init(fathom_sinf((f32)state->iTime) * 0.5f, 1.0f, 2.0f);
    camera_forward = fathom_vec3_normalize(fathom_vec3_sub(camera_look_at, camera_position)); /* Z-Axis */
    camera_right = fathom_vec3_normalize(fathom_vec3_cross(camera_forward, world_up));        /* X-Axis */
    camera_up = fathom_vec3_normalize(fathom_vec3_cross(camera_right, camera_forward));       /* Y-Axis */
    camera_fov = 1.5f;
  }

  /******************************/
  /* Draw                       */
  /******************************/
  glUseProgram(main_shader->header.program);

  /* General uniforms */
  glUniform3f(main_shader->loc_iResolution, (f32)state->window_width, (f32)state->window_height, 1.0f);
  glUniform1f(main_shader->loc_iTime, (f32)state->iTime);

  /* Camera uniforms */
  glUniform3f(main_shader->loc_camera_position, camera_position.x, camera_position.y, camera_position.z);
  glUniform3f(main_shader->loc_camera_forward, camera_forward.x, camera_forward.y, camera_forward.z);
  glUniform3f(main_shader->loc_camera_right, camera_right.x, camera_right.y, camera_right.z);
  glUniform3f(main_shader->loc_camera_up, camera_up.x, camera_up.y, camera_up.z);
  glUniform1f(main_shader->loc_camera_fov, camera_fov);

  /* Grid uniforms */
  glUniform3i(main_shader->loc_atlas_brick_dim, (i32)grid_lod0.atlas_bricks_per_row, (i32)0, (i32)0);
  glUniform3f(main_shader->loc_inverse_atlas_size, grid_lod0.atlas_inverse_dimensions.x, grid_lod0.atlas_inverse_dimensions.y, grid_lod0.atlas_inverse_dimensions.z);
  glUniform3f(main_shader->loc_grid_start, grid_lod0.start.x, grid_lod0.start.y, grid_lod0.start.z);
  glUniform1f(main_shader->loc_cell_size, grid_lod0.cell_size);
  glUniform1f(main_shader->loc_truncation, grid_lod0.truncation_distance);

  /* Bind textures to texture units */
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D, brickMapTex);
  glUniform1i(main_shader->loc_brick_map_texture, 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_3D, atlasTex);
  glUniform1i(main_shader->loc_atlas_texture, 1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_3D, materialTex);
  glUniform1i(main_shader->loc_material_texture, 2);

  glBindVertexArray(main_vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

/* #############################################################################
 * # [SECTION] Main Entry Point
 * #############################################################################
 */
FATHOM_API i32 start(i32 argc, u8 **argv)
{
  /* Default fragment shader file name to load if no file is passed as an argument in cli */
  s8 *fragment_shader_file_name = (argv && argc > 1) ? (s8 *)argv[1] : "fathom.fs";

  win32_fathom_state state = {0};
  shader_main main_shader = {0};
  shader_font font_shader = {0};
  shader_recording recording_shader = {0};

  u32 main_vao;
  u32 font_vao;
  u32 glyph_vbo;

  state.running = 1;
  state.window_title = "fathom v0.1 (F1=Debug UI, F2=Screen Recording, R=Reset, P=Pause, F9=Borderless, F11=Fullscreen)";
  state.window_width = 800;
  state.window_height = 600;
  state.window_clear_color_r = 0.2f;
  state.window_clear_color_g = 0.2f;
  state.window_clear_color_b = 0.2f;
  state.target_frames_per_second = 30; /* 60 FPS, 0 = unlimited */
  state.controller.check_needed = 1;   /* By default we have to query first XInput state */

  (void)GL_TEXTURE1;
  (void)GL_RED_INTEGER;
  (void)GL_LINEAR;
  (void)GL_TRIANGLES;
  (void)GL_PACK_ALIGNMENT;
  (void)GL_RGB;
  (void)GL_TEXTURE_3D;
  (void)GL_TEXTURE_WRAP_R;
  (void)GL_R16UI;

  /******************************/
  /* Set Process Priorities     */
  /******************************/
  if (!win32_enable_high_priority())
  {
    win32_print("[WARNING] Failed to set high priority process\n");
  }

  /******************************/
  /* Set DPI aware mode         */
  /******************************/
  if (!win32_enable_dpi_awareness())
  {
    win32_print("[WARNING] Cannot set DPI awareness\n");
  }

  /******************************/
  /* HighRes timer for Sleep(1) */
  /******************************/
  if (!win32_enable_high_resolution_timer())
  {
    win32_print("[WARNING] Cannot set win32 high resolution timer using Winmm.dll (timeBeginPeriod)\n");
  }

  /******************************/
  /* Load XInput Controller     */
  /******************************/
  if (!xinput_load())
  {
    win32_print("[WARNING] Could not load XInput (xinput1_4.dll, xinput1_3.dll or xinput9_1_0.dll)! XInput gamepads/controllers (e.g. XBOX) are not functional.\n");
  }

  /******************************/
  /* Window and OpenGL context  */
  /******************************/
  if (!opengl_create_context(&state))
  {
    win32_print("[ERROR] Could not create opengl context!\n");
    return 1;
  }

  glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &state.gl_max_3d_texture_size);

  /* Avoid clear color flickering */
  glViewport(0, 0, (i32)state.window_width, (i32)state.window_height);
  glClearColor(state.window_clear_color_r, state.window_clear_color_g, state.window_clear_color_b, state.window_clear_color_a);
  glDisable(GL_FRAMEBUFFER_SRGB);
  glDisable(GL_MULTISAMPLE);
  glClear(GL_COLOR_BUFFER_BIT);

  SwapBuffers(state.device_context);

  /* Make the window visible */
  ShowWindow(state.window_handle, SW_SHOW);

  /******************************/
  /* Initialize Shaders         */
  /******************************/
  {
    /* Generate a dummy vao with no buffer */
    glGenVertexArrays(1, &main_vao);
    glBindVertexArray(main_vao);

    /* Load Fragment Shader source code from file */
    win32_print("[opengl] load shader file: ");
    win32_print(fragment_shader_file_name);
    win32_print("\n");

    opengl_shader_load_shader_main(&main_shader, fragment_shader_file_name);
    opengl_shader_load_shader_font(&font_shader);
    opengl_shader_load_shader_recording(&recording_shader);
  }

  /******************************/
  /* Initialize Font Texture    */
  /******************************/
  {
    /* Generate font texture */
    u8 fathom_font_pixels[FATHOM_FONT_WIDTH * FATHOM_FONT_HEIGHT];
    u32 tex;

    /* OpenGL does not allow 1bit packed texture data so we convert each bit to 1 byte */
    unpack_1bit_to_8bit(fathom_font_pixels, fathom_font, FATHOM_FONT_WIDTH, FATHOM_FONT_HEIGHT);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, FATHOM_FONT_WIDTH, FATHOM_FONT_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, fathom_font_pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glActiveTexture(GL_TEXTURE0);
  }

  /******************************/
  /* Initialize Font Buffers    */
  /******************************/
  {
    static f32 quad_vertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f};

    u32 quad_vbo;

    glGenVertexArrays(1, &font_vao);
    glBindVertexArray(font_vao);

    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    /* pos (location = 0) */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), FATHOM_NULL);
    glVertexAttribDivisor(0, 0);

    glGenBuffers(1, &glyph_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, glyph_vbo);

    /* iGlyph (location = 1) */
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 4, GL_UNSIGNED_SHORT, sizeof(glyph), FATHOM_NULL);
    glVertexAttribDivisor(1, 1);
  }

  {
    i64 perf_freq;
    i64 time_start;
    i64 time_start_fps_cap;
    i64 time_last;

    i32 thread_count = win32_process_thread_count();

    FILETIME fs_last = win32_file_mod_time(fragment_shader_file_name);

    QueryPerformanceFrequency(&perf_freq);
    QueryPerformanceCounter(&time_start);
    QueryPerformanceCounter(&time_start_fps_cap);

    time_last = time_start;

    while (state.running)
    {
      i64 time_now;

      /******************************/
      /* Timing                     */
      /******************************/
      {
        QueryPerformanceCounter(&time_now);

        state.iTimeDelta = (f64)(time_now - time_last) / (f64)perf_freq;
        state.iTime = (f64)(time_now - time_start) / (f64)perf_freq;

        time_last = time_now;

        if (state.iTimeDelta > 0.0)
        {
          state.iFrameRate = 1.0 / state.iTimeDelta;
        }
      }

      /******************************/
      /* Idle when window minimized */
      /******************************/
      if (state.window_minimized)
      {
        MSG msg;
        GetMessageA(&msg, 0, 0, 0);
        DispatchMessageA(&msg);
        continue;
      }

      /******************************/
      /* Hot Reload Fragment Shader */
      /******************************/
      {
        FILETIME fs_now = win32_file_mod_time(fragment_shader_file_name);

        if (CompareFileTime(&fs_now, &fs_last) != 0)
        {
          opengl_shader_load_shader_main(&main_shader, fragment_shader_file_name);
          fs_last = fs_now;

          /* Reset iTime elapsed seconds on hot reload */
          QueryPerformanceCounter(&time_start);
          state.iFrame = 0;
        }
      }

      /******************************/
      /* Input Processing           */
      /******************************/
      {
        MSG message = {0};
        u32 i;

        u64 *src = (u64 *)state.keys_is_down;
        u64 *dst = (u64 *)state.keys_was_down;

        /* 256 bytes / 8 bytes (u64) = 32 chunks. */
        for (i = 0; i < 8; ++i)
        {
          *dst++ = *src++;
          *dst++ = *src++;
          *dst++ = *src++;
          *dst++ = *src++;
        }

        state.mouse_left_was_down = state.mouse_left_is_down;
        state.mouse_right_was_down = state.mouse_right_is_down;

        /* Reset accumulated mouse relative speeds every frame before processing new mouse messages */
        state.mouse_dx = 0;
        state.mouse_dy = 0;

        while (PeekMessageA(&message, state.window_handle, 0, 0, PM_REMOVE))
        {
          DispatchMessageA(&message);
        }
      }

      /* Get current frames mouse position */
      {
        POINT p;
        GetCursorPos(&p);
        ScreenToClient(state.window_handle, &p);

        state.mouse_x = p.x;
        state.mouse_y = (i32)state.window_height - 1 - p.y;
      }

      /******************************/
      /* XInput Controller          */
      /******************************/
      if (XInputGetState)
      {

        /* If we recieve a WM_DEVICECHANGE message (hardware connected or disconnected to machine)
         * we need to check again the XInput Controller state.
         */
        if (state.controller.check_needed)
        {
          u8 i = 0;

          XINPUT_STATE xinput_state = {0};

          state.controller.connected = 0;

          for (i = 0; i < XINPUT_USER_MAX_COUNT; ++i)
          {
            u32 result = XInputGetState(i, &xinput_state);

            if (result == 0)
            {
              state.controller.id = i;
              state.controller.connected = 1;
              break;
            }
          }

          state.controller.check_needed = 0;
        }

        if (state.controller.connected)
        {
          XINPUT_STATE xinput_state = {0};
          u32 result = XInputGetState(state.controller.id, &xinput_state);

          if (result == 0)
          {
            XINPUT_GAMEPAD *gp = &xinput_state.Gamepad;
            state.controller.button_a = (gp->wButtons & XINPUT_GAMEPAD_A) ? 1 : 0;
            state.controller.button_b = (gp->wButtons & XINPUT_GAMEPAD_B) ? 1 : 0;
            state.controller.button_x = (gp->wButtons & XINPUT_GAMEPAD_X) ? 1 : 0;
            state.controller.button_y = (gp->wButtons & XINPUT_GAMEPAD_Y) ? 1 : 0;
            state.controller.shoulder_left = (gp->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 1 : 0;
            state.controller.shoulder_right = (gp->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 1 : 0;
            state.controller.dpad_up = (gp->wButtons & XINPUT_GAMEPAD_DPAD_UP) ? 1 : 0;
            state.controller.dpad_down = (gp->wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? 1 : 0;
            state.controller.dpad_left = (gp->wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? 1 : 0;
            state.controller.dpad_right = (gp->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? 1 : 0;
            state.controller.start = (gp->wButtons & XINPUT_GAMEPAD_START) ? 1 : 0;
            state.controller.back = (gp->wButtons & XINPUT_GAMEPAD_BACK) ? 1 : 0;
            state.controller.stick_left = (gp->wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? 1 : 0;
            state.controller.stick_right = (gp->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? 1 : 0;
            state.controller.trigger_left_value = xinput_process_trigger(gp->bLeftTrigger);
            state.controller.trigger_right_value = xinput_process_trigger(gp->bRightTrigger);
            state.controller.trigger_left = state.controller.trigger_left_value > 0.0f ? 1 : 0;
            state.controller.trigger_right = state.controller.trigger_right_value > 0.0f ? 1 : 0;
            state.controller.stick_left_x = xinput_process_thumbstick(gp->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
            state.controller.stick_left_y = xinput_process_thumbstick(gp->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
            state.controller.stick_right_x = xinput_process_thumbstick(gp->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
            state.controller.stick_right_y = xinput_process_thumbstick(gp->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
          }
          else
          {
            state.controller.check_needed = 1;
            state.controller.connected = 0;
          }
        }
      }

      /******************************/
      /* Full or Borderless (F9,F11)*/
      /******************************/
      if (state.keys_is_down[0x78] && !state.keys_was_down[0x78]) /* F9 */
      {
        state.borderless_enabled = !state.borderless_enabled;

        if (state.borderless_enabled && !state.fullscreen_enabled)
        {
          win32_window_enter_borderless(&state);
        }
        else
        {
          state.fullscreen_enabled = 0;
          state.borderless_enabled = 0;
          win32_window_enter_windowed(&state);
        }
      }

      if (state.keys_is_down[0x7A] && !state.keys_was_down[0x7A]) /* F11 */
      {
        state.fullscreen_enabled = !state.fullscreen_enabled;

        if (state.fullscreen_enabled && !state.borderless_enabled)
        {
          win32_window_enter_fullscreen(&state);
        }
        else
        {
          state.fullscreen_enabled = 0;
          state.borderless_enabled = 0;
          win32_window_enter_windowed(&state);
        }
      }

      /******************************/
      /* Handle Window Size changes */
      /******************************/
      if (state.window_size_changed && !state.screen_recording_enabled)
      {
        state.window_width = state.window_width_pending;
        state.window_height = state.window_height_pending;

        glViewport(0, 0, (i32)state.window_width, (i32)state.window_height);

        state.window_size_changed = 0;
      }

      glClear(GL_COLOR_BUFFER_BIT);

      /******************************/
      /* Pause Shader (P)           */
      /******************************/
      if (state.keys_is_down[0x50] && !state.keys_was_down[0x50]) /* P */
      {
        state.shader_paused = !state.shader_paused;
      }

      /******************************/
      /* Reset Timer (R)            */
      /******************************/
      if (state.keys_is_down[0x52] && !state.keys_was_down[0x52]) /* R */
      {
        /* Reset iTime elapsed seconds on hot reload */
        QueryPerformanceCounter(&time_start);
        state.iFrame = 0;
      }

      /******************************/
      /* Main Application Logic     */
      /******************************/
      fathom_render_grid(&state, &main_shader, main_vao);

      /******************************/
      /* UI Rendering (F1 pressed)  */
      /******************************/
      if (state.keys_is_down[0x70] && !state.keys_was_down[0x70]) /* F1 */
      {
        state.ui_enabled = !state.ui_enabled;
      }

      if (state.ui_enabled)
      {

#define GLYPH_BUFFER_SIZE 512
        static glyph glyph_buffer[GLYPH_BUFFER_SIZE];
        static u32 glyph_buffer_count = 0; /* Total glyph count */
        static u32 glyph_count_static = 0; /* Static glyphs only need to be buffered once */
        static u16 offset_x = 0;
        static u16 offset_y = 0;
        static u8 glyph_initialized = 0;
        static s8 tmp[128];
        static u32 handle_count = 0;
        static process_memory_info mem = {0};
        static f32 font_scale = 2.0f;

        u16 default_color = pack_rgb565(255, 255, 255);

        u16 advance_x = glyph_advance_x(font_scale);
        u16 advance_y = glyph_advance_y(font_scale);
        u16 separator_start_col = 11;
        u16 text_start_col = separator_start_col + 2;
        u16 offset_x_start = 10;
        u16 offset_y_start = 10;

        fathom_sb t = {0};
        t.size = sizeof(tmp);
        t.buffer = tmp;

        if (!glyph_initialized)
        {
          u32 i = 0;

          offset_x = offset_x_start;
          offset_y = offset_y_start;

          glyph_add(
              glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count,
              "STATE\n"
              "FPS\n"
              "FPS TARGET\n"
              "FPS RAW\n"
              "FRAME\n"
              "DELTA\n"
              "TIME\n"
              "MOUSE X/Y\n"
              "MOUSE DX/DY\n"
              "SIZE X/Y\n"
              "THREADS\n"
              "HANDLES\n"
              "MEM WORK\n"
              "MEM PEAK\n"
              "MEM COMMIT\n"
              "GL VENDOR\n"
              "GL RENDER\n"
              "GL VERSION\n"
              "CONTROLLER\n"
              "STICK L X/Y\n"
              "STICK R X/Y\n"
              "TRIGGER L/R\n"
              "BUTTONS\n"
              "GL ERROR FS\n",
              &offset_x, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);

          offset_x = (u16)(offset_x_start + advance_x * separator_start_col);
          offset_y = offset_y_start;

          /* Vertical ":" separator*/
          for (i = 0; i < 24; ++i)
          {

            if (i == 15)
            {
              u16 offset_x_new = (u16)(offset_x_start + advance_x * text_start_col);
              glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, state.gl_vendor, &offset_x_new, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);
            }
            else if (i == 16)
            {
              u16 offset_x_new = (u16)(offset_x_start + advance_x * text_start_col);
              glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, state.gl_renderer, &offset_x_new, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);
            }
            else if (i == 17)
            {
              u16 offset_x_new = (u16)(offset_x_start + advance_x * text_start_col);
              glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, state.gl_version, &offset_x_new, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);
            }

            glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, ":\n", &offset_x, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);
          }

          glyph_count_static = glyph_buffer_count;
          glyph_initialized = 1;
        }

        offset_x = (u16)(offset_x_start + advance_x * text_start_col);
        offset_y = offset_y_start;

        glyph_buffer_count = glyph_count_static;

        if (state.borderless_enabled)
        {
          glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "BORDERLESS ", &offset_x, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);
        }
        else if (state.fullscreen_enabled)
        {
          glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "FULLSCREEN ", &offset_x, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);
        }
        else
        {
          glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "WINDOWED ", &offset_x, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);
        }
        if (state.shader_paused)
        {
          glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "PAUSED ", &offset_x, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);
        }
        if (state.screen_recording_enabled)
        {
          glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "RECORDING ", &offset_x, &offset_y, pack_rgb565(255, 0, 0), GLYPH_STATE_BLINK, font_scale);
        }

#define CALC_GLYPH(txt, color)                                                                                                              \
  do                                                                                                                                        \
  {                                                                                                                                         \
    offset_x = (u16)(offset_x_start + advance_x * text_start_col);                                                                          \
    offset_y = (u16)(offset_y + advance_y);                                                                                                 \
    glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, txt.buffer, &offset_x, &offset_y, color, GLYPH_STATE_NONE, font_scale); \
    txt.length = 0;                                                                                                                         \
  } while (0)

        fathom_sb_f64(&t, state.iFrameRate, 2);
        CALC_GLYPH(t, default_color);

        /* Control Target FPS with arrow keys (0 = unlimited) */
        {
          u32 txt_length_temp = 0;

          if (state.keys_is_down[0x25] && !state.keys_was_down[0x25]) /* Left Arrow */
          {
            state.target_frames_per_second -= state.target_frames_per_second < 10 ? state.target_frames_per_second : 10;
          }

          if (state.keys_is_down[0x27] && !state.keys_was_down[0x27]) /* Right Arrow */
          {
            state.target_frames_per_second += 10;
          }

          offset_x = (u16)(offset_x_start + advance_x * (text_start_col - 1));
          offset_y = (u16)(offset_y + advance_y);
          glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "<", &offset_x, &offset_y, pack_rgb565(255, 0, 0), GLYPH_STATE_BLINK, font_scale);
          offset_y = (u16)(offset_y - advance_y);

          fathom_sb_f64(&t, state.target_frames_per_second, 2);
          txt_length_temp = t.length;
          CALC_GLYPH(t, default_color);

          offset_x = (u16)(offset_x_start + advance_x * (text_start_col + txt_length_temp));
          glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, ">", &offset_x, &offset_y, pack_rgb565(0, 255, 0), GLYPH_STATE_BLINK | GLYPH_STATE_HFLIP, font_scale);
        }

        fathom_sb_f64(&t, state.iFrameRateRaw, 2);
        CALC_GLYPH(t, default_color);
        fathom_sb_i32(&t, state.iFrame);
        CALC_GLYPH(t, default_color);
        fathom_sb_f64(&t, state.iTimeDelta, 6);
        CALC_GLYPH(t, default_color);
        fathom_sb_f64(&t, state.iTime, 6);
        CALC_GLYPH(t, default_color);
        fathom_sb_i32(&t, state.mouse_x);
        fathom_sb_s8(&t, "/");
        fathom_sb_i32(&t, state.mouse_y);
        CALC_GLYPH(t, default_color);
        fathom_sb_i32(&t, state.mouse_dx);
        fathom_sb_s8(&t, "/");
        fathom_sb_i32(&t, state.mouse_dy);
        CALC_GLYPH(t, default_color);
        fathom_sb_i32(&t, (i32)state.window_width);
        fathom_sb_s8(&t, "/");
        fathom_sb_i32(&t, (i32)state.window_height);
        CALC_GLYPH(t, default_color);
        fathom_sb_i32(&t, thread_count);
        CALC_GLYPH(t, default_color);

        GetProcessHandleCount(GetCurrentProcess(), &handle_count);
        fathom_sb_i32(&t, (i32)handle_count);
        CALC_GLYPH(t, default_color);

        win32_process_memory(&mem);
        fathom_sb_i32(&t, (i32)(mem.working_set / 1024));
        CALC_GLYPH(t, default_color);
        fathom_sb_i32(&t, (i32)(mem.peak_working_set / 1024));
        CALC_GLYPH(t, default_color);
        fathom_sb_i32(&t, (i32)(mem.private_bytes / 1024));
        CALC_GLYPH(t, default_color);

        offset_y = (u16)(offset_y + advance_y * 3);

        if (state.controller.connected)
        {
          fathom_sb_s8(&t, "CONNECTED");
          CALC_GLYPH(t, pack_rgb565(0, 255, 0));
          fathom_sb_f64(&t, state.controller.stick_left_x, 6);
          fathom_sb_s8(&t, "/");
          fathom_sb_f64(&t, state.controller.stick_left_y, 6);
          CALC_GLYPH(t, default_color);
          fathom_sb_f64(&t, state.controller.stick_right_x, 6);
          fathom_sb_s8(&t, "/");
          fathom_sb_f64(&t, state.controller.stick_right_y, 6);
          CALC_GLYPH(t, default_color);
          fathom_sb_f64(&t, state.controller.trigger_left_value, 6);
          fathom_sb_s8(&t, "/");
          fathom_sb_f64(&t, state.controller.trigger_right_value, 6);
          CALC_GLYPH(t, default_color);

          fathom_sb_s8(&t, "");

          /* clang-format off */
          if (state.controller.button_a)       fathom_sb_s8(&t, "A ");
          if (state.controller.button_b)       fathom_sb_s8(&t, "B ");
          if (state.controller.button_x)       fathom_sb_s8(&t, "X ");
          if (state.controller.button_y)       fathom_sb_s8(&t, "Y ");
          if (state.controller.dpad_left)      fathom_sb_s8(&t, "DLEFT ");
          if (state.controller.dpad_right)     fathom_sb_s8(&t, "DRIGHT ");
          if (state.controller.dpad_up)        fathom_sb_s8(&t, "DUP ");
          if (state.controller.dpad_down)      fathom_sb_s8(&t, "DDOWN ");
          if (state.controller.stick_left)     fathom_sb_s8(&t, "LSTICK ");
          if (state.controller.stick_right)    fathom_sb_s8(&t, "RSTICK ");
          if (state.controller.shoulder_left)  fathom_sb_s8(&t, "LSHOULDER ");
          if (state.controller.shoulder_right) fathom_sb_s8(&t, "RSHOULDER ");
          /* clang-format on */

          CALC_GLYPH(t, default_color);
        }
        else
        {
          fathom_sb_s8(&t, "NOT FOUND");
          CALC_GLYPH(t, pack_rgb565(255, 165, 0));

          offset_y = (u16)(offset_y + advance_y * 4);
        }

        if (main_shader.header.had_failure)
        {
          fathom_sb_s8(&t, shader_info_log);
          CALC_GLYPH(t, pack_rgb565(255, 0, 0));
        }
        else
        {
          fathom_sb_s8(&t, "NONE");
          CALC_GLYPH(t, pack_rgb565(0, 255, 0));
        }

#undef CALC_GLYPH

        /* Show grid memory */
        {
          u16 offset_memory_x = 400;
          u16 offset_memory_y = 10;

          glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "MEM BRICK MAP: \n", &offset_memory_x, &offset_memory_y, pack_rgb565(255, 255, 255), GLYPH_STATE_NONE, font_scale);
          glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "MEM ATLAS    : \n", &offset_memory_x, &offset_memory_y, pack_rgb565(255, 255, 255), GLYPH_STATE_NONE, font_scale);
          glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "SDF CALLS    : \n", &offset_memory_x, &offset_memory_y, pack_rgb565(255, 255, 255), GLYPH_STATE_NONE, font_scale);
          glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "CALC TIME    : \n", &offset_memory_x, &offset_memory_y, pack_rgb565(255, 255, 255), GLYPH_STATE_NONE, font_scale);
          glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "BRICK COUNT  : \n", &offset_memory_x, &offset_memory_y, pack_rgb565(255, 255, 255), GLYPH_STATE_NONE, font_scale);
          glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "ATLAS DIM    : \n", &offset_memory_x, &offset_memory_y, pack_rgb565(255, 255, 255), GLYPH_STATE_NONE, font_scale);
          glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "MAX 3D TEXRES: ", &offset_memory_x, &offset_memory_y, pack_rgb565(255, 255, 255), GLYPH_STATE_NONE, font_scale);

          t.length = 0;
          fathom_sb_f64(&t, (f64)state.mem_brick_map_bytes / 1024.0 / 1024.0, 4);
          fathom_sb_s8(&t, "\n");
          fathom_sb_f64(&t, (f64)state.mem_atlas_bytes / 1024.0 / 1024.0, 4);
          fathom_sb_s8(&t, "\n");
          fathom_sb_i32(&t, (i32)state.grid_sdf_invocations);
          fathom_sb_s8(&t, "\n");
          fathom_sb_f64(&t, ((f64)(state.grid_calc_time_end - state.grid_calc_time_start) * 1000.0) / (f64)perf_freq, 4);
          fathom_sb_s8(&t, "\n");
          fathom_sb_i32(&t, (i32)state.grid_active_brick_count);
          fathom_sb_s8(&t, "\n");
          fathom_sb_i32(&t, (i32)state.grid_atlas_width);
          fathom_sb_s8(&t, "/");
          fathom_sb_i32(&t, (i32)state.grid_atlas_height);
          fathom_sb_s8(&t, "/");
          fathom_sb_i32(&t, (i32)state.grid_atlas_depth);
          fathom_sb_s8(&t, "\n");
          fathom_sb_i32(&t, (i32)state.gl_max_3d_texture_size);

          offset_memory_y = 10;
          glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, t.buffer, &offset_memory_x, &offset_memory_y, pack_rgb565(255, 255, 255), GLYPH_STATE_NONE, font_scale);
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(font_shader.header.program);
        glUniform3f(font_shader.loc_iResolution, (f32)state.window_width, (f32)state.window_height, 1.0f);
        glUniform1f(font_shader.loc_iTime, (f32)state.iTime);
        glUniform4f(font_shader.loc_iTextureInfo, FATHOM_FONT_WIDTH, FATHOM_FONT_HEIGHT, FATHOM_FONT_GLYPH_WIDTH, FATHOM_FONT_GLYPH_HEIGHT);
        glUniform1i(font_shader.loc_iTexture, 0);
        glUniform1f(font_shader.loc_iFontScale, font_scale);
        glBindVertexArray(font_vao);

        (void)GL_DYNAMIC_DRAW;
        glBufferData(GL_ARRAY_BUFFER, (i32)(glyph_buffer_count * sizeof(glyph)), glyph_buffer, GL_STREAM_DRAW);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, glyph_buffer_count);

        glDisable(GL_BLEND);
      }

      /******************************/
      /* Screen Recording (F2)      */
      /******************************/
      {
        static u8 *framebuffer;
        static void *video_file_handle;

        if (state.keys_is_down[0x71] && !state.keys_was_down[0x71]) /* F2 */
        {
          state.screen_recording_enabled = !state.screen_recording_enabled;
        }

        if (state.screen_recording_enabled)
        {

          u32 written = 0;

          if (!state.screen_recording_initialized)
          {
            s8 buffer[128];
            fathom_sb t = {0};

            t.size = sizeof(buffer);
            t.buffer = buffer;

            /* Create recording output file name.
             * Format:  fathom_capture_<window_width>x<window_height>_<target_frames_per_second>.raw
             * Example: fathom_capture_800x600_60.raw
             */
            fathom_sb_s8(&t, "fathom_capture_");
            fathom_sb_i32(&t, (i32)(state.window_width));
            fathom_sb_s8(&t, "x");
            fathom_sb_i32(&t, (i32)(state.window_height));
            fathom_sb_s8(&t, "_");
            fathom_sb_i32(&t, (i32)(state.target_frames_per_second));
            fathom_sb_s8(&t, ".raw");

            framebuffer = VirtualAlloc(0, state.window_width * state.window_height * 3, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            video_file_handle = CreateFileA(t.buffer, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

            glPixelStorei(GL_PACK_ALIGNMENT, 1);

            state.screen_recording_initialized = 1;
          }
          glReadPixels(0, 0, (i32)state.window_width, (i32)state.window_height, GL_RGB, GL_UNSIGNED_BYTE, framebuffer);

          WriteFile(video_file_handle, framebuffer, state.window_width * state.window_height * 3, &written, 0);

          /* Render recording indicator */
          glUseProgram(recording_shader.header.program);
          glUniform3f(recording_shader.loc_iResolution, (f32)state.window_width, (f32)state.window_height, 1.0f);
          glUniform1f(recording_shader.loc_iTime, (f32)state.iTime);
          glBindVertexArray(main_vao);
          glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        else if (state.screen_recording_initialized)
        {
          VirtualFree(framebuffer, 0, MEM_RELEASE);
          CloseHandle(video_file_handle);
          state.screen_recording_initialized = 0;
        }
      }

      SwapBuffers(state.device_context);

      /* Measure RAW FPS (rendering without cap)*/
      {
        i64 time_render_now;
        QueryPerformanceCounter(&time_render_now);
        state.iFrameRateRaw = 1.0 / ((f64)(time_render_now - time_last) / (f64)perf_freq);
      }

      /******************************/
      /* Frame Rate Limiting        */
      /******************************/
      if (state.target_frames_per_second > 0)
      {
        i64 time_end;

        f64 frame_time;
        f64 remaining;
        f64 target_frame_time = 1.0 / (f64)state.target_frames_per_second;

        QueryPerformanceCounter(&time_end);

        frame_time = (f64)(time_end - time_start_fps_cap) / (f64)perf_freq;
        remaining = target_frame_time - frame_time;

        if (remaining > 0.0)
        {
          /* Sleep most of it (milliseconds) */
          if (remaining > 0.0005)
          {
            u32 sleep_ms = (u32)((remaining - 0.00025) * 1000.0);

            if (sleep_ms > 0)
            {
              Sleep(sleep_ms);
            }
          }

          /* Spin for the rest */
          for (;;)
          {
            QueryPerformanceCounter(&time_end);

            frame_time = (f64)(time_end - time_start_fps_cap) / (f64)perf_freq;

            if (frame_time >= target_frame_time)
            {
              break;
            }
          }
        }

        /* Start timing next frame */
        time_start_fps_cap = time_end;
      }

      state.iFrame++;
    }
  }

  return 0;
}

/* #############################################################################
 * # [SECTION] nostdlib entry point
 * #############################################################################
 */
#ifdef __clang__
#elif __GNUC__
__attribute((externally_visible))
#endif
i32 WinMainCRTStartup(void)
{
  u8 *cmdline = (u8 *)GetCommandLineA();
  u8 *argv[8];
  i32 argc = 0;

  i32 return_code;

  /* Parse command line arguments into argv */
  while (*cmdline)
  {
    /* skip whitespace */
    while (*cmdline == ' ' || *cmdline == '\t')
    {
      cmdline++;
    }

    if (!*cmdline)
    {
      break;
    }

    if (argc < 9)
    {
      argv[argc++] = cmdline;
    }

    /* parse token (basic, no quote handling) */
    while (*cmdline && *cmdline != ' ' && *cmdline != '\t')
    {
      cmdline++;
    }

    if (*cmdline)
    {
      *cmdline++ = '\0';
    }
  }

  argv[argc] = (u8 *)0;

  /* Run the program and exit with return code */
  return_code = start(argc, argv);
  ExitProcess((u32)return_code);
  return return_code;
}

/*
   ------------------------------------------------------------------------------
   This software is available under 2 licenses -- choose whichever you prefer.
   ------------------------------------------------------------------------------
   ALTERNATIVE A - MIT License
   Copyright (c) 2026 nickscha
   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is furnished to do
   so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   ------------------------------------------------------------------------------
   ALTERNATIVE B - Public Domain (www.unlicense.org)
   This is free and unencumbered software released into the public domain.
   Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
   software, either in source code form or as a compiled binary, for any purpose,
   commercial or non-commercial, and by any means.
   In jurisdictions that recognize copyright laws, the author or authors of this
   software dedicate any and all copyright interest in the software to the public
   domain. We make this dedication for the benefit of the public at large and to
   the detriment of our heirs and successors. We intend this dedication to be an
   overt act of relinquishment in perpetuity of all present and future rights to
   this software under copyright law.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
   ------------------------------------------------------------------------------
*/
