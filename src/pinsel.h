#ifndef PINSEL_H
#define PINSEL_H

#define SANE_SCALE_MARGIN 0.03
#define VERSION "0.0.0"

// just something to keep the argument parsing more readable
#define MATCH2(a, b) strcmp(a, b) == 0
#define MATCH3(a, b, c) strcmp(a, b) == 0 || MATCH2(a, c)
#define GET_MACRO(_1,_2,_3,NAME,...) NAME
#define MATCH(...) else if (GET_MACRO(__VA_ARGS__, MATCH3, MATCH2)(__VA_ARGS__))

#endif
