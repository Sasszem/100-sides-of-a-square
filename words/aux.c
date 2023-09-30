#include "forth_words.h"
#include <time.h>
#include "../simple_compile.h"

static void word_sleep() {
    struct timespec tspec;
    dn_cell_t ms = dn_pop();
    tspec.tv_sec = ms / 1000;
    ms = ms % 1000;
    tspec.tv_nsec = 1000000 * ms;
    nanosleep(&tspec, NULL);
}

void words_set_aux() {
    dn_create_system_word("MS", word_sleep);
}