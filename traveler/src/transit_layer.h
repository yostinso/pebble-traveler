#ifndef __TRANSIT_LAYER_H
#define __TRANSIT_LAYER_H

#include "traveler.h"

static const char DIRECTIONS[2][5] = {
  [TRANSIT_DIR_INBOUND] =  "In: ",
  [TRANSIT_DIR_OUTBOUND] = "Out:"
};

typedef struct {
  Layer layer; // If the layer is first, we can treat this struct as a "subclass" of Layer
  char title[4];
  enum transit_direction direction;
  char terminus[5];
  char eta_s[4+1+5]; // 4 digits, 1 space, 5 chars for "mins\0"
} TransitLayer;

void transit_layer_init(TransitLayer *tl, GRect bounds, const char *title);
void transit_layer_update_proc(TransitLayer *tl, GContext *ctx);
bool transit_layer_set_title(TransitLayer *tl, const char *title);
bool transit_layer_set_direction(TransitLayer *tl, enum transit_direction direction);
bool transit_layer_set_terminus(TransitLayer *tl, const char *terminus);
bool transit_layer_set_eta(TransitLayer *tl, unsigned int eta);
void _transit_layer_draw_border(TransitLayer *tl, GContext *ctx);
void _transit_layer_draw_title(TransitLayer *tl, GContext *ctx);
void _transit_layer_draw_endpoint(TransitLayer *tl, GContext *ctx);

void transit_layer_init(TransitLayer *tl, GRect bounds, const char *title) {
  layer_init(&tl->layer, bounds);
  transit_layer_set_title(tl, title);
  layer_set_update_proc(&tl->layer, (LayerUpdateProc)transit_layer_update_proc);
}

void transit_layer_update_proc(TransitLayer *tl, GContext *ctx) {
  _transit_layer_draw_border(tl, ctx);
  _transit_layer_draw_title(tl, ctx);
  _transit_layer_draw_endpoint(tl, ctx);
}

bool transit_layer_set_title(TransitLayer *tl, const char *title) {
  if (title == NULL) {
    strcpy(tl->title, "");
  } else {
    strncpy(tl->title, title, sizeof(tl->title)-1);
  }
  layer_mark_dirty(&tl->layer);
  if (title == NULL || strlen(tl->title) <= sizeof(tl->title)-1 || *(title+sizeof(tl->title)) == '\0') {
    return true;
  } else {
    // String too long to fit!
    return false;
  }
}

bool transit_layer_set_direction(TransitLayer *tl, enum transit_direction direction) {
  if (tl->direction != direction) {
    tl->direction = direction;
    layer_mark_dirty(&tl->layer);
  }
  return true;
}

bool transit_layer_set_terminus(TransitLayer *tl, const char *terminus) {
  if (terminus == NULL && strlen(terminus) == 0) {
    strcpy(tl->terminus, "");
  } else {
    // Copy initials to tl->terminus
    size_t t_i = 0, i;
    tl->terminus[t_i++] = terminus[0];

    size_t clen = strlen(terminus);
    for (i = 1; i < clen && t_i < sizeof(tl->terminus)-1; i++) {
      if (
          terminus[i-1] == ' ' && (
            (terminus[i] >= 65 && terminus[i] <= 90) ||
            (terminus[i] >= 97 && terminus[i] <= 122)
          )
         ) {
        // Alpha char before a space
        tl->terminus[t_i++] = terminus[i];
      }
    }
    for (i = t_i; i < sizeof(tl->terminus); i++) {
      tl->terminus[i] = '\0';
    }

    APP_LOG(APP_LOG_LEVEL_INFO, "Read initials %s from %s", tl->terminus, terminus);
  }
  layer_mark_dirty(&tl->layer);
  return true;
}

bool transit_layer_set_eta(TransitLayer *tl, unsigned int eta) {
  char eta_s[4+1+5]; // 4 digits, 1 space, 5 chars for "mins\0"

  int num_chars;

  if (eta > 90) {
    eta /= 60;
    if (eta > 9999) { eta = 9999; }
    num_chars = snprintf(eta_s, 5, "%d", eta); // wrote n - 1 digits
    eta_s[num_chars] = ' '; // write a space after the digits
    if (eta == 1) { strcpy(eta_s+num_chars+1, "hour"); } else { strcpy(eta_s+num_chars+1, "hrs"); }
  } else {
    num_chars = snprintf(eta_s, 5, "%d", eta); // wrote n - 1 digits
    eta_s[num_chars] = ' '; // write a space after the digits
    if (eta == 1) { strcpy(eta_s+num_chars+1, "min"); } else { strcpy(eta_s+num_chars+1, "mins"); }
  }

  strcpy(tl->eta_s, eta_s);

  layer_mark_dirty(&tl->layer);
  return true;
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
  GSize max_size = graphics_text_layout_get_max_used_size(ctx, tl->title, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(0, 0, PEBBLE_WIDTH, PEBBLE_HEIGHT), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  GRect circle_box = GRect(center.x - max_size.w/2, center.y - max_size.h + radius/2, max_size.w, max_size.h);
  graphics_context_set_text_color(ctx, GColorWhite);

  //APP_LOG(APP_LOG_LEVEL_INFO, "BOX: %s", tl->title);
  //APP_LOG(APP_LOG_LEVEL_INFO, "  center: %d, %d", center.x, center.y);
  //APP_LOG(APP_LOG_LEVEL_INFO, "  circle_box: %d, %d, %d, %d\n", circle_box.origin.x, circle_box.origin.y, circle_box.size.w, circle_box.size.h);
  graphics_text_draw(ctx, tl->title, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), circle_box, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

void _transit_layer_draw_endpoint(TransitLayer *tl, GContext *ctx) {
  graphics_context_set_text_color(ctx, GColorBlack);
  GRect bounds = layer_get_bounds(&tl->layer);
  int title_height = (bounds.size.w / 4) * 2 + 1; // radius * 2 = diameter

  // Direction:
  const char *direction = DIRECTIONS[tl->direction];
  GSize max_size_in  = graphics_text_layout_get_max_used_size(ctx, DIRECTIONS[TRANSIT_DIR_INBOUND],  fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(0, 0, PEBBLE_WIDTH, PEBBLE_HEIGHT), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  GSize max_size_out = graphics_text_layout_get_max_used_size(ctx, DIRECTIONS[TRANSIT_DIR_OUTBOUND], fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(0, 0, PEBBLE_WIDTH, PEBBLE_HEIGHT), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  int max_w = max_size_in.w > max_size_out.w ? max_size_in.w : max_size_out.w;
  GRect direction_box = GRect(2, title_height + 2, max_w, max_size_in.h);
  graphics_text_draw(ctx, direction, fonts_get_system_font(FONT_KEY_GOTHIC_18), direction_box, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

  // Terminus:
  if (tl->terminus && tl->terminus[0]) {
    max_size_in = graphics_text_layout_get_max_used_size(ctx, tl->terminus, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0, 0, PEBBLE_WIDTH, PEBBLE_HEIGHT), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    GRect terminus_box = GRect(direction_box.origin.x + direction_box.size.w + 1, direction_box.origin.y, max_size_in.w, max_size_in.h);
    graphics_text_draw(ctx, tl->terminus, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), terminus_box, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  }

  // ETA:
  if (tl->eta_s && tl->eta_s[0]) {
    max_size_in  = graphics_text_layout_get_max_used_size(ctx, tl->eta_s, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0, 0, PEBBLE_WIDTH, PEBBLE_HEIGHT), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    GRect eta_box = GRect(direction_box.origin.x, direction_box.origin.y + direction_box.size.h - 2, bounds.size.w - direction_box.origin.x - 2, max_size_in.h);
    graphics_text_draw(ctx, tl->eta_s, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), eta_box, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  }
}

#endif
