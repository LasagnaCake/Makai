#ifndef CTL_CONTAINER_NODEGRAPH_MIXED_H
#define CTL_CONTAINER_NODEGRAPH_MIXED_H

#include "../map/map.hpp"

/// @brief Node graphs.
namespace CTL::NodeGraph {
	/// @brief Mixed graph.
	/// @tparam TKey Node key type.
	/// @tparam TValue Edge weight type.
	/// @tparam D Whether the graph is a directed graph.
	template <class TKey, class TValue, bool D>
	struct Mixed {
		/// @brief Graph node connection.
		/// @tparam T Weight type.
		template<class T>
		struct Connection;

		/// @brief Graph node connection.
		/// @tparam T Weight type.
		template <Type::Void T>
		struct Connection<T> {
			/// @brief Whether the connection exists.
			bool exists = true;

			/// @brief Empty constructor.
			constexpr Connection() {}

			/// @brief Returns whether the connection exists.
			constexpr operator bool() const {return exists;}
		};

		/// @brief Graph node connection.
		/// @tparam T Weight type.
		template<Type::NonVoid T>
		struct Connection<T> {
			/// @brief Whether the connection exists.
			bool exists;
			/// @brief Connection weight.
			T value;

			/// @brief Empty constructor.
			constexpr Connection() {}
			/// @brief Constructs the connection with a weight.
			constexpr Connection(T const& value): exists(true), value(value) {}

			/// @brief Returns whether the connection exists.
			constexpr operator bool() const {return exists;}
			
			/// @brief Assignment operator overloading.
			constexpr Connection& operator=(T const& v) {value = v; return *this;}
		};
		
		/// @brief Node accessor.
		/// @tparam T Node value type.
		template <class T>
		struct Accessor;
		
		/// @brief Node accessor.
		/// @tparam T Node value type.
		template <Type::Void T>
		struct Accessor<T> {
			/// @brief Assignment operator overloading.
			template <class _>
			constexpr Accessor operator=(_)				{graph.connect(from, to); return *this;		}
			/// @brief Returns whether the connection exists.
			/// @return Whether connection exists.
			constexpr bool exists() const				{return graph.connected(from, to);			}
		private:
			/// @brief Starting point of the connection.
			TKey const from;
			/// @brief End point of the connection.
			TKey const to;
			/// @brief Parent graph.
			Mixed& graph;
			friend struct Mixed;
		};

		/// @brief Node accessor.
		/// @tparam T Node value type.
		template <Type::NonVoid T>
		struct Accessor<T> {
			/// @brief Assignment operator overloading.
			constexpr Accessor operator=(T const& v)	{graph.connect(from, to, v); return *this;	}
			/// @brief Returns the connection's weight.
			constexpr operator T() const				{return graph.weight(from, to);				}
			/// @brief Returns whether the connection exists.
			/// @return Whether connection exists.
			constexpr bool exists() const				{return graph.connected(from, to);			}
		private:
			/// @brief Starting point of the connection.
			TKey const from;
			/// @brief End point of the connection.
			TKey const to;
			/// @brief Parent graph.
			Mixed& graph;
			friend struct Mixed;
		};

		/// @brief Constant node accessor.
		/// @tparam T Node value type.
		template <class T>
		struct ConstAccessor;
		
		/// @brief Constant node accessor.
		/// @tparam T Node value type.
		template <Type::Void T>
		struct ConstAccessor<T> {
			/// @brief Returns whether the connection exists.
			/// @return Whether connection exists.
			constexpr bool exists() const	{return graph.connected(from, to);	}
		private:
			/// @brief Starting point of the connection.
			TKey const from;
			/// @brief End point of the connection.
			TKey const to;
			/// @brief Parent graph.
			Mixed const& graph;
			friend struct Mixed;
		};

		/// @brief Constant node accessor.
		/// @tparam T Node value type.
		template <Type::NonVoid T>
		struct ConstAccessor<T> {
			/// @brief Returns the connection value.
			constexpr operator T() const	{return graph.weight(from, to);		}
			/// @brief Returns whether the connection exists.
			/// @return Whether connection exists.
			constexpr bool exists() const	{return graph.connected(from, to);	}
		private:
			/// @brief Starting point of the connection.
			TKey const from;
			/// @brief End point of the connection.
			TKey const to;
			/// @brief Parent graph.
			Mixed const& graph;
			friend struct Mixed;
		};

		/// @brief Relation storage type.
		using Relations	= Map<TKey, Map<TKey, Connection<TValue>>>;

		/// @brief Whether the graph is directed.
		constexpr static bool IS_DIRECTED = D;

		/// @brief Multi-dimensional accessor operator overloading.
		constexpr Accessor<TValue> operator()(TKey const& from, TKey const& to) {
			return {from, to, *this};
		}

		/// @brief Multi-dimensional accessor operator overloading.
		constexpr ConstAccessor<TValue> operator()(TKey const& from, TKey const& to) const {
			return {from, to, *this};
		}

		/// @brief Multi-dimensional accessor operator overloading.
		constexpr Accessor<TValue> operator[](FirstSecondPair<TKey, TKey> const& connection) {
			return {connection.first, connection.second, *this};
		}

		/// @brief Multi-dimensional accessor operator overloading.
		constexpr ConstAccessor<TValue> operator[](FirstSecondPair<TKey, TKey> const& connection) const {
			return {connection.first, connection.second, *this};
		}

		/// @brief Creates a connection between two nodes, if applicable.
		/// @param from Node to connect from.
		/// @param to Node to connect to.
		/// @return Reference to self.
		constexpr Mixed& connect(TKey const& from, TKey const& to)
		requires (Type::Void<TValue>) {
			if (from == to) return *this;
			forward[from][to]	= {};
			reverse[from][to]	= {};
			if (!IS_DIRECTED) {
				forward[to][from] = {};
				reverse[to][from] = {};
			}
			return *this;
		}
		
		/// @brief Returns the weight for a given connection.
		/// @param from Start node of connection to get value for.
		/// @param to End node of connection to get value for.
		/// @return Edge value.
		constexpr TValue weight(TKey const& from, TKey const& to) const requires (Type::NonVoid<TValue>) {
			return forward[from][to];
		}

		/// @brief Creates a connection between two nodes, if applicable.
		/// @param from Node to connect from.
		/// @param to Node to connect to.
		/// @param weight Edge weight.
		/// @return Reference to self.
		constexpr Mixed& connect(TKey const& from, TKey const& to, TValue const& weight = TValue())
		requires (Type::NonVoid<TValue>) {
			if (from == to) return *this;
			forward[from][to]	= {weight};
			reverse[from][to]	= {weight};
			if (!IS_DIRECTED) {
				forward[to][from] = {weight};
				reverse[to][from] = {weight};
			}
			return *this;
		}

		/// @brief Removes a connection between two nodes, if applicable.
		/// @param from Node to disconnect from.
		/// @param to Node to disconnect to.
		/// @return Reference to self.
		constexpr Mixed& disconnect(TKey const& from, TKey const& to) {
			if (from == to) return *this;
			forward[from][to].exists = false;
			reverse[from][to].exists = false;
			if (!IS_DIRECTED) {
				forward[to][from].exists = false;
				reverse[to][from].exists = false;
			}
			return *this;
		}

		/// @brief Clears all connections going to the node.
		/// @param node Node to clear connections from.
		/// @return Reference to self.
		constexpr Mixed& disconnect(TKey const& node) {
			if (forward.contains(node))
				for (auto child: forward[node]) {
					reverse[child.key][node].exists		= false;
					if constexpr (!IS_DIRECTED) {
						reverse[node][child.key].exists	= false;
						forward[child.key][node].exists	= false;
					}
				}
			forward[node].clear();
			return *this;
		}

		/// @brief Clears all relations for every node.
		/// @return Reference to self.
		constexpr Mixed& disconnectAll() {
			forward.clear();
			reverse.clear();
			return *this;
		}

		/// @brief Returns all destinations of a given node.
		/// @param node Node to get children.
		/// @return Children of node.
		constexpr List<TKey> startingFrom(TKey const& node) const {
			List<TKey> dest;
			if (forward.contains(node)) {
				dest.resize(forward[node].size());
				for (auto child : forward[node])
					if (child.exists) dest.pushBack(child.key);
			}
			return dest;
		}

		/// @brief Returns the amount of neighbours a given node has.
		/// @param node Node to get neighbour count.
		/// @return Children count of node.
		constexpr usize neighbourCount(TKey const& node) const {
			usize count = 0;
			if (forward.contains(node)) {
				for (auto const& dest : forward[node])
					if (dest.exists) ++count;
			}
			return count;
		}

		/// @brief Returns whether a node is a "root" node (i.e. has no parent).
		/// @param node Node to check.
		/// @return Whether node is a root node.
		constexpr bool isRootNode(TKey const& node) const {
			if (reverse.contains(node)) {
				for (auto const& child : reverse[node])
					if (child.value) return false;
			}
			return true;
		}

		/// @brief Returns whether a node is a "leaf" node (i.e. has no children).
		/// @param node Node to check.
		/// @return Whether node is a leaf node.
		constexpr bool isLeafNode(TKey const& node) const {
			if (forward.contains(node)) {
				for (auto const& child : forward[node])
					if (child.value) return false;
			}
			return true;
		}

		/// @brief Returns all root nodes.
		/// @return Root nodes.
		constexpr List<TKey> roots() const {
			List<TKey> roots;
			for (auto const& i: forward)
				if (isRootNode(i.key))
					roots.pushBack(i.key);
			return roots;
		}

		/// @brief Returns all leaf nodes.
		/// @return Leaf nodes.
		constexpr List<TKey> leaves() const {
			List<TKey> leaves;
			for (auto const& i: reverse)
				if (isLeafNode(i.key))
					leaves.pushBack(i.key);
			return leaves;
		}

		/// @brief Returns whether a node can be reached from another node.
		/// @param from Node to start from.
		/// @param from Node to end in.
		/// @return Whether node are connected.
		constexpr bool connected(TKey const& from, TKey const& to) const {
			if constexpr (IS_DIRECTED) return bridged(from, to);
			else return bridged(from, to) || bridged(to, from);
		}

		/// @brief Traverses via Depth-First Search across the node tree.
		/// @tparam
		///		TFunction Function type.
		///		The first parameter it takes is the `source` node,
		///		and the second is the `destination` node.
		///		The third value is the connection value.
		///		If node does not have a parent, it will be set to itself.
		/// @param Function to execure for every node in the tree.
		template<Type::Functional<void(TKey const&, TKey const&, TValue const&)> TFunction>
		constexpr void dfsTraverse(TFunction const& func) const {
			List<TKey> nodeRoots = roots();
			for (auto const root : nodeRoots) {
				List<KeyValuePair<TKey, TKey>> stack;
				stack.pushBack({root, root});
				TKey current;
				TKey parent;
				while (stack.size()) {
					auto const relation = stack.popBack();
					parent	= relation.key;
					current	= relation.value;
					func(parent, current);
					if (!isLeafNode(current)) {
						for (auto& child: startingFrom(current))
							stack.pushBack({current, child});
					}
				}
			}
		}

	private:
		constexpr bool bridged(usize const from, usize const to) const {
			if (from == to) return true;
			List<usize> stack;
			stack.pushBack(from);
			usize current;
			while (!stack.empty()) {
				if (stack.rfind(to) != -1) return true;
				current = stack.popBack();
				if (!isLeafNode(current))
					stack.appendBack(startingFrom(current));
			}
			return false;
		}

		/// @brief Parent-child relations.
		Relations	forward;
		/// @brief Child-parent relations.
		Relations	reverse;
	};
	
	/// @brief Directed weighted graph.
	/// @tparam TKey Node key type.
	/// @tparam TValue Edge weight type.
	template <class TKey, Type::NonVoid TValue>
	using DW = Mixed<TKey, TValue, true>;

	/// @brief Undirected weighted graph.
	/// @tparam TKey Node key type.
	/// @tparam TValue Edge weight type.
	template <class TKey, Type::NonVoid TValue>
	using UW = Mixed<TKey, TValue, false>;

	/// @brief Directed unweighted graph.
	/// @tparam TKey Node key type.
	template <class TKey>
	using DU = Mixed<TKey, void, true>;

	/// @brief Undirected unweighted graph.
	/// @tparam TKey Node key type.
	template <class TKey, class TValue>
	using UU = Mixed<TKey, void, false>;
}

#endif