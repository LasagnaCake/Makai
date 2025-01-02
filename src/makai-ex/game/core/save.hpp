#ifndef MAKAILIB_EX_GAME_CORE_SAVE_H
#define MAKAILIB_EX_GAME_CORE_SAVE_H

#include<makai/makai.hpp>

/// @brief Game extensions.
namespace Makai::Ex::Game {
	/// @brief Save file view.
	struct SaveView: Makai::JSON::JSONView {
		using JSONView::JSONView;

		/// @brief Saves the current view's contents to disk.
		/// @param path Path to save contents to.
		/// @return Const reference to self.
		SaveView const& save(String const& path) const {
			saveToFile(path);
			return *this;
		}

		/// @brief Saves the current view's contents to disk.
		/// @param path Path to save contents to.
		/// @param pass File password.
		/// @return Const reference to self.
		SaveView const& save(String const& path, String const& pass) const {
			saveToFile(path, pass);
			return *this;
		}

		/// @brief Saves the current view's contents to disk.
		/// @param path Path to save contents to.
		/// @return Reference to self.
		SaveView& save(String const& path) {
			saveToFile(path);
			return *this;
		}
		
		/// @brief Saves the current view's contents to disk.
		/// @param path Path to save contents to.
		/// @param pass File password.
		/// @return Reference to self.
		SaveView& save(String const& path, String const& pass) {
			saveToFile(path, pass);
			return *this;
		}

		/// @brief Loads content from disk.
		/// @param path Path to content to load.
		/// @return Reference to self.
		SaveView& load(String const& path) {
			view() = Makai::File::getJSON(path);
			return *this;
		}

		/// @brief Loads a content from disk.
		/// @param path Path to content to load.
		/// @param pass File password.
		/// @return Reference to self.
		SaveView& load(String const& path, String const& pass) {
			view() = Makai::JSON::parse(Makai::Tool::Arch::loadEncryptedTextFile(path));
			return *this;
		}
		/// @brief Returns whether there is content stored.
		/// @return Whether contents exists.
		bool exists() const {return isObject();}

	private:
		void saveToFile(String const& path) const {
			if (exists()) Makai::File::saveText(path, toString());
		}

		void saveToFile(String const& path, String const& pass) const {
			if (exists()) Makai::Tool::Arch::saveEncryptedTextFile(path, toString(), pass);
		}
	};

	/// @brief Save file.
	struct Save {
		/// @brief Empty constructor.
		Save() {}

		/// @brief Constructs the save file.
		/// @param data Save file contents.
		explicit Save(Makai::JSON::JSONValue const& data): data(data)	{}
		/// @brief Constructs the save file.
		/// @param data Save file contents.
		explicit Save(Makai::JSON::JSONView const& data): data(data)	{}

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
			data = Makai::JSON::object();
			return *this;
		}

		/// @brief Saves a file to disk.
		/// @param path Path to save file.
		/// @return Const reference to self.
		Save const& save(String const& path) const {
			SaveView(data).save(path);
			return *this;
		}

		/// @brief Saves a file to disk.
		/// @param path Path to save file.
		/// @return Reference to self.
		/// @param pass File password.
		/// @return Const reference to self.
		Save const& save(String const& path, String const& pass) const {
			SaveView(data).save(path, pass);
			return *this;
		}

		/// @brief Saves a file to disk.
		/// @param path Path to save file.
		/// @return Reference to self.
		Save& save(String const& path) {
			SaveView(data).save(path);
			return *this;
		}

		/// @brief Saves a file to disk.
		/// @param path Path to save file.
		/// @return Reference to self.
		/// @param pass File password.
		/// @return Reference to self.
		Save& save(String const& path, String const& pass) {
			SaveView(data).save(path, pass);
			return *this;
		}

		/// @brief Loads a save file from disk.
		/// @param path Path to save file. 
		/// @return Reference to self.
		Save& load(String const& path) {
			SaveView(data).load(path);
			return *this;
		}

		/// @brief Loads a save file from disk.
		/// @param path Path to save file. 
		/// @param pass File password.
		/// @return Reference to self.
		Save& load(String const& path, String const& pass) {
			SaveView(data).load(path, pass);
			return *this;
		}

		/// @brief Gets a value from the save.
		/// @tparam T Value type.
		/// @param key Member name.
		/// @param fallback Fallback value.
		/// @return Value, or fallback.
		template<class T>
		T get(String const& key, T const& fallback) {
			return data[key].get<T>(fallback);
		}

		/// @brief Member access operator.
		/// @param key Member to get.
		/// @return Const view to member.
		SaveView const operator[](String const& key) const {
			return data[key];
		}
		
		/// @brief Member access operator.
		/// @param key Member to get.
		/// @return View to member.
		SaveView operator[](String const& key) {
			return data[key];
		}

		/// @brief Returns a view to the save's contents.
		/// @return View to contents.
		SaveView				view()	{return data;}
		/// @brief Returns the save as a JSON object.
		/// @return View to contents.
		Makai::JSON::JSONValue	value()	{return data;}

		/// @brief Assignment operator.
		/// @param value Value to assign.
		/// @return Reference to self.
		/// @note Value must be a JSON object.
		Save& operator=(Makai::JSON::JSONView const& value) {
			if (!value.isObject())
				throw Error::InvalidValue(
					"Save value must be a JSON object!",
					CTL_CPP_PRETTY_SOURCE
				);
			data = value;
			return *this;
		}

		/// @brief Returns whether there is a save stored.
		/// @return Whether save exists.
		bool exists() const {return data.isObject();}

	private:
		/// @brief Save file contents.
		Makai::JSON::JSONValue data = Makai::JSON::object();
	};
}

#endif