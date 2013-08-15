#ifndef __TEXT_LAYER_H
#define __TEXT_LAYER_H

#define TEXTLAYER_MAX 32
#define FONT_SIZE 24
typedef struct {
  TextLayer layer; // If the layer is first, we can treat this struct as a "subclass" of TextLayer
  char text[TEXTLAYER_MAX];
} TextLayerWithString;

void t_text_layer_init(TextLayerWithString *tl, const char *str, GRect rect);
void set_text(TextLayerWithString *tl, const char *str);
void set_time(TextLayerWithString *tl, PblTm *t, const char *fmt);

void t_text_layer_init(TextLayerWithString *tl, const char *str, GRect rect) {
  if (rect.size.h < FONT_SIZE) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Container is not as big as font; got %d, expected %d", rect.size.h, FONT_SIZE);
  }
  set_text(tl, str);
  text_layer_init(&tl->layer, rect);
  text_layer_set_text_alignment(&tl->layer, GTextAlignmentCenter);
  text_layer_set_font(&tl->layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
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
    fmt = "%H:%M";
  }
  string_format_time(tl->text, TEXTLAYER_MAX, fmt, t);
  text_layer_set_text(&tl->layer, tl->text);
}

#endif
