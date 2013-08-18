#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "traveler.h"
#include "text_layer.h"
//#include "battery_layer.h"
#include "transit_layer.h"

// TODO Real data from phone
// TODO Select button changes inbound/outbound
// TODO Keep an array of layers, up/down scrolls them

#define MY_UUID { 0xE0, 0x93, 0x49, 0x61, 0x16, 0x0A, 0x43, 0x15, 0xAA, 0x0E, 0xDB, 0x8F, 0x7B, 0x6D, 0x6E, 0xB9 }
PBL_APP_INFO(MY_UUID,
             "Traveler", "Aradine",
             0, 1, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

Window window;
TextLayerWithString time_layer;
//BatteryLayer battery_layer;
TransitLayer transit_layer_1, transit_layer_2;

void handle_init(AppContextRef ctx) {
  window_init(&window, "Traveler");
  window_stack_push(&window, false);

  t_text_layer_init(&time_layer, "Huh", GRect(0, 5, 144, 24));
  //battery_layer_init(&battery_layer, 20, GRect(120, 17, 20, 10));

  transit_layer_init(&transit_layer_1, GRect(5, 35, PEBBLE_WIDTH/2-10, PEBBLE_WIDTH/2), "22");
  transit_layer_init(&transit_layer_2, GRect(PEBBLE_WIDTH/2+5, 35, PEBBLE_WIDTH/2-10, PEBBLE_WIDTH/2), "N");

  transit_layer_set_direction(&transit_layer_1, TRANSIT_DIR_OUTBOUND);
  transit_layer_set_terminus(&transit_layer_1, "The Quick Brown Fox Jumps Over The Lazy Dog");
  transit_layer_set_eta(&transit_layer_1, 12);

  layer_add_child(&window.layer, &time_layer.layer.layer);
  //layer_add_child(&window.layer, &battery_layer.layer);
  layer_add_child(&window.layer, &transit_layer_1.layer);
  layer_add_child(&window.layer, &transit_layer_2.layer);
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
