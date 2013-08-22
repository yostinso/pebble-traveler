#ifndef __TRAVELER_H
#define __TRAVELER_H
#define MY_UUID { 0xE0, 0x93, 0x49, 0x61, 0x16, 0x0A, 0x43, 0x15, 0xAA, 0x0E, 0xDB, 0x8F, 0x7B, 0x6D, 0x6E, 0xB9 }
// E0:93:49:61:16:0A:43:15:AA:0E:DB:8F:7B:6D:6E:B9

#define PEBBLE_WIDTH 144
#define PEBBLE_HEIGHT 168

enum transit_direction {
  TRANSIT_DIR_INBOUND,
  TRANSIT_DIR_OUTBOUND
};

typedef struct {
  char terminus[4];
  uint32_t eta;
} TerminusInfo;

typedef struct {
  char line[4];
  TerminusInfo inbound;
  TerminusInfo outbound;
} LineInfo;


void lines_clear();
void lines_add(LineInfo *new_line);
void lines_remove(const char *name);
void lines_remove_at_index(unsigned int index);
void lines_changed();

void line_recvd_handler(DictionaryIterator *recvd, void *context);
void line_recvd_failed(void *context, AppMessageResult reason);
bool dict_to_line_info(DictionaryIterator *iter, LineInfo *line_info);

void log_lines(); // XXX DEBUG
#endif
