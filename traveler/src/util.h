#ifndef __UTIL_H
#define __UTIL_H

#define TEXTLAYER_MAX 32
typedef struct {
  char text[TEXTLAYER_MAX];
  TextLayer layer;
} TextLayerWithString;

void init(TextLayerWithString *tl, const char *str, GRect rect);
void set_text(TextLayerWithString *tl, const char *str);
void set_time(TextLayerWithString *tl, PblTm *t, const char *fmt);

void init(TextLayerWithString *tl, const char *str, GRect rect) {
  set_text(tl, str);
  text_layer_init(&tl->layer, rect);
  text_layer_set_text_alignment(&tl->layer, GTextAlignmentCenter);
  text_layer_set_font(&tl->layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
}
void set_text(TextLayerWithString *tl, const char *str) {
  if (str) {
    strncpy(tl->text, str, TEXTLAYER_MAX);
  } else {
    strcpy(tl->text, "");
  }
  text_layer_set_text(&tl->layer, tl->text);
}
void set_time(TextLayerWithString *tl, PblTm *t, const char *fmt) {
  if (!fmt) {
    fmt = "%H:%M:%S";
  }
  string_format_time(tl->text, TEXTLAYER_MAX, fmt, t);
  text_layer_set_text(&tl->layer, tl->text);
}

#endif
