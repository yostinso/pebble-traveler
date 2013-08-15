#ifndef __BATT_LAYER_H
#define __BATT_LAYER_H

typedef struct {
  Layer layer; // If the layer is first, we can treat this struct as a "subclass" of Layer
  int percent;
  GPath outline;
} BatteryLayer;

void battery_layer_init(BatteryLayer *bl, int value, GRect rect);
void battery_layer_update_proc(BatteryLayer *bl, GContext *ctx);
void _battery_layer_draw_outline(BatteryLayer *bl, GContext *ctx);

void battery_layer_init(BatteryLayer *bl, int value, GRect rect) {
  layer_init(&bl->layer, rect);
  layer_set_update_proc(&bl->layer, (LayerUpdateProc)battery_layer_update_proc);
  // OR:
  // bl->update_proc = (LayerUpdateProc)battery_layer_update_proc;
}

void battery_layer_update_proc(BatteryLayer *bl, GContext *ctx) {
  _battery_layer_draw_outline(bl, ctx);
  
}

void _battery_layer_draw_outline(BatteryLayer *bl, GContext *ctx) {
  /* Draw battery outline
     the battery is horizontal and as tall and wide as the container
     the "tip" is 2% of the width of the container or 2px, whichever is greater
     the "tip" is 30% of the height or 5px, whichever is greater
     0------------1
     |            |
     |            2-3
     |              |
     |            5-4
     |            |
     7------------6
   */

  GRect bounds = layer_get_bounds(&bl->layer);
  bounds.size.w--;
  bounds.size.h--;
  int16_t tip_width = (bounds.size.w * 0.025);
  if (tip_width < 2) { tip_width = 2; }
  int16_t tip_height = (bounds.size.h * 0.3);
  if (tip_height < 4) { tip_height = 4; }
  int16_t tip_height_offset = (bounds.size.h - tip_height) / 2;

  //APP_LOG(APP_LOG_LEVEL_INFO, "Tip is %dx%d\n", tip_width, tip_height);

  GPathInfo outline_info = {
    .num_points = 8,
    .points = (GPoint []) {
      { bounds.origin.x, bounds.origin.y }, // Top left (0)
      { bounds.origin.x + bounds.size.w - tip_width, bounds.origin.y }, // Top right (1)
      { bounds.origin.x + bounds.size.w - tip_width, bounds.origin.y + tip_height_offset }, // Tip top left (2)
      { bounds.origin.x + bounds.size.w, bounds.origin.y + tip_height_offset }, // Tip top right (3)
      { bounds.origin.x + bounds.size.w, bounds.origin.y + bounds.size.h - tip_height_offset }, // Tip bottom right (4)
      { bounds.origin.x + bounds.size.w - tip_width, bounds.origin.y + bounds.size.h - tip_height_offset }, // Tip bottom left (5)
      { bounds.origin.x + bounds.size.w - tip_width, bounds.origin.y + bounds.size.h }, // Bottom right (6)
      { bounds.origin.x, bounds.origin.y + bounds.size.h } // Bottom left (7)
    }
  };

  gpath_init(&bl->outline, &outline_info);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  gpath_draw_outline(ctx, &bl->outline);
}

#endif
