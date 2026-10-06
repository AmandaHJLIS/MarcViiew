#ifndef UI_FONT_H
#define UI_FONT_H

/*
 * Initializes the experimental MarcViiew UI font.
 *
 * The renderer remains libogc's normal console renderer. This module only
 * supplies it with an alternate 8x16 glyph table.
 */
void ui_font_init(void);

#endif
