/*
  contains functions for coverting the time
  taken from Codeproject article
  http://www.codeproject.com/Articles/17576/SystemTime-to-VariantTime-with-Milliseconds
*/

#ifndef COMMON_DATETIME_H_
#define COMMON_DATETIME_H_

BOOL SystemTimeToVariantTimeWithMilliseconds(
  SYSTEMTIME st,
  double *dVariantTime);

BOOL VariantTimeToSystemTimeWithMilliseconds(
  double dVariantTime,
  SYSTEMTIME *st);

#endif  // COMMON_DATETIME_H_
