#include "File.hpp"
#include "Directory.hpp"
#include "Exception.hpp"

#include <TialUtility/TialUtility.hpp>

#define TIAL_MODULE "Tial::VFS::File"

Tial::VFS::FileDevice::FileDevice(const std::shared_ptr<Driver::OpenFile> &file): file(file) {}

std::streamsize Tial::VFS::FileDevice::read(char *buffer, std::streamsize bufferSize) const {
	LOGN1 << "buffer = " << static_cast<const void*>(buffer) << ", bufferSize = " << bufferSize
		<< ", this->pos = " << static_cast<int>(pos);
	auto result = file->read(pos, buffer, bufferSize);
	LOGN1 << "result = " << result;
	pos += result;
	return result;
}

std::streamsize Tial::VFS::FileDevice::write(const char *buffer, std::streamsize bufferSize) {
	LOGN1 << "buffer = " << static_cast<const void*>(buffer) << ", bufferSize = " << bufferSize
		<< ", this->pos = " << static_cast<int>(pos);
	auto result = file->write(pos, buffer, bufferSize);
	pos += result;
	LOGN1 << "result = " << result;
	return result;
}

std::streampos Tial::VFS::FileDevice::tell() const {
	LOGN1;
	return pos;
}

std::streampos Tial::VFS::FileDevice::seek(boost::iostreams::stream_offset offset, std::ios_base::seekdir direction) {
	LOGN1 << "offset = " << offset << ", direction = " << direction;
	switch(direction) {
	case std::ios_base::beg: pos = offset; break;
	case std::ios_base::cur: pos += offset; break;
	case std::ios_base::end: pos = file->size()+offset; break;
	default: assert(false); break;
	}
	return pos;
}

std::streamsize Tial::VFS::FileDevice::size() const {
	return file->size();
}

Tial::VFS::Stream::Stream(
	const std::shared_ptr<Driver::OpenFile> &file,
	intmax_t offset,
	std::ios_base::seekdir direction
): _device(FileDevice(file)) {
	open(_device);
	seekg(offset, direction);
	seekp(offset, direction);
}

Tial::VFS::Stream::Stream(const Stream &other): std::ios(), boost::iostreams::stream<FileDevice>() {
	(*this) = other;
}

void Tial::VFS::Stream::open(const FileDevice &device) {
	if(is_open())
		THROW Exceptions::AlreadyOpened();

	_device = device;
	boost::iostreams::stream<FileDevice>::open(device);
}

const Tial::VFS::FileDevice *Tial::VFS::Stream::operator->() const {
	return &_device;
}

Tial::VFS::Stream::Stream() {}

Tial::VFS::Stream& Tial::VFS::Stream::operator=(const Tial::VFS::Stream &stream) {
	if(is_open())
		close();
	if(stream->file) {
		open(FileDevice(stream->file));
		seekg(stream->tell(), beg);
		seekp(stream->tell(), beg);
	}
	return *this;
}

std::streamsize Tial::VFS::Stream::size() const {
	return _device.size();
}

Tial::VFS::Mapping::Mapping(const std::shared_ptr<File> &file): file(file->_map()), lock(this->file->mutex) {}

Tial::VFS::Mapping &Tial::VFS::Mapping::operator=(Mapping &&other) {
	lock = std::move(other.lock);
	file = std::move(other.file);
	return *this;
}

void *Tial::VFS::Mapping::get() const {
	if(!file)
		THROW Exceptions::UnassignedAccessor("Mapping");
	return file->get();
}

size_t Tial::VFS::Mapping::size() const {
	LOGN1;
	if(!file)
		THROW Exceptions::UnassignedAccessor("Mapping");
	return file->size();
}

void Tial::VFS::Mapping::resize(size_t size) {
	LOGN1;
	if(!file)
		THROW Exceptions::UnassignedAccessor("Mapping");
	file->resize(size);
}

bool Tial::VFS::Mapping::assigned() const {
	return static_cast<bool>(file);
}

Tial::VFS::Mapping::operator bool() const {
	return assigned();
}

bool Tial::VFS::Mapping::operator!() const {
	return !static_cast<bool>(*this);
}

Tial::VFS::File::File(
	const std::shared_ptr<Root> &root,
	const std::shared_ptr<Directory> &parent,
	const std::string &name
): Object(root, parent, name) {}

std::shared_ptr<Tial::VFS::Driver::OpenFile> Tial::VFS::File::_open() {
	auto d = parent()->driver();
	return d.second->open(d.first/name());
}

std::shared_ptr<Tial::VFS::Driver::MappedFile> Tial::VFS::File::_map() {
	auto d = parent()->driver();
	return d.second->map(d.first/name());
}

void Tial::VFS::File::validate() {
	LOGN2;

	if(valid() == Validity::Valid)
		return;

	checkIfBroken();

	LOGN1 << "*this = " << *this;

	auto d = parent()->driver();

	try {
		auto entry = d.second->get(d.first/_name);
	} catch(const Exceptions::ElementNotFound &e) {
		markBroken();
	}

	markValid();
}

Tial::VFS::Stream Tial::VFS::File::open(intmax_t offset, std::ios_base::seekdir direction) {
	validate();
	return Stream(std::dynamic_pointer_cast<File>(shared_from_this())->_open(), offset, direction);
}

Tial::VFS::Mapping Tial::VFS::File::map() {
	validate();
	return Mapping(std::dynamic_pointer_cast<File>(shared_from_this()));
}

uintmax_t Tial::VFS::File::size() {
	validate();
	auto d = parent()->driver();
	return d.second->size(d.first/name());
}

void Tial::VFS::File::resize(uintmax_t size) {
	validate();
	auto d = parent()->driver();
	d.second->resize(d.first/name(), size);
}

void Tial::VFS::File::remove() {
	validate();
	auto d = parent()->driver();
	d.second->removeFile(d.first/name());
	markBroken();
}
