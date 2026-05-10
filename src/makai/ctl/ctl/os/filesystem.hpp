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

	/// @brief Copies a file/directory. If it is a directory, it also copies its contents.
	/// @param from Source to copy from.
	/// @param to Destination to copy to.
	inline void copy(String const& from, String const& to) {
		std::error_code ec;
		fs::copy(from.cstr(), to.cstr(), fs::copy_options::update_existing | fs::copy_options::recursive, ec);
		if (ec)
			throw Error::FailedAction("Could not copy file(s)!", ec.message());
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

	inline StringList filesIn(String const& folder) {
		if (!isDirectory(folder)) return {};
		StringList files;
		for (auto const& e: fs::directory_iterator(folder.std())) {
			if (e.is_directory()) continue;
			String name = e.path().filename().string();
			String path = concatenate(folder, name);
			files.pushBack(path);
		}
		return files;
	}

	inline StringList foldersIn(String const& folder) {
		if (!isDirectory(folder)) return {};
		StringList folders;
		for (auto const& e: fs::directory_iterator(folder.std())) {
			if (!e.is_directory()) continue;
			String name = e.path().stem().string();
			String path = concatenate(folder, name);
			folders.pushBack(path);
		}
		return folders;
	}

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
		throw Error::InvalidValue(e.what());
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
		return resolve("/proc/self/exe").splitAtLast('/').front();
		#endif
	}

	inline String asSharedLibrary(String const name) {
		#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
		return name + ".dll";
		#else
		return name + ".so";
		#endif
	}

	inline String asExecutable(String const name) {
		#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
		return name + ".exe";
		#else
		return name;
		#endif
	}
}

CTL_NAMESPACE_END

#endif // CTL_OS_FILESYSTEM_H
