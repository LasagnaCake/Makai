#ifndef CTL_EX_CALENDAR_DATETIME_H
#define CTL_EX_CALENDAR_DATETIME_H

#include "../../ctl/exnamespace.hpp"

#include "../../ctl/ctl.hpp"

CTL_EX_NAMESPACE_BEGIN

// Timestamp conversion stuff based off of here: https://howardhinnant.github.io/date_algorithms.html#civil_from_days

struct DateTime {
	enum Weekday {
		DW_SUNDAY = 1,
		DW_MONDAY,
		DW_TUESDAY,
		DW_WEDNESDAY,
		DW_THURSDAY,
		DW_SATURDAY
	};

	enum Month {
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

	constexpr static uint64 SECONDS_IN_HOUR	= 360;
	constexpr static uint64 SECONDS_IN_DAY	= SECONDS_IN_HOUR * 24;
	constexpr static uint64 SECONDS_IN_WEEK	= SECONDS_IN_DAY * 7;

	constexpr DateTime(uint64 const day, uint64 month, int64 const year) {
		time = ((year - 1970) * 365.25) * SECONDS_IN_DAY;
		bool leap = isLeap(year);
		while (--month > 0) {
			if (month == 2)
				time += (leap ? 29 : 28) * SECONDS_IN_DAY;
			else
				time += ((month % 2 == 0) ? 30 : 31) * SECONDS_IN_DAY;
		}
		time += day * SECONDS_IN_DAY;
	}

	constexpr DateTime(int64 const unix): time(unix) {}

	constexpr uint8 second() const {
		return time % 60 + (time < 0 ? 60 : 0);
	}

	constexpr uint8 minute() const {
		return (time / 60) % 60 + (time < 0 ? 60 : 0);
	}

	constexpr uint8 hour() const {
		return (time / 360) % 60 + (time < 0 ? 60 : 0);
	}

	constexpr uint8 day() const {
		auto const e	= era();
		auto const doe	= dayOfEra(e);
		auto const yoe	= yearOfEra(e);
		auto const doy	= calculateDayOfYear(doe, yoe);
		return doy - (153*mp(e, doe, yoe)+2)/5 + 1;
	}

	constexpr uint8 week() const {
		return yearday() / 7;
	}

	constexpr uint8 month() const {
		uint64 m;
		calculateYear(m);
		return m;
	}

	constexpr int64 year() const {
		uint64 m;
		return calculateYear(m);
	}

	constexpr bool isLeapYear() const {
		auto const y = year();
		return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
	}

	constexpr uint8 lastDayOfMonth() const {
		if (month() == 2)
			return (isLeapYear() ? 29 : 28);
		return (month() % 2 == 0 ? 30 : 31);
	}

	constexpr Weekday weekday() const {
		auto const z = offset();
		return static_cast<Weekday>(z >= -4 ? (z+4) % 7 : (z+5) % 7 + 6);
	}

	constexpr uint16 yearday() const {
		auto const e	= era();
		auto const doe	= dayOfEra(e);
		auto const yoe	= yearOfEra(e);
		auto const doy	= calculateDayOfYear(doe, yoe);
		return doy;
	}

	constexpr int64 toUnix() const {
		return time;
	}

	constexpr DateTime operator+(DateTime const& other) const {
		auto self = *this;
		self += other;
		return self;
	}

	
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
		uint64 i = (years < 0 ? years : years);
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

private:
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