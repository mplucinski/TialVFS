#include "Root.hpp"

Tial::VFS::Root::Root(): Directory(nullptr, nullptr, "/") {}

Tial::VFS::Path Tial::VFS::Root::path() const {
	return "/";
}

std::shared_ptr<Tial::VFS::Root> Tial::VFS::Root::root() {
	return std::dynamic_pointer_cast<Root>(shared_from_this());
}

std::shared_ptr<Tial::VFS::Directory> Tial::VFS::Root::parent() const {
	return nullptr;
}
