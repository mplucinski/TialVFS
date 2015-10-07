#pragma once
#include "TialVFSExport.hpp"

#include <memory>
#include <string>
#include <vector>

#include <TialUtility/Logger.hpp>

#include "Common.hpp"

namespace Tial {
namespace VFS {

class Object;
class Directory;

class Mapping;

class TIALVFS_EXPORT Driver: public std::enable_shared_from_this<Driver> {
	std::string name;

private:
	void mark(const Path &path, std::function<void(const std::shared_ptr<Object> &)> function);

protected:
	void markInvalid(const Path &path);
	void markBroken(const Path &path);

	std::vector<std::weak_ptr<Directory>> mountPoints;

public:
	class OpenFile {
	public:
		virtual ~OpenFile() = 0;
		virtual size_t read(size_t pos, void *buffer, size_t bufferSize) = 0;
		virtual size_t write(size_t pos, const void *buffer, size_t bufferSize) = 0;
		virtual size_t size() = 0;
	};

	class MappedFile {
		std::recursive_mutex mutex;
	public:
		virtual ~MappedFile() = 0;
		virtual void *get() = 0;
		virtual size_t size() = 0;
		virtual void resize(size_t size) = 0;

		friend class VFS::Mapping;
	};

	class FileEntry {
	public:
		FileEntry(const std::string &fileName, bool directory);

		std::string fileName;
		bool directory;
	};

	explicit Driver(const std::string &name);
	virtual ~Driver() = 0;
	virtual FileEntry get(const Path &path) = 0;
	virtual std::vector<FileEntry> listDirectory(const Path &path) = 0;
	virtual uintmax_t size(const Path &path) = 0;
	virtual void resize(const Path &path, uintmax_t size) = 0;
	virtual void createFile(const Path &path) = 0;
	virtual void removeFile(const Path &path) = 0;
	virtual void createDirectory(const Path &path) = 0;
	virtual void removeDirectory(const Path &path) = 0;
	virtual std::shared_ptr<OpenFile> open(const Path &path) = 0;
	virtual std::shared_ptr<MappedFile> map(const Path &path) = 0;

	void registerMountPoint(const std::shared_ptr<Directory> &directory);
	void unregisterMountPoint(const std::shared_ptr<Directory> &directory);

	friend Utility::Logger::Stream &operator<<(Utility::Logger::Stream &stream,
		const Driver &driver);
};

}
}
