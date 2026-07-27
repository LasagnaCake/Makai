#ifndef CTL_EX_CALENDAR_ZONE_H
#define CTL_EX_CALENDAR_ZONE_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/ctl.hpp"

#ifdef CTL_ON_WINDOWS
#include <timezoneapi.h>
#else
#endif

CTL_EX_NAMESPACE_BEGIN

/// @brief Timezone information object.
struct Zone {
	constexpr Zone(): code(0) {}

	constexpr Zone(int8 const hour, uint8 const minute = 0) {
		code = CTL::Math::abs<int64>(hour) * 60 + minute;
		code *= CTL::Math::sign(hour);
	}

	constexpr static int64 convert(int64 const stamp, Zone const from, Zone const to) {
		return stamp - from.code + to.code;
	}

	constexpr Zone operator-(Zone const other) const {return fromOffset(code - other.code);}
	constexpr Zone operator+(Zone const other) const {return fromOffset(code + other.code);}

	constexpr Zone& operator-=(Zone const other) {code -= other.code; return *this;}
	constexpr Zone& operator+=(Zone const other) {code += other.code; return *this;}

	constexpr static Zone fromOffset(int64 const utc) {Zone z; z.code = utc; return z;}

	static Zone current() {
		#ifdef CTL_ON_WINDOWS
		TIME_ZONE_INFORMATION tzinfo;
		GetTimeZoneInformation(&tzinfo);
		return fromOffset(tzinfo.Bias);
		#else
		time_t const local = time(NULL);
		ref<tm> const tx = gmtime(&local);
		tx->tm_isdst = -1;
		time_t const gmt = mktime(tx);
		return fromOffset(difftime(local, gmt) / 60);
		#endif
	}

	constexpr static Zone utc() {
		return Zone(0);
	}

	constexpr int64 offset() const {return code;}
private:
	int64 code;
};

CTL_EX_NAMESPACE_END

#endif
