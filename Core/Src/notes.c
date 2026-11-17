
#include "notes.h"
#include <math.h>

// Names of the 12 notes in an octave
static const char *note_names[12] =
{
    "C", "C#", "D", "D#", "E", "F",
    "F#", "G", "G#", "A", "A#", "B"
};

// Convert a frequency into its nearest musical note
void notes_detect(uint32_t frequency,
                  const char **note_name,
                  int32_t *octave)
{
    // Convert tenths of Hz into Hz
    float hz = frequency / 10.0f;

    // Handle invalid frequencies
    if (hz <= 0.0f)
    {
        *note_name = "--";
        *octave = 0;
        return;
    }

    // Convert frequency into a MIDI note number
    // A4 (440 Hz) corresponds to MIDI note 69
    float midi = 69.0f + 12.0f * log2f(hz / 440.0f);

    // Round to the nearest musical note
    int32_t note_number = (int32_t)roundf(midi);

    // Determine the note within the octave
    int32_t note_index = note_number % 12;

    if (note_index < 0)
    {
        note_index += 12;
    }

    // Determine the octave number
    *octave = (note_number / 12) - 1;

    if (note_number < 0)
    {
        *octave -= 1;
    }

    // Return the note name
    *note_name = note_names[note_index];
}
