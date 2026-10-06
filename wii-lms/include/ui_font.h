#ifndef MARC_VIIEW_UI_FONT_H
#define MARC_VIIEW_UI_FONT_H

#include <gccore.h>

int ui_font_init(void *xfb, GXRModeObj *rmode);
void ui_font_shutdown(void);
int ui_font_ready(void);
int ui_font_line_height(void);
int ui_font_text_width(const char *text);
void ui_font_draw_centered(const char *text, int y);

#endif
