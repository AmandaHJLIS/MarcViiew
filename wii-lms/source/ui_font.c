#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogc/system.h>
#include <ogc/video.h>
#include "ui_font.h"

#define UI_FONT_FG 0xFF80FF80u
#define UI_FONT_BG 0x00800080u

static void *ui_xfb = NULL;
static GXRModeObj *ui_rmode = NULL;
static sys_fontheader *ui_font = NULL;
static int ui_ready = 0;

static unsigned int utf8_next(const unsigned char **p)
{
    const unsigned char *s = *p;
    unsigned int c = *s++;
    if (c < 0x80) { *p = s; return c; }
    if ((c & 0xE0) == 0xC0 && (s[0] & 0xC0) == 0x80) {
        c = ((c & 0x1F) << 6) | (s[0] & 0x3F); s++;
        *p = s; return c;
    }
    if ((c & 0xF0) == 0xE0 && (s[0] & 0xC0) == 0x80 &&
        (s[1] & 0xC0) == 0x80) {
        c = ((c & 0x0F) << 12) | ((s[0] & 0x3F) << 6) |
            (s[1] & 0x3F); s += 2;
        *p = s; return c;
    }
    if ((c & 0xF8) == 0xF0 && (s[0] & 0xC0) == 0x80 &&
        (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80) {
        c = ((c & 0x07) << 18) | ((s[0] & 0x3F) << 12) |
            ((s[1] & 0x3F) << 6) | (s[2] & 0x3F); s += 3;
        *p = s; return c;
    }
    *p = s;
    return '?';
}

static unsigned char glyph_nibble(void *image, int sheet_width, int x, int y)
{
    unsigned char *base = (unsigned char *)image;
    unsigned int offset;
    unsigned char value;

    offset = ((sheet_width / 8) << 5) * (unsigned int)(y / 8);
    offset += ((unsigned int)(x / 8) << 5);
    offset += (unsigned int)(y % 8) << 2;
    offset += (unsigned int)(x % 8) / 2;

    value = base[offset];
    return (x & 1) ? (value & 0x0F) : (value >> 4);
}

static int glyph_width(unsigned int codepoint)
{
    void *image = NULL;
    int xpos = 0;
    int ypos = 0;
    int width = 0;

    SYS_GetFontTexture(
        (s32)codepoint,
        &image,
        &xpos,
        &ypos,
        &width
    );

    if (!image || width <= 0)
        return ui_font != NULL ? ui_font->cell_width : 8;

    return width;
}

static void draw_glyph(unsigned int codepoint, int px, int py)
{
    void *image = NULL;
    int xpos = 0;
    int ypos = 0;
    int width = 0;
    int x;
    int y;

    if (!ui_ready)
        return;

    /*
     * The libogc system-font API supplies the Wii's already-expanded
     * I4 texture sheet. We sample that sheet directly into the same
     * YUY2 framebuffer format used by the libogc console.
     *
     * This deliberately avoids GX/libgui. The UI therefore gets a
     * font path that works with MarcViiew's existing framebuffer
     * console rather than depending on the upstream GUI stack.
     */
    SYS_GetFontTexture(
        (s32)codepoint,
        &image,
        &xpos,
        &ypos,
        &width
    );

    if (!image)
        return;

    for (y = 0; y < ui_font->cell_height; ++y)
    {
        unsigned char *row =
            (unsigned char *)ui_xfb +
            (py + y) * ui_rmode->fbWidth * VI_DISPLAY_PIX_SZ;

        for (x = 0; x < width; ++x)
        {
            int screen_x = px + x;
            unsigned char *pair;
            unsigned int old;
            unsigned int color;

            if (screen_x < 0 || screen_x >= (int)ui_rmode->fbWidth)
                continue;

            pair =
                row +
                (screen_x & ~1) * VI_DISPLAY_PIX_SZ;

            color =
                glyph_nibble(
                    image,
                    ui_font->sheet_width,
                    xpos + x,
                    ypos + y
                )
                ? UI_FONT_FG
                : UI_FONT_BG;

            old = *(unsigned int *)pair;

            if (screen_x & 1)
                old = (old & 0xFF00FFFFu) | (color & 0x00FF0000u);
            else
                old = (old & 0x00FFFFFFu) | (color & 0xFF000000u);

            *(unsigned int *)pair = old;
        }
    }
}

int ui_font_init(void *xfb, GXRModeObj *rmode)
{
    size_t size;

    if (!xfb || !rmode)
        return 0;

    ui_xfb = xfb;
    ui_rmode = rmode;

    size =
        SYS_GetFontEncoding() == 1
            ? SYS_FONTSIZE_SJIS
            : SYS_FONTSIZE_ANSI;

    ui_font =
        (sys_fontheader *)memalign(
            32,
            size
        );

    if (!ui_font)
        return 0;

    memset(ui_font, 0, size);

    if (!SYS_InitFont(ui_font))
    {
        free(ui_font);
        ui_font = NULL;
        return 0;
    }

    ui_ready = 1;
    return 1;
}

void ui_font_shutdown(void)
{
    if (ui_font)
        free(ui_font);

    ui_font = NULL;
    ui_ready = 0;
}

int ui_font_text_width(const char *text)
{
    const unsigned char *p;
    int width = 0;

    if (!ui_ready || !text)
        return 0;

    p = (const unsigned char *)text;

    while (*p)
    {
        unsigned int codepoint = utf8_next(&p);

        if (codepoint == '\n')
            break;

        width += glyph_width(codepoint);
    }

    return width;
}

void ui_font_draw_centered(const char *text, int y)
{
    const unsigned char *p;
    int width;
    int x;

    if (!ui_ready || !text)
        return;

    width = ui_font_text_width(text);
    x = ((int)ui_rmode->fbWidth - width) / 2;

    if (x < 0)
        x = 0;

    p = (const unsigned char *)text;

    while (*p)
    {
        unsigned int codepoint = utf8_next(&p);

        if (codepoint == '\n')
            break;

        draw_glyph(
            codepoint,
            x,
            y
        );

        x += glyph_width(codepoint);
    }
}

int ui_font_line_height(void)
{
    return ui_ready ? ui_font->cell_height : 16;
}

int ui_font_ready(void)
{
    return ui_ready;
}
