#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tosc_tinyosc {
  const char *address;
  const char *format;
  const char *marker;
  const char *buffer;
  int len;
} tosc_tinyosc;

int tosc_read(tosc_tinyosc *o, const char *buffer, const int len);

int32_t tosc_getNextInt32(tosc_tinyosc *o);

float tosc_getNextFloat(tosc_tinyosc *o);

const char *tosc_getNextString(tosc_tinyosc *o);

void tosc_getNextBlob(tosc_tinyosc *o, const char **buffer, int *len);

int tosc_write(char *buffer, const int len, const char *address,
    const char *fmt, ...);

void tosc_printOscBuffer(const char *buffer, const int len);

#ifdef __cplusplus
}
#endif

