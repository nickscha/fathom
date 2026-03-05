#ifndef FATHOM_UI_H
#define FATHOM_UI_H

#include "fathom_types.h"

/* #############################################################################
 * # [SECTION] UI
 * #############################################################################
 */
#define FATHOM_UI_MAX_STACK 8

typedef struct fathom_rect
{
    u16 x;
    u16 y;
    u16 w;
    u16 h;

} fathom_rect;

typedef struct fathom_ui_context
{
    u16 mouse_x;
    u16 mouse_y;
    u16 mouse_x_prev;
    u16 mouse_y_prev;
    u8 mouse_left_is_down;
    u8 mouse_left_was_down;
    u8 mouse_right_is_down;
    u8 mouse_right_was_down;

    u8 hot_id;    /* Hovered */
    u8 active_id; /* Clicked */

    /* Layout Stack */
    fathom_rect stack[FATHOM_UI_MAX_STACK];
    i32 stack_ptr;
    u16 cursor_y;
    u16 padding;

} fathom_ui_context;

typedef enum fathom_ui_state
{
    FATHOM_UI_IDLE = 0,
    FATHOM_UI_HOVER = (1 << 0),   /* Mouse is over the rect */
    FATHOM_UI_PRESSED = (1 << 1), /* Mouse just went down this frame */
    FATHOM_UI_HELD = (1 << 2),    /* Mouse is currently down */
    FATHOM_UI_RELEASED = (1 << 3) /* Mouse was released (clicked) */

} fathom_ui_state;

typedef struct fathom_ui_result
{
    u16 x;
    u16 y;
    u16 w;
    u16 h;

    u8 state;
} fathom_ui_result;

FATHOM_API FATHOM_INLINE u8 fathom_ui_internal_check_rect(u16 x, u16 y, u16 rx, u16 ry, u16 rw, u16 rh)
{
    return (x >= rx && x <= rx + rw && y >= ry && y <= ry + rh);
}

FATHOM_API fathom_ui_result fathom_ui_internal_process(fathom_ui_context *ctx, u8 id, u16 x, u16 y, u16 w, u16 h)
{
    fathom_ui_result res;
    u8 is_over;

    /* Logic: If x/y are 0 and we are in a stack, use relative positioning */
    if (x == 0 && y == 0 && ctx->stack_ptr > 0)
    {
        fathom_rect *p = &ctx->stack[ctx->stack_ptr - 1];
        res.x = p->x + ctx->padding;
        res.y = p->y + ctx->cursor_y + ctx->padding;
        res.w = (w == 0) ? (p->w - (ctx->padding * 2)) : w; /* 0 width = fill parent */
        res.h = h;

        /* Advance cursor for next widget */
        ctx->cursor_y += h + ctx->padding;
    }
    else
    {
        res.x = x;
        res.y = y;
        res.w = w;
        res.h = h;
    }

    /* Standard Interaction Logic */
    res.state = FATHOM_UI_IDLE;
    is_over = fathom_ui_internal_check_rect(ctx->mouse_x, ctx->mouse_y, res.x, res.y, res.w, res.h);

    if (is_over)
    {
        ctx->hot_id = id;
        res.state |= FATHOM_UI_HOVER;

        if (ctx->active_id == 0 && ctx->mouse_left_is_down && !ctx->mouse_left_was_down)
        {
            ctx->active_id = id;
            res.state |= FATHOM_UI_PRESSED;
        }
    }

    if (ctx->active_id == id)
    {
        res.state |= FATHOM_UI_HELD;

        if (!ctx->mouse_left_is_down && ctx->mouse_left_was_down)
        {
            ctx->active_id = 0;

            if (is_over)
            {
                res.state |= FATHOM_UI_RELEASED;
            }
        }
    }
    return res;
}

FATHOM_API FATHOM_INLINE void fathom_ui_begin(fathom_ui_context *ctx)
{
    ctx->mouse_x_prev = ctx->mouse_x;
    ctx->mouse_y_prev = ctx->mouse_y;
    ctx->hot_id = 0;
}

FATHOM_API FATHOM_INLINE void fathom_ui_end(fathom_ui_context *ctx)
{
    /*
    ctx->mouse_left_was_down = ctx->mouse_left_is_down;
    ctx->mouse_right_was_down = ctx->mouse_right_is_down;
    */
}

FATHOM_API FATHOM_INLINE void fathom_ui_panel_begin(fathom_ui_context *ctx, u8 x, u16 y, u16 w, u16 h)
{
    if (ctx->stack_ptr < FATHOM_UI_MAX_STACK)
    {
        fathom_rect *r = &ctx->stack[ctx->stack_ptr++];
        r->x = x;
        r->y = y;
        r->w = w;
        r->h = h;

        ctx->cursor_y = 0;
    }
}

FATHOM_API FATHOM_INLINE void fathom_ui_panel_end(fathom_ui_context *ctx)
{
    if (ctx->stack_ptr > 0)
    {
        u16 panel_h = ctx->stack[ctx->stack_ptr - 1].h;
        ctx->stack_ptr--;

        if (ctx->stack_ptr > 0)
        {
            ctx->cursor_y += panel_h + ctx->padding;
        }
    }
}

FATHOM_API FATHOM_INLINE fathom_ui_result fathom_ui_button(fathom_ui_context *ctx, u8 id, u16 x, u16 y, u16 w, u16 h)
{
    return fathom_ui_internal_process(ctx, id, x, y, w, h);
}

FATHOM_API FATHOM_INLINE fathom_ui_result fathom_ui_checkbox(fathom_ui_context *ctx, u8 id, u16 x, u16 y, u16 w, u16 h, u8 *is_checked)
{
    fathom_ui_result res = fathom_ui_internal_process(ctx, id, x, y, w, h);

    if (res.state & FATHOM_UI_RELEASED)
    {
        *is_checked = !(*is_checked);
    }

    return res;
}

FATHOM_API FATHOM_INLINE void fathom_ui_drag_header(fathom_ui_context *ctx, u8 id, u16 *win_x, u16 *win_y, u16 win_w)
{
    fathom_ui_result res = fathom_ui_internal_process(ctx, id, *win_x, *win_y, win_w, 20);

    if (res.state & FATHOM_UI_HELD)
    {
        *win_x += (ctx->mouse_x - ctx->mouse_x_prev);
        *win_y += (ctx->mouse_y - ctx->mouse_y_prev);
    }
}

FATHOM_API FATHOM_INLINE fathom_ui_result fathom_ui_radio(fathom_ui_context *ctx, u8 id, u16 x, u16 y, u16 size, i32 *current_val, i32 radio_val)
{
    fathom_ui_result res = fathom_ui_internal_process(ctx, id, x, y, size, size);

    if (res.state & FATHOM_UI_RELEASED)
    {
        *current_val = radio_val;
    }

    return res;
}

FATHOM_API FATHOM_INLINE fathom_ui_result fathom_ui_slider(fathom_ui_context *ctx, u8 id, u16 x, u16 y, u16 w, u16 h, f32 *val)
{
    fathom_ui_result res = fathom_ui_internal_process(ctx, id, x, y, w, h);

    if (res.state & FATHOM_UI_HELD)
    {
        f32 mouse_pos = (f32)(ctx->mouse_x - res.x);

        *val = mouse_pos / (f32)res.w;

        if (*val < 0.0f)
        {
            *val = 0.0f;
        }
        if (*val > 1.0f)
        {
            *val = 1.0f;
        }
    }

    return res;
}

#endif /* FATHOM_UI_H */
