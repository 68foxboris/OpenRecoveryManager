#ifndef RECOVERY_BOXINFO_H
#define RECOVERY_BOXINFO_H

#include <stddef.h>

/* A value of /usr/lib/enigma.info, empty when missing. */
void boxinfo_value(const char *key, char *value, size_t size);
/* getBoxName() of enigma2, the name of the box in the feeds of openATV. */
void boxinfo_box_name(char *box, size_t size);

#endif
