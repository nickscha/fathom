#ifndef FATHOM_H
#define FATHOM_H

#include "fathom_types.h"

/* #############################################################################
 * # [SECTION] Platform Input
 * #############################################################################
 */
#define FATHOM_INPUT_KEYS_COUNT 256

typedef struct fathom_platform_input_mouse
{
    i32 mouse_dx; /* Relative movement delta for x  */
    i32 mouse_dy; /* Relative movement delta for y  */
    i32 mouse_x;  /* Mouse position on screen for x */
    i32 mouse_y;  /* Mouse position on screen for y */
    f32 mouse_scroll;
    u8 mouse_left_is_down;
    u8 mouse_left_was_down;
    u8 mouse_right_is_down;
    u8 mouse_right_was_down;

} fathom_platform_input_mouse;

typedef struct fathom_platform_input_keyboard
{
    u8 keys_is_down[FATHOM_INPUT_KEYS_COUNT];
    u8 keys_was_down[FATHOM_INPUT_KEYS_COUNT];

} fathom_platform_input_keyboard;

typedef struct fathom_platform_input_controller
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

} fathom_platform_input_controller;

typedef struct fathom_platform_input
{
    fathom_platform_input_mouse mouse;
    fathom_platform_input_keyboard keyboard;
    fathom_platform_input_controller controller;

} fathom_platform_input;

/* #############################################################################
 * # [SECTION] Platform Window
 * #############################################################################
 */
typedef struct fathom_platform_window
{
    u32 window_width;
    u32 window_height;

    f32 window_clear_color_r;
    f32 window_clear_color_g;
    f32 window_clear_color_b;
    f32 window_clear_color_a;

} fathom_platform_window;

/* #############################################################################
 * # [SECTION] Platform API
 * #############################################################################
 */
typedef u8 (*fathom_platform_api_io_print)(s8 *string);
typedef u8 (*fathom_platform_api_io_file_size)(s8 *filename, u32 *file_size);
typedef u8 (*fathom_platform_api_io_file_read)(s8 *filename, u8 *buffer, u32 buffer_size);

typedef struct fathom_platform_api
{
    fathom_platform_api_io_print io_print;
    fathom_platform_api_io_file_size io_file_size;
    fathom_platform_api_io_file_read io_file_read;

} fathom_platform_api;

/* #############################################################################
 * # [SECTION] Main entry point (fathom_update)
 * #############################################################################
 */
u8 fathom_update_stub(fathom_platform_api *api, fathom_platform_window *window, fathom_platform_input *input)
{
    api->io_print("[fathom][error] No 'fathom_update' function has been set! Using 'fathom_update_stub'!\n");
    api->io_print("[fathom][error] Define the following function in your code:\n\n");
    api->io_print("  u8 fathom_update(fathom_platform_api *api, fathom_platform_window *window, fathom_platform_input *input)\n  {\n  /* your code */\n  }\n\n");
    api->io_print("[fathom][error]\n");

    window->window_clear_color_r = 1.0f;
    window->window_clear_color_g = 0.0f;
    window->window_clear_color_b = 0.0f;

    return 0;
}

typedef u8 (*fathom_update_function)(
    fathom_platform_api *api,       /* Platform provided functinos */
    fathom_platform_window *window, /* Platform window informatino */
    fathom_platform_input *input    /* Platform input */
);

static fathom_update_function fathom_update = fathom_update_stub;

#endif /* FATHOM_H */
