#ifndef CTL_EX_COLOR_CORE_H
#define CTL_EX_COLOR_CORE_H

#include "../../ctl/ctl.hpp"
#include "../math/vector.hpp"
#include "../data/value.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Color space facilities.
namespace Color {
	using RGBF	= Math::Vector3;
	using RGBAF	= Math::Vector4;

	template<usize W>
	using Channel = Meta::Select<(W-1)/8,uint8,uint16,uint32,uint32,uint64,uint64,uint64,uint64>;

	consteval usize maxof(usize const ch) {
		return ((2<<ch)-1);
	}

	enum class ChannelOrder: uint8 {
		CCO_RGB = 0x40,
		CCO_BGR,
		CCO_RBG,
		CCO_BRG,
		CCO_GRB,
		CCO_GBR,
		CCO_RGBA = 0x80,
		CCO_ARGB,
	}

	template <ChannelOrder O, usize... CS, class TChannel>
	struct TColorI;

	template <class T>
	struct Colorable: Self<T> {
		using Self<T>::self;

		constexpr RGBF normalized() const requires (
			T::ORDER >= ChannelOrder::CCO_RGB && T::ORDER < ChannelOrder::CCO_RGBA
		) {
			return RGBF(self().r, self().g, self().b) / T::MAX;
		}

		constexpr RGBAF normalized() const requires (
			T::ORDER >= ChannelOrder::CCO_RGBA
		) {
			return RGBAF(self().r, self().g, self().b, self().a) / T::MAX;
		}

		constexpr operator decltype(normalized())() const {
			return normalized();
		}
	};

	template <usize RS, usize GS = RS, usize BS = GS, Type::Unsigned TChannel = Channel<W>>
	struct [[CTL_PACKED_STRUCT]] TColorI<ChannelOrder::RGB, RS, GS, BS, TChannel>, Colorable<TColorI<ChannelOrder::RGB, RS, GS, BS, TChannel>> {
		constexpr auto ORDER const = ChannelOrder::RGB;

		TChannel r: RS;
		TChannel g: GS;
		TChannel b: BS;

		consteval static RGBF const MAX = RGBAF(
			maxof(RS),
			maxof(GS),
			maxof(BS)
		);
	};

	template <usize RS, usize GS = RS, usize BS = GS, usize AS = BS, Type::Unsigned TChannel = Channel<W>>
	struct [[CTL_PACKED_STRUCT]] TColorI<ChannelOrder::RGBA, RS, GS, BS, AS, TChannel>, Colorable<TColorI<ChannelOrder::RGBA, RS, GS, BS, AS, TChannel>>{
		TChannel r: RS;
		TChannel g: GS;
		TChannel b: BS;
		TChannel a: AS;

		consteval static RGBAF const MAX = RGBAF(
			maxof(RS),
			maxof(GS),
			maxof(BS),
			maxof(AS)
		);
	};

	using RGBAi2		= TColorI<ChannelOrder::RGBA, 2>;
	using RGBAi3		= TColorI<ChannelOrder::RGBA, 3>;
	using RGBAi8b2321	= TColorI<ChannelOrder::RGBA, 2, 3, 2, 1>;
	using RGBAi4		= TColorI<ChannelOrder::RGBA, 4>;
	using RGBAi5		= TColorI<ChannelOrder::RGBA, 5>;
	using RGBAi16b5551	= TColorI<ChannelOrder::RGBA, 5, 5, 5, 1>;
	using RGBAi16b4543	= TColorI<ChannelOrder::RGBA, 4, 5, 4, 3>;
	using RGBAi6		= TColorI<ChannelOrder::RGBA, 6>;
	using RGBAi8		= TColorI<ChannelOrder::RGBA, 8>;
	using RGBAi12		= TColorI<ChannelOrder::RGBA, 12>;
	using RGBAi16		= TColorI<ChannelOrder::RGBA, 16>;
	using RGBAi24		= TColorI<ChannelOrder::RGBA, 24>;
	using RGBAi32		= TColorI<ChannelOrder::RGBA, 32>;
	using RGBAi48		= TColorI<ChannelOrder::RGBA, 48>;
	using RGBAi64		= TColorI<ChannelOrder::RGBA, 64>;

	using RGBi2			= TColorI<ChannelOrder::RGB, 2>;
	using RGBi3			= TColorI<ChannelOrder::RGB, 3>;
	using RGBi8b332		= TColorI<ChannelOrder::RGB, 3, 3, 2>;
	using RGBi4			= TColorI<ChannelOrder::RGB, 4>;
	using RGBi5			= TColorI<ChannelOrder::RGB, 5>;
	using RGBi16b565	= TColorI<ChannelOrder::RGB, 5, 6, 5>;
	using RGBi6			= TColorI<ChannelOrder::RGB, 6>;
	using RGBi16b664	= TColorI<ChannelOrder::RGB, 6, 6, 4>;
	using RGBi8			= TColorI<ChannelOrder::RGB, 8>;
	using RGBi12		= TColorI<ChannelOrder::RGB, 12>;
	using RGBi16		= TColorI<ChannelOrder::RGB, 16>;
	using RGBi24		= TColorI<ChannelOrder::RGB, 24>;
	using RGBi32		= TColorI<ChannelOrder::RGB, 32>;
	using RGBi48		= TColorI<ChannelOrder::RGB, 48>;
	using RGBi64		= TColorI<ChannelOrder::RGB, 64>;

	/// @brief Partial implementations.
	namespace Partial {
		/// @brief Hue conversion facilitator.
		constexpr float hueify(float t) {
			if(t < 0) t++;
            if(t > 1) t--;
            if(t < 1.0/6.0) return 6.0 * t;
            if(t < 1.0/2.0) return 1;
            if(t < 2.0/3.0) return (2.0/3.0 - t) * 6.0;
            return 0;
		}
	}

	// TODO: Optimize this
	/// @brief Converts a linear hue to a "RGB-like" color.
	/// @param hue Hue to convert.
	/// @return RGB-like color.
	constexpr RGBAF toPastel(float hue) {
		hue *= PI;
		RGBF res(
			cos(hue),
			cos(hue + TAU * (1.0/3.0)),
			cos(hue + TAU * (2.0/3.0))
		);
		res = (res^2.0).normalized() * SQRT2;
		return RGBAF(res.clamped(0, 1), 1);
	}

	/// @brief Converts a linear hue to an RGBA color.
	/// @param h Hue to convert.
	/// @return RGBA color.
	constexpr RGBAF toRGBA(float h) {
		h -= floor(h);
		return RGBAF(
			Partial::hueify(h + 1.0/3.0),
			Partial::hueify(h),
			Partial::hueify(h - 1.0/3.0),
			1
		);
	}

	/// @brief Creates a white color with a given alpha.
	/// @param a Alpha channel.
	/// @return Resulting color.
	constexpr RGBAF alpha(float a) {
		return RGBAF(1, 1, 1, CTL::Math::clamp(a, -0.1f, 1.0f));
	}

	/// @brief Creates a color from a set of RGBA values.
	/// @param r Red channel.
	/// @param g Green channel.
	/// @param b Blue channel.
	/// @param a Alpha channel. By default, it is `1.0`.
	/// @return Resulting color.
	constexpr RGBAF fromRGBA(float r, float g, float b, float a = 1) {
		return RGBAF(r, g, b, a);
	}

	/// @brief Creates a color from a set of RGBA values.
	/// @param r Red channel.
	/// @param g Green channel.
	/// @param b Blue channel.
	/// @param a Alpha channel. By default, it is `1.0`.
	/// @return Resulting color.
	constexpr RGBA8 fromRGBA8(uint8 r, uint8 g, uint8 b, uint8 a = 255) {
		return RGBA8(r, g, b, a).normalized();
	}

	/// @brief Creates a gray tone from a given intensity, with a given alpha.
	/// @param l Luminance intensity.
	/// @param a Alpha channel. By default, it is `1.0`.
	/// @return Resulting color.
	constexpr RGBAF fromLuma(float l, float a = 1) {
		return RGBAF(l, l, l, a);
	}

	/// @brief Creates a gray tone from a given 8-bit intensity, with a given 8-bit alpha.
	/// @param l Luminance intensity.
	/// @param a Alpha channel. By default, it is `255`.
	/// @return Resulting color.
	constexpr RGBA8 fromLuma8(uint8 l, uint8 a = 255) {
		return RGBA8(l, l, l, a).normalized();
	}

	/// @brief Creates an RGB color from a set of HSL values.
	/// @param h Hue.
	/// @param s Saturation.
	/// @param l Luminosity.
	/// @param a Alpha channel. By default, it is `1.0`.
	/// @return Resulting color.
	constexpr RGBAF fromHSL(float h, float s, float l, float a = 1) {
		RGBAF res = hueToRGB(h);
		res *= (l * 2);
		RGBAF gray(RGBF((res.x + res.y + res.z) / 3), 1);
		res = Math::lerp(gray, res, RGBAF(s));
		res.w = a;
		return res.clamped(0, 1);
	}

	/// @brief Creates an RGB color from a set of OPC (Opponent Process Channel) values.
	/// @param rg Red-Green difference.
	/// @param by Blue-Yellow difference.
	/// @param l Luminosity.
	/// @param a Alpha channel. By default, it is `1.0`.
	/// @return Resulting color.
	constexpr RGBAF fromOPC(float rg, float by, float l, float a = 1) {
		RGBF rgb{(1 - rg) + (by / 2), rg + (by / 2), 1 - by};
		rgb *= l;
		return RGBAF(rgb, a);
	}

	/// @brief Creates a color from an RGBA hex code.
	/// @param code Hex code.
	/// @return Resulting color.
	constexpr RGBAF fromRGBAHex(uint32 code) {
		uint8
			r = (code >> 24)	& 0xFF,
			g = (code >> 16)	& 0xFF,
			b = (code >> 8)		& 0xFF,
			a = (code)			& 0xFF
		;
		return RGBAF(r, g, b, a) / 255;
	}

	/// @brief Creates a color from an RGB hex code.
	/// @param code Hex code.
	/// @return Resulting color.
	constexpr RGBF fromRGBHex(uint32 code) {
		uint8
			r = (code >> 16)	& 0xFF,
			g = (code >> 8)		& 0xFF,
			b =	(code)			& 0xFF
		;
		return RGBF(r, g, b, 255) / 255;
	}

	/// @brief Creates a color from a hex code string.
	/// @param code Hex code string.
	/// @return Resulting color.
	constexpr Nullable<RGBAF> fromHexString(String const& code) {
		code = Makai::Regex::replace(code, "(#|0x)", "");
		if (code.empty())
			return null;
		if (code.size() < 3 || code.size() > 8 || !code.isHex())
			return null;
		String nc;
		if (code.size() <= 4) {
			nc.reserve(code.size() * 2);
			for (auto& c: code)
				nc.pushBack(c);
		}
		if (nc.size() == 6)
			return fromRGBHex(toUInt32(nc, 16));
		return fromRGBAHex(toUInt32(nc, 16));
	}

	/// @brief Creates a color from a dynamic value.
	/// @param v dynamic value.
	/// @return Resulting color.
	constexpr Nullable<RGBAF> fromDynamicValue(Data::Value const& v) {
		try {
			if (v.isArray())
				return RGBAF(
					v.fetch<float>(0, 0),
					v.fetch<float>(1, 0),
					v.fetch<float>(2, 0),
					v.fetch<float>(3, 1)
				);
			else if (v.isString())
				return fromHexString(v.getString());
			else if (v.isNumber())
				return v.getReal();
			else if (v.isVector())
				return v.getVector();
		} catch (...) {}
		return null;
	}

	/// @brief Converts an RGBA hex code to an RGB one.
	/// @param color Code to convert.
	/// @return Resulting code.
	constexpr uint32 toRGBHex(uint32 const color) {
		return color >> 8;
	}

	/// @brief Converts an RGB hex code to an RGBA one.
	/// @param color Code to convert.
	/// @return Resulting code.
	constexpr uint32 toRGBAHex(uint32 const color) {
		return (color << 8) | 0xFF;
	}

	/// @brief Converts a color to an RGBA hex code.
	/// @param color Color to convert.
	/// @return Resulting code.
	constexpr uint32 toRGBAHex(RGBAF const& color) {
		uint8
			r = CTL::Math::clamp(color.x, 0.0f, 1.0f) * 255,
			g = CTL::Math::clamp(color.y, 0.0f, 1.0f) * 255,
			b = CTL::Math::clamp(color.z, 0.0f, 1.0f) * 255,
			a = CTL::Math::clamp(color.w, 0.0f, 1.0f) * 255
		;
		uint32 code =
			(((uint32)r) << 24)
		|	(((uint32)g) << 16)
		|	(((uint32)b) << 8)
		|	((uint32)a)
		;
		return code;
	}

	/// @brief Converts a color to an RGB hex code.
	/// @param color Color to convert.
	/// @return Resulting code.
	constexpr uint32 toRGBHex(RGBF const& color) {
		return toRGBAHex(color) >> 8;
	}

	/// @brief Converts a color to a hex code string.
	/// @param color Color to convert.
	/// @param toRGB Whether to exclude the alpha channel. By default, it is `false`.
	/// @param webColor Whether `#` should be used as the identifier, instead of `0x`. By default, it is `true`.
	/// @return Resulting hex code string.
	constexpr String toHexString(
		RGBAF	const& color,
		bool	const& toRGB	= false,
		bool	const& webColor	= true
	) {
		String code;
		code += (webColor ? "#" : "0x");
		uint32 hci = (toRGB ? toRGBHex(color) : toRGBAHex(color));
		const uint8 hclen{toRGB ? uint8(6u) : uint8(8u)};
		uint8 nib{};
		for (usize i = 0; i < hclen; ++i) {
			nib = hci >> ((hclen - 1) - i) * 4;
			code += (char)(
				((nib & 0xF) < 0xA)
			?	((nib & 0xF) + 0x30)
			:	((nib & 0xF) - 0xA + 0x41)
			);
		}
		return code;
	}

	/// @brief No color.
	constexpr RGBAF NONE	= RGBAF(0, 0, 0, 0);
	// Transparency
	/// @brief Fully transparent.
	constexpr RGBAF CLEAR			= alpha(.0);
	/// @brief 12.5% opaque.
	constexpr RGBAF SEMIMISTY		= alpha(.125);
	/// @brief 25% opaque.
	constexpr RGBAF MISTY			= alpha(.25);
	/// @brief 37.5% opaque.
	constexpr RGBAF SEMILUCENT		= alpha(.375);
	/// @brief 50% opaque.
	constexpr RGBAF LUCENT			= alpha(.50);
	/// @brief 62.5% opaque.
	constexpr RGBAF SEMIMILKY		= alpha(.625);
	/// @brief 75% opaque.
	constexpr RGBAF MILKY			= alpha(.75);
	/// @brief 87.5% opaque.
	constexpr RGBAF SEMISOLID		= alpha(.875);
	/// @brief Fully opaque.
	constexpr RGBAF SOLID			= alpha(1);
	// Luminance
	/// @brief White.
	constexpr RGBAF WHITE			= fromLuma(1, 1);
	/// @brief Light gray.
	constexpr RGBAF LIGHTGRAY		= fromLuma(.75, 1);
	/// @brief Gray.
	constexpr RGBAF GRAY			= fromLuma(.5, 1);
	/// @brief Dark gray.
	constexpr RGBAF DARKGRAY		= fromLuma(.25, 1);
	/// @brief Black.
	constexpr RGBAF BLACK			= fromLuma(0, 1);
	// Primary Colors
	/// @brief Red.
	constexpr RGBAF RED				= RGBAF(1,	0,	0,	1);
	/// @brief Green.
	constexpr RGBAF GREEN			= RGBAF(0,	1,	0,	1);
	/// @brief Blue.
	constexpr RGBAF BLUE			= RGBAF(0,	0,	1,	1);
	// Secondary Colors
	/// @brief Yellow.
	constexpr RGBAF YELLOW		= RGBAF(1,	1,	0,	1);
	/// @brief Magenta.
	constexpr RGBAF MAGENTA		= RGBAF(1,	0,	1,	1);
	/// @brief Cyan.
	constexpr RGBAF CYAN		= RGBAF(0,	1,	1,	1);
	// Tertiary Colors
	/// @brief Orange.
	constexpr RGBAF ORANGE		= RGBAF(1,	.5,	0,	1);
	/// @brief Azure.
	constexpr RGBAF AZURE		= RGBAF(0,	.5,	1,	1);
	/// @brief Teal.
	constexpr RGBAF TEAL		= RGBAF(0,	1,	.5,	1);
	/// @brief Lime.
	constexpr RGBAF LIME		= RGBAF(.5,	1,	0,	1);
	/// @brief Purple.
	constexpr RGBAF PURPLE		= RGBAF(.5,	0,	1,	1);
	/// @brief Pink.
	constexpr RGBAF PINK		= RGBAF(1,	0,	.5,	1);

	/// @brief Six-color rainbow.
	constexpr RGBAF rainbow6[] = {
		RED,
		YELLOW,
		GREEN,
		CYAN,
		BLUE,
		MAGENTA
	};

	/// @brief Eight-color rainbow.
	constexpr RGBAF rainbow8[] = {
		RED,
		ORANGE,
		YELLOW,
		LIME,
		GREEN,
		CYAN,
		BLUE,
		PURPLE,
		MAGENTA
	};

	/// @brief 13-color rainbow.
	constexpr RGBAF rainbow[] = {
		RED,
		ORANGE,
		YELLOW,
		LIME,
		GREEN,
		TEAL,
		CYAN,
		AZURE,
		BLUE,
		PURPLE,
		MAGENTA,
		PINK
	};
}

CTL_EX_NAMESPACE_END

#endif
