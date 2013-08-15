#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "util.h"


#define MY_UUID { 0xE0, 0x93, 0x49, 0x61, 0x16, 0x0A, 0x43, 0x15, 0xAA, 0x0E, 0xDB, 0x8F, 0x7B, 0x6D, 0x6E, 0xB9 }
PBL_APP_INFO(MY_UUID,
             "Traveler", "Aradine",
             0, 1, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

Window window;
TextLayerWithString time_layer;

void handle_init(AppContextRef ctx) {
  window_init(&window, "Traveler");
  window_stack_push(&window, false);

  init(&time_layer, "Huh", GRect(0, 10, 144, 30));

  layer_add_child(&window.layer, &time_layer.layer.layer);
}

void handle_tick(AppContextRef ctx, PebbleTickEvent *event) {
  PblTm *t = event->tick_time;
  set_time(&time_layer, t, NULL);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .tick_info = {
      .tick_handler = &handle_tick,
      .tick_units = SECOND_UNIT
    }
  };
  app_event_loop(params, &handlers);
}