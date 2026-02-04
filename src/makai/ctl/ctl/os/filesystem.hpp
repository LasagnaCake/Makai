#ifndef CTL_OS_FILESYSTEM_H
#define CTL_OS_FILESYSTEM_H

#include <filesystem>
#include "../namespace.hpp"
#include "../container/strings/string.hpp"
#include "../container/error.hpp"
#include "../algorithm/strconv.hpp"

#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
#include <libloaderapi.h>
#endif

CTL_NAMESPACE_BEGIN

/// @brief Filesystem-related facilities.
namespace OS::FS {
	namespace {
		namespace fs = std::filesystem;
	}

	/// @brief Directory path separator.
	enum class PathSeparator: char {
		PS_POSIX	= '/',
		PS_WINDOWS	= '\\'
	};

	#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
	/// @brief Path separator that the system uses.
	constexpr PathSeparator SEPARATOR = PathSeparator::PS_WINDOWS;
	#else
	/// @brief Path separator that the system uses.
	constexpr PathSeparator SEPARATOR = PathSeparator::PS_POSIX;
	#endif

	/// @brief Checks to see if the specified path exists.
	/// @param path Path to check.
	/// @return Whether the path exists.
	inline bool exists(String const& path) {
		return fs::exists(path.std());
	}

	/// @brief Checks to see if the specified path is a directory.
	/// @param dir Path to check.
	/// @return Whether the path is a directory.
	inline bool isDirectory(String const& dir) {
		return fs::is_directory(dir.std());
	}

	/// @brief Replaces all path separator characters with the specified path separator.
	/// @param path Path to standardize.
	/// @param sep Separator to use.
	/// @return Standardized path.
	constexpr String standardize(String const& path, PathSeparator const& sep) {
		return path.replaced({'\\','/'}, (char)sep);
	}

	/// @brief Replaces all path separator characters with the OS's path separator.
	/// @param path Path to standardize.
	/// @param sep Separator to use.
	/// @return Standardized path.
	constexpr String standardize(String const& path) {
		return standardize(path, SEPARATOR);
	}

	/// @brief Creates a directory, while also creating its parents, if they don't exist.
	/// @param dir Directory to make.
	inline void makeDirectory(String const& dir) {
		if (dir.isNullOrSpaces()) return;
		if (!isDirectory(dir) || !exists(dir)) {
			fs::create_directories(dir.std());
		}
	}

	/// @brief Creates a series of directories, while also creating their parents, if they don't exist.
	/// @param dirs Directories to make.
	inline void makeDirectory(StringList const& dirs) {
		for (auto& d: dirs)
			makeDirectory(d);
	}

	/// @brief Creates a series of directories, while also creating their parents, if they don't exist.
	/// @tparam ...Args Argument types.
	/// @param ...args Directories to make.
	template <typename... Args>
	inline void makeDirectory(Args const&... args)
	requires (sizeof...(Args) > 1) {
		(..., makeDirectory(toString(args)));
	}

	/// @brief Deletes a file/directory. If it is a directory, it also deletes its contents.
	/// @param path Element to delete.
	inline void remove(String const& path) {
		fs::remove_all(path.std());
	}

	/// @brief Deletes a series of files/directories. If it is a directory, it also deletes its contents.
	/// @param paths Elements to delete.
	inline void remove(StringList const& paths) {
		for (auto& d: paths)
			remove(d);
	}

	/// @brief Deletes a series of files/directories. If it is a directory, it also deletes its contents.
	/// @tparam ...Args Argument types.
	/// @param ...args Elements to delete.
	template <typename... Args>
	inline void remove(Args const&... args) {
		(remove(toString(args)), ...);
	}

	/// @brief Concatenates a two paths together.
	/// @param root Path to concatenate.
	/// @param path Path to concatenate with.
	/// @return Concatenated path.
	constexpr String concatenate(String const& root, String const& path) {
		if (root.empty()) return path;
		String res = root;
		if (!path.empty()) res += "/" + path;
		return res;
	}

	/// @brief Sequentially concatenates a series of paths together.
	/// @param root Path to concatenate.
	/// @param path Paths to concatenate with.
	/// @return Concatenated path.
	constexpr String concatenate(String const& root, StringList const& paths) {
		String res = root;
		for(auto& path: paths) {
			if (!path.empty()) res = concatenate(res, path);
		}
		return res;
	}

	/// @brief Sequentially concatenates a series of paths together.
	/// @tparam ...Args Argument types.
	/// @param root Path to concatenate.
	/// @param path First path to concatenate with.
	/// @param ...args Subsequent paths to concatenate with.
	/// @return Concatenated path.
	template<typename... Args>
	constexpr String concatenate(String const& root, String const& path, Args const&... args) {
		String res = concatenate(root, path);
		(..., res.appendBack("/" + toString(args)));
		return res;
	}

	/// @brief Filesystem implementations.
	namespace Impl {
		constexpr String pathDirectory(String const& s) {
			if (s.empty()) return "";
			return (s[0] == '/' ? "" : "/") + s;
		}
	}

	/// @brief Sequentially concatenates a series of paths together.
	/// @tparam ...Args Argument types.
	/// @param root Path to concatenate.
	/// @param ...paths paths to concatenate with.
	/// @return Concatenated path.
	template <typename... Args>
	constexpr String concatenate(String const& root, Args const&... paths) {
		return root + (... + Impl::pathDirectory(toString(paths)));
	}

	/// @brief Returns the file extension.
	/// @param path Path to get from.
	/// @return File extension.
	constexpr String fileExtension(String const& path) {
		auto sp = path.splitAtLast('.');
		return sp.size() > 1 ? sp.back() : "";
	}

	/*constexpr String fileName(String const& path, bool const removeExtension = false) {
		String result = path.splitAtLast({'\\', '/'}).back();
		return (removeExtension ? result.splitAtLast('.').front() ? result);
	}*/

	/// @brief Returns the directory of the file pointed by the path.
	/// @param path Path to get from.
	/// @param removeExtension Whether to remove the file extension.
	/// @return File name.
	inline String fileName(String const& path, bool removeExtension = false) {
		return String(removeExtension ? fs::path(path.std()).stem().string() : fs::path(path.std()).filename().string());
	}

	/// @brief Returns the parent directory of a given path.
	/// @param path Path to get from.
	/// @return Parent directory.
	constexpr String parentDirectory(String const& path) {
		StringList splitPath = path.splitAtFirst({'\\', '/'});
		if (splitPath.size() > 1)
			return splitPath.front();
		return "";
	}

	/// @brief Returns the directory of the file pointed by the path.
	/// @param path Path to get from.
	/// @return File directory.
	inline String directoryFromPath(String const& path) {
		/*auto const split = path.splitAtLast({'\\', '/'});
		if (split.size() < 2) return "";
		return split.front();*/
		return String(fs::path(path.std()).remove_filename().string());
	}

	/// @brief Returns the path without the parent directory.
	/// @param path Path to get from.
	/// @return Child path.
	constexpr String childPath(String const& path) {
		StringList dirs = path.splitAtFirst({'\\', '/'});
		if (dirs.size() > 1)
			return dirs.back();
		return "";
	}

	/// @brief Directory file tree.
	struct FileTree {
		/// @brief File tree entry.
		struct Entry {
			/// @brief Default constructor.
			constexpr Entry() {}

			/// @brief Constructs a path entry as a file.
			/// @param name Entry name.
			/// @param path Relative path to entry.
			constexpr Entry(String const& name, String const& path):
				ename(name),
				epath(path) {}

			/// @brief Constructs a path entry as a directory.
			/// @param name Entry name.
			/// @param path Relative path to entry.
			/// @param children Directory contents.
			constexpr Entry(String const& name, String const& path, List<Entry>	const& children):
				ename(name),
				epath(path),
				children(children),
				folder(true) {}

			/// @brief Copy constructor.
			/// @param other `Entry` to copy from.
			constexpr Entry(Entry const& other):
				ename(other.ename),
				epath(other.epath),
				children(other.children),
				folder(other.folder) {}

			/// @brief Move constructor.
			/// @param other `Entry` to move.
			constexpr Entry(Entry&& other):
				ename(CTL::move(other.ename)),
				epath(CTL::move(other.epath)),
				children(CTL::move(other.children)),
				folder(CTL::move(other.folder)) {}

			/// @brief Iterates through the entry's children, performing the passed operation.
			/// @tparam TFunction Operation type.
			/// @param op Operation to perform.
			/// @return Reference to self.
			template<Type::Functional<void(Entry&)> TFunction>
			constexpr Entry& forEach(TFunction const& op) {
				for (Entry& e: children)
					op(e);
				return *this;
			}

			/// @brief Iterates through the entry's children, performing the passed operation.
			/// @tparam TFunction Operation type.
			/// @param op Operation to perform.
			/// @return Const reference to self.
			template<Type::Functional<void(Entry const&)> TFunction>
			constexpr Entry const& forEach(TFunction const& op) const {
				for (Entry const& e: children)
					op(e);
				return *this;
			}

			/// @brief Copy assignment operator.
			/// @param other `Entry` to copy from.
			/// @return Reference to self.
			constexpr Entry& operator=(Entry const& other) {
				ename		= other.ename;
				epath		= other.epath;
				folder		= other.folder;
				children	= other.children;
				return *this;
			}

			/// @brief Move assignment operator.
			/// @param other `Entry` to move.
			/// @return Reference to self.
			constexpr Entry& operator=(Entry&& other) {
				ename		= CTL::move(other.ename);
				epath		= CTL::move(other.epath);
				folder		= CTL::move(other.folder);
				children	= CTL::move(other.children);
				return *this;
			}

			/// @brief Returns an iterator to the beginning of the entry's children.
			/// @return Iterator to beginning of entry's children.
			constexpr auto begin()			{return children.begin();	}
			/// @brief Returns an iterator to the end of the entry's children.
			/// @return Iterator to end of entry's children.
			constexpr auto end()			{return children.end();		}
			/// @brief Returns a pointer to the beginning of the entry's children.
			/// @return Pointer to beginning of entry's children.
			constexpr auto cbegin()			{return children.cbegin();	}
			/// @brief Returns a pointer to the end of the entry's children.
			/// @return Pointer to end of entry's children.
			constexpr auto cend()			{return children.cend();	}

			/// @brief Returns an iterator to the beginning of the entry's children.
			/// @return Iterator to beginning of entry's children.
			constexpr auto begin() const	{return children.begin();	}
			/// @brief Returns an iterator to the end of the entry's children.
			/// @return Iterator to end of entry's children.
			constexpr auto end() const		{return children.end();		}
			/// @brief Returns a pointer to the beginning of the entry's children.
			/// @return Pointer to beginning of entry's children.
			constexpr auto cbegin() const	{return children.cbegin();	}
			/// @brief Returns a pointer to the end of the entry's children.
			/// @return Pointer to end of entry's children.
			constexpr auto cend() const		{return children.cend();	}

			/// @brief Returns the path of all files inside the directory, and subdirectories (if entry is a directory).
			/// @return Path of all files inside the directory. If not a directory, returns own path.
			constexpr StringList getAllFiles() const {
				if (!folder) return StringList().pushBack(epath);
				StringList files;
				for (Entry const& e: children) {
					if (!e.folder)
						files.pushBack(e.epath);
					else {
						StringList subfiles = e.getAllFiles();
						files.appendBack(subfiles);
					}
				}
				return files;
			}

			/// @brief Returns the entry's children, if entry is a directory.
			/// @return The entry's children.
			constexpr List<Entry> getChildren() const {return children;}

			/// @brief Returns the entry's children, if entry is a directory.
			/// @return The entry's children.
			constexpr operator List<Entry>() const	{return getChildren();	}
			/// @brief Returns the path to the entry.
			/// @return Path to the entry.
			constexpr operator String() const		{return path();			}

			/// @brief Returns whether the entry is a directory.
			/// @return Whether the entry is a directory.
			constexpr bool isFolder() const {return folder;}

			/// @brief Returns the entry's name.
			/// @return The entry's name.
			constexpr String name() const {return ename;}
			/// @brief Returns the path to the entry.
			/// @return Path to the entry.
			constexpr String path() const {return epath;}

			/// @brief Returns whether the entry has any children.
			/// @return Whether the entry has any children.
			constexpr usize empty() const {return children.empty();	}

		private:
			/// @brief Entry name.
			String ename;
			/// @brief Path to entry.
			String epath;

			/// @brief Child entries, if entry is a directory.
			List<Entry>	children	= List<Entry>();
			/// @brief Whether the entry is a directory.
			bool		folder		= false;
		};

		/// @brief Constructs a file tree for a given path.
		/// @param path Path to construct a file tree for.
		FileTree(String const& path): tree(getStructure(path)) {}

		/// @brief Constructs a file tree from an entry.
		/// @param entry Entry to construct a file tree from.
		constexpr FileTree(Entry const& entry): tree(entry) {}

		/// @brief Copy constructor.
		/// @param entry Entry to copy from.
		constexpr FileTree(FileTree const& other): tree(other.tree) {}

		/// @brief Returns the head of the file tree.
		/// @return Head of the file tree.
		constexpr operator Entry() const {return tree;}

		/// @brief Builds a directory structure for a path.
		/// @param path Path to entry.
		/// @return Directory structure. If path does not lead to a folder, simply returns an entry for that file.
		static inline Entry getStructure(String const& path) {
			if (!exists(path))
				throw Error::InvalidValue("Path \"" +path+ "\" does not exist!", CTL_CPP_PRETTY_SOURCE);
			if (!isDirectory(path)) return Entry(fileName(path), path);
			return Entry(
				String(fs::path(path.std()).stem().string()),
				path,
				getFolderContents(path)
			);
		}

		/// @brief Returns the contents of a folder.
		/// @param folder Folder to get the contents of.
		/// @return Contents of the folder. If path does not lead to a folder, returns an empty list.
		/// @note Will also build the directory structure of all subfolders.
		static inline List<Entry> getFolderContents(String const& folder) {
			if (!isDirectory(folder)) return List<Entry>();
			List<Entry> entries;
			for (auto const& e: fs::directory_iterator(folder.std())) {
				String name = String((e.is_directory()) ? e.path().stem().string() : e.path().filename().string());
				String path = concatenate(folder, name);
				if (e.is_directory())
					entries.pushBack(Entry(name, path, getFolderContents(path)));
				else
					entries.pushBack(Entry(name, path));
			}
			return entries;
		}

		/// @brief Head of the file tree.
		Entry tree;
	};

	/// @brief Returns the executable's current working directory.
	/// @return Current working directory.
	inline String currentDirectory() {
		return std::filesystem::current_path().string();
	}

	/// @brief Resolves a path to its absolute path.
	/// @param path Path to resolve.
	/// @return Absolute path.
	inline String absolute(String const& path) {
		return std::filesystem::absolute(path.std()).string();
	}

	/// @brief Resolves a path to an existing item.
	/// @param path Path to resolve.
	/// @return Canonical path to item.
	inline String resolve(String const& path) try {
		return std::filesystem::canonical(path.std()).string();
	} catch (std::filesystem::filesystem_error const& e) {
		throw InvalidValueException(e.what());
	}

	/// @brief Returns the executable's storage directory.
	/// @return Location of executable.
	inline String sourceLocation() {
		#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
		String src = String(1024, '\0');
		GetModuleFileName(NULL, src.data(), src.size());
		src = src.sliced(0, src.find('\0')).replace('\\', '/').splitAtLast('/').front();
		return resolve(src.size() ? src : ".");
		#else
		return resolve("/proc/self/exe");
		#endif
	}
}

CTL_NAMESPACE_END

#endif // CTL_OS_FILESYSTEM_H
