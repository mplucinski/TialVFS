#include "Exception.hpp"

Tial::VFS::Exception::Exception(const std::string &message): Utility::Exception(message) {}

Tial::VFS::Exception::~Exception() {}

Tial::VFS::Exceptions::InvalidPath::InvalidPath(const Path &path): Exception("Invalid path"), path(path) {}

Tial::VFS::Exceptions::ElementNotFound::ElementNotFound(const Path &path, const Path &parent)
	: Exception("Element not found: " + std::string(path) + " in " + std::string(parent)), path(path) {}

Tial::VFS::Exceptions::ElementKindInvalid::ElementKindInvalid(const Path &path, const std::string &message)
	: Exception("Element kind invalid: " + std::string(path) + " : " + message), path(path) {}

Tial::VFS::Exceptions::ElementBroken::ElementBroken(): Exception("Element is broken") {}

Tial::VFS::Exceptions::AlreadyMounted::AlreadyMounted(const Path &path)
	: Exception("Already mounted driver in mount point: "+std::string(path)), path(path) {}

Tial::VFS::Exceptions::NoMountPoint::NoMountPoint(const Path &path)
	: Exception("No such mount point: "+std::string(path)), path(path) {}

Tial::VFS::Exceptions::ElementAlreadyExists::ElementAlreadyExists(const Path &path)
	: Exception("Element already exists: "+std::string(path)), path(path) {}

Tial::VFS::Exceptions::DirectoryNotEmpty::DirectoryNotEmpty(const Path &path)
	: Exception("Directory is not empty: "+std::string(path)), path(path) {}

Tial::VFS::Exceptions::IOFailed::IOFailed(const Path &path)
	: Exception("I/O failed: "+std::string(path)), path(path) {}

Tial::VFS::Exceptions::UnassignedAccessor::UnassignedAccessor(const std::string &accessorType)
	: Exception("Unassigned accessor: "+accessorType), accessorType(accessorType) {}

Tial::VFS::Exceptions::AlreadyOpened::AlreadyOpened()
	: Exception("Stream already opened") {}
