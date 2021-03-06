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

PBL_APP_INFO(MY_UUID,
             "Traveler", "Aradine",
             0, 1, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);


Window window;
TextLayerWithString time_layer;
TransitLayer transit_layers[2];
LineInfo lines[MAX_LINES];
unsigned int num_lines = 0;
unsigned int scroll_offset = 0;
static const uint32_t LINE_DICT_KEY = 0x00000101;

void handle_init(AppContextRef ctx) {
  window_init(&window, "Traveler");
  window_stack_push(&window, false);

  t_text_layer_init(&time_layer, "Huh", GRect(0, 5, 144, 24));

  transit_layer_init(&transit_layers[0], GRect(5, 35, PEBBLE_WIDTH/2-10, PEBBLE_WIDTH/2), "22");
  transit_layer_init(&transit_layers[1], GRect(PEBBLE_WIDTH/2+5, 35, PEBBLE_WIDTH/2-10, PEBBLE_WIDTH/2), "N");

  lines_changed();

  layer_add_child(&window.layer, &time_layer.layer.layer);
  layer_add_child(&window.layer, &transit_layers[0].layer);
  layer_add_child(&window.layer, &transit_layers[1].layer);
}

void handle_tick(AppContextRef ctx, PebbleTickEvent *event) {
  PblTm *t = event->tick_time;
  set_time(&time_layer, t, NULL);
}

void handle_btn_down(void *context, PebbleButtonEvent *evt) {
  switch (evt->button_id) {
    case BUTTON_ID_SELECT:
      // Toggle inbound/outbound
      for (int i = 0; i < 2; i++) {
        transit_layer_set_direction(
          &transit_layers[i],
          (transit_layer_get_direction(&transit_layers[i]) == TRANSIT_DIR_INBOUND) ? TRANSIT_DIR_OUTBOUND : TRANSIT_DIR_INBOUND
        );
      }
      lines_changed();
      break;
    case BUTTON_ID_UP:
      // Scroll lines up
      scroll_offset--;
      lines_changed(); // will rebalance scroll_offset if necessary
      break;
    case BUTTON_ID_DOWN:
      // Scroll lines down
      scroll_offset++;
      lines_changed();
      break;
    default:
      break;
  }
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
        .in_received = handle_line_recvd,
        .in_dropped =  handle_recv_failed
      }
    },
    .input_handlers = {
      .buttons = {
        .down = handle_btn_down
      }
    }
  };
  app_event_loop(params, &handlers);
}

bool dict_to_line_info(DictionaryIterator *iter, LineInfo *line_info) {
  // FYI: uint32_t is big-endian (e.g. 0x01 + 0x02 + 0x04 + 0x08 == 0x01020408)
  Tuple *tuple = dict_find(iter, LINE_DICT_KEY);
  APP_LOG(APP_LOG_LEVEL_ERROR, "Processing a message");
  if (tuple) {
    if (tuple->length != sizeof(LineInfo)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "LineInfo dict value is not the same size as LineInfo: %d != %d...", tuple->length, sizeof(LineInfo));
    } else if (tuple->type != TUPLE_BYTE_ARRAY) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "LineInfo dict value is not the same type as LineInfo: %d != %d...", tuple->type, TUPLE_BYTE_ARRAY);
    } else {
      memcpy(line_info, tuple->value->data, tuple->length);
      return true;
    }
  }
  return false;
}

void lines_clear() {
  num_lines = 0;
  lines_changed();
}

void lines_add(LineInfo *new_line) {
  // TODO: Can I copy a struct with assignment?
  unsigned int i;
  for (i = 0; i < num_lines; i++) {
    if (strcmp(lines[i].line, new_line->line) == 0) {
      // Exists, overwrite
      lines[num_lines++] = *new_line;
      return;
    }
  }
  // New entry
  if (num_lines < MAX_LINES) {
    lines[num_lines++] = *new_line;
    lines_changed();
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to add line %s; max lines added", new_line->line);
  }
}

void lines_remove_at_index(unsigned int index) {
  if (index < num_lines) {
    num_lines--;
    for (unsigned int i = index; i < num_lines; i++) {
      lines[i] = lines[i+1];
    }
    lines_changed();
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
    APP_LOG(APP_LOG_LEVEL_DEBUG, "    In:  %s - %d.", lines[i].inbound.terminus, lines[i].inbound.eta);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "    Out: %s - %d.", lines[i].outbound.terminus, lines[i].outbound.eta);
  }
}

void handle_line_recvd(DictionaryIterator *recvd, void *context) {
  LineInfo li;
  if (dict_to_line_info(recvd, &li)) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Adding a received line: %s", li.line);
    lines_add(&li);
  }
}

void handle_recv_failed(void *context, AppMessageResult reason) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Line update failed: %d", reason);
}
void lines_changed() {
  // Find places to point the line displays
  if (scroll_offset > MAX_LINES) {
    // We wrapped going negative (unsigned int...)
    scroll_offset = num_lines - 1;
  } else if (scroll_offset >= num_lines) {
    // We wrapped off the top
    scroll_offset = 0;
  }

  unsigned int displayed_lines[2] = { scroll_offset, 0 };

  // Second display:
  displayed_lines[1] = scroll_offset + 1;
  if (displayed_lines[1] > MAX_LINES) {
    displayed_lines[1] = num_lines - 1;
  } else if (displayed_lines[1] >= num_lines && displayed_lines[0] != 0) {
    displayed_lines[1] = 0;
  }

  // Update the rendering layers
  for (int dl = 0; dl < 2; dl++) {
    if (displayed_lines[dl] < num_lines) {
      LineInfo *line = &lines[displayed_lines[dl]];
      enum transit_direction dir = transit_layer_get_direction(&transit_layers[dl]); // TODO: Select inbound/outbound

      TerminusInfo *terminus = (dir == TRANSIT_DIR_INBOUND) ? &line->inbound : &line->outbound;

      transit_layer_set_direction(&transit_layers[dl], dir);
      transit_layer_set_title(&transit_layers[dl], line->line);
      transit_layer_set_terminus(&transit_layers[dl], terminus->terminus);
      transit_layer_set_eta(&transit_layers[dl], terminus->eta);
      transit_layer_show(&transit_layers[dl]);
    } else {
      transit_layer_hide(&transit_layers[dl]);
    }
  }

  log_lines();
}

