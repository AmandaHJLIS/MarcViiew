#include "ui_font.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ogc/console.h>
#include <ogc/system.h>
#include <gccore.h>

/*
 * Experimental font bridge.
 *
 * libogc's console renderer is deliberately left untouched. It consumes an
 * 8x16, 1-bit ConsoleFont. We start with libogc's normal font and replace the
 * upper half of the table with glyphs rasterised from the Wii's system ROM
 * font. This gives us a safe place to experiment with extended characters
 * without writing YUY2 pixels ourselves.
 */

#define UI_FONT_GLYPHS 256
#define UI_FONT_GLYPH_BYTES 16
#define UI_FONT_TEXEL_STRIDE 32
#define UI_FONT_TEXEL_BUFFER_SIZE 1024

static u8 *ui_font_gfx = NULL;
static void *ui_system_font = NULL;

static u8 ui_i4_pixel(
    const u8 *image,
    int stride,
    int x,
    int y
) {
    const u8 *p =
        image +
        ((y / 8) * ((stride << 1) / 8) << 5) +
        ((x / 8) << 5) +
        ((x % 8) / 2) +
        ((y % 8) << 2);

    if (x & 1)
        return *p & 0x0F;

    return (*p >> 4) & 0x0F;
}

static void ui_build_system_glyph(
    u32 codepoint,
    u8 *destination,
    const sys_fontheader *header
) {
    u8 texels[UI_FONT_TEXEL_BUFFER_SIZE];
    s32 width = 0;

    memset(
        texels,
        0,
        sizeof(texels)
    );

    SYS_GetFontTexel(
        (s32)codepoint,
        texels,
        0,
        UI_FONT_TEXEL_STRIDE,
        &width
    );

    /*
     * The ROM font is a tiled intensity texture. Convert it to the exact
     * 1-bit, 8x16 format expected by libogc's console renderer.
     *
     * Horizontal sampling is scaled to the console's eight-pixel cell.
     * Vertical sampling is likewise scaled to sixteen rows.
     */
    for (
        int y = 0;
        y < UI_FONT_GLYPH_BYTES;
        y++
    ) {
        u8 row = 0;

        int source_y =
            (y * header->cell_height) /
            UI_FONT_GLYPH_BYTES;

        if (
            source_y >=
            header->cell_height
        )
            source_y =
                header->cell_height - 1;

        for (
            int x = 0;
            x < 8;
            x++
        ) {
            int source_x0 =
                (x * header->cell_width) / 8;

            int source_x1 =
                ((x + 1) * header->cell_width) / 8;

            if (
                source_x1 <=
                source_x0
            )
                source_x1 =
                    source_x0 + 1;

            int lit = 0;

            for (
                int sx = source_x0;
                sx < source_x1;
                sx++
            ) {
                if (
                    sx < UI_FONT_TEXEL_STRIDE &&
                    ui_i4_pixel(
                        texels,
                        UI_FONT_TEXEL_STRIDE,
                        sx,
                        source_y
                    ) >= 4
                ) {
                    lit = 1;
                    break;
                }
            }

            if (lit)
                row |=
                    (u8)(1 << (7 - x));
        }

        destination[y] = row;
    }
}

void ui_font_init(void)
{
    PrintConsole *console;
    ConsoleFont font;
    sys_fontheader *header;

    console =
        consoleGetDefault();

    if (
        !console ||
        !console->font.gfx
    )
        return;

    if (!ui_font_gfx) {
        ui_font_gfx =
            memalign(
                32,
                UI_FONT_GLYPHS *
                UI_FONT_GLYPH_BYTES
            );

        if (!ui_font_gfx)
            return;

        memcpy(
            ui_font_gfx,
            console->font.gfx,
            UI_FONT_GLYPHS *
            UI_FONT_GLYPH_BYTES
        );
    }

    /*
     * SYS_InitFont needs its documented ROM-font buffer size. Keep it in
     * normal MEM1 heap memory; the buffer is released after rasterisation.
     */
    if (!ui_system_font) {
        ui_system_font =
            memalign(
                32,
                SYS_FONTSIZE_ANSI
            );

        if (!ui_system_font)
            return;

        if (
            !SYS_InitFont(
                (sys_fontheader *)ui_system_font
            )
        ) {
            free(ui_system_font);
            ui_system_font = NULL;
            return;
        }
    }

    header =
        (sys_fontheader *)ui_system_font;

    if (
        header->cell_width == 0 ||
        header->cell_height == 0
    )
        return;

    /*
     * Keep ASCII exactly as libogc supplies it. Only the extended byte range
     * is experimental for now.
     */
    for (
        u32 codepoint = 0x80;
        codepoint <= 0xFF;
        codepoint++
    ) {
        if (
            codepoint >= header->first_char &&
            codepoint <= header->last_char
        ) {
            ui_build_system_glyph(
                codepoint,
                ui_font_gfx +
                    codepoint *
                    UI_FONT_GLYPH_BYTES,
                header
            );
        }
    }

    font.gfx =
        ui_font_gfx;

    font.asciiOffset = 0;
    font.numChars =
        UI_FONT_GLYPHS;

    /*
     * This is the important safety boundary: consoleSetFont() feeds the
     * glyph table back into libogc's existing console renderer.
     */
    consoleSetFont(
        NULL,
        &font
    );
}
