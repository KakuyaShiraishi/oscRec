
#include <netinet/in.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "oscRec.h"

int tosc_read(tosc_tinyosc *o, const char *buffer, const int len) {
    o->address = buffer;

  int i = 0;
  while (i < len && buffer[i] != ',') ++i;
  if (i == len) return -1;

    o->format = buffer + i + 1;

  while (i < len && buffer[i] != '\0') ++i;
  if (i == len) return -2;

  i = (i + 4) & ~0x3;
  o->marker = buffer + i;

  o->buffer = buffer;
  o->len = len;

  return 0;
}

int32_t tosc_getNextInt32(tosc_tinyosc *o) {
  const int32_t i = (int32_t) ntohl(*((uint32_t *) o->marker));
  o->marker += 4;
  return i;
}

float tosc_getNextFloat(tosc_tinyosc *o) {
  const uint32_t i = ntohl(*((uint32_t *) o->marker));
  o->marker += 4;
  return *((float *) (&i));
}

const char *tosc_getNextString(tosc_tinyosc *o) {
  int i = (int) strlen(o->marker);
  if (o->marker + i >= o->address + o->len) return NULL;
  const char *s = o->marker;
  i = (i + 4) & ~0x3;
  o->marker += i;
  return s;
}

void tosc_getNextBlob(tosc_tinyosc *o, const char **buffer, int *len) {
  int i = (int) ntohl(*((uint32_t *) o->marker));
  if (o->marker + 4 + i <= o->buffer + o->len) {
    *len = i;
    *buffer = o->marker + 4;
    i = (i + 7) & ~0x3;
    o->marker += i;
  } else {
    *len = 0;
    *buffer = NULL;
  }
}

int tosc_write(char *buffer, const int len,
    const char *address, const char *format, ...) {
  va_list ap;
  va_start(ap, format);

  memset(buffer, 0, len);
  int i = (int) strlen(address);
  if (address == NULL || i >= len) return -1;
  strcpy(buffer, address);
  i = (i + 4) & ~0x3;
  buffer[i++] = ',';
  int s_len = (int) strlen(format);
  if (format == NULL || (i + s_len) >= len) return -2;
  strcpy(buffer+i, format);
  i = (i + 4 + s_len) & ~0x3;

  for (int j = 0; format[j] != '\0'; ++j) {
    switch (format[j]) {
      case 'b': {
        const uint32_t n = (uint32_t) va_arg(ap, int);
        if (i + 4 + n > len) return -3;
        char *b = (char *) va_arg(ap, void *);
        *((uint32_t *) (buffer+i)) = htonl(n); i += 4;
        memcpy(buffer+i, b, n);
        i = (i + 3 + n) & ~0x3;
        break;
      }
      case 'f': {
        if (i + 4 > len) return -3;
        const float f = (float) va_arg(ap, double);
        *((uint32_t *) (buffer+i)) = htonl(*((uint32_t *) &f));
        i += 4;
        break;
      }
      case 'i': {
        if (i + 4 > len) return -3;
        const uint32_t k = (uint32_t) va_arg(ap, int);
        *((uint32_t *) (buffer+i)) = htonl(k);
        i += 4;
        break;
      }
      case 's': {
        const char *str = (const char *) va_arg(ap, void *);
        s_len = (int) strlen(str);
        if (i + s_len >= len) return -3;
        strcpy(buffer+i, str);
        i = (i + 4 + s_len) & ~0x3;
        break;
      }
      case 'T': // true
      case 'F': // false
      case 'N': // nil
      case 'I': // infinitum
          break;
      default: return -4; // unknown
    }
  }

  va_end(ap);
  return i;
}

void tosc_printOscBuffer(const char *buffer, const int len) {
  tosc_tinyosc osc;
  const int err = tosc_read(&osc, buffer, len);
  if (err == 0) {
    printf("[%i bytes] %s %s",
        len,
        osc.address,
        osc.format);

    for (int i = 0; osc.format[i] != '\0'; i++) {
      switch (osc.format[i]) {
        case 'b': {
          const char *b = NULL;
          int n = 0;
          tosc_getNextBlob(&osc, &b, &n);
          printf(" [%i]", n);
          for (int j = 0; j < n; j++) printf("%02X", b[j] & 0xFF); 
          break;
        }
        case 'f': printf(" %g", tosc_getNextFloat(&osc)); break;
        case 'i': printf(" %i", tosc_getNextInt32(&osc)); break;
        case 's': printf(" %s", tosc_getNextString(&osc)); break;
        case 'F': printf(" false"); break;
        case 'I': printf(" inf"); break;
        case 'N': printf(" nil"); break;
        case 'T': printf(" true"); break;
        default: printf(" Unknown format: '%c'", osc.format[i]); break;
      }
    }
    printf("\n");
  } else {
    printf("Error while reading OSC buffer: %i\n", err);
  }
}
