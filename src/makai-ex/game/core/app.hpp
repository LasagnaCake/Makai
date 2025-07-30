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

		/// @brief Audio track.
		struct Track {
			Audio::GroupInstance	instance;
			SoundType				soundType = SoundType::EST_PRELOADED;
		};

		/// @brief Audio tracks container.
		using Tracks = Map<usize, Track>;

		/// @brief Master track.
		Audio::GroupInstance const master;
		
		/// @brief Audio tracks database.
		Tracks tracks;

		/// @brief constructor. Same parameters as `Makai::App`.
		App(Config::App const& cfg):
			Makai::App(cfg), master(audio.createGroup()) {}

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

		/// @brief Returns a track by a given ID. Creates the track, if it does not exist.
		/// @param track Track to get.
		/// @return Requested track.
		Track& fetchTrack(usize const track) {
			if (!tracks.contains(track)) tracks[track] = {audio.createGroup(master)};
			return tracks[track];
		}

		/// @brief Creates a sound on a given track.
		/// @param path Path to audio file.
		/// @param trackID Track to assign audio to.
		/// @return Sound instance, or `nullptr` on failure.
		Audio::SoundInstance createOnTrack(String const& path, usize const trackID) {
			auto const track = fetchTrack(trackID);
			return audio.createSound(path, track.soundType, track.instance);
		}
	};
}

#endif