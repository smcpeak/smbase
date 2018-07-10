// datetime.h
// Some date/time routines.

#ifndef DATETIME_H
#define DATETIME_H

#include "macros.h"                    // NOTEQUAL_OPERATOR
#include "sm-override.h"               // OVERRIDE
#include "str.h"                       // string

#include <iostream>                    // std::ostream

#include <stdint.h>                    // int64_t


// An integer number of seconds since 1970-01-01T00:00:00Z, ignoring
// leap seconds.
typedef int64_t UnixTime;


// Interface for querying date/time, for example from the OS.
//
// The main reason for the existence of this interface, rather than
// just having global functions, is to facilitate testing.
class DateTimeProvider {
public:      // funcs
  virtual ~DateTimeProvider() {}          // Silence warnings.

  // Get the current time.
  virtual UnixTime getCurrentUnixTime() = 0;

  // Get local time zone offset right now.
  //
  // If there is a local convention like daylight saving time, the
  // output of this function includes it if and only if (the provider
  // thinks) it applies right now.
  virtual int getLocalTzOffsetMinutes() = 0;
};

// Return a singleton object that queries the OS.
DateTimeProvider *getOSDateTimeProvider();


inline UnixTime getCurrentUnixTime()
  { return getOSDateTimeProvider()->getCurrentUnixTime(); }

inline int getLocalTzOffsetMinutes()
  { return getOSDateTimeProvider()->getLocalTzOffsetMinutes(); }


// For testing purposes, a provider that just yields specific values.
class FixedDateTimeProvider : public DateTimeProvider {
public:      // data
  // Values to yield.
  UnixTime unixTime;
  int tzOffsetMinutes;

public:      // funcs
  FixedDateTimeProvider(UnixTime u, int t)
    : unixTime(u),
      tzOffsetMinutes(t)
  {}

  UnixTime getCurrentUnixTime() OVERRIDE;
  int getLocalTzOffsetMinutes() OVERRIDE;
};


// Represent a date/time, at the resolution of one second, in a
// particular time zone.
//
// This class is meant to straightforwardly encompass the fields of an
// RFC 3339 date/time string.  RFC 3339 is a "profile" of ISO 8601.
//
// Each field has a documented range, but that range is only enforced
// by validateFieldsForRFC3339().  See the implementation of that
// function for comments describing rationale for the ranges for some
// of the fields.
class DateTimeSeconds {
public:      // data
  // Common era, Gregorian calendar year, in [0,9999].
  int year;

  // Month where 1 is January and 12 is December.
  int month;

  // Day of month in [1,31].
  int day;

  // In [0,23].
  int hour;

  // In [0,59].
  int minute;

  // In [0,60], 60 meaning a leap second.
  int second;

  // Minutes of offset of the time from UTC, where positive is East of
  // the prime meridian, in [-24*60,24*60].
  int tzOffsetMinutes;

public:      // funcs
  // Sets to 1970-01-01 00:00:00 +00:00.
  DateTimeSeconds();

  // Copy all fields.
  DateTimeSeconds(DateTimeSeconds const &obj);
  DateTimeSeconds& operator= (DateTimeSeconds const &obj);

  // Compare for exact equality, including time zone.
  bool operator== (DateTimeSeconds const &obj) const;
  NOTEQUAL_OPERATOR(DateTimeSeconds);

  // There is no operator< defined since the presence of the time
  // zone offset, and possible presence of values outside their
  // documented ranges, would make its meaning potentially misleading.

  // Convert from unix time.  The result may have a year outside the
  // documented range [0,9999] if the input magnitude is large enough.
  void fromUnixTime(UnixTime unixTime, int tzOffsetMinutes);

  // Convert to unix time.  If a field is outside its documented range,
  // the result is meant to be a "smooth" continuation of the function,
  // although that is somewhat subjective.
  UnixTime toUnixTime() const;

  // Get the current date, time, and local time zone from the OS (or
  // a specified provider) and populate this object with them.
  void fromCurrentTime(DateTimeProvider *provider = NULL);

  // Validate that the fields conform to their documented ranges.  If
  // one does not, throw xFormat.
  //
  // See the implementation for comments on the rationale for the
  // chosen ranges.
  void validateFields() const;

  // Return in format: "YYYY-MM-DD hh:mm:ss +hh:mm" where '+' might
  // actually be '-'.  If the value in any field is outside its usual
  // range, it is straightforwardly included in the string without
  // special treatment.  That means there might be more digits than
  // suggested by the format string above, and might be extra '-'
  // characters due to negative values.
  string toString() const;

  // Date and time only: "YYYY-MM-DD hh:mm:ss".
  string dateTimeString() const;

  // Date only: "YYYY-MM-DD".
  string dateString() const;

  // "hh:mm:ss".
  string timeString() const;

  // "+hh:ss" or "-hh:ss".
  string zoneString() const;

  // Write in same format as 'toString'.
  std::ostream& insertOstream(std::ostream &os) const;
  friend std::ostream& operator<< (std::ostream &os, DateTimeSeconds const &obj)
    { obj.insertOstream(os); return os; }
};


// Return the current time in the format of
// DateTimeSeconds::dateTimeString().
string localTimeString();


#endif // DATETIME_H
