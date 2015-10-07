#include "Driver.hpp"
#include "Directory.hpp"
#include "Exception.hpp"

#include <TialUtility/TialUtility.hpp>

#define TIAL_MODULE "Tial::VFS::Driver"

void Tial::VFS::Driver::mark(const Path &path, std::function<void(const std::shared_ptr<Object> &)> function) {
	assert(!path.empty());
	assert(path[0] == "/");
	for(auto &i: mountPoints) {
		auto mountPoint = i.lock();
		if(!mountPoint)
			THROW std::logic_error("Mount point is no longer available");
		if(path.size() == 1)
			function(mountPoint);
		else
			function(mountPoint->get(path.subpath(1)));
	}
}

void Tial::VFS::Driver::markInvalid(const Path &path) {
	mark(path, [](const std::shared_ptr<Object> &object) {
		object->markInvalid();
	});
}

void Tial::VFS::Driver::markBroken(const Path &path) {
	mark(path, [](const std::shared_ptr<Object> &object) {
		object->markBroken();
	});
}

Tial::VFS::Driver::OpenFile::~OpenFile() {}

Tial::VFS::Driver::MappedFile::~MappedFile() {}

Tial::VFS::Driver::FileEntry::FileEntry(const std::string &fileName, bool directory):
		fileName(fileName), directory(directory) {}

Tial::VFS::Driver::Driver(const std::string &name): name(name) {}

Tial::VFS::Driver::~Driver() {}

void Tial::VFS::Driver::registerMountPoint(const std::shared_ptr<Directory> &directory) {
	mountPoints.push_back(directory);
}

void Tial::VFS::Driver::unregisterMountPoint(const std::shared_ptr<Directory> &directory) {
	auto i = std::find_if(mountPoints.begin(), mountPoints.end(), [&directory](std::weak_ptr<Directory> &mountPoint)->bool{
		return mountPoint.lock() == directory;
	});
	if(i == mountPoints.end())
		THROW Exceptions::NoMountPoint(directory->path());
	mountPoints.erase(i);
}

Tial::Utility::Logger::Stream &Tial::VFS::operator<<(
	Tial::Utility::Logger::Stream &stream,
	const Tial::VFS::Driver &driver
) {
	stream << "VFS::Driver{" << driver.name << "}";
	return stream;
}