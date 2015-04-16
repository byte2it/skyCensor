/*
  contains functions for coverting the time
  taken from Codeproject article
  http://www.codeproject.com/Articles/17576/SystemTime-to-VariantTime-with-Milliseconds
*/

#include "StdAfx.h"
#include "OleAuto.h"

#define ONETHOUSANDMILLISECONDS  .0000115740740740

BOOL SystemTimeToVariantTimeWithMilliseconds(
  SYSTEMTIME st,
  double *dVariantTime) {
    BOOL retVal = TRUE;

    WORD wMilliSeconds = st.wMilliseconds;
    st.wMilliseconds = 0;
    double dWithoutms;
    retVal = SystemTimeToVariantTime(&st, &dWithoutms);
    double OneMilliSecond =  ONETHOUSANDMILLISECONDS/1000;
    *dVariantTime = dWithoutms +  (OneMilliSecond * wMilliSeconds);

    return retVal;
}

BOOL VariantTimeToSystemTimeWithMilliseconds(
  double dVariantTime,
  SYSTEMTIME *st) {
    BOOL retVal = TRUE;
    double halfsecond = ONETHOUSANDMILLISECONDS / 2.0;
    retVal = VariantTimeToSystemTime(dVariantTime - halfsecond, st);
    if (retVal == FALSE) {
        return retVal;
    }

    double fraction = dVariantTime  - static_cast<int>(dVariantTime);
    double hours;
    hours = fraction = (fraction - static_cast<int>(fraction)) * 24;
    double minutes;
    minutes = (hours - static_cast<int>(hours)) * 60;
    double seconds;
    seconds = (minutes - static_cast<int>(minutes)) * 60;
    double milliseconds;
    milliseconds = (seconds - static_cast<int>(seconds)) * 1000;
    // rounding off millisecond to the nearest millisecond
    milliseconds = milliseconds + 0.5;
    // Fractional calculations may yield in results like
    if (milliseconds < 1.0 || milliseconds > 999.0)
      milliseconds = 0;
    // 0.00001 or 999.9999 which should actually be zero
    // (slightly above or below limits are actually zero)

    if (milliseconds)
        st->wMilliseconds = (WORD) milliseconds;
    else
        retVal = VariantTimeToSystemTime(dVariantTime, st);

    return retVal;
}
