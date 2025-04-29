#ifndef MAKAILIB_EX_GAME_DANMAKU_ANIMA_DECODE_H
#define MAKAILIB_EX_GAME_DANMAKU_ANIMA_DECODE_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game::Danmaku::Anima {
	template<usize D>
	constexpr Math::Vector<D> toVector(String const& str, Math::Vector<D> const& fallback = 0)
	requires (D == 1) {
		if (str.empty()) return fallback;
		try {
			return String::toNumber<T>(str);
		} catch (...) {
			throw Error::InvalidValue(
				toString("Invalid value of [", str, "] for ", String(nameof<Vector<D>>()), "!"),
				CTL_CPP_PRETTY_SOURCE
			);
		}
	}

	template<usize D>
	constexpr Math::Vector<D> toVector(String const& str, Math::Vector<D> const& fallback = 0)
	requires (D > 1) {
		if (str.empty()) return fallback;
		if (D == 4 && str.front() == '#')
			return Graph::Color::fromHexCodeString(str);
		StringList components = str.split(',');
		Math::Vector<D> out;
		usize const end = (components.size() < D ? components.size() : D);
		for (usize i = 0; i < end; ++i) {
			try {
				if (end == 1) out = toFloat(components[i]);
				else out.data[i] = toFloat(components[i]);
			} catch (...) {
				throw Error::InvalidValue(
					toString("Invalid value of [", str, "] for ", String(nameof<Math::Vector<D>>()), " property!"),
					CTL_CPP_PRETTY_SOURCE
				);
			}
		}
		return out;
	}

	constexpr Math::Ease::Mode toEaseMode(String const& param) {
		if (param.empty()) return Math::Ease::linear;
		StringList ease = param.splitAtFirst('.');
		if (param.size() == 1) return Math::Ease::getMode(ease.front(), "linear");
		return Math::Ease::getMode(ease.front(), ease.back());
	}
}

#endif