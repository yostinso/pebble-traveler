#ifndef __TRAVELER_H
#define __TRAVELER_H

#define PEBBLE_WIDTH 144
#define PEBBLE_HEIGHT 168

enum transit_direction {
  TRANSIT_DIR_INBOUND,
  TRANSIT_DIR_OUTBOUND
};

typedef struct {
  char terminus[5];
  unsigned int eta;
} TerminusInfo;

typedef struct {
  char line[4];
  TerminusInfo inbound;
  TerminusInfo outbound;
} LineInfo;

void lines_clear();
void lines_add(LineInfo *new_line);
void lines_remove(const char *name);
void lines_remove_at_index(int index);

void line_recvd_handler(DictionaryIterator *recvd, void *context);
void line_recvd_failed(void *context, AppMessageResult reason);
bool dict_to_line_info(DictionaryIterator *iter, LineInfo *line_info);

void log_lines(); // XXX DEBUG
#endif
