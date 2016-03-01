#pragma once
#include "TialVFSExport.hpp"
#include <iostream>
#include "Object.hpp"
#include "Driver.hpp"

#include <boost/iostreams/stream.hpp>

namespace Tial {
namespace VFS {

class Driver;
class File;
class Stream;

class TIALVFS_EXPORT FileDevice {
	std::shared_ptr<Driver::OpenFile> file;

    mutable std::streampos pos
//FIXME: workaround for bug in Clang/C2 (https://llvm.org/bugs/show_bug.cgi?id=23542)
#if !(BOOST_COMP_CLANG && BOOST_OS_WINDOWS)
    = 0
#endif
    ;

    FileDevice()
//FIXME: workaround for bug in Clang/C2 (https://llvm.org/bugs/show_bug.cgi?id=23542)
#if !(BOOST_COMP_CLANG && BOOST_OS_WINDOWS)
    = default
#endif
    ;

	explicit FileDevice(const std::shared_ptr<Driver::OpenFile> &file);
public:
	typedef char char_type;
	typedef boost::iostreams::seekable_device_tag category;

	std::streamsize read(char *buffer, std::streamsize bufferSize) const;
	std::streamsize write(const char *buffer, std::streamsize bufferSize);
	std::streampos seek(boost::iostreams::stream_offset offset, std::ios_base::seekdir direction);
	std::streampos tell() const;
	std::streamsize size() const;

	friend class Stream;
};

class TIALVFS_EXPORT Stream: public boost::iostreams::stream<FileDevice> {
	FileDevice _device;

	Stream(const std::shared_ptr<Driver::OpenFile> &file, intmax_t offset, std::ios_base::seekdir direction);
	const FileDevice *operator->() const;
	void open(const FileDevice &device);
public:
	Stream();
	Stream(const Stream &other);
	Stream &operator=(const Stream &other);
	std::streamsize size() const;

	friend class File;
};

class TIALVFS_EXPORT Mapping {
	std::shared_ptr<Driver::MappedFile> file;
	std::unique_lock<std::recursive_mutex> lock;

	explicit Mapping(const std::shared_ptr<File> &file);
public:
	Mapping() = default;
	Mapping(const Mapping &) = delete;
	Mapping(Mapping &&) = default;

	Mapping &operator=(const Mapping &) = delete;
	Mapping &operator=(Mapping &&);

	size_t size() const;
	void resize(size_t size);
	void *get() const;

	template<typename T>
	T *as() {
		return reinterpret_cast<T*>(get());
	}

	template<typename T>
	const T *as() const {
		return reinterpret_cast<T*>(get());
	}

	bool assigned() const;
	operator bool() const;
	bool operator!() const;

	friend class File;
};

class TIALVFS_EXPORT File: public Object {
	File(const std::shared_ptr<Root> &root, const std::shared_ptr<Directory> &parent, const std::string &name);
	std::shared_ptr<Driver::OpenFile> _open();
	std::shared_ptr<Driver::MappedFile> _map();
	virtual void validate() override;
public:
	Stream open(intmax_t offset = 0, std::ios_base::seekdir direction = std::ios_base::beg);
	Mapping map();

	uintmax_t size();
	void resize(uintmax_t size);

	virtual void remove() override;

	friend class FileDevice;
	friend class Mapping;
	friend class Directory;
};

}
}