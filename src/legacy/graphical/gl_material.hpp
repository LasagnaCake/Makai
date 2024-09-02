namespace Module {
	struct Effect {
		bool enabled = false;
	};

	struct Limitable {
		float
			start	= 0.0,
			stop	= 10.0;
	};

	struct Variable {
		float	strength = 1;
	};

	struct Variable2D {
		Vector2	strength = 1;
	};

	struct Variable3D {
		Vector3	strength = 1;
	};

	struct Variable4D {
		Vector4	strength = 1;
	};

	struct ColorableRGBA {
		Vector4	color = Color::WHITE;
	};

	struct ColorableRGB {
		Vector3	color = 1;
	};

	struct Imageable2D {
		Texture2D image = nullptr;
	};

	struct Channelable {
		int channel = -1;
	};

	struct Transformable2D {
		Transform2D trans;
	};

	struct Positionable2D {
		Vector2 position;
	};

	struct Invertible {
		bool invert = false;
	};

	struct Tuneable {
		float
			frequency	= 0,
			amplitude	= 0,
			shift		= 0;
	};

	struct Tuneable2D {
		Vector2
			frequency	= Vector2(0),
			amplitude	= Vector2(0),
			shift		= Vector2(0);
	};

	struct Sizeable {
		float size = 0;
	};

	struct Sizeable2D {
		Vector2 size = 0;
	};

	struct Countable {
		size_t count = 1;
	};
}

namespace {
	using namespace Module;
	using Shader::Shader;
}

// Generic Material Effects

enum class BlendFunction: unsigned int {
	BF_ZERO = 0,
	BF_ONE,
	BF_SRC,
	BF_ONE_MINUS_SRC,
	BF_DST,
	BF_ONE_MINUS_DST,
};

enum class BlendEquation: unsigned int {
	BE_ADD,
	BE_SUBTRACT,
	BE_MULTIPLY,
	BE_DIVIDE,
	BE_REVERSE_SUBTRACT,
	BE_REVERSE_DIVIDE,
	BE_ADD_CLAMP,
	BE_SUBTRACT_CLAMP,
	BE_MULTIPLY_CLAMP,
	BE_DIVIDE_CLAMP,
	BE_REVERSE_SUBTRACT_CLAMP,
	BE_REVERSE_DIVIDE_CLAMP,
	BE_MAX,
	BE_MIN,
};

enum class BlendSource: unsigned int {
	BS_ZERO,
	BS_ONE,
	BS_COLOR,
	BS_ALPHA,
	BS_NOISE,
	BS_SOLID_COLOR,
	BS_SOLID_ALPHA,
};

struct BlendMode {
	BlendFunction source		= BlendFunction::BF_SRC;
	BlendFunction destination	= BlendFunction::BF_DST;
	BlendEquation equation		= BlendEquation::BE_MULTIPLY;
};

struct BlendSetting {
	BlendMode	color	= {BlendFunction::BF_ONE, BlendFunction::BF_DST, BlendEquation::BE_MULTIPLY};
	BlendMode	alpha	= {BlendFunction::BF_SRC, BlendFunction::BF_ONE, BlendEquation::BE_MULTIPLY};
};

struct GradientEffect: Effect, Channelable, Invertible {
	Vector4
		begin	= Color::BLACK,
		end		= Color::WHITE;
};

struct NegativeEffect: Effect, Variable {};

struct ImageEffect: Effect, Imageable2D {};

struct WarpEffect: ImageEffect, Transformable2D {
	unsigned int
		channelX = 0,
		channelY = 1;
};

// Object Material Effects

struct TextureEffect: ImageEffect {
	float alphaClip = 0.1;
};

struct EmissionEffect: ImageEffect, Variable {
};

struct NormalMapEffect: ImageEffect, Variable {
};

// Buffer Material Effects

struct MaskEffect: Effect, Imageable2D, Transformable2D, Invertible {
	Vector4	albedo		= 1;
	Vector4	accent		= 0;
	bool	relative	= false;
};

enum WaveShape: unsigned int {
	WS_SQUARE = 0,
	WS_SINE,
	WS_BIN_SINE,
	WS_ABS_SINE,
	WS_ABS_BIN_SINE,
	WS_TRIANGLE,
	WS_BIN_TRIANGLE,
	WS_ABS_TRIANGLE,
	WS_ABS_BIN_TRIANGLE,
	WS_HALF_SINE,
	WS_BIN_HALF_SINE,
	WS_ABS_HALF_SINE,
	WS_ABS_BIN_HALF_SINE,
	WS_HALF_TRIANGLE,
	WS_BIN_HALF_TRIANGLE,
	WS_ABS_HALF_TRIANGLE,
	WS_ABS_BIN_HALF_TRIANGLE,
	WS_SIMPLE_NOISE,
	WS_BIN_SIMPLE_NOISE,
};

struct WaveEffect: Effect, Tuneable2D {
	WaveShape	shape	= WS_SQUARE;
};

struct RainbowEffect: Effect, Variable {
	Vector2 frequency		= 0.0;
	Vector2 shift			= 0.0;
	bool	absoluteColor	= false;
	bool	polar			= false;
	float	polarShift		= 0.0;
};

struct BlurEffect: Effect, Variable2D {};

struct OutlineEffect: Effect, Sizeable2D, ColorableRGBA {
	bool relativeAlpha = true;
};

struct PolarWarpEffect: Effect, Sizeable, Positionable2D, Variable2D, ColorableRGBA {
	float	tintStrength	= 1;
	bool	fishEye			= true;
};

enum class NoiseType: unsigned int {
	NT_NOISE_SIMPLE = 0,
	NT_NOISE_GOLD,
	NT_NOISE_SUPER
};

/// SRC = Pixel Color, DST = Noise
struct NoiseBlendMode: BlendSetting {};

struct NoiseEffect: Effect, Variable, Transformable2D {
	float			seed	= 1;
	NoiseType		type	= NoiseType::NT_NOISE_SUPER;
	NoiseBlendMode	blend;
};

// World Material Effects

struct FogEffect: Effect, Limitable, ColorableRGBA, Variable {};

struct AmbientEffect: ColorableRGB, Variable {};

// Extra Data

enum class BufferDebugView: unsigned int {
	BDV_NONE = 0,
	BDV_DEPTH,
};

enum class ObjectDebugView: unsigned int {
	ODV_NONE = 0,
	ODV_NORMAL,
};

// Materials

struct BaseObjectMaterial {
	Vector4	color = Color::WHITE;
};

struct ObjectMaterial: BaseObjectMaterial {
	bool shaded			= false;
	bool illuminated	= false;
	float			hue			= 0;
	float			saturation	= 1;
	float			luminosity	= 1;
	float			brightness	= 0;
	float			contrast	= 1;
	Vector2			uvShift;
	TextureEffect	texture;
	NormalMapEffect	normalMap;
	EmissionEffect	emission;
	WarpEffect		warp;
	NegativeEffect	negative;
	GradientEffect	gradient;
	List<Vector3>	instances = {Vec3(0, 0, 0)};
	GLuint culling		= GL_FRONT_AND_BACK;
	GLuint fill			= GL_FILL;
	ObjectDebugView	debug	= ObjectDebugView::ODV_NONE;
};

struct BaseBufferMaterial {
	Vector4 background = Color::NONE;
};

struct BufferMaterial: BaseBufferMaterial {
	Vector4
		color	= Color::WHITE,
		accent	= Color::NONE;
	float			hue			= 0;
	float			saturation	= 1;
	float			luminosity	= 1;
	float			brightness	= 0;
	float			contrast	= 1;
	Vector2			uvShift;
	MaskEffect		mask;
	WarpEffect		warp;
	NegativeEffect	negative;
	BlurEffect		blur;
	OutlineEffect	outline;
	WaveEffect		wave;
	WaveEffect		prism;
	PolarWarpEffect	polarWarp;
	GradientEffect	gradient;
	RainbowEffect	rainbow;
	NoiseEffect		noise;
	BufferDebugView	debug	= BufferDebugView::BDV_NONE;
};

struct BaseWorldMaterial {};

struct WorldMaterial: BaseWorldMaterial {
	FogEffect		nearFog;
	FogEffect		farFog;
	AmbientEffect	ambient;
};

void setMaterial(Shader& shader, ObjectMaterial& material) {
	// UV Data
	shader["uvShift"](material.uvShift);
	// Texture
	if (material.texture.image && material.texture.enabled && material.texture.image->exists()) {
		shader["imgTexture.enabled"](true, 0, material.texture.alphaClip);
		material.texture.image->enable(0);
	} else shader["imgTexture.enabled"](false);
	// Emission Texture
	if (material.emission.image && material.emission.enabled && material.emission.image->exists()) {
		shader["emission.enabled"](true, 1, material.emission.strength);
		material.emission.image->enable(1);
	} else shader["emission.enabled"](false);
	// Normal Map Texture
	if (material.normalMap.image && material.normalMap.enabled && material.normalMap.image->exists()) {
		shader["normalMap.enabled"](true, 2, material.normalMap.strength);
		material.normalMap.image->enable(2);
	} else shader["normalMap.enabled"](false);
	// Texture warping
	if (material.warp.image && material.warp.enabled && material.warp.image->exists()) {
		shader["warp.enabled"](true, 8, material.warp.channelX, material.warp.channelY);
		material.warp.image->enable(8);
		shader["warpTrans.position"](
			material.warp.trans.position,
			material.warp.trans.rotation,
			material.warp.trans.scale
		);
	} else shader["warp.enabled"](false);
	// Color inversion
	if (material.negative.enabled) {
		shader["negative.enabled"](true, material.negative.strength);
	} else shader["negative.enabled"](false);
	// Color to gradient
	if (material.gradient.enabled) {
		shader["gradient.enabled"](
			true,
			material.gradient.channel,
			material.gradient.begin,
			material.gradient.end,
			material.gradient.invert
		);
	} else shader["gradient.enabled"](false);
	// Lighted
	shader["shade.enabled"](material.shaded);
	shader["lights.enabled"](material.illuminated);
	// Albedo
	shader["albedo"](material.color);
	// HSLBC data
	shader["hue"](material.hue);
	shader["saturation"](material.saturation);
	shader["luminosity"](material.luminosity);
	shader["brightness"](material.brightness);
	shader["contrast"](material.contrast);
	// Instance data
	shader["instances"](material.instances);
	// Debug data
	shader["debugView"]((unsigned int)material.debug);
}

// TODO: The rest of this rat's nest
void setMaterial(Shader& shader, BufferMaterial& material) {
	// Set UV data
	shader["uvShift"](material.uvShift);
	// Set color data
	shader["albedo"](material.color);
	shader["accent"](material.accent);
	// Set mask data
	if (material.mask.enabled && material.mask.image && material.mask.image->exists()) {
		shader["useMask"](true);
		shader["mask"](9);
		material.mask.image->enable(9);
		shader["invertMask"](material.mask.invert);
		shader["relativeMask"](material.mask.relative);
		shader["maskShift"](material.mask.trans.position);
		shader["maskRotate"](material.mask.trans.rotation);
		shader["maskScale"](material.mask.trans.scale);
		shader["maskAlbedo"](material.mask.albedo);
		shader["maskAccent"](material.mask.accent);
	} else shader["useMask"](false);
	// Set texture warping data
	if (material.warp.image && material.warp.enabled && material.warp.image->exists()) {
		shader["useWarp"](true);
		shader["warpTexture"](8);
		material.warp.image->enable(8);
		shader["warpChannelX"](material.warp.channelX);
		shader["warpChannelY"](material.warp.channelY);
	} else shader["useWarp"](false);
	// Set color to gradient data
	if (material.gradient.enabled){
		shader["useGradient"](true);
		shader["gradientChannel"](material.gradient.channel);
		shader["gradientStart"](material.gradient.begin);
		shader["gradientEnd"](material.gradient.end);
		shader["gradientInvert"](material.gradient.invert);
	} else shader["useGradient"](false);
	// set screen wave data
	if (material.wave.enabled) {
		shader["useWave"](true);
		shader["waveAmplitude"](material.wave.amplitude);
		shader["waveFrequency"](material.wave.frequency);
		shader["waveShift"](material.wave.shift);
		shader["waveShape"](material.wave.shape);
	} else shader["useWave"](false);
	// set screen prism data
	if (material.prism.enabled) {
		shader["usePrism"](true);
		shader["prismAmplitude"](material.prism.amplitude);
		shader["prismFrequency"](material.prism.frequency);
		shader["prismShift"](material.prism.shift);
		shader["prismShape"](material.prism.shape);
	} else shader["usePrism"](false);
	// Set color inversion
	if (material.negative.enabled) {
		shader["useNegative"](true);
		shader["negativeStrength"](material.negative.strength);
	} else shader["useNegative"](false);
	// Set rainbow data
	if (material.rainbow.enabled) {
		shader["useRainbow"](true);
		shader["rainbowFrequency"](material.rainbow.frequency);
		shader["rainbowShift"](material.rainbow.shift);
		shader["rainbowStrength"](material.rainbow.strength);
		shader["rainbowAbsolute"](material.rainbow.absoluteColor);
		shader["rainbowPolar"](material.rainbow.polar);
		shader["rainbowPolarShift"](material.rainbow.polarShift);
	} else shader["useRainbow"](false);
	// Set blur data
	if (material.blur.enabled) {
		shader["useBlur"](true);
		shader["blurStrength"](material.blur.strength);
	} else shader["useBlur"](false);
	// Set polar warp data
	if (material.polarWarp.enabled) {
		shader["usePolarWarp"](true);
		shader["polarWarpStrength"](material.polarWarp.strength);
		shader["polarWarpSize"](material.polarWarp.size);
		shader["polarWarpPosition"](material.polarWarp.position);
		shader["polarWarpColor"](material.polarWarp.color);
		shader["polarWarpTintStrength"](material.polarWarp.tintStrength);
		shader["polarWarpFishEye"](material.polarWarp.fishEye);
	} else shader["usePolarWarp"](false);
	// Set outline data
	if (material.outline.enabled) {
		shader["useOutline"](true);
		shader["outlineSize"](material.outline.size);
		shader["outlineColor"](material.outline.color);
		shader["outlineMatchAlpha"](material.outline.relativeAlpha);
	} else shader["useOutline"](false);
	// Set noise data
	if (material.noise.enabled) {
		shader["useNoise"](true);
		shader["noiseOffset"](material.noise.trans.position);
		//shader["noiseRotation"](material.noise.trans.rotation);
		shader["noiseStrength"](material.noise.trans.scale);
		shader["noiseScale"](material.noise.strength);
		shader["noiseSeed"](material.noise.seed);
		shader["noiseType"]((unsigned int)material.noise.type);
		shader["noiseBlendSrcColorFunc"]((unsigned int)material.noise.blend.color.source);
		shader["noiseBlendDstColorFunc"]((unsigned int)material.noise.blend.color.destination);
		shader["noiseBlendColorEq"]((unsigned int)material.noise.blend.color.equation);
		shader["noiseBlendSrcAlphaFunc"]((unsigned int)material.noise.blend.alpha.source);
		shader["noiseBlendDstAlphaFunc"]((unsigned int)material.noise.blend.alpha.destination);
		shader["noiseBlendAlphaEq"]((unsigned int)material.noise.blend.alpha.equation);
	} else shader["useNoise"](false);
	// Set debug data
	shader["debugView"]((unsigned int)material.debug);
	// Set HSLBC data
	shader["hue"](material.hue);
	shader["saturation"](material.saturation);
	shader["luminosity"](material.luminosity);
	shader["brightness"](material.brightness);
	shader["contrast"](material.contrast);
}

void setMaterial(Shader& shader, WorldMaterial& material) {
	// Fog
	if (material.farFog.enabled) {
		shader["farFog.enabled"](
			true,
			material.farFog.start,
			material.farFog.stop,
			material.farFog.strength
		);
		shader["farFog.color"](material.farFog.color);
	} else shader["farFog.enabled"](false);
	// Void
	if (material.nearFog.enabled) {
		shader["nearFog.enabled"](
			true,
			material.nearFog.start,
			material.nearFog.stop,
			material.nearFog.strength
		);
		shader["nearFog.color"](material.nearFog.color);
	} else shader["nearFog.enabled"](false);
	// Ambient light
	shader["ambient.color"](material.ambient.color, material.ambient.strength);
}

template<class T, class BASE>
concept ValidMaterial =
	Type::Subclass<T, BASE>
&&	requires (Shader& s, T& mat) {
		setMaterial(s, mat);
	}
;

template<class T> concept UsableObjectMaterial	= ValidMaterial<T, BaseObjectMaterial>;
template<class T> concept UsableBufferMaterial	= ValidMaterial<T, BaseBufferMaterial>;
template<class T> concept UsableWorldMaterial	= ValidMaterial<T, BaseWorldMaterial>;

JSONData saveImageEffect(ImageEffect& effect, String const& folder, String const& path) {
	JSONData def;
	def["enabled"] = effect.enabled;
	if (effect.image && effect.image->exists()) {
		effect.image->saveToFile(FileSystem::concatenatePath(folder, path));
		def["image"] = {
			{"path", path},
			{"minFilter", effect.image->getTextureMinFilter()},
			{"magFilter", effect.image->getTextureMagFilter()}
		};
	} else def["enabled"] = false;
	return def;
}

ImageEffect loadImageEffect(
	JSONData& effect,
	String const& sourcepath,
	Texture2D& texture
) {
	try {
		ImageEffect fx;
		fx.enabled = effect["enabled"].get<bool>();
		fx.image = texture = Texture2D::fromJSON(effect["image"], sourcepath);
		return fx;
	} catch (JSON::exception const& e) {
		throw Error::FailedAction(
			"Failed at getting image effect!",
			__FILE__,
			toString(__LINE__),
			"loadImageEffect",
			e.what(),
			"Please check to see if values are correct!"
		);
	}
}

ObjectMaterial fromObjectMaterialDefinition(
	JSONData def,
	String const& definitionFolder,
	Texture2D& texture,
	Texture2D& normalMap,
	Texture2D& emission,
	Texture2D& warp
) {
	ObjectMaterial mat;
	try {
		auto& dmat = def;
		// Set color
		if(dmat["color"].is_array()) {
			mat.color = Drawer::colorFromJSON(dmat["color"]);
		}
		else if (dmat["color"].is_string())
			mat.color = Color::fromHexCodeString(dmat["color"].get<String>());
		// Set color & shading params
		#define _SET_BOOL_PARAM(PARAM) if(dmat[#PARAM].is_boolean()) mat.PARAM = dmat[#PARAM].get<bool>()
		_SET_BOOL_PARAM(shaded);
		_SET_BOOL_PARAM(illuminated);
		#undef _SET_BOOL_PARAM
		#define _SET_FLOAT_PARAM(PARAM) if(dmat[#PARAM].is_number()) mat.PARAM = dmat[#PARAM].get<float>()
		_SET_FLOAT_PARAM(hue);
		_SET_FLOAT_PARAM(saturation);
		_SET_FLOAT_PARAM(luminosity);
		_SET_FLOAT_PARAM(brightness);
		_SET_FLOAT_PARAM(contrast);
		#undef _SET_FLOAT_PARAM
		// Set UV shift
		if(dmat["uvShift"].is_array()) {
			mat.uvShift.x = dmat["uvShift"][0].get<float>();
			mat.uvShift.y = dmat["uvShift"][1].get<float>();
		}
		// Set texture
		if (dmat["texture"].is_object()) {
			auto fx = loadImageEffect(dmat["texture"], definitionFolder, texture);
			mat.texture.enabled	= fx.enabled;
			mat.texture.image	= fx.image;
			if (dmat["texture"]["alphaClip"].is_number())
				mat.texture.alphaClip	= dmat["texture"]["alphaClip"].get<float>();
		}
		// Set normal map texture
		if (dmat["normalMap"].is_object()) {
			auto fx = loadImageEffect(dmat["normalMap"], definitionFolder, normalMap);
			mat.normalMap.enabled	= fx.enabled;
			mat.normalMap.image		= fx.image;
			if (dmat["normalMap"]["strength"].is_number())
				mat.normalMap.strength	= dmat["normalMap"]["strength"].get<float>();
		}
		// Set emission texture
		if (dmat["emission"].is_object()) {
			auto fx = loadImageEffect(dmat["emission"], definitionFolder, emission);
			mat.emission.enabled	= fx.enabled;
			mat.emission.image		= fx.image;
			if (dmat["emission"]["strength"].is_number())
				mat.emission.strength	= dmat["emission"]["strength"].get<float>();
		}
		// Set warp texture
		if (dmat["warp"].is_object()) {
			auto fx = loadImageEffect(dmat["warp"], definitionFolder, warp);
			mat.warp.enabled	= fx.enabled;
			mat.warp.image		= fx.image;
			{
				auto& mwtrans = dmat["warp"]["trans"];
				mat.warp.trans.position = VecMath::fromJSONArrayV2(mwtrans["position"]);
				mat.warp.trans.rotation = mwtrans["rotation"].get<float>();
				mat.warp.trans.scale = VecMath::fromJSONArrayV2(mwtrans["scale"], 1);
			}
			mat.warp.channelX = dmat["warp"]["channelX"];
			mat.warp.channelY = dmat["warp"]["channelY"];
		}
		// Set negative
		if (dmat["negative"].is_object()) {
			mat.negative.enabled	= dmat["negative"]["enabled"].get<bool>();
			mat.negative.strength	= dmat["negative"]["strength"].get<float>();
		}
		// Set gradient
		if (dmat["gradient"].is_object()) {
			mat.gradient.enabled	= dmat["gradient"]["enabled"].get<bool>();
			mat.gradient.channel	= dmat["gradient"]["channel"].get<unsigned int>();
			auto& dgbegin	= dmat["gradient"]["begin"];
			auto& dgend		= dmat["gradient"]["end"];
			if (dgbegin.is_array())
				mat.gradient.begin = Drawer::colorFromJSON(dgbegin);
			else if (dgbegin.is_string())
				mat.gradient.begin = Color::fromHexCodeString(dgbegin.get<String>());
			if (dgend.is_array())
				mat.gradient.end = Drawer::colorFromJSON(dgend);
			else if (dgend.is_string())
				mat.gradient.end = Color::fromHexCodeString(dgend.get<String>());
			mat.gradient.invert	= dmat["gradient"]["invert"].get<bool>();
		}
		// Set instances
		if (dmat["instances"].is_array()) {
			mat.instances.clear();
			for(auto& inst: dmat["instances"])
				mat.instances.push_back(VecMath::fromJSONArrayV3(inst));
		}
		// Set culling, fill & view
		if (dmat["culling"].is_number()) {
			switch (dmat["culling"].get<unsigned int>()) {
				default:
				case 0: mat.culling = GL_FRONT_AND_BACK;	break;
				case 1: mat.culling = GL_FRONT;				break;
				case 2: mat.culling = GL_BACK;				break;
			}
		}
		if (dmat["fill"].is_number()) {
			switch (dmat["fill"].get<unsigned int>()) {
				default:
				case 0: mat.fill = GL_FILL;		break;
				case 1: mat.fill = GL_LINE;		break;
				case 2: mat.fill = GL_POINT;	break;
			}
		}
		if (dmat["debug"].is_number())
			mat.debug = (ObjectDebugView)dmat["debug"].get<unsigned int>();
	} catch (JSON::exception const& e) {
		throw Error::FailedAction(
			"Failed at getting material values!",
			__FILE__,
			toString(__LINE__),
			"extendFromDefinition",
			e.what(),
			"Please check to see if values are correct!"
		);
	}
	return mat;
}

JSONData getMaterialDefinition(
	ObjectMaterial& mat,
	String const& definitionFolder,
	String const& texturesFolder,
	bool integratedTextures = false
) {
	JSONData def;
	// Copy instances
	List<JSONData> instanceData;
	for (Vector3& inst: mat.instances) {
		instanceData.push_back({inst.x, inst.y, inst.z});
	}
	// Define object
	def = {
		{"color", Color::toHexCodeString(mat.color, false, true)},
		{"shaded", mat.shaded},
		{"illuminated", mat.illuminated},
		{"hue", mat.hue},
		{"saturation", mat.saturation},
		{"luminosity", mat.luminosity},
		{"brightness", mat.brightness},
		{"contrast", mat.contrast},
		{"uvShift", {mat.uvShift.x, mat.uvShift.y}},
		{"negative", {
			{"enabled", mat.negative.enabled},
			{"strength", mat.negative.strength}
		}},
		{"gradient", {
			{"enabled", mat.gradient.enabled},
			{"channel", mat.gradient.channel},
			{"begin", {mat.gradient.begin.x, mat.gradient.begin.y, mat.gradient.begin.z, mat.gradient.begin.w}},
			{"end", {mat.gradient.end.x, mat.gradient.end.y, mat.gradient.end.z, mat.gradient.end.w}},
			{"invert", mat.gradient.invert}
		}},
		{"instances", instanceData},
		{"debugView", (unsigned int)mat.debug}
	};
	switch (mat.fill) {
		case GL_FILL:	def["material"]["fill"] = 0; break;
		case GL_LINE:	def["material"]["fill"] = 1; break;
		case GL_POINT:	def["material"]["fill"] = 2; break;
	}
	switch (mat.culling) {
		case GL_FRONT_AND_BACK:	def["material"]["culling"] = 0; break;
		case GL_FRONT:			def["material"]["culling"] = 1; break;
		case GL_BACK:			def["material"]["culling"] = 2; break;
	}
	// Save image texture
	if (!integratedTextures) {
		def["warp"]		= saveImageEffect(mat.warp, definitionFolder, texturesFolder + "/warp.tga");
		def["texture"]	= saveImageEffect(mat.texture, definitionFolder, texturesFolder + "/texture.tga");
		def["emission"]	= saveImageEffect(mat.emission, definitionFolder, texturesFolder + "/emission.tga");
	} else {
		// TODO: integrated textures
	}
	// Set stuff
	def["texture"]["alphaClip"] = mat.texture.alphaClip;
	def["emission"]["strength"] = mat.emission.strength;
	def["warp"]["channelX"] = mat.warp.channelX;
	def["warp"]["channelY"] = mat.warp.channelY;
	def["warp"]["trans"] = {
		{"position",	{mat.warp.trans.position.x,	mat.warp.trans.position.y	}	},
		{"rotation",	mat.warp.trans.rotation										},
		{"scale",		{mat.warp.trans.scale.x,	mat.warp.trans.scale.y		}	}
	};
	// Return definition
	return def;
}