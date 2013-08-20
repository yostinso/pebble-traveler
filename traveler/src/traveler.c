#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "traveler.h"
#include "text_layer.h"
#include "transit_layer.h"

// TODO: Can't I get the size of an array?
#define MAX_LINES 10


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
TransitLayer transit_layer_1, transit_layer_2;
LineInfo lines[MAX_LINES];
unsigned int num_lines = 0;
static const uint32_t LINE_DICT_KEY = 0x00000101;

void handle_init(AppContextRef ctx) {
  window_init(&window, "Traveler");
  window_stack_push(&window, false);

  t_text_layer_init(&time_layer, "Huh", GRect(0, 5, 144, 24));

  transit_layer_init(&transit_layer_1, GRect(5, 35, PEBBLE_WIDTH/2-10, PEBBLE_WIDTH/2), "22");
  transit_layer_init(&transit_layer_2, GRect(PEBBLE_WIDTH/2+5, 35, PEBBLE_WIDTH/2-10, PEBBLE_WIDTH/2), "N");

  transit_layer_set_direction(&transit_layer_1, TRANSIT_DIR_OUTBOUND);
  transit_layer_set_terminus(&transit_layer_1, "The Quick Brown Fox Jumps Over The Lazy Dog");
  transit_layer_set_eta(&transit_layer_1, 12);

  layer_add_child(&window.layer, &time_layer.layer.layer);
  layer_add_child(&window.layer, &transit_layer_1.layer);
  layer_add_child(&window.layer, &transit_layer_2.layer);
}

void handle_tick(AppContextRef ctx, PebbleTickEvent *event) {
  PblTm *t = event->tick_time;
  set_time(&time_layer, t, NULL);
}

void pbl_main(void *params) {
  const uint32_t line_info_dict_size = dict_calc_buffer_size(1, sizeof(LineInfo));
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .tick_info = {
      .tick_handler = &handle_tick,
      .tick_units = SECOND_UNIT
    },
    .messaging_info = {
      .buffer_sizes = {
        .inbound = line_info_dict_size,
        .outbound = 16
      },
      .default_callbacks.callbacks = {
        .in_received = line_recvd_handler,
        .in_dropped =  line_recvd_failed
      }
    }
  };
  app_event_loop(params, &handlers);
}

bool dict_to_line_info(DictionaryIterator *iter, LineInfo *line_info) {
  Tuple *tuple = dict_find(iter, LINE_DICT_KEY);
  if (tuple) {
    if (tuple->length != sizeof(LineInfo)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "LineInfo dict value is not the same size as LineInfo: %d != %d", tuple->length, sizeof(LineInfo));
    } else if (tuple->type != TUPLE_BYTE_ARRAY) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "LineInfo dict value is not the same type as LineInfo: %d != %d", tuple->type, TUPLE_BYTE_ARRAY);
    } else {
      memcpy(line_info, tuple->value->data, tuple->length);
      return true;
    }
  }
  return false;
}

void lines_clear() {
  num_lines = 0;
}

void lines_add(LineInfo *new_line) {
  // TODO: Can I copy a struct with assignment?
  unsigned int i;
  for (i = 0; i < num_lines; i++) {
    if (strcmp(lines[i].line, new_line->line) == 0) {
      // Exists, overwrite
      memcpy(&lines[num_lines++], new_line, sizeof(LineInfo));
      return;
    }
  }
  // New entry
  if (i < num_lines) {
    memcpy(&lines[num_lines++], new_line, sizeof(LineInfo));
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to add line %s; max lines added", new_line->line);
  }
}

void lines_remove_at_index(int index) {
  num_lines--;
  for (unsigned int i = index; i < num_lines; i++) {
    memcpy(&lines[i], &lines[i+1], sizeof(LineInfo));
  }
}

void lines_remove(const char *name) {
  for (unsigned int i = 0; i < num_lines; i++) {
    if (strncmp(lines[i].line, name, 4) == 0) {
      lines_remove_at_index(i);
      break;
    }
  }
}

void log_lines() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Logging %d lines", num_lines);
  for (unsigned int i = 0; i < num_lines; i++) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "  %s", lines[i].line);
  }
}

void line_recvd_handler(DictionaryIterator *recvd, void *context) {
  LineInfo li;
  if (dict_to_line_info(recvd, &li)) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Adding a received line: %s", li.line);
    lines_add(&li);
  }
}

void line_recvd_failed(void *context, AppMessageResult reason) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Line update failed: %d", reason);
}
