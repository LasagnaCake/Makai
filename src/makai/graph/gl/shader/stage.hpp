#ifndef MAKAILIB_GRAPH_SHADER_STAGE_H
#define MAKAILIB_GRAPH_SHADER_STAGE_H

#include "../../file/file.hpp"
#include "../../compat/ctl.hpp"

/// @brief Shader pipeline facilites.
namespace Makai::Graph::Pipeline {
	/// @brief Shader pipeline stage.
	class Stage {
		struct StageProgram;

		using StageInstance = Instance<StageProgram>;

		/// @brief Underlying pipeline stage instance.
		StageInstance instance;

	public:
		/// @brief Returns whether the pipeline stage exists.
		/// @return Whether pipeline stage exists.
		bool			exists() const;
		/// @brief Returns the pipeline stage's ID.
		/// @return Pipeline stage ID.
		uint32			id() const;
		/// @brief Returns the pipeline stage's type.
		/// @return Pipeline stage type.
		SLF::ShaderType	type() const;

		Stage();
		~Stage();

		Stage(Stage const&)	= default;
		Stage(Stage&&)		= default;

		Stage& create(String const& path);
		Stage& create(String const& source, ShaderType const type);
		Stage& make(String const& path);
		Stage& make(String const& source, ShaderType const type);
		Stage& destroy();
		Stage& unbind();

		operator bool() const {return exists();}

		Stage& operator=(Stage const&)	= default;
		Stage& operator=(Stage&&)		= default;
	};
}

#endif
