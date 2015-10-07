#pragma once
#include "TialVFSExport.hpp"

#include <TialUtility/Exception.hpp>

#include "Common.hpp"

namespace Tial {
namespace VFS {

class TIALVFS_EXPORT Exception: public Utility::Exception {
public:
	explicit Exception(const std::string &message);
	virtual ~Exception() override;
};

namespace Exceptions {

class TIALVFS_EXPORT InvalidPath: public Exception {
	Path path;
public:
	explicit InvalidPath(const Path &path);
};

class TIALVFS_EXPORT ElementNotFound: public Exception {
	Path path;
public:
	explicit ElementNotFound(const Path &path, const Path &parent);
};

class TIALVFS_EXPORT ElementKindInvalid: public Exception {
	Path path;
public:
	explicit ElementKindInvalid(const Path &path, const std::string &message);
};

class TIALVFS_EXPORT ElementBroken: public Exception {
public:
	ElementBroken();
};

class TIALVFS_EXPORT AlreadyMounted: public Exception {
	Path path;
public:
	explicit AlreadyMounted(const Path &path);
};

class TIALVFS_EXPORT NoMountPoint: public Exception {
	Path path;
public:
	explicit NoMountPoint(const Path &path);
};

class TIALVFS_EXPORT ElementAlreadyExists: public Exception {
	Path path;
public:
	explicit ElementAlreadyExists(const Path &path);
};

class TIALVFS_EXPORT DirectoryNotEmpty: public Exception {
	Path path;
public:
	explicit DirectoryNotEmpty(const Path &path);
};

class TIALVFS_EXPORT IOFailed: public Exception {
	Path path;
public:
	explicit IOFailed(const Path &path);
};

class TIALVFS_EXPORT UnassignedAccessor: public Exception {
	std::string accessorType;
public:
	explicit UnassignedAccessor(const std::string &accessorType);
};

class TIALVFS_EXPORT AlreadyOpened: public Exception {
public:
	AlreadyOpened();
};

}

}
}