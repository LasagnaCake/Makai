#ifndef MAKAILIB_EX_GAME_CORE_SAVE_H
#define MAKAILIB_EX_GAME_CORE_SAVE_H

#include<makai/makai.hpp>

/// @brief Game extensions.
namespace Makai::Ex::Game {
	/// @brief Save file.
	struct Save: JSON::Value {
		/// @brief Default constructor.
		Save() {}

		/// @brief Loads a save file from disk.
		/// @param path Path to save file.
		Save(String const& path)						{load(path);		}
		/// @brief Loads a save file from disk.
		/// @param path Path to save file.
		/// @param pass File password.
		Save(String const& path, String const& pass)	{load(path, pass);	}

		/// @brief Saves a file to disk, then clears the object's contents.
		/// @param path Path to save file.
		/// @return Reference to self.
		Save& close(String const& path)						{return save(path).clear();}
		/// @brief Saves a file to disk, then clears the object's contents.
		/// @param path Path to save file.
		/// @return Reference to self.
		/// @param pass File password.
		/// @return Reference to self.
		Save& close(String const& path, String const& pass)	{return save(path).clear();}
		
		/// @brief Clears the object's contents.
		/// @return Reference to self.
		Save& clear() {
			operator=(Makai::JSON::Value::object());
			return *this;
		}

		/// @brief Saves a file to disk.
		/// @param path Path to save file.
		/// @return Const reference to self.
		Save const& save(String const& path) const {
			saveToFile(path);
			return *this;
		}

		/// @brief Saves a file to disk.
		/// @param path Path to save file.
		/// @return Reference to self.
		/// @param pass File password.
		/// @return Const reference to self.
		Save const& save(String const& path, String const& pass) const {
			saveToFile(path, pass);
			return *this;
		}

		/// @brief Saves a file to disk.
		/// @param path Path to save file.
		/// @return Reference to self.
		Save& save(String const& path) {
			saveToFile(path);
			return *this;
		}

		/// @brief Saves a file to disk.
		/// @param path Path to save file.
		/// @return Reference to self.
		/// @param pass File password.
		/// @return Reference to self.
		Save& save(String const& path, String const& pass) {
			saveToFile(path, pass);
			return *this;
		}

		/// @brief Loads a save file from disk.
		/// @param path Path to save file. 
		/// @return Reference to self.
		Save& load(String const& path) {
			operator=(File::getJSON(path));
			return *this;
		}

		/// @brief Loads a save file from disk.
		/// @param path Path to save file. 
		/// @param pass File password.
		/// @return Reference to self.
		Save& load(String const& path, String const& pass) {
			operator=(JSON::parse(Tool::Arch::loadEncryptedTextFile(path)));
			return *this;
		}

		/// @brief Gets a value from the save.
		/// @tparam T Value type.
		/// @param key Member name.
		/// @param fallback Fallback value.
		/// @return Value, or fallback.
		template<class T>
		T get(String const& key, T const& fallback) {
			return operator[](key).get<T>(fallback);
		}
		
		/// @brief Gets a value from the save.
		/// @tparam T Value type.
		/// @param key Member name.
		/// @param fallback Fallback value.
		/// @return Value, or fallback.
		template<class T>
		T get(T const& fallback) {
			return Value::get<T>(fallback);
		}

		/// @brief Gets a value from the save.
		/// @tparam T Value type.
		/// @param key Member name.
		/// @param fallback Fallback value.
		/// @return Value, or fallback.
		template<class T>
		T get() {
			return Value::get<T>();
		}

		/// @brief Member access operator.
		/// @param key Member to get.
		/// @return Const view to member.
		template <class T>
		Save const operator[](T const& key) const {
			return Value::operator[](key);
		}
		
		/// @brief Member access operator.
		/// @param key Member to get.
		/// @return View to member.
		template <class T>
		Value& operator[](T const& key) {
			return Value::operator[](key);
		}

		/// @brief Returns the save as a JSON object.
		/// @return View to contents.
		JSON::Value	value()	const	{return copy<JSON::Value>(*this);	}

		/// @brief Assignment operator.
		/// @param value Value to assign.
		/// @return Reference to self.
		/// @note Value must be a JSON object.
		Save& operator=(Makai::JSON::Value const& value) {
			if (!value.isObject())
				throw Error::InvalidValue(
					"Save value must be a JSON object!",
					CTL_CPP_PRETTY_SOURCE
				);
			Value::operator=(value);
			return *this;
		}

		/// @brief Returns whether there is a save stored.
		/// @return Whether save exists.
		constexpr bool exists() const {return isObject();}

	private:
		void saveToFile(String const& path) const {
			if (exists()) File::saveText(path, toString());
		}

		void saveToFile(String const& path, String const& pass) const {
			if (exists()) Tool::Arch::saveEncryptedTextFile(path, toString(), pass);
		}
	};
}

#endif