// datetime.cc
// code for datetime.h

#include "datetime.h"                  // this module

#include "exc.h"                       // checkFormat
#include "syserr.h"                    // xsyserror
#include "xassert.h"                   // xassert

#include <time.h>                      // time_t, time()

#ifdef __MINGW32__
#  define WIN32_LEAN_AND_MEAN
#  include "windows.h"                 // GetTimeZoneInformation
#endif // __MINGW32__


// ------------------------ OSDateTimeProvider ----------------------
class OSDateTimeProvider : public DateTimeProvider {
public:      // funcs
  UnixTime getCurrentUnixTime() override;
  int getLocalTzOffsetMinutes() override;
};


UnixTime OSDateTimeProvider::getCurrentUnixTime()
{
  time_t t = time(NULL);
  xassert(t != ((time_t)-1));          // this never fails in practice

  // I will just assume this is unix time.
  return (UnixTime)t;
}


int OSDateTimeProvider::getLocalTzOffsetMinutes()
{
#ifdef __MINGW32__
  // On mingw, tzset yields a 0 time zone under most circumstances
  // because it wants to use TZ and zoneinfo, which are not normally
  // installed, so we use the Windows API.
  TIME_ZONE_INFORMATION tzi;
  DWORD res = GetTimeZoneInformation(&tzi);
  if (res == TIME_ZONE_ID_INVALID) {
    xsyserror("GetTimeZoneInformation");
  }
  if (res == TIME_ZONE_ID_STANDARD) {
    return - (tzi.Bias + tzi.StandardBias);
  }
  else if (res == TIME_ZONE_ID_DAYLIGHT) {
    return - (tzi.Bias + tzi.DaylightBias);
  }
  else {
    return - tzi.Bias;
  }

#else
  // Sets global variable 'timezone' ...
  tzset();

  // ... which is an offset in seconds with opposite sense to that
  // of RFC 3339.  We round fractional minutes down.
  return (-timezone) / 60;
#endif // !__MINGW32__
}


DateTimeProvider *getOSDateTimeProvider()
{
  static OSDateTimeProvider provider;
  return &provider;
}


// ---------------------- FixedDateTimeProvider --------------------
UnixTime FixedDateTimeProvider::getCurrentUnixTime()
{
  return this->unixTime;
}

int FixedDateTimeProvider::getLocalTzOffsetMinutes()
{
  return this->tzOffsetMinutes;
}


// ------------------------ DateTimeSeconds ----------------------
DateTimeSeconds::DateTimeSeconds()
  : year(1970),
    month(1),
    day(1),
    hour(0),
    minute(0),
    second(0),
    tzOffsetMinutes(0)
{}


DateTimeSeconds::DateTimeSeconds(DateTimeSeconds const &obj)
  : DMEMB(year),
    DMEMB(month),
    DMEMB(day),
    DMEMB(hour),
    DMEMB(minute),
    DMEMB(second),
    DMEMB(tzOffsetMinutes)
{}


DateTimeSeconds& DateTimeSeconds::operator= (DateTimeSeconds const &obj)
{
  CMEMB(year);
  CMEMB(month);
  CMEMB(day);
  CMEMB(hour);
  CMEMB(minute);
  CMEMB(second);
  CMEMB(tzOffsetMinutes);
  return *this;
}


bool DateTimeSeconds::operator== (DateTimeSeconds const &obj) const
{
  return
    EMEMB(year) &&
    EMEMB(month) &&
    EMEMB(day) &&
    EMEMB(hour) &&
    EMEMB(minute) &&
    EMEMB(second) &&
    EMEMB(tzOffsetMinutes);
}


static bool divisible(int n, int d)
{
  return n % d == 0;
}

static bool isLeapYear(int year)
{
  return (divisible(year, 4) && !divisible(year, 100)) ||
         divisible(year, 400);
}

// Non-leap-year days per (0-based) month.
static int const nlyDaysPerMonth[] = {
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

// Days in the given year CE and 1-based month, or 0 if out of range.
static int daysInMonth(int month, int year)
{
  if (1 <= month && month <= 12) {
    int ret = nlyDaysPerMonth[month-1];
    if (month == 2 && isLeapYear(year)) {
      ret++;
    }
    return ret;
  }
  else {
    return 0;
  }
}

// Given a value that is n*unitSize+m, where m is in [0,unitSize-1],
// return n and set 'value' to m.  Conceptually, 'n' is the number of
// whole large units, while 'm' is the remaining small units (for
// example, 'n' might be minutes and 'm' seconds).
template <class T>
T extractUnits(T &value, T unitSize)
{
  T ret = value / unitSize;
  value = value % unitSize;

  // When the remainder is negative, shift it up to be positive and
  // simultaneously take one away from the quotient.
  if (value < 0) {
    ret--;
    value += unitSize;
  }

  return ret;
}

// Allow mixing certain types.
inline int64_t extractUnits(int64_t &value, int unitSize)
{
  return extractUnits<int64_t>(value, (int64_t)unitSize);
}


// Number of days from 1970-01-01T00:00:00 to 2001-01-01T00:00:00.
static int64_t const daysTo2001 = 365*31 + 8;

// Number of days in various blocks of years, including leap days.
static int64_t const daysIn400Years = 365*400 + 97;
static int64_t const daysIn100Years = 365*100 + 24;
static int64_t const daysIn4Years = 365*4 + 1;


void DateTimeSeconds::fromUnixTime(UnixTime unixTime, int tzOffsetMinutes_)
{
  // Adjust 'unixTime' so it is the number of seconds since
  // 1970-01-01T00:00:00 in the desired time zone.
  unixTime += tzOffsetMinutes_ * 60;

  // Realign to an epoch of 2001-01-01T00:00:00.  This way we start
  // on a non-leap-year, at the beginning of the periodic cycle, with
  // leap year events (or non-events) on the final year of each block.
  static int64_t const secondsPerDay = 60 * 60 * 24;
  unixTime -= secondsPerDay * daysTo2001;

  // Split into days and seconds in the day.
  int64_t seconds = unixTime;
  int64_t days = extractUnits(seconds, secondsPerDay);

  // Calculate year by splitting off blocks of various-sized years.
  this->year = 2001;
  this->year += extractUnits(days, daysIn400Years) * 400;
  this->year += extractUnits(days, daysIn100Years) * 100;
  this->year += extractUnits(days, daysIn4Years) * 4;
  this->year += extractUnits(days, 365);

  // Calculate month.
  {
    int m = 1;
    while (m <= 12) {
      int dim = daysInMonth(m, this->year);
      if (days < dim) {
        break;
      }
      else {
        days -= dim;
        m++;
      }
    }
    xassert(m <= 12);      // otherwise there is a bug above

    this->month = m;
  }

  // Convert to 1-based day.
  this->day = days+1;

  // Break down 'seconds' into usual units.
  this->hour = extractUnits(seconds, 60*60);
  this->minute = extractUnits(seconds, 60);
  this->second = seconds;

  this->tzOffsetMinutes = tzOffsetMinutes_;
}


UnixTime DateTimeSeconds::toUnixTime() const
{
  // Convert 'this->year' into number of days since 2001-01-01.
  int64_t years = this->year - 2001;
  int64_t days = 0;
  days += extractUnits(years, 400) * daysIn400Years;
  days += extractUnits(years, 100) * daysIn100Years;
  days += extractUnits(years, 4) * daysIn4Years;
  days += years * 365;

  // Add 'this->month'.
  for (int m=0; m < this->month; m++) {
    days += daysInMonth(m, this->year);
  }

  // Add 'this->day', which is 1-based.
  days += (this->day - 1);

  // Realign to 1970-01-01 epoch.
  days += daysTo2001;

  // Convert days to hours, add 'this->hour'
  UnixTime unixTime = days * 24 + this->hour;

  // Hours to minutes.
  unixTime = unixTime * 60 + this->minute;

  // Apply time zone.
  unixTime -= this->tzOffsetMinutes;

  // Minutes to seconds.
  unixTime = unixTime * 60 + this->second;

  return unixTime;
}


void DateTimeSeconds::fromCurrentTime(DateTimeProvider *provider)
{
  if (!provider) {
    provider = getOSDateTimeProvider();
  }
  this->fromUnixTime(
    provider->getCurrentUnixTime(),
    provider->getLocalTzOffsetMinutes());
}


static void checkRange(int64_t value, int64_t smallest, int64_t largest,
                       char const *name)
{
  if (!( smallest <= value && value <= largest )) {
    xformat(stringb(name << " is " << value <<
                    ", but must be in [" << smallest <<
                    ", " << largest << "]"));
  }
}

void DateTimeSeconds::validateFields() const
{
  // In general, my goal is to enforce the intersection of ISO 8601 and
  // RFC 3339.

  // ISO 8601 allows years beyond this range under some circumstances,
  // but RFC 3339 does not.
  checkRange(this->year, 0, 9999, "Year");

  checkRange(this->month, 1, 12, "Month");

  checkRange(this->day, 1, 31, "Day");

  // ISO 8601 allows 24, meaning "end of day", while RFC 3339 does not.
  checkRange(this->hour, 0, 23, "Hour");

  checkRange(this->minute, 0, 59, "Minute");

  checkRange(this->second, 0, 60, "Second");

  // RFC 3339 allows "-0".  I do not, and ISO 8601 does not either.
  //
  // The range of +/- 24h is based on my interpretation of the RFC 3339
  // grammar, which permits hour=24 despite the prohibition on its use
  // in the time field.
  checkRange(this->tzOffsetMinutes, -24*60, 24*60, "TZ offset");
}


string DateTimeSeconds::toString() const
{
  return stringb(this->dateTimeString() << ' ' << this->zoneString());
}

string DateTimeSeconds::dateTimeString() const
{
  return stringb(this->dateString() << ' ' << this->timeString());
}

string DateTimeSeconds::dateString() const
{
  return stringf("%04d-%02d-%02d",
    this->year,
    this->month,
    this->day);
}

string DateTimeSeconds::timeString() const
{
  return stringf("%02d:%02d:%02d",
    this->hour,
    this->minute,
    this->second);
}

string DateTimeSeconds::zoneString() const
{
  int minuteOffset = this->tzOffsetMinutes;
  char signChar = '+';
  if (minuteOffset < 0) {
    signChar = '-';
    minuteOffset = -minuteOffset;
  }
  int hourOffset = extractUnits(minuteOffset, 60);

  return stringf("%c%02d:%02d",
    signChar,
    hourOffset,
    minuteOffset);
}


std::ostream& DateTimeSeconds::insertOstream(std::ostream &os) const
{
  return os << this->toString();
}


string localTimeString()
{
  DateTimeSeconds d;
  d.fromCurrentTime();
  return d.dateTimeString();
}


// EOF
