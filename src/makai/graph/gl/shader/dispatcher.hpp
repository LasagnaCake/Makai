#ifndef MAKAILIB_GRAPH_SHADER_DISPATCHER_H
#define MAKAILIB_GRAPH_SHADER_DISPATCHER_H

#include "../../file/file.hpp"
#include "../../compat/ctl.hpp"

#include "stage.hpp"

/// @brief Shader pipeline facilites.
namespace Makai::Graph::Pipeline {
	/// @brief Shader pipeline dispatcher.
	class Dispatcher {
		class DispatcherProgram;

		using DispatcherInstance = Instance<DispatcherProgram>;

		/// @brief Underlying pipeline dispatcher instance.
		DispatcherInstance instance;

	public:
		/// @brief Returns whether the pipeline dispatcher exists.
		/// @return Whether pipeline dispatcher exists.
		bool exists() const;
		/// @brief Returns the pipeline dispatcher's ID.
		/// @return Pipeline dispatcher ID.
		uint32 id() const;

		Dispatcher();
		~Dispatcher();
	};
}

#endif
