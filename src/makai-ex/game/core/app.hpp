#ifndef MAKAILIB_EX_GAME_CORE_APP_H
#define MAKAILIB_EX_GAME_CORE_APP_H

#include <makai/makai.hpp>

/// @brief Game extensions.
namespace Makai::Ex::Game {
	/// @brief Dual (2D & 3D) camera.
	struct DualCamera {
		/// @brief 3D camera.
		Graph::Camera3D cam2D;
		/// @brief 2D camera.
		Graph::Camera3D cam3D;

		/// @brief Enables one of the cameras.
		/// @param set2D Whether to enable the 2D camera. By default, it is `false`.
		void use(bool const set2D = false) {
			Graph::Global::camera = set2D ? cam2D : cam3D;
		}
	};

	/// @brief Base game application.
	struct App: Makai::App {
		using SoundType = Audio::Engine::SoundType;

		using Makai::App::App;

		/// @brief Audio track container.
		struct Tracks {
			/// @brief Music track.
			Audio::GroupInstance music;
			/// @brief Sound effects track.
			Audio::GroupInstance sfx;
		};

		/// @brief Audio tracks.
		Tracks const tracks;

		App(Config::App const& cfg):
			Makai::App(cfg),
			tracks{audio.createGroup(), audio.createGroup()} {}

		/// @brief Layer material map.
		using LayerMap = Map<usize, Graph::Material::BufferMaterial>;

		/// @brief Destructor.
		virtual ~App() {}

		/// @brief Framebuffer material.
		Graph::Material::BufferMaterial& frame = getFrameBuffer().material;
		/// @brief Layerbuffer material.
		Graph::Material::BufferMaterial& layer = getLayerBuffer().material;

		/// @brief Materials for each layer.
		LayerMap layers;

		/// @brief Global camera.
		DualCamera camera;

		/// @brief Gets called when the application begins rendering a layer, before the the layer buffer is cleared.
		/// @param layerID Layer currently being processed. 
		void onLayerDrawBegin(usize const layerID) override {
			layer = layers[layerID];
		}

		/// @brief Creates a sound in the SFX track.
		/// @param path Path to audio file.
		/// @param type Sound type. By default, it is `EST_PRELOADED`.
		/// @return Sound instance, or `nullptr` on failure.
		Audio::SoundInstance createSFX(String const& path, SoundType const type = SoundType::EST_PRELOADED) {
			return audio.createSound(path, type, tracks.sfx);
		}

		/// @brief Creates a sound in the music track.
		/// @param path Path to audio file.
		/// @param type Sound type. By default, it is `EST_STREAMED`.
		/// @return Sound instance, or `nullptr` on failure.
		Audio::SoundInstance createMusic(String const& path, SoundType const type = SoundType::EST_STREAMED) {
			return audio.createSound(path, type, tracks.music);
		}
	};
}

#endif