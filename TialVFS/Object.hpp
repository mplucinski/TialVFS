#pragma once
#include "TialVFSExport.hpp"

#include <memory>
#include <string>

#include <TialUtility/Logger.hpp>

#include "Common.hpp"

namespace Tial {
namespace VFS {

class Directory;
class Root;

class TIALVFS_EXPORT Object: public std::enable_shared_from_this<Object> {
public:
	enum class Validity {
		Valid, // does contain correct data, may be used directly
		Invalid, // does not contain correct data (only name, parent and root are correct), must be validated before using
		Broken // does not contain correct data, and cannot be validated anymore
	};

private:
	std::weak_ptr<Root> _root;
	std::weak_ptr<Directory> _parent;
	mutable Validity _valid = Validity::Invalid;

protected:
	std::string _name;

	Object(const std::shared_ptr<Root> &root, const std::shared_ptr<Directory> &parent, const std::string &name);
	virtual void validate(); // make object Valid
	void validate() const;
	void checkIfBroken() const;
	virtual void markValid(); // mark object as Valid
	virtual void markInvalid(); // make object Invalid
	virtual void markBroken(); // make object Broken

public:
	virtual ~Object() = 0;

	std::string name() const;
	Validity valid() const;

	virtual Path path() const;
	virtual std::shared_ptr<Root> root();
	virtual std::shared_ptr<Directory> parent() const;

	virtual void remove();

	friend class Directory;
	friend class Driver;
	friend Utility::Logger::Stream &operator<<(Utility::Logger::Stream &s, const Object &object);
};

TIALVFS_EXPORT std::ostream &operator<<(std::ostream &os, const Object::Validity &validity);

}
}