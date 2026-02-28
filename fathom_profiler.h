#ifndef FATHOM_PROFILER_H
#define FATHOM_PROFILER_H

#include "fathom_types.h"

/* #############################################################################
 * # [SECTION] Performance Profiler
 * #############################################################################
 */
#define FATHOM_PROFILER_MAX_ENTRIES 512
#define FATHOM_PROFILER_ENTRY_INVALID 0xFFFFFFFF

typedef struct fathom_profiler_entry
{
    s8 *name;
    s8 *file;
    u32 line;

    f64 time_ms_begin;
    f64 time_ms_end;

} fathom_profiler_entry;

static fathom_profiler_entry fathom_profiler_entries[FATHOM_PROFILER_MAX_ENTRIES];
static u32 fathom_profiler_entries_count = 0;

FATHOM_API f64 fathom_profiler_time_ms(void);

FATHOM_API FATHOM_INLINE u32 fathom_profiler_string_equals(s8 *a, s8 *b)
{
    while (*a && *b)
    {
        if (*a != *b)
        {
            return 0;
        }

        a++;
        b++;
    }

    return (*a == *b);
}

FATHOM_API FATHOM_INLINE u32 fathom_profiler_find_entry(s8 *name)
{
    u32 i;

    for (i = 0; i < fathom_profiler_entries_count; ++i)
    {
        if (fathom_profiler_string_equals(fathom_profiler_entries[i].name, name))
        {
            return i;
        }
    }

    return FATHOM_PROFILER_ENTRY_INVALID;
}

FATHOM_API FATHOM_INLINE void fathom_profiler_begin(s8 *name, s8 *file, u32 line)
{
    fathom_profiler_entry entry;
    entry.name = name;
    entry.file = file;
    entry.line = line;
    entry.time_ms_begin = fathom_profiler_time_ms();

    fathom_profiler_entries[fathom_profiler_entries_count++] = entry;
}

FATHOM_API FATHOM_INLINE void fathom_profiler_end(s8 *name)
{
    f64 time_ms_end = fathom_profiler_time_ms();

    u32 entry_id = fathom_profiler_find_entry(name);

    if (entry_id != FATHOM_PROFILER_ENTRY_INVALID)
    {
        fathom_profiler_entries[entry_id].time_ms_end = time_ms_end;
    }
}

#define FATHOM_PROFILER_BEGIN(name) fathom_profiler_begin(#name, __FILE__, __LINE__)
#define FATHOM_PROFILER_END(name) fathom_profiler_end(#name)

#endif /* FATHOM_PROFILER_H */