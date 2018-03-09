#include <stdio.h>
#include "alist.h"
#include "font.h"

apair_t fh_ial[] = {
    {"normal",  FH_NORMAL},
    {"large",   FH_LARGE},
    {"huge",    FH_HUGE},
    {"small",   FH_SMALL},
    {"tiny",    FH_TINY},
    {NULL,      -1},
};

apair_t fh_act_ial[] = {
    {"1.0",     FH_NORMAL},
    {"1.414",   FH_LARGE},
    {"2.0",     FH_HUGE},
    {"0.707",   FH_SMALL},
    {"0.5",     FH_TINY},
    {NULL,      -1},
};

apair_t ff_ial[] = {
    {"serif",       FF_SERIF},
    {"sanserif",    FF_SANSERIF},
    {"italic",      FF_ITALIC},
    {"type",        FF_TYPE},
    {NULL,      -1},
};

apair_t ff_act_ial[] = {
    {"Times-Roman",    FF_SERIF},
    {"Helvetica",      FF_SANSERIF},
    {"Times-Italic",   FF_ITALIC},
    {"Courier",        FF_TYPE},
    {NULL,      -1},
};

