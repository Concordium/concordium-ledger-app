#pragma once

typedef struct {
    int tm_sec;  /* seconds,  range 0 to 59          */
    int tm_min;  /* minutes, range 0 to 59           */
    int tm_hour; /* hours, range 0 to 23             */
    int tm_mday; /* day of the month, range 1 to 31  */
    int tm_mon;  /* month, range 0 to 11             */
    int tm_year; /* The number of years since 1900   */
    int tm_wday; /* day of the week, range 0 to 6    */
    int tm_yday; /* day in the year, range 0 to 365  */
} tm;

/**
 * Converts seconds since epoch to a time struct.
 */
int secondsToTm(long long, tm *);

/**
 * Writes a time struct to dst as human readable text in
 * the format yyyy-mm-dd hh:mm:ss.
 */
int timeToDisplayText(tm time, uint8_t *dst, size_t dstLength);
