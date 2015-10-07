#pragma once
#include "TialVFSExport.hpp"

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <TialUtility/TialUtility.hpp>

#include "Common.hpp"
#include "Driver.hpp"
#include "File.hpp"
#include "Object.hpp"

#define TIAL_MODULE "Tial::VFS::Directory"

namespace Tial {
namespace VFS {

struct BasenameHash: public std::hash<std::string> {
	size_t operator()(const std::string &x) const;
};

struct BasenameCompare {
	bool operator()(const std::string &x, const std::string &y) const;
};

class TIALVFS_EXPORT Directory: public Object {
	std::shared_ptr<Driver> _driver;
	std::recursive_mutex mutex;
	std::unordered_map<std::string, std::shared_ptr<Object>, BasenameHash, BasenameCompare> _content;

	virtual void validate() override;
	virtual void markInvalid() override;
	virtual void markBroken() override;
	std::shared_ptr<Tial::VFS::Object> entryToObject(const Driver::FileEntry &entry);
	static bool entryMatchesObject(const Driver::FileEntry &entry, const std::shared_ptr<Object> &object);
	std::pair<Path, std::shared_ptr<Driver>> driver() const;
	std::shared_ptr<Object> get(const Path &path);

protected:
	Directory(const std::shared_ptr<Root> &root, const std::shared_ptr<Directory> &directory, const std::string &name);

public:
	void mount(const std::shared_ptr<Driver> &driver);
	void unmount();

	std::vector<std::shared_ptr<Object>> content();
	std::vector<std::shared_ptr<Object>> collect();

	template<typename T>
	std::shared_ptr<T> get(const Path &path) {
		auto object = std::dynamic_pointer_cast<T>(get(path));
		assert(object);
		return object;
	}

	std::vector<std::shared_ptr<Object>> getAll(const Path &path);

	std::shared_ptr<File> createFile(const std::string &name);
	std::shared_ptr<Directory> createDirectory(const std::string &name);

	virtual void remove() override;

	friend class Driver;
	friend class File;
};

}
}

#undef TIAL_MODULE
