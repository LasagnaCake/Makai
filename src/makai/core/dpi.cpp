#include "dpi.hpp"

#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__)
#ifndef MAKAILIB_WINDOWS_8_1_OR_LATER
#include <windows.h>
#else
#include <shellscalingapi.h>
#endif // MAKAILIB_WINDOWS_8_1_OR_LATER

void Makai::setDPIAware() {
	#ifndef MAKAILIB_WINDOWS_8_1_OR_LATER
	SetProcessDPIAware();
	#else
	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
	#endif // MAKAILIB_WINDOWS_8_1_OR_LATER
}

#ifndef MAKAILIB_WINDOWS_8_1_OR_LATER
[[gnu::warning("Disabling DPI awareness is not available for Windows 8.0 and earlier!")]]
#endif // MAKAILIB_WINDOWS_8_1_OR_LATER
void Makai::setDPIUnaware() {
	#ifdef MAKAILIB_WINDOWS_8_1_OR_LATER
	SetProcessDpiAwareness(PROCESS_DPI_UNAWARE);
	#endif // MAKAILIB_WINDOWS_8_1_OR_LATER
}

#else

[[gnu::warning("DPI awareness is not supported on other platforms!")]]
void Makai::setDPIAware()	{}
[[gnu::warning("DPI awareness is not supported on other platforms!")]]
void Makai::setDPIUnaware()	{}

#endif
