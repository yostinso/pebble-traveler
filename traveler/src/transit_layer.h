#ifndef __TRANSIT_LAYER_H
#define __TRANSIT_LAYER_H

#define TRANSIT_TITLE_MAX 3

typedef struct {
  Layer layer; // If the layer is first, we can treat this struct as a "subclass" of Layer
  char title[TRANSIT_TITLE_MAX+1];
} TransitLayer;

void transit_layer_init(TransitLayer *tl, GRect bounds, const char *title);
void transit_layer_update_proc(TransitLayer *tl, GContext *ctx);
bool transit_layer_set_title(TransitLayer *tl, const char *title);
void _transit_layer_draw_border(TransitLayer *tl, GContext *ctx);
void _transit_layer_draw_title(TransitLayer *tl, GContext *ctx);

void transit_layer_init(TransitLayer *tl, GRect bounds, const char *title) {
  layer_init(&tl->layer, bounds);
  transit_layer_set_title(tl, title);
  layer_set_update_proc(&tl->layer, (LayerUpdateProc)transit_layer_update_proc);
}

void transit_layer_update_proc(TransitLayer *tl, GContext *ctx) {
  _transit_layer_draw_border(tl, ctx);
  _transit_layer_draw_title(tl, ctx);
}

bool transit_layer_set_title(TransitLayer *tl, const char *title) {
  if (title == NULL) {
    strcpy(tl->title, "");
  } else {
    strncpy(tl->title, title, TRANSIT_TITLE_MAX);
  }
  layer_mark_dirty(&tl->layer);
  if (title == NULL || strlen(tl->title) <= TRANSIT_TITLE_MAX || *(title+TRANSIT_TITLE_MAX+1) == '\0') {
    return true;
  } else {
    // String too long to fit!
    return false;
  }
}

void _transit_layer_draw_border(TransitLayer *tl, GContext *ctx) {
  GRect bounds = layer_get_bounds(&tl->layer);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_round_rect(ctx, bounds, 3);
}

void _transit_layer_draw_title(TransitLayer *tl, GContext *ctx) {
  // Draw a little centered circle at the top for the line number
  GRect bounds = layer_get_bounds(&tl->layer);
  int radius = bounds.size.w / 4;
  GPoint center = {
    .x = bounds.origin.x + (bounds.size.w / 2),
    .y = bounds.origin.y + radius + 1
  };
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, center, radius);

  // Draw the title in the circle
  GRect circle_box = GRect(center.x - radius, center.y - radius, radius*2, radius*2);
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_text_draw(
      ctx,
      "hey",
      FONT_KEY_GOTHIC_14_BOLD,
      circle_box,
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentCenter,
      NULL
      );
  
}

#endif
