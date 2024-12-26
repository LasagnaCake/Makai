#include <sstream>

#include "../../ctl/ctl.hpp"
#include "color.hpp"

using namespace Makai; using namespace Makai::Graph;

using namespace Color;

Vector4 Color::fromHexCodeString(String code) {
	// Remove hex markers if necessary
	code = Regex::replace(code, "(#|0x)", "");
	// Check if color exists
	if (code.empty())
		throw Error::InvalidValue(
			"Color value does not exist!",
			"Make sure the color values are correct!",
			CTL_CPP_PRETTY_SOURCE
		);
	// Check if color value is valid
	if (code.size() < 3 || code.size() > 8 || !code.isHex())
		throw Error::InvalidValue(
			"Invalid color value \"#"+ code +"\"!",
			"Make sure the color values are correct!",
			CTL_CPP_PRETTY_SOURCE
		);
	// Bit fuckery for 4-bit color
	if (code.size() <= 4) {
		std::stringstream nc;
		nc
		<< code[0] << code[0]
		<< code[1] << code[1]
		<< code[2] << code[2]
		;
		if (code.size() == 4)
			nc << code[3] << code[3];
		code = nc.str();
	}
	// Convert to uint32
	if (code.size() == 6)
		return fromHexCodeRGB(toUInt32(code, 16));
	return fromHexCodeRGBA(toUInt32(code, 16));
}

Vector4 Color::fromJSON(JSON::JSONData const& v) {
	try {
		if (v.isArray())
			return Vector4(
				v[0].get<float>(0),
				v[1].get<float>(0),
				v[2].get<float>(0),
				v[3].get<float>(1)
			);
		else if (v.isString())
			return Color::fromHexCodeString(v.get<String>());
		else if (v.isNumber())
			return Vector4(v.get<float>(1));
	} catch (...) {}
	return Vector4(1);
}
