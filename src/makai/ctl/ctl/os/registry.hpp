#ifndef CTL_OS_SYSTEM_H
#define CTL_OS_SYSTEM_H

#include "../namespace.hpp"
#include "../container/strings/string.hpp"
#include "../container/error.hpp"

#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
#include <winreg.h>
#include <windows.h>
#endif

CTL_NAMESPACE_BEGIN

/// @brief Windows Registry manipulation facilities.
namespace OS::Registry {
	enum class Domain {
		ORD_HKEY_CLASSES_ROOT,
		ORD_HKEY_CURRENT_CONFIG,
		ORD_HKEY_CURRENT_USER,
		ORD_HKEY_LOCAL_MACHINE,
		ORD_HKEY_USERS,
	};

	namespace {
		constexpr HKEY keyOf(Domain const domain) {
			HKEY reg;
			switch (domain) {
				case Domain::ORD_HKEY_CLASSES_ROOT:		reg = HKEY_CLASSES_ROOT;
				case Domain::ORD_HKEY_CURRENT_CONFIG:	reg = HKEY_CURRENT_CONFIG;
				case Domain::ORD_HKEY_CURRENT_USER:		reg = HKEY_CURRENT_USER;
				case Domain::ORD_HKEY_LOCAL_MACHINE:	reg = HKEY_LOCAL_MACHINE;
				case Domain::ORD_HKEY_USERS:			reg = HKEY_USERS;
			}
			return reg;
		}
	}

	inline void setString(Domain const domain, String const& key, String const& value, bool const lazy = false) {
		#ifdef CTL_ON_WINDOWS
		RegSetValueEx(keyOf(domain), key.cstr(), 0, lazy ? REG_EXPAND_SZ : REG_SZ, (ref<byte const>)value.cstr(), value.size());
		#else
		#endif
	}

	inline void setUInt32(Domain const domain, String const& key, uint32 const value) {
		#ifdef CTL_ON_WINDOWS
		RegSetValueEx(keyOf(domain), key.cstr(), 0, REG_DWORD, (ref<byte const>)&value, sizeof(value));
		#else
		#endif
	}

	inline void setUInt64(Domain const domain, String const& key, uint64 const value) {
		#ifdef CTL_ON_WINDOWS
		RegSetValueEx(keyOf(domain), key.cstr(), 0, REG_QWORD, (ref<byte const>)&value, sizeof(value));
		#else
		#endif
	}

	inline void setStringList(Domain const domain, String const& key, StringList const& value) {
		#ifdef CTL_ON_WINDOWS
		String buf = value.join("\0") + "\0\0";
		RegSetValueEx(keyOf(domain), key.cstr(), 0, REG_MULTI_SZ, (ref<byte const>)buf.cstr(), buf.size());
		#else
		#endif
	}

	inline void setBytes(Domain const domain, String const& key, Binary<> const& value) {
		#ifdef CTL_ON_WINDOWS
		RegSetValueEx(keyOf(domain), key.cstr(), 0, REG_BINARY, value.data(), value.size());
		#else
		#endif
	}

	inline String getString(Domain const domain, String const& key, bool const lazy = false) {
		#ifdef CTL_ON_WINDOWS
		String buf = String().reserve(4096, '\0');
		uint32 sz;
		DWORD type;
		auto const result = RegGetValueA(
			keyOf(domain),
			key.cstr(),
			NULL,
			(lazy ? RRF_RT_REG_EXPAND_SZ : RRF_RT_REG_SZ) | RRF_NOEXPAND,
			&type,
			(pointer)buf.cstr(),
			&sz
		);
		if (result != ERROR_SUCCESS)
			throw Error::InvalidValue("Failed to get registry value!", CTL_CPP_PRETTY_SOURCE);
		return buf.resize(sz-1);
		#else
		return "";
		#endif
	}

	inline StringList getStringList(Domain const domain, String const& key) {
		#ifdef CTL_ON_WINDOWS
		String buf = String().reserve(4096, '\0');
		uint32 sz;
		DWORD type;
		auto const result = RegGetValueA(
			keyOf(domain),
			key.cstr(),
			NULL,
			RRF_RT_REG_MULTI_SZ | RRF_NOEXPAND,
			&type,
			(pointer)buf.cstr(),
			&sz
		);
		if (result != ERROR_SUCCESS)
			throw Error::InvalidValue("Failed to get registry value!", CTL_CPP_PRETTY_SOURCE);
		return buf.resize(sz-2).split('\0');
		#else
		return {};
		#endif
	}

	inline uint32 getUInt32(Domain const domain, String const& key) {
		#ifdef CTL_ON_WINDOWS
		uint32 out;
		uint32 sz;
		DWORD type;
		auto const result = RegGetValueA(
			keyOf(domain),
			key.cstr(),
			NULL,
			RRF_RT_DWORD | RRF_NOEXPAND,
			&type,
			(pointer)&out,
			&sz
		);
		if (result != ERROR_SUCCESS)
			throw Error::InvalidValue("Failed to get registry value!", CTL_CPP_PRETTY_SOURCE);
		return out;
		#else
		return 0;
		#endif
	}

	inline uint64 getUInt64(Domain const domain, String const& key) {
		#ifdef CTL_ON_WINDOWS
		uint64 out;
		uint32 sz;
		DWORD type;
		auto const result = RegGetValueA(
			keyOf(domain),
			key.cstr(),
			NULL,
			RRF_RT_QWORD | RRF_NOEXPAND,
			&type,
			(pointer)&out,
			&sz
		);
		if (result != ERROR_SUCCESS)
			throw Error::InvalidValue("Failed to get registry value!", CTL_CPP_PRETTY_SOURCE);
		return out;
		#else
		return 0;
		#endif
	}

	inline Binary<> getBytes(Domain const domain, String const& key) {
		#ifdef CTL_ON_WINDOWS
		Binary<> buf = Binary<>().reserve(4096, '\0');
		uint32 sz;
		DWORD type;
		auto const result = RegGetValueA(
			keyOf(domain),
			key.cstr(),
			NULL,
			RRF_RT_REG_BINARY | RRF_NOEXPAND,
			&type,
			(pointer)buf.data(),
			&sz
		);
		if (result != ERROR_SUCCESS)
			throw Error::InvalidValue("Failed to get registry value!", CTL_CPP_PRETTY_SOURCE);
		return buf.resize(sz);
		#else
		return {};
		#endif
	}
}

CTL_NAMESPACE_END

#endif // CTL_OS_SYSTEM_H
