#ifndef CTL_EX_CALENDAR_DATETIME_H
#define CTL_EX_CALENDAR_DATETIME_H

#include "../../ctl/exnamespace.hpp"

#include "../../ctl/ctl.hpp"

CTL_EX_NAMESPACE_BEGIN

// Timestamp conversion stuff based off of here: https://howardhinnant.github.io/date_algorithms.html#civil_from_days

/// @brief Date and time object.
struct DateTime {
	/// @brief Weekday.
	enum class Weekday {
		DW_SUNDAY = 1,
		DW_MONDAY,
		DW_TUESDAY,
		DW_WEDNESDAY,
		DW_THURSDAY,
		DW_SATURDAY
	};

	/// @brief Month.
	enum class Month {
		DM_JANUARY = 1,
		DM_FEBRUARY,
		DM_MARCH,
		DM_APRIL,
		DM_MAY,
		DM_JUNE,
		DM_JULY,
		DM_AUGUST,
		DM_SEPTEMBER,
		DM_OCTOBER,
		DM_NOVEMBER,
		DM_DECEMBER
	};

	/// @brief How many seconds are in a minute.
	constexpr static uint64 SECONDS_IN_MINUTE	= 60;
	/// @brief How many seconds are in an hour.
	constexpr static uint64 SECONDS_IN_HOUR		= SECONDS_IN_MINUTE * 360;
	/// @brief How many seconds are in a day.
	constexpr static uint64 SECONDS_IN_DAY		= SECONDS_IN_HOUR * 24;
	/// @brief How many seconds are in a week.
	constexpr static uint64 SECONDS_IN_WEEK		= SECONDS_IN_DAY * 7;

	/// @brief Time stamp.
	struct Stamp {
		/// @brief Year.
		int64	year		= 0;
		/// @brief Month.
		uint64	month:	4	= 0;
		/// @brief Day.
		uint64	day:	5	= 0;
		/// @brief Hour.
		uint64	hour:	6	= 0;
		/// @brief Minute.
		uint64	minute:	6	= 0;
		/// @brief Second.
		uint64	second:	6	= 0;

		/// @brief Converts the time stamp to an ISO string.
		/// @return Stamp as ISO string.
		String toString() const {
			String dt;
			dt += ::CTL::toString(year)		+ "-";
			dt += ::CTL::toString(month)	+ "-";
			dt += ::CTL::toString(day)		+ "T";
			dt += ::CTL::toString(hour)		+ ":";
			dt += ::CTL::toString(minute)	+ ":";
			dt += ::CTL::toString(second)	+ "Z";
			return dt;
		}
	};

	/// @brief Constructs the object from a date.
	/// @param year Date year.
	/// @param month Date month.
	/// @param day Date day.
	constexpr explicit DateTime(int64 const year, uint8 const month, uint8 const day) {
		if (((month % 12) + 1) == 2)
			buildFromDate(year, (month % 12) + 1, (day % (isLeap(year) ? 29 : 28)) + 1);
		else 
			buildFromDate(year, (month % 12) + 1, (day % ((month % 2 == 0) ? 30 : 31)) + 1);
	}

	/// @brief Constructs the object from a date and a time.
	/// @param year Date year.
	/// @param month Date month.
	/// @param day Date day.
	/// @param year Time hour.
	/// @param month Time minute.
	/// @param day Time second.
	constexpr explicit DateTime(
		int32 const year,
		uint8 const month,
		uint8 const day,
		uint8 const hour,
		uint8 const minute,
		uint8 const second
	): DateTime(year, month, day) {
		time += (hour % 60) * SECONDS_IN_HOUR + (minute % 60) * SECONDS_IN_MINUTE + (second % 60);
	}

	/// @brief Constructs the object from a datetime stamp.
	/// @param stamp Date time stamp.
	constexpr DateTime(Stamp const& time):
		DateTime(time.year, time.month, time.day, time.hour, time.minute, time.second) {}

	/// @brief Constructs the object from a UNIX timestamp.
	/// @param stamp UNIX timestamp.
	explicit constexpr DateTime(int64 const unix = 0): time(unix) {}

	/// @brief Returns the datetime's second.
	/// @return Second.
	constexpr uint8 second() const {
		return (Cast::as<uint64>(time) % 60) + (time < 0 ? 60 : 0);
	}

	/// @brief Returns the datetime's minute.
	/// @return Minute.
	constexpr uint8 minute() const {
		return (Cast::as<uint64>(time / 60) % 60) + (time < 0 ? 60 : 0);
	}

	/// @brief Returns the datetime's hour.
	/// @return Hour.
	constexpr uint8 hour() const {
		return (Cast::as<uint64>(time / 360) % 60) + (time < 0 ? 60 : 0);
	}

	/// @brief Returns the datetime's day of the month.
	/// @return Month day.
	constexpr uint8 day() const {
		auto const e	= era();
		auto const doe	= dayOfEra(e);
		auto const yoe	= yearOfEra(e);
		auto const doy	= calculateDayOfYear(doe, yoe);
		return doy - (153*mp(e, doe, yoe)+2)/5 + 1;
	}

	/// @brief Returns the datetime's week.
	/// @return Week.
	constexpr uint8 week() const {
		return yearday() / 7;
	}

	/// @brief Returns the datetime's month.
	/// @return Month.
	constexpr uint8 month() const {
		uint64 m;
		calculateYear(m);
		return m;
	}

	/// @brief Returns the datetime's year.
	/// @return Year.
	constexpr int32 year() const {
		uint64 m;
		return calculateYear(m);
	}

	/// @brief Returns whether the datetime's year is a leap year.
	/// @return Whether datetime year is a leap year.
	constexpr bool isLeapYear() const {
		auto const y = year();
		return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
	}

	/// @brief Returns the datetime month's last day.
	/// @return Last day of month.
	constexpr uint8 lastDayOfMonth() const {
		if (month() == 2)
			return (isLeapYear() ? 29 : 28);
		return (month() % 2 == 0 ? 30 : 31);
	}

	/// @brief Returns the datetime's day of the week.
	/// @return Week day.
	constexpr Weekday weekday() const {
		auto const z = offset();
		return static_cast<Weekday>(z >= -4 ? (z+4) % 7 : (z+5) % 7 + 6);
	}

	/// @brief Returns the datetime's day of the year.
	/// @return Year day.
	constexpr uint16 yearday() const {
		auto const e	= era();
		auto const doe	= dayOfEra(e);
		auto const yoe	= yearOfEra(e);
		auto const doy	= calculateDayOfYear(doe, yoe);
		return doy;
	}

	/// @brief Returns the datetime as a stamp.
	/// @return Datetime as stamp.
	constexpr Stamp toStamp() const {
		return {
			year(),
			month(),
			day(),
			hour(),
			minute(),
			second()
		};
	}

	/// @brief Returns the datetime as a UNIX timestamp.
	/// @return Datetime as UNIX timestamp.
	constexpr int64 toUnix() const {
		return time;
	}

	/// @brief Returns the datetime as an ISO string.
	/// @return Datetime as ISO string.
	constexpr String toISOString() const {
		return toStamp().toString();
	}

	/// @brief Addition operator overloading.
	constexpr DateTime operator+(DateTime const& other) const {
		auto self = *this;
		self += other;
		return self;
	}

	/// @brief Subtraction operator overloading.
	constexpr DateTime operator-(DateTime const& other) const {
		auto self = *this;
		self -= other;
		return self;
	}

	constexpr DateTime& operator+=(DateTime const& other) {
		addYears(other.year());
		addMonths(other.month());
		addDays(other.day());
		time += other.time % SECONDS_IN_DAY + (other.time < 0 ? SECONDS_IN_DAY : 0);
		return *this;
	}
	
	constexpr DateTime& operator-=(DateTime const& other) {
		addYears(-other.year());
		addMonths(-other.month());
		addDays(-other.day());
		time -= other.time % SECONDS_IN_DAY + (other.time < 0 ? SECONDS_IN_DAY : 0);
		return *this;
	}

	constexpr DateTime& addSeconds(int64 const seconds) {
		time += seconds;
		return *this;
	}
	
	constexpr DateTime& addMinutes(int64 const minutes) {
		time += minutes * 60;
		return *this;
	}

	constexpr DateTime& addHours(int64 const hours) {
		time += hours * SECONDS_IN_HOUR;
		return *this;
	}

	constexpr DateTime& addDays(int64 const days) {
		time += days * SECONDS_IN_DAY;
		return *this;
	}

	constexpr DateTime& addMonths(int64 const months) {
		auto m				= (months < 0 ? -months : months) % 12;
		auto const years	= months / 12;
		addYears(years);
		while (m > 0) {
			if (month() == 2)
				time += (isLeapYear() ? 29 : 28) * SECONDS_IN_DAY * (months < 0 ? -1 : 1);
			else
				time += ((m % 2 == 0) ? 30 : 31) * SECONDS_IN_DAY * (months < 0 ? -1 : 1);
			--m;
		}
		return *this;
	}

	constexpr DateTime& addYears(int64 const years) {
		uint64 i = (years < 0 ? -years : years);
		uint64 curYear = year();
		while (years != 0) {
			if (years < 0) {
				time -= years * (isLeap(curYear) ? 366 : 365) * SECONDS_IN_DAY;
				--curYear;
			} else {
				time += years * (isLeap(curYear) ? 366 : 365) * SECONDS_IN_DAY;
				++curYear;
			}
			--i;
		}
		return *this;
	}

	static DateTime now() {
		return DateTime(OS::Time::Clock::sinceEpoch<OS::Time::Seconds>());
	}

	constexpr static DateTime epoch() {
		return DateTime(0);
	}

	constexpr static DateTime fromISOString(String iso) {
		iso = iso.replace({'\t', '\n'}, ' ').eraseLike(' ');
		if (iso.find('T')) {
			auto components = iso.splitAtFirst('T');
			return fromISODateString(components.front()) + fromISOTimeString(components.back());
		} else if (iso.find(':') != -1)
			return fromISOTimeString(iso);
		else
			return fromISODateString(iso);
	}

	/*constexpr String toString(String const& format) const {
	}*/

private:
	constexpr static DateTime fromISODateString(String const& date) {
		DateTime dt;
		auto components = date.split('-');
		if (components.size() > 0)
			dt.addYears(toInt64(components[0]) - epoch().year());
		if (components.size() > 1)
			dt.addMonths(toInt64(components[1]) - epoch().month());
		if (components.size() > 2)
			dt.addDays(toInt64(components[2]) - epoch().day());
		return dt;
	}

	constexpr static DateTime fromISOTimeString(String const& time) {
		if (time.rfind('Z') != -1)
			return fromISOTimeString(time.substring(0, -2));
		else if ((time.rfind('+') != -1) || (time.rfind('-') != -1)) {
			auto components = time.splitAtLast({'+', '-'});
			DateTime dt = fromISOTimeString(components.front());
			auto zone = components.back().split(':');
			int64 zoneSecs = 0;
			if (components.size() > 0)
				zoneSecs += toInt64(components[0]) * SECONDS_IN_HOUR;
			if (components.size() > 1)
				zoneSecs += toInt64(components[1]) * SECONDS_IN_MINUTE * CTL::Math::sign(zoneSecs);
			if (components.size() > 2)
				zoneSecs += toInt64(components[2]) * CTL::Math::sign(zoneSecs);
			dt.addSeconds(zoneSecs);
			return dt;
		} else {
			DateTime dt;
			auto components = time.split(':');
			if (components.size() > 0)
				dt.addHours(toInt64(components[0]));
			if (components.size() > 1)
				dt.addMinutes(toInt64(components[1]));
			if (components.size() > 2)
				dt.addSeconds(toInt64(components[2]));
			return dt;
		}
	}

	constexpr void buildFromDate(int64 const year, uint8 month, uint8 const day) {
		time = ((year - 1970) * 365.25) * SECONDS_IN_DAY;
		bool leap = isLeap(year);
		while (--month > 0) {
			if (month == 2)
				time += (leap ? 29 : 28) * SECONDS_IN_DAY;
			else
				time += ((month % 2 == 0) ? 30 : 31) * SECONDS_IN_DAY;
		}
		time += (day-1) * SECONDS_IN_DAY;
	}

	constexpr static bool isLeap(uint64 const year) {
		return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
	}

	constexpr int64 offset() const {
		return time / 86400 + 719468;
	}

	constexpr int64 era() const {
		int64 z = offset();
		return (z >= 0 ? z : z - 146096) / 146097;
	}

	constexpr uint64 dayOfEra(int64 const era) const {
		return (offset() - era * 146097);
	}

	constexpr uint64 yearOfEra(int64 const era) const {
		const auto doe = dayOfEra(era);
		return (doe - doe/1460 + doe/36524 - doe/146096) / 365;
	}

	constexpr uint64 calculateDayOfYear(uint64 const doe, int64 const yoe) const {
		return doe - (365 * yoe + yoe / 4 - yoe / 100);
	}

	constexpr int64 calculateYear(uint64& month) const {
		auto const e = era();
		auto const p = mp(e, dayOfEra(e), yearOfEra(e));
		month = p + (p < 10 ? 3 : -9);
		return (static_cast<int64>(yearOfEra(e)) + e * 400) + (month <= 2);
	}

	constexpr int64 mp(int64 const era, uint64 const doe, int64 const yoe) const {
		auto const doy	= calculateDayOfYear(doe, yoe);
		return (5 * doy + 2) / 153;
	}

	int64 time;
};

CTL_EX_NAMESPACE_END

#endif