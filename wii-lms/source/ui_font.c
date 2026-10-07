#include "ui_font.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
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
#define UI_FONT_THRESHOLD 3

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
    s32 glyph_width = 0;
    int source_height;
    int copy_width;
    int x_offset;
    int y_offset;

    memset(
        texels,
        0,
        sizeof(texels)
    );

    /*
     * SYS_GetFontTexel() already extracts one glyph from the Wii ROM font.
     * The destination is still tiled I4 data, so we only need to decode that
     * temporary image here.
     *
     * The important distinction is between cell_width and glyph width:
     * cell_width is the full font cell, while glyph_width is the actual
     * character advance/ink width. Treating the entire cell as the glyph
     * squashes normal-width characters into a tiny mark.
     */
    SYS_GetFontTexel(
        (s32)codepoint,
        texels,
        0,
        UI_FONT_TEXEL_STRIDE,
        &glyph_width
    );

    source_height = header->cell_height;

    if (
        source_height <= 0 ||
        glyph_width <= 0
    )
        return;

    if (glyph_width > UI_FONT_GLYPH_BYTES / 2)
        copy_width = UI_FONT_GLYPH_BYTES / 2;
    else
        copy_width = glyph_width;

    /*
     * Keep glyphs centred in the console's 8-pixel cell. If the system font
     * glyph is wider than eight pixels, sample it down; if it is narrower,
     * preserve its native width rather than stretching it.
     */
    x_offset =
        (UI_FONT_GLYPH_BYTES / 2 - copy_width) / 2;

    /*
     * The console is 16 pixels high. Centre a shorter system-font cell rather
     * than stretching it vertically; for a 16-pixel system cell this is a
     * direct 1:1 copy.
     */
    y_offset =
        (UI_FONT_GLYPH_BYTES - source_height) / 2;

    if (y_offset < 0)
        y_offset = 0;

    memset(
        destination,
        0,
        UI_FONT_GLYPH_BYTES
    );

    for (
        int y = 0;
        y < UI_FONT_GLYPH_BYTES;
        y++
    ) {
        int source_y;

        if (source_height == UI_FONT_GLYPH_BYTES) {
            source_y = y;
        } else {
            source_y =
                ((y - y_offset) * source_height) /
                UI_FONT_GLYPH_BYTES;
        }

        if (
            source_y < 0 ||
            source_y >= source_height
        )
            continue;

        u8 row = 0;

        for (
            int x = 0;
            x < copy_width;
            x++
        ) {
            int source_x;

            if (glyph_width <= UI_FONT_GLYPH_BYTES / 2) {
                source_x = x;
            } else {
                source_x =
                    (x * glyph_width) /
                    (UI_FONT_GLYPH_BYTES / 2);
            }

            if (
                source_x >= glyph_width ||
                source_x >= UI_FONT_TEXEL_STRIDE
            )
                continue;

            /*
             * When a system-font glyph is wider than the console cell, one
             * destination pixel represents more than one source texel.
             * Sample the whole source interval and keep the strongest texel.
             * This preserves thin strokes much better than taking only the
             * first source pixel in the interval.
             */
            {
                int source_x_end;

                if (glyph_width <= UI_FONT_GLYPH_BYTES / 2) {
                    source_x_end = source_x + 1;
                } else {
                    source_x_end =
                        ((x + 1) * glyph_width) /
                        (UI_FONT_GLYPH_BYTES / 2);

                    if (source_x_end <= source_x)
                        source_x_end = source_x + 1;
                }

                if (source_x_end > glyph_width)
                    source_x_end = glyph_width;

                u8 strongest = 0;

                for (
                    int sample_x = source_x;
                    sample_x < source_x_end;
                    sample_x++
                ) {
                    u8 texel =
                        ui_i4_pixel(
                            texels,
                            UI_FONT_TEXEL_STRIDE,
                            sample_x,
                            source_y
                        );

                    if (texel > strongest)
                        strongest = texel;
                }

                if (strongest >= 3) {
                    row |=
                        (u8)(1 << (
                            7 -
                            (x + x_offset)
                        ));
                }
            }
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
