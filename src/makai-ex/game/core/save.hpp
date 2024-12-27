#ifndef MAKAILIB_EX_GAME_CORE_SAVE_H
#define MAKAILIB_EX_GAME_CORE_SAVE_H

#include<makai/makai.hpp>

/// @brief Game extensions.
namespace Makai::Ex::Game {

	/// @brief Save file view.
	struct SaveView: Makai::JSON::JSONView {
		using JSONView::JSONView;

		SaveView const& save(String const& path) const {
			saveToFile(path);
			return *this;
		}

		SaveView const& save(String const& path, String const& pass) const {
			saveToFile(path, pass);
			return *this;
		}

		SaveView& save(String const& path) {
			saveToFile(path);
			return *this;
		}
		
		SaveView& save(String const& path, String const& pass) {
			saveToFile(path, pass);
			return *this;
		}

		SaveView& load(String const& path) {
			view() = Makai::File::getJSON(path);
			return *this;
		}

		SaveView& load(String const& path, String const& pass) {
			view() = Makai::JSON::parse(Makai::Tool::Arch::loadEncryptedTextFile(path));
			return *this;
		}

	private:
		void saveToFile(String const& path) const {
			Makai::File::saveText(path, toString());
		}

		void saveToFile(String const& path, String const& pass) const {
			Makai::Tool::Arch::saveEncryptedTextFile(path, toString(), pass);
		}
	};

	struct Save {
		Save() {}

		Save(Makai::JSON::JSONValue const& data): data(data)	{}
		Save(Makai::JSON::JSONView const& data): data(data)		{}

		Save(String const& path)						{load(path);		}
		Save(String const& path, String const& pass)	{load(path, pass);	}

		Save& close(String const& path)						{return save(path).destroy();}
		Save& close(String const& path, String const& pass)	{return save(path).destroy();}
		
		Save& destroy() {
			data = Makai::JSON::object();
			return *this;
		}

		Save const& save(String const& path) const {
			SaveView(data).save(path);
			return *this;
		}

		Save const& save(String const& path, String const& pass) const {
			SaveView(data).save(path, pass);
			return *this;
		}

		Save& save(String const& path) {
			SaveView(data).save(path);
			return *this;
		}

		Save& save(String const& path, String const& pass) {
			SaveView(data).save(path, pass);
			return *this;
		}

		Save& load(String const& path) {
			SaveView(data).load(path);
			return *this;
		}

		Save& load(String const& path, String const& pass) {
			SaveView(data).load(path, pass);
			return *this;
		}

		template<class T>
		T get(String const& key, T const& fallback) {
			return data[key].get<T>(fallback);
		}

		SaveView const operator[](String const& key) const {
			return data[key];
		}
		
		SaveView operator[](String const& key) {
			return data[key];
		}

		Makai::JSON::JSONView view()	{return data;}
		Makai::JSON::JSONValue value()	{return data;}

		Save& operator=(Makai::JSON::JSONView const& value) {
			if (!value.isObject())
				throw Error::InvalidValue(
					"Save value must be a JSON object!",
					CTL_CPP_PRETTY_SOURCE
				);
			data = value;
			return *this;
		}

		bool exists() {return data.isObject();}

	private:
		Makai::JSON::JSONValue data = Makai::JSON::object();
	};
}

#endif