#ifndef MAKAILIB_GRAPH_COLOR_H
#define MAKAILIB_GRAPH_COLOR_H

#include "../../compat/ctl.hpp"
#include "../../file/json.hpp"

/// @brief Color space facilities.
namespace Makai::Graph::Color {
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
	constexpr Vector4 hueToPastel(float hue) {
		hue *= PI;
		Vector3 res(
			cos(hue),
			cos(hue + TAU * (1.0/3.0)),
			cos(hue + TAU * (2.0/3.0))
		);
		res = (res^2.0).normalized() * SQRT2;
		return Vector4(res.clamped(0, 1), 1);
	}

	/// @brief Converts a linear hue to an RGB color.
	/// @param h Hue to convert.
	/// @return RGB color.
	constexpr Vector4 hueToRGB(float h) {
		h -= floor(h);
		return Vector4(
			Partial::hueify(h + 1.0/3.0),
			Partial::hueify(h),
			Partial::hueify(h - 1.0/3.0),
			1
		);
	}

	/// @brief Creates a white color with a given alpha.
	/// @param a Alpha channel.
	/// @return Resulting color.
	constexpr Vector4 alpha(float a) {
		return Vector4(1, 1, 1, Math::clamp(a, -0.1f, 1.0f));
	}

	/// @brief Creates a color from a set of RGBA values.
	/// @param r Red channel.
	/// @param g Green channel.
	/// @param b Blue channel.
	/// @param a Alpha channel. By default, it is `1.0`.
	/// @return Resulting color.
	constexpr Vector4 fromRGBA(float r, float g, float b, float a = 1) {
		return Vector4(r, g, b, a);
	}
	
	/// @brief Creates a color from a set of 8-bit RGBA values.
	/// @param r Red channel.
	/// @param g Green channel.
	/// @param b Blue channel.
	/// @param a Alpha channel. By default, it is `255`.
	/// @return Resulting color.
	constexpr Vector4 fromRGBA255(uint8 r, uint8 g, uint8 b, uint8 a = 255) {
		return Vector4(r, g, b, a) / 255;
	}

	/// @brief Creates a gray tone from a given intensity, with a given alpha.
	/// @param gray Gray tone intensity.
	/// @param a Alpha channel. By default, it is `1.0`.
	/// @return Resulting color.
	constexpr Vector4 fromGrayscale(float gray, float a = 1) {
		return Vector4(gray, gray, gray, a);
	}

	/// @brief Creates a gray tone from a given 8-bit intensity, with a given 8-bit alpha.
	/// @param gray Gray tone intensity.
	/// @param a Alpha channel. By default, it is `255`.
	/// @return Resulting color.
	constexpr Vector4 fromGrayscale255(uint8 gray, uint8 a = 255) {
		return fromGrayscale(gray, a) / 255;
	}

	/// @brief Creates an RGB color from a set of HSL values.
	/// @param h Hue.
	/// @param s Saturation.
	/// @param l Luminosity.
	/// @param a Alpha channel. By default, it is `1.0`.
	/// @return Resulting color.
	constexpr Vector4 fromHSL(float h, float s, float l, float a = 1) {
		Vector4 res = hueToRGB(h);
		res *= (l * 2);
		Vector4 gray(Vector3((res.x + res.y + res.z) / 3), 1);
		res = Math::lerp(gray, res, Vector4(s));
		res.w = a;
		return res.clamped(0, 1);
	}

	/// @brief Creates a color from an RGBA hex code.
	/// @param code Hex code.
	/// @return Resulting color.
	constexpr Vector4 fromHexCodeRGBA(uint32 code) {
		uint8
			r = (code >> 24)	& 0xFF,
			g = (code >> 16)	& 0xFF,
			b = (code >> 8)		& 0xFF,
			a = (code)			& 0xFF
		;
		return Vector4(r, g, b, a) / 255;
	}

	/// @brief Creates a color from an RGB hex code.
	/// @param code Hex code.
	/// @return Resulting color.
	constexpr Vector4 fromHexCodeRGB(uint32 code) {
		uint8
			r = (code >> 16)	& 0xFF,
			g = (code >> 8)		& 0xFF,
			b =	(code)			& 0xFF
		;
		return Vector4(r, g, b, 255) / 255;
	}

	/// @brief Creates a color from a hex code string.
	/// @param code Hex code string.
	/// @return Resulting color.
	Vector4 fromHexCodeString(String code);

	/// @brief Creates a color from a JSON value.
	/// @param json JSON value.
	/// @return Resulting color.
	Vector4 fromJSON(JSON::JSONData const& json);

	/// @brief Converts an RGBA hex code to an RGB one.
	/// @param color Code to convert.
	/// @return Resulting code.
	constexpr uint32 toHexCodeRGB(uint32 const color) {
		return color >> 8;
	}

	/// @brief Converts an RGB hex code to an RGBA one.
	/// @param color Code to convert.
	/// @return Resulting code.
	constexpr uint32 toHexCodeRGBA(uint32 const color) {
		return (color << 8) | 0xFF;
	}

	/// @brief Converts a color to an RGBA hex code.
	/// @param color Color to convert.
	/// @return Resulting code.
	constexpr uint32 toHexCodeRGBA(Vector4 const& color) {
		uint8
			r = Math::clamp(color.x, 0.0f, 1.0f) * 255,
			g = Math::clamp(color.y, 0.0f, 1.0f) * 255,
			b = Math::clamp(color.z, 0.0f, 1.0f) * 255,
			a = Math::clamp(color.w, 0.0f, 1.0f) * 255
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
	constexpr uint32 toHexCodeRGB(Vector4 const& color) {
		return toHexCodeRGBA(color) >> 8;
	}

	/// @brief Converts a color to a hex code string.
	/// @param color Color to convert.
	/// @param toRGB Whether to exclude the alpha channel. By default, it is `false`.
	/// @param webColor Whether `#` should be used as the identifier, instead of `0x`. By default, it is `true`.
	/// @return Resulting hex code string.
	constexpr String toHexCodeString(
		Vector4	const& color,
		bool	const& toRGB	= false,
		bool	const& webColor	= true
	) {
		String code;
		code += (webColor ? "#" : "0x");
		uint32 hci = (toRGB ? toHexCodeRGB(color) : toHexCodeRGBA(color));
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
	constexpr Vector4 NONE			= Vector4(0);
	// Transparency
	/// @brief Fully transparent.
	constexpr Vector4 CLEAR			= alpha(.0);
	/// @brief 12.5% opaque.
	constexpr Vector4 SEMIMISTY		= alpha(.125);
	/// @brief 25% opaque.
	constexpr Vector4 MISTY			= alpha(.25);
	/// @brief 37.5% opaque.
	constexpr Vector4 SEMILUCENT	= alpha(.375);
	/// @brief 50% opaque.
	constexpr Vector4 LUCENT		= alpha(.50);
	/// @brief 62.5% opaque.
	constexpr Vector4 SEMIMILKY		= alpha(.625);
	/// @brief 75% opaque.
	constexpr Vector4 MILKY			= alpha(.75);
	/// @brief 87.5% opaque.
	constexpr Vector4 SEMISOLID		= alpha(.875);
	/// @brief Fully opaque.
	constexpr Vector4 SOLID			= alpha(1);
	// Luminance
	/// @brief White.
	constexpr Vector4 WHITE			= fromGrayscale(1, 1);
	/// @brief Light gray.
	constexpr Vector4 LIGHTGRAY		= fromGrayscale(.75, 1);
	/// @brief Gray.
	constexpr Vector4 GRAY			= fromGrayscale(.5, 1);
	/// @brief Dark gray.
	constexpr Vector4 DARKGRAY		= fromGrayscale(.25, 1);
	/// @brief Black.
	constexpr Vector4 BLACK			= fromGrayscale(0, 1);
	// Primary Colors
	/// @brief Red.
	constexpr Vector4 RED			= Vector4(1,	0,	0,	1);
	/// @brief Green.
	constexpr Vector4 GREEN			= Vector4(0,	1,	0,	1);
	/// @brief Blue.
	constexpr Vector4 BLUE			= Vector4(0,	0,	1,	1);
	// Secondary Colors
	/// @brief Yellow.
	constexpr Vector4 YELLOW		= Vector4(1,	1,	0,	1);
	/// @brief Magenta.
	constexpr Vector4 MAGENTA		= Vector4(1,	0,	1,	1);
	/// @brief Cyan.
	constexpr Vector4 CYAN			= Vector4(0,	1,	1,	1);
	// Tertiary Colors
	/// @brief Orange.
	constexpr Vector4 ORANGE		= Vector4(1,	.5,	0,	1);
	/// @brief Azure.
	constexpr Vector4 AZURE			= Vector4(0,	.5,	1,	1);
	/// @brief Teal.
	constexpr Vector4 TEAL			= Vector4(0,	1,	.5,	1);
	/// @brief Lime.
	constexpr Vector4 LIME			= Vector4(.5,	1,	0,	1);
	/// @brief Purple.
	constexpr Vector4 PURPLE		= Vector4(.5,	0,	1,	1);
	/// @brief Pink.
	constexpr Vector4 PINK			= Vector4(1,	0,	.5,	1);

	/// @brief Six-color rainbow.
	constexpr Vector4 rainbow6[] = {
		RED,
		YELLOW,
		GREEN,
		CYAN,
		BLUE,
		MAGENTA
	};

	/// @brief Eight-color rainbow.
	constexpr Vector4 rainbow8[] = {
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
	constexpr Vector4 rainbow[] = {
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

#endif // MAKAILIB_GRAPH_COLOR_H
