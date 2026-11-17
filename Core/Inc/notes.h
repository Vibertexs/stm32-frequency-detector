
#ifndef NOTES_H
#define NOTES_H

#include <stdint.h>

// Convert frequency (in tenths of Hz) to a musical note
void notes_detect(uint32_t frequency,
                  const char **note_name,
                  int32_t *octave);

#endif
