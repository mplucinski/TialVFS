#include "Object.hpp"
#include "Directory.hpp"
#include "Exception.hpp"

#define TIAL_MODULE "Tial::VFS::Object"

Tial::VFS::Object::Object(
	const std::shared_ptr<Root> &root,
	const std::shared_ptr<Directory> &parent,
	const std::string &name
): _root(root), _parent(parent), _name(name) {}

void Tial::VFS::Object::validate() {
	if(_valid == Validity::Broken)
		THROW Exceptions::ElementBroken();
	_valid = Validity::Valid;
}

void Tial::VFS::Object::validate() const {
	const_cast<Object*>(this)->validate();
}

void Tial::VFS::Object::markValid() {
	LOGN3 << "Marking " << *this << " as valid";
	_valid = Validity::Valid;
}

void Tial::VFS::Object::markInvalid() {
	LOGN3 << "Marking " << *this << " as invalid";
	_valid = Validity::Invalid;
}

void Tial::VFS::Object::markBroken() {
	LOGN3 << "Marking " << *this << " as broken";
	_valid = Validity::Broken;
}

void Tial::VFS::Object::checkIfBroken() const {
	if(_valid == Validity::Broken) {
		LOGW << "Element " << *this << " is broken";
		THROW Exceptions::ElementBroken();
	}
}

Tial::VFS::Object::~Object() {}

std::string Tial::VFS::Object::name() const {
	validate();
	return _name;
}

Tial::VFS::Object::Validity Tial::VFS::Object::valid() const {
	return _valid;
}

std::shared_ptr<Tial::VFS::Root> Tial::VFS::Object::root() {
	checkIfBroken();
	return _root.lock();
}

std::shared_ptr<Tial::VFS::Directory> Tial::VFS::Object::parent() const {
	checkIfBroken();
	return _parent.lock();
}

Tial::VFS::Path Tial::VFS::Object::path() const {
	checkIfBroken();
	assert(parent());
	return parent()->path()/_name;
}

void Tial::VFS::Object::remove() {
	THROW Exception("not implemented");
}

Tial::Utility::Logger::Stream &
Tial::VFS::operator<<(Utility::Logger::Stream &s, const Object &object) {
	return s << typeid(object) << reinterpret_cast<const void*>(&object) << "{" << object._name << "}";
}

std::ostream &Tial::VFS::operator<<(std::ostream &os, const Object::Validity &validity) {
	switch(validity) {
	case Object::Validity::Valid: os << "Validity::Valid"; break;
	case Object::Validity::Invalid: os << "Validity::Invalid"; break;
	case Object::Validity::Broken: os << "Validity::Broken"; break;
	default: os << "Validity::<unknown>"; break;
	}
	return os;
}

