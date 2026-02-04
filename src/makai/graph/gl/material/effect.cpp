#include "effect.hpp"

using namespace Makai; using namespace Makai::Graph;

namespace JSON = Makai::JSON;

JSON::Value Material::saveImageEffect(Material::Effect::Image& effect, CTL::String const& folder, String const& path) {
	JSON::Value def;
	def.operator[]("enabled") = effect.enabled;
	if (effect.image && effect.image.exists()) {
		effect.image.saveToFile(OS::FS::concatenate(folder, path));
		def.operator[]("image") = JSON::Object{
			JSON::Entry{"path", path},
			JSON::Entry{"minFilter", (uint32)effect.image.minFilter()},
			JSON::Entry{"magFilter", (uint32)effect.image.magFilter()}
		};
	} else def.operator[]("enabled") = false;
	return def;
}

Material::Effect::Image Material::loadImageEffect(
	JSON::Value effect,
	CTL::String const& sourcepath,
	Texture2D& texture
) {
	try {
		using namespace CTL::Literals::Text;
		Material::Effect::Image fx;
		fx.enabled = effect.operator[]("enabled"s).get<bool>();
		fx.image = texture = Texture2D::fromJSON(effect.operator[]("image"s).get<String>(), sourcepath);
		return fx;
	} catch (std::exception const& e) {
		throw Error::FailedAction(
			"Failed at getting image effect!",
			e.what(),
			"Please check to see if values are correct!",
			CTL_CPP_PRETTY_SOURCE
		);
	}
}
