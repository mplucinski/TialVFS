#include "NativeFSDriver.hpp"
#include "Exception.hpp"

#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#define TIAL_MODULE "Tial::VFS::NativeFSDriver"

static Tial::Utility::NativePath toAbsolute(const Tial::Utility::NativePath &path) {
	if(path.absolute())
		return path;
	return Tial::Utility::NativeDirectory::current().path()/path;
}

Tial::VFS::NativeFSDriver::NativeFSDriver(const Utility::NativePath &nativeDirectory, const std::string &name)
		: Driver(_prepareName(name, nativeDirectory)), nativeDirectory(toAbsolute(nativeDirectory)) {
	assert(this->nativeDirectory.absolute());
}

std::string Tial::VFS::NativeFSDriver::_prepareName(
	const std::string &name, const Utility::NativePath nativeDirectory
) {
	if(!name.empty())
		return name;
	return "NativeFSDriver{nativePath = " + std::string(nativeDirectory) + "}";
}

Tial::VFS::NativeFSDriver::NativeFileDescriptor::NativeFileDescriptor(
	const std::shared_ptr<NativeFSDriver> &driver,
	const Utility::NativePath &nativePath,
	bool openAutomatically
) : driver(driver), nativePath(nativePath) {
	if(openAutomatically)
		descriptorOpen();
}

Tial::VFS::NativeFSDriver::NativeFileDescriptor::~NativeFileDescriptor() {
	descriptorClose();
}

void Tial::VFS::NativeFSDriver::NativeFileDescriptor::descriptorOpen() {
	LOGN3;
#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	if(fd != -1)
		THROW Exceptions::AlreadyOpened();

	fd = ::open(std::string(nativePath).c_str(), O_RDWR);
	if(fd == -1)
		THROW std::system_error(errno, std::system_category());
#else
#error "Platform not supported"
#endif
}

void Tial::VFS::NativeFSDriver::NativeFileDescriptor::descriptorClose() {
	LOGN3;
#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	if(fd != -1 && ::close(fd) == -1)
		THROW std::system_error(errno, std::system_category());

	fd = -1;
#else
#error "Platform not supported"
#endif
}

size_t Tial::VFS::NativeFSDriver::NativeFileDescriptor::size() const {
#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	assert(fd);
	struct ::stat data;
	if(::fstat(fd, &data) != 0)
		THROW std::system_error(errno, std::system_category());
	return data.st_size;
#else
#error "Platform not supported"
#endif
}

void Tial::VFS::NativeFSDriver::cleanupOpenDescriptors() {
	LOGN3;
	std::unique_lock<std::recursive_mutex> lock(openDescriptorsMutex);
	for(auto i = openDescriptors.begin(); i != openDescriptors.end();) {
		if(i->second.expired())
			i = openDescriptors.erase(i);
		else
			++i;
	}
}

uintmax_t Tial::VFS::NativeFSDriver::sizeNative(const Path &path) {
	LOGN1 << "path = " << path;

#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	struct ::stat data;
	if(::stat(std::string(path).c_str(), &data) != 0)
		THROW std::system_error(errno, std::system_category());
	return data.st_size;
#else
#error "Platform not supported"
#endif
}

void Tial::VFS::NativeFSDriver::resizeNative(const Utility::NativePath &path, uintmax_t size) {
	std::unique_lock<std::recursive_mutex> lock(openDescriptorsMutex);
	LOGN1 << "path = " << path << ", size = " << size;
	assert(path.absolute());

	cleanupOpenDescriptors();

	auto descriptors = openDescriptors.equal_range(path);
	for(auto i = descriptors.first; i != descriptors.second; ++i)
		i->second.lock()->descriptorClose();

#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	if(::truncate(std::string(path).c_str(), size) != 0)
		THROW std::system_error(errno, std::system_category());
#else
#error "Platform not supported"
#endif

	for(auto i = descriptors.first; i != descriptors.second; ++i)
		i->second.lock()->descriptorOpen();
}

Tial::VFS::NativeFSDriver::NativeOpenFile::NativeOpenFile(
	const std::shared_ptr<NativeFSDriver> &driver,
	const Utility::NativePath &nativePath
) : NativeFileDescriptor(driver, nativePath) {}

void Tial::VFS::NativeFSDriver::NativeOpenFile::seek(size_t pos) {
#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	assert(fd);
	if(::lseek(fd, pos, SEEK_SET) == -1)
		THROW std::system_error(errno, std::system_category());
#else
#error "Platform not supported"
#endif
}

Tial::VFS::NativeFSDriver::NativeOpenFile::~NativeOpenFile() {}

size_t Tial::VFS::NativeFSDriver::NativeOpenFile::read(size_t pos, void *buffer, size_t bufferSize) {
	LOGN3;
#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	seek(pos);
	ssize_t r = ::read(fd, buffer, bufferSize);
	if(r == -1)
		THROW std::system_error(errno, std::system_category());
	assert(r >= 0);
	return static_cast<size_t>(r);
#else
#error "Platform not supported"
#endif
}

size_t Tial::VFS::NativeFSDriver::NativeOpenFile::write(size_t pos, const void *buffer, size_t bufferSize) {
	LOGN3;
#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	seek(pos);
	ssize_t r = ::write(fd, buffer, bufferSize);
	if(r == -1)
		THROW std::system_error(errno, std::system_category());
	assert(r >= 0);
	return static_cast<size_t>(r);
#else
#error "Platform not supported"
#endif
}

size_t Tial::VFS::NativeFSDriver::NativeOpenFile::size() {
	return NativeFileDescriptor::size();
}


Tial::VFS::NativeFSDriver::NativeMappedFile::NativeMappedFile(
	const std::shared_ptr<NativeFSDriver> &driver,
	const Utility::NativePath &nativePath
) : NativeFileDescriptor(driver, nativePath, false) {
	descriptorOpen();
}

Tial::VFS::NativeFSDriver::NativeMappedFile::~NativeMappedFile() {
	descriptorClose();
}

void Tial::VFS::NativeFSDriver::NativeMappedFile::descriptorOpen() {
	LOGN3;
	NativeFileDescriptor::descriptorOpen();

#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	assert(_ptr == nullptr);
	assert(_size == 0);

	_size = size();
	LOGN3 << "this->_size = " << _size;

	if(_size > 0) {
		_ptr = ::mmap(0, _size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if(_ptr == MAP_FAILED) {
			_ptr = nullptr;
			THROW std::system_error(errno, std::system_category());
		}
	}
#else
#error "Platform not supported"
#endif
}

void Tial::VFS::NativeFSDriver::NativeMappedFile::descriptorClose() {
	LOGN3 << "this->_ptr = " << _ptr << ", this->_size = " << _size;
#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	int result = 0;
	if(_ptr)
		result = ::munmap(_ptr, _size);

	_ptr = nullptr;
	_size = 0;

	if(result != 0)
		THROW std::system_error(errno, std::system_category());
#else
#error "Platform not supported"
#endif

	NativeFileDescriptor::descriptorClose();
}

void *Tial::VFS::NativeFSDriver::NativeMappedFile::get() {
	LOGN3;
#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	return _ptr;
#else
#error "Platform not supported"
#endif
}

size_t Tial::VFS::NativeFSDriver::NativeMappedFile::size() {
	LOGN3;
#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	return driver->sizeNative(nativePath);
#else
#error "Platform not supported"
#endif
}

void Tial::VFS::NativeFSDriver::NativeMappedFile::resize(size_t size) {
	LOGN1 << "size = " << size;
#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	driver->resizeNative(nativePath, size);
#else
#error "Platform not supported"
#endif
}

Tial::VFS::NativeFSDriver::FileEntry Tial::VFS::NativeFSDriver::get(const Path &path) {
	Utility::NativePath realPath = nativeDirectory/path;
	LOGN1 << "path = " << path << ", native path = " << realPath;

#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	struct ::stat data;
	if(::stat(std::string(realPath).c_str(), &data) != 0)
		THROW std::system_error(errno, std::system_category());

	bool directory = ((data.st_mode & S_IFDIR) == S_IFDIR);
	return {realPath[realPath.size()-1], directory};
#else
#error "Platform not supported"
#endif
}

std::vector<Tial::VFS::NativeFSDriver::FileEntry> Tial::VFS::NativeFSDriver::listDirectory(const Path &path) {
	Utility::NativePath realPath = nativeDirectory/path;
	LOGN1 << "path = " << path << ", native path = " << realPath;

	std::vector<Tial::VFS::NativeFSDriver::FileEntry> v;

#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	std::unique_ptr<DIR, std::function<void(DIR*)>> realDirectory(
		::opendir(std::string(realPath).c_str()),
		[](DIR *ptr) {
			::closedir(ptr);
		}
	);
	if(!realDirectory)
		THROW std::system_error(errno, std::system_category());

	struct dirent *entry;
	while((entry = ::readdir(realDirectory.get()))) {
		std::string name(entry->d_name);
		if(name == "." || name == "..")
			continue;

		v.push_back({name, entry->d_type == DT_DIR});
	}

#else
#error "Platform not supported"
#endif
	return v;
}

uintmax_t Tial::VFS::NativeFSDriver::size(const Path &path) {
	LOGN1 << "path = " << path;
	return sizeNative(nativeDirectory/path);
}

void Tial::VFS::NativeFSDriver::resize(const Path &path, uintmax_t size) {
	LOGN1 << "path = " << path;
	resizeNative(nativeDirectory/path, size);
}

void Tial::VFS::NativeFSDriver::createFile(const Path &path) {
	Utility::NativePath realPath = nativeDirectory/path;
	LOGN1 << "path = " << path << ", native path = " << realPath;

#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	int fd = ::open(std::string(realPath).c_str(), O_CREAT | O_EXCL, S_IRWXU | S_IRWXG | S_IRWXO);
	if(fd == -1) {
		if(errno == EEXIST)
			THROW Exceptions::ElementAlreadyExists(path);
		THROW std::system_error(errno, std::system_category());
	}

	if(::close(fd) == -1)
		THROW std::system_error(errno, std::system_category());
#else
#error "Platform not supported"
#endif
}

void Tial::VFS::NativeFSDriver::removeFile(const Path &path) {
	Utility::NativePath realPath = nativeDirectory/path;
	LOGN1 << "path = " << path << ", native path = " << realPath;

#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	if(::unlink(std::string(realPath).c_str()) == -1) {
		if(errno == ENOENT)
			THROW Exceptions::ElementNotFound(path, Path());
		THROW std::system_error(errno, std::system_category());
	}
#else
#error "Platform not supported"
#endif
}

void Tial::VFS::NativeFSDriver::createDirectory(const Path &path) {
	Utility::NativePath realPath = nativeDirectory/path;
	LOGN1 << "path = " << path << ", native path = " << realPath;

#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	if(::mkdir(std::string(realPath).c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0) {
		if(errno == EEXIST)
			THROW Exceptions::ElementAlreadyExists(path);
		THROW std::system_error(errno, std::system_category());
	}
#else
#error "Platform not supported"
#endif
}

void Tial::VFS::NativeFSDriver::removeDirectory(const Path &path) {
	Utility::NativePath realPath = nativeDirectory/path;
	LOGN1 << "path = " << path << ", native path = " << realPath;

#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
	if(::rmdir(std::string(realPath).c_str()) != 0) {
		if(errno == ENOTEMPTY)
			THROW Exceptions::DirectoryNotEmpty(path);
		THROW std::system_error(errno, std::system_category());
	}
#else
#error "Platform not supported"
#endif
}

std::shared_ptr<Tial::VFS::Driver::OpenFile> Tial::VFS::NativeFSDriver::open(const Path &path) {
	std::unique_lock<std::recursive_mutex> lock(openDescriptorsMutex);
	Utility::NativePath realPath = nativeDirectory/path;
	LOGN1 << "path = " << path << ", native path = " << realPath;
	return descriptor<NativeOpenFile>(realPath);
}

std::shared_ptr<Tial::VFS::Driver::MappedFile> Tial::VFS::NativeFSDriver::map(const Path &path) {
	Utility::NativePath realPath = nativeDirectory/path;
	std::unique_lock<std::recursive_mutex> lock(openDescriptorsMutex);
	LOGN1 << "path = " << path << ", native path = " << realPath;
	return descriptor<NativeMappedFile>(realPath);
}
