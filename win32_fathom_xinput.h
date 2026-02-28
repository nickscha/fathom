#ifndef WIN32_FATHOM_XINPUT_H
#define WIN32_FATHOM_XINPUT_H

#include "fathom_types.h"
#include "win32_fathom_api.h"

/* #############################################################################
 * # [SECTION] XInput gamepad controller support
 * #############################################################################
 */
#define XINPUT_USER_MAX_COUNT 4
#define XINPUT_GAMEPAD_DPAD_UP 0x0001
#define XINPUT_GAMEPAD_DPAD_DOWN 0x0002
#define XINPUT_GAMEPAD_DPAD_LEFT 0x0004
#define XINPUT_GAMEPAD_DPAD_RIGHT 0x0008
#define XINPUT_GAMEPAD_START 0x0010
#define XINPUT_GAMEPAD_BACK 0x0020
#define XINPUT_GAMEPAD_LEFT_THUMB 0x0040
#define XINPUT_GAMEPAD_RIGHT_THUMB 0x0080
#define XINPUT_GAMEPAD_LEFT_SHOULDER 0x0100
#define XINPUT_GAMEPAD_RIGHT_SHOULDER 0x0200
#define XINPUT_GAMEPAD_A 0x1000
#define XINPUT_GAMEPAD_B 0x2000
#define XINPUT_GAMEPAD_X 0x4000
#define XINPUT_GAMEPAD_Y 0x8000
#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE 7849
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD 30

typedef struct XINPUT_GAMEPAD
{
    u16 wButtons;
    u8 bLeftTrigger;
    u8 bRightTrigger;
    i16 sThumbLX;
    i16 sThumbLY;
    i16 sThumbRX;
    i16 sThumbRY;
} XINPUT_GAMEPAD;

typedef struct XINPUT_STATE
{
    u32 dwPacketNumber;
    XINPUT_GAMEPAD Gamepad;
} XINPUT_STATE;

typedef u32(__stdcall *XInputGetStateFunc)(u32 dwUserIndex, XINPUT_STATE *pState);

static XInputGetStateFunc XInputGetState = 0;

FATHOM_API u8 xinput_load(void)
{
    void *xinput_lib = LoadLibraryA("xinput1_4.dll");

    if (!xinput_lib)
    {
        xinput_lib = LoadLibraryA("xinput1_3.dll");
    }

    if (!xinput_lib)
    {
        xinput_lib = LoadLibraryA("xinput9_1_0.dll");
    }

    if (!xinput_lib)
    {
        return 0;
    }

    *(void **)(&XInputGetState) = GetProcAddress(xinput_lib, "XInputGetState");

    return 1;
}

FATHOM_API f32 xinput_process_thumbstick(i16 value, i16 deadzone)
{
    f32 result = 0.0f;

    if (value > deadzone)
    {
        result = (f32)(value - deadzone) / (32767.0f - deadzone);
    }
    else if (value < -deadzone)
    {
        result = (f32)(value + deadzone) / (32768.0f - deadzone);
    }

    return result;
}

FATHOM_API FATHOM_INLINE f32 xinput_process_trigger(u8 value)
{
    return value > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? (f32)value / 255.0f : 0.0f;
}

#endif /* WIN32_FATHOM_XINPUT_H */