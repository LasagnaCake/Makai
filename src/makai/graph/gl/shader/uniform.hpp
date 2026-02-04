#ifndef MAKAILIB_GRAPH_SHADER_UNIFORM_H
#define MAKAILIB_GRAPH_SHADER_UNIFORM_H

#include "../../../compat/ctl.hpp"
#include "../core.hpp"

/// @brief Graphical facilites.
namespace Makai::Graph {
	/// @brief Decays the enumerator type to a valid shader integer type.
	/// @tparam T Enum type to decay.
	template<Type::Enumerator T>
	using ShaderEnum = Meta::DualType<Type::Unsigned<T>, uint32, int>;

	/// @brief Shader uniform access interface.
	struct Uniform {
		/// @brief Uniform name.
		String const name;

		/// @brief Constructs the uniform.
		/// @param name Uniform name.
		/// @param id Shader ID.
		Uniform(String const& name, uint32 const id);

		/// @brief Sets the uniform.
		/// @param value Value to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void set(bool const value, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @param value Value to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void set(int const value, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @param value Value to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void set(uint32 const value, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @param value Value to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void set(float const value, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @param value Value to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void set(double const value, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @param value Value to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void set(Vector2 const& value, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @param value Value to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void set(Vector3 const& value, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @param value Value to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void set(Vector4 const& value, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @param value Value to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void set(Matrix3x3 const& value, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @param value Value to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void set(Matrix4x4 const& value, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @param values Values to set.
		/// @param count Count of values to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void setArray(int const* const values, usize const count, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @param values Values to set.
		/// @param count Count of values to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void setArray(uint32 const* const values, usize const count, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @param values Values to set.
		/// @param count Count of values to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void setArray(float const* const values, usize const count, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @param values Values to set.
		/// @param count Count of values to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void setArray(double const* const values, usize const count, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @param values Values to set.
		/// @param count Count of values to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void setArray(Vector2 const* const values, usize const count, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @param values Values to set.
		/// @param count Count of values to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void setArray(Vector3 const* const values, usize const count, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @param values Values to set.
		/// @param count Count of values to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void setArray(Vector4 const* const values, usize const count, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @param values Values to set.
		/// @param count Count of values to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		void setArray(Matrix4x4 const* const values, usize const count, usize const offset = 0) const;

		/// @brief Sets the uniform.
		/// @tparam T Element type.
		/// @param values List of values to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		template <typename T>
		inline void set(List<T> const& values, usize const offset = 0) const {
			this->offset = 0;
			setArray((T*)values.data(), values.size(), offset);
			this->offset = values.size();
		}

		/// @brief Sets the uniform.
		/// @tparam T Enumerator type.
		/// @param value Value to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		template <Type::Enumerator T>
		inline void set(T const value, usize const offset = 0) const {
			set(static_cast<ShaderEnum<T>>(value), offset);
		}

		/// @brief Sets the uniform.
		/// @tparam T Element type.
		/// @tparam S Span size.
		/// @param values Values to set.
		/// @param offset Uniform location offset. By default, it is zero.
		/// @warning `offset` should NEVER be used directly!
		template <typename T, usize S = CTL::DYNAMIC_SIZE>
		inline void set(Span<T, S> const& values, usize const offset = 0) const {
			this->offset = 0;
			setArray((T*)values.data(), values.size(), offset);
			this->offset = values.size();
		}

		/// @brief Sets this uniform, and the ones that follow it.
		/// @tparam ...Args Argument types.
		/// @param ...args Values to set.
		template <typename... Args>
		inline void setSequence(Args const&... args) const {
			this->offset = 0;
			usize off = 0;
			(setSpecial(args, off), ...);
			offset = off;
		}

		/// @brief Processes a series of classes.
		/// @tparam T Class type.
		/// @param values Values to process.
		/// @param func Process to execute.
		template <Makai::Type::Class T>
		inline void forEach(List<T> const& values, Functor<void(T const&, Uniform const&, usize const)> func) const {
			usize i = 0;
			for (T const& val: values)
				func(val, Uniform(name, id, location + offset), i++);
		}

		/// @brief Sets the uniform.
		/// @tparam T Value type.
		/// @param value Value to set.
		template <typename T>			inline void operator()(T const& value) const			{setSequence(value);	}
		/// @brief Sets this uniform, and the ones that follow it.
		/// @tparam ...Args Argument types.
		/// @param ...args Values to set.
		template <typename... Args>		inline void operator()(Args const&... args) const		{setSequence(args...);	}
		/// @brief Sets the uniform.
		/// @tparam T Element type.
		/// @param values List of values to set.
		template <typename T>			inline void operator()(List<T> const& values) const		{setSequence(values);	}
		/// @brief Sets the uniform.
		/// @tparam T Element type.
		/// @tparam S Span size.
		/// @param values Values to set.
		template <typename T, usize S>	inline void operator()(Span<T, S> const& values) const	{setSequence(values);	}

		/// @brief Uniform member access operator.
		/// @param member Member to get.
		/// @return Uniform for member.
		Uniform operator[](String const& member) const;

	private:
		constexpr Uniform(
			String  const& _name,
			uint32 const _id,
			uint32 const _location
		): name(_name), id(_id), location(_location) {}

		template<typename T>
		inline void setSpecial(T const& value, usize& offset) const {
			set(value, offset);
			++offset;
		}

		template <typename T>
		inline void setSpecial(List<T> const& values, usize& offset) const {
			set(values, offset);
			offset += this->offset;
		}

		template <typename T, usize S>
		inline void setSpecial(Span<T, S> const& values, usize& offset) const {
			set(values, offset);
			offset += this->offset;
		}

		uint32 getUniformArray(usize const offset) const;

		uint32 getUniform() const;

		uint32 getUniform(String const& append) const;

		/// @brief Shader associated with the uniform.
		uint32 const id;
		/// @brief Uniform location.
		intmax const location;

		/// @brief Uniform location offset.
		usize mutable offset;
	};
}

#endif // MAKAILIB_GRAPH_UNIFORM_H
