#include "Directory.hpp"

#include <queue>

#include <boost/algorithm/string.hpp>

#include "Exception.hpp"
#include "Root.hpp"

#define TIAL_MODULE "Tial::VFS::Directory"

size_t Tial::VFS::BasenameHash::operator()(const std::string &x) const {
	return std::hash<std::string>::operator()(boost::algorithm::to_lower_copy(x));
}

bool Tial::VFS::BasenameCompare::operator()(const std::string &x, const std::string &y) const {
	LOGN3 << "Comparing " << x << " with " << y;
	bool result = boost::iequals(x, y);
	LOGN3 << "Result = " << result;
	return result;
}

void Tial::VFS::Directory::validate() {
	LOGN2;

	if(valid() == Validity::Valid)
		return;

	checkIfBroken();

	LOGN1 << "*this = " << *this;

	std::unique_lock<std::recursive_mutex> lock(mutex);
	auto d = driver();

	LOGN2 << "Populating " << path() << " using driver " << d.second
			<< " with relative path " << d.first;

	auto entries = d.second->listDirectory(d.first);
	for(const auto &entry: entries) {
		LOGN3 << "entry = " << entry.fileName;
		auto existing = _content.find(entry.fileName);
		if(existing != _content.end()) {
			if(entryMatchesObject(entry, existing->second)) {
				LOGN3 << "Element " << entry.fileName << " already exists and is correct, skipping";
				continue;
			}
			LOGN3 << "Element " << entry.fileName << " already exists but differs, removing";
			existing->second->markBroken();
			_content.erase(existing);
		}
		LOGN3 << "Element " << entry.fileName << " is emplaced";
		_content.emplace(entry.fileName, entryToObject(entry));
	}

	for(auto element = _content.begin(); element != _content.end();) {
		for(const auto &entry: entries)
			if(entry.fileName == element->second->_name) {
				++element;
				goto elementLoopEnd;
			}

		element->second->markBroken();
		element = _content.erase(element);

elementLoopEnd:;
	}

	markValid();
}

void Tial::VFS::Directory::markInvalid() {
	Object::markInvalid();

	for(auto &element: _content)
		element.second->markInvalid();
}

void Tial::VFS::Directory::markBroken() {
	Object::markBroken();

	for(auto &element: _content)
		element.second->markBroken();
}

bool Tial::VFS::Directory::entryMatchesObject(const Driver::FileEntry &entry, const std::shared_ptr<Object> &object) {
	if(entry.fileName != object->_name)
		return false;

	if(entry.directory != static_cast<bool>(std::dynamic_pointer_cast<Directory>(object)))
		return false;

	return true;
}

std::shared_ptr<Tial::VFS::Object> Tial::VFS::Directory::entryToObject(const Driver::FileEntry &entry) {
	LOGN3 << "parent = " << *this << ", entry.fileName = " << entry.fileName;
	auto p = std::dynamic_pointer_cast<Directory>(shared_from_this());
	if(!entry.directory)
		return std::shared_ptr<Object>(new File(root(), p, entry.fileName));
	else
		return std::shared_ptr<Object>(new Directory(root(), p, entry.fileName));
}

std::pair<Tial::VFS::Path, std::shared_ptr<Tial::VFS::Driver>> Tial::VFS::Directory::driver() const {
	LOGN3 << *this;
	std::shared_ptr<Driver> driver = _driver;
	Path path;
	auto elem = std::dynamic_pointer_cast<const Directory>(shared_from_this());
	while(!driver) {
		path = Path(elem->_name)/path;
		elem = elem->parent();
		if(!elem)
			THROW Exception("No parent set");
		driver = elem->_driver;
	}
	LOGN3 << "found driver " << driver << " with relative path " << path;
	return std::make_pair(Path("/")/path, driver);
}

std::shared_ptr<Tial::VFS::Object> Tial::VFS::Directory::get(const Path &path) {
	validate();

	LOGN2 << "path = " << path;
	if(std::string(path).find("*") != std::string::npos) {
		auto all = getAll(path);
		if(all.empty())
			THROW Exceptions::ElementNotFound(path, this->path());
		return all[0];
	}

	assert(!path.empty());
	if(path.size() == 1) {
		try {
			for(auto i: _content)
				LOGN3 << "_content = " << i;
			auto child = _content.at(path[0]);
			assert(child);
			LOGN3 << "Returning child of type " << Utility::typeId(*child);
			return child;
		} catch(const std::out_of_range &e) {
			THROW Exceptions::ElementNotFound(path, this->path());
		}
	}
	auto head = get<Directory>(path[0]);
	assert(head);
	auto child = head->get(path.subpath(1));
	assert(child);
	LOGN3 << "Returning child of type " << Utility::typeId(*child);
	return child;
}

Tial::VFS::Directory::Directory(const std::shared_ptr<Root> &root, const std::shared_ptr<Directory> &directory,
	const std::string &name): Object(root, directory, name) {}

void Tial::VFS::Directory::mount(const std::shared_ptr<Driver> &driver) {
	LOGI << "Mounting driver " << driver << " on " << path();
	if(_driver)
		THROW Exceptions::AlreadyMounted(path());
	_driver = driver;
	_driver->registerMountPoint(std::dynamic_pointer_cast<Directory>(shared_from_this()));
	markInvalid();
}

void Tial::VFS::Directory::unmount() {
	LOGI << "Unmounting driver " << _driver << " on " << path();

	if(!_driver)
		THROW Exceptions::NoMountPoint(path());

	_driver->unregisterMountPoint(std::dynamic_pointer_cast<Directory>(shared_from_this()));
	_driver.reset();

	markInvalid();

	for(auto &element: _content)
		element.second->markBroken();
	_content.clear();
}

std::vector<std::shared_ptr<Tial::VFS::Object>> Tial::VFS::Directory::content() {
	LOGN2 << "*this = " << *this;
	validate();

	std::vector<std::shared_ptr<Tial::VFS::Object>> v;
	for(auto i: _content)
		v.push_back(i.second);
	return v;
//	return content(std::dynamic_pointer_cast<Directory>(shared_from_this()), "");
}

std::vector<std::shared_ptr<Tial::VFS::Object>> Tial::VFS::Directory::collect() {
	LOGN1 << "Collecting all subelements of " << *this;
	std::vector<std::shared_ptr<Tial::VFS::Object>> v;
	for(auto i: content()) {
		v.push_back(i);
		if(auto dir = std::dynamic_pointer_cast<Directory>(i)) {
			auto collection = dir->collect();
			v.insert(v.end(), collection.begin(), collection.end());
		}
	}
	return v;
}

std::vector<std::shared_ptr<Tial::VFS::Object>> Tial::VFS::Directory::getAll(const Path &path) {
	LOGN1 << "Getting all subelements of " << this->path() << " with path " << path;

	std::vector<std::shared_ptr<Tial::VFS::Object>> v;

	std::unordered_map<Path, std::shared_ptr<Tial::VFS::Object>> map;
	// left part of pattern => directory in which it should match
	std::queue<std::pair<Path, std::shared_ptr<Directory>>> pending;

	pending.push(std::make_pair(
		path, std::dynamic_pointer_cast<Directory>(shared_from_this())
	));

	while(!pending.empty()) {
		auto element = pending.front();
		pending.pop();

		if(element.first.empty()) {
			LOGN1 << "Empty pattern, adding " << element.second << " and exiting";
			map.emplace(element.second->path(), element.second);
			continue;
		}

		LOGN2 << "Processing element " << element;
		auto pattern = element.first[0];
		if(pattern == "**") {
			// collect ALL subdirectories recursively
			for(auto entry: element.second->collect())
				if(auto dir = std::dynamic_pointer_cast<Directory>(entry)) {
					LOGN3 << "Found entry " << dir;
					pending.push(std::make_pair(element.first.subpath(1), dir));
				} else if(element.first.size() == 1) {
					v.push_back(entry);
				}
		} else {
			auto content = element.second->content();
			LOGN2 << "Received " << content.size() << " elements";
			for(auto entry: content) {
				LOGN3 << "Processing entry " << entry->name();
				if(Utility::Wildcards::match(pattern, entry->name())) {
					LOGN3 << "Found matching entry " << entry << " (to"
							<< pattern << ", pattern left " << element.first.size() << ")";
					if(element.first.size() == 1)
						map.emplace(entry->path(), entry);
					if(auto dir = std::dynamic_pointer_cast<Directory>(entry))
						pending.push(std::make_pair(element.first.subpath(1), dir));
				}
			}
		}
	}

	for(auto i: map)
		v.push_back(i.second);
	return v;
}

std::shared_ptr<Tial::VFS::File> Tial::VFS::Directory::createFile(const std::string &name) {
	validate();

	LOGN2 << "this = " << *this << ", name = " << name;
	auto d = driver();
	d.second->createFile(d.first/name);

	markInvalid();
	return get<File>(name);
}

std::shared_ptr<Tial::VFS::Directory> Tial::VFS::Directory::createDirectory(const std::string &name) {
	validate();

	LOGN2 << "this = " << *this << ", name = " << name;
	auto d = driver();
	d.second->createDirectory(d.first/name);

	markInvalid();
	return get<Directory>(name);
	//return createDirectory(std::dynamic_pointer_cast<Directory>(shared_from_this()), name);
}

void Tial::VFS::Directory::remove() {
	validate();

	LOGN2 << "this = " << *this;
	auto d = driver();

	try {
		d.second->removeDirectory(d.first);
	} catch(Exceptions::DirectoryNotEmpty&) {
		for(auto i: content())
			i->remove();
		d.second->removeDirectory(d.first);
	}

	parent()->markInvalid();
	markBroken();
}
