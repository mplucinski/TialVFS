#include "MemoryDriver.hpp"

#include "Exception.hpp"

#include <TialUtility/TialUtility.hpp>

#define TIAL_MODULE "Tial::VFS::MemoryDriver"

Tial::VFS::MemoryDriver::Node::Node(bool directory): directory(directory) {
	LOGN3;
}

Tial::VFS::MemoryDriver::FileEntry
Tial::VFS::MemoryDriver::Node::get(const Path &path) {
	auto node = getNode(path);
	return {path[path.size()-1], node->directory};
}

std::shared_ptr<Tial::VFS::MemoryDriver::Node>
Tial::VFS::MemoryDriver::Node::getNode(const Path &path) {
	LOGN2 << "path = " << path;
	assert(!path.empty());
	if(path.size() == 1)
		return elements.at(path[0]);
	return elements.at(path[0])->getNode(path.subpath(1));
}

std::vector<Tial::VFS::MemoryDriver::FileEntry>
Tial::VFS::MemoryDriver::Node::listDirectory(const Path &path) {
	LOGN2 << "path = " << path;
	if(path.empty()) {
		std::vector<Tial::VFS::MemoryDriver::FileEntry> v;
		for(auto &&i: elements) {
			LOGN3 << "adding element " << i.first;
			v.push_back({i.first, i.second->directory});
		}
		return v;
	}
	return elements.at(path[0])->listDirectory(path.subpath(1));
}

void Tial::VFS::MemoryDriver::Node::createNode(const Path &path, bool directory) {
	LOGN3 << "Creating node " << path << " (directory = " << directory << ")";
	assert(!path.empty());
	if(path.size() == 1) {
		if(elements.find(path[0]) != elements.end())
			THROW Exceptions::ElementAlreadyExists(path);
		elements.emplace(std::make_pair(path[0], std::make_shared<Node>(directory)));
	}
	else if(path.size() >= 2)
		elements[path[0]]->createNode(path.subpath(1), directory);

}

void Tial::VFS::MemoryDriver::Node::removeNode(const Path &path) {
	LOGN3 << "Removing node " << path;
	assert(!path.empty());
	if(path.size() == 1)
		elements.erase(path[0]);
	else if(path.size() >= 2)
		elements[path[0]]->removeNode(path.subpath(1));
}

Tial::VFS::MemoryDriver::MemoryOpenFile::MemoryOpenFile(
	const std::shared_ptr<Node> &node): node(node) {}

size_t Tial::VFS::MemoryDriver::MemoryOpenFile::read(size_t pos, void *buffer, size_t bufferSize) {
	LOGN3 << "buffer = " << buffer << ", bufferSize = " << bufferSize;
	size_t toRead = std::min(node->data.size() - pos, bufferSize);
	auto inputStart = node->data.cbegin()+pos;
	auto inputEnd = inputStart+toRead;
	auto outputStart = reinterpret_cast<uint8_t*>(buffer);
	auto outputEnd = std::copy(inputStart, inputEnd, outputStart);
	assert(outputEnd == outputStart+toRead);
	unused(outputEnd);
	return toRead;
}

size_t Tial::VFS::MemoryDriver::MemoryOpenFile::write(size_t pos, const void *buffer, size_t bufferSize) {
	LOGN3 << "buffer = " << buffer << ", bufferSize = " << bufferSize;
	auto inputStart = reinterpret_cast<const uint8_t*>(buffer);
	auto inputSize = bufferSize;
	auto outputStart = node->data.begin()+pos;
	auto outputSize = node->data.end()-outputStart;
	assert(outputSize >= 0);

	auto overwritePartSize = std::min(inputSize, static_cast<size_t>(outputSize));

	std::copy(inputStart, inputStart+overwritePartSize, outputStart);
	std::copy(inputStart+overwritePartSize, inputStart+inputSize, std::inserter(node->data, node->data.end()));
	return bufferSize;
}

size_t Tial::VFS::MemoryDriver::MemoryOpenFile::size() {
	LOGN3;
	return node->data.size();
}

Tial::VFS::MemoryDriver::MemoryMappedFile::MemoryMappedFile(const std::weak_ptr<Node> &node): node(node) {}

void *Tial::VFS::MemoryDriver::MemoryMappedFile::get() {
	LOGN3 << "this = " << reinterpret_cast<void*>(this);
	return reinterpret_cast<void*>(node.lock()->data.data());
}

size_t Tial::VFS::MemoryDriver::MemoryMappedFile::size() {
	return node.lock()->data.size();
}

void Tial::VFS::MemoryDriver::MemoryMappedFile::resize(size_t size) {
	LOGN3 << "size = " << size;
	node.lock()->data.resize(size);
}

Tial::VFS::MemoryDriver::MemoryDriver(const std::string &name): Driver(name) {}

Tial::VFS::MemoryDriver::FileEntry
Tial::VFS::MemoryDriver::get(const Path &path) {
	LOGN2 << "Getting file " << path;
	assert(path.absolute());
	return root->get(path.subpath(1));
}

std::vector<Tial::VFS::MemoryDriver::FileEntry>
Tial::VFS::MemoryDriver::listDirectory(const Path &path) {
	LOGN2 << "Listing directory " << path;
	assert(path.absolute());
	return root->listDirectory(path.subpath(1));
}

uintmax_t Tial::VFS::MemoryDriver::size(const Path &path) {
	LOGN2 << "path = " << path;
	assert(path.absolute());
	return root->getNode(path.subpath(1))->data.size();
}

void Tial::VFS::MemoryDriver::resize(const Path &path, uintmax_t size) {
	LOGN2 << "path = " << path << " size = " << size;
	assert(path.absolute());
	root->getNode(path.subpath(1))->data.resize(size);
}

void Tial::VFS::MemoryDriver::createFile(const Path &path) {
	LOGN2 << "Creating file " << path;
	assert(path.absolute());
	root->createNode(path.subpath(1), false);
}

void Tial::VFS::MemoryDriver::removeFile(const Path &path) {
	LOGN2 << "Removing file " << path;
	assert(path.absolute());
	if(root->getNode(path.subpath(1))->directory)
		THROW Exceptions::ElementKindInvalid(path, "expected file");
	root->removeNode(path.subpath(1));
}

void Tial::VFS::MemoryDriver::createDirectory(const Path &path) {
	LOGN2 << "Creating directory " << path;
	assert(path.absolute());
	root->createNode(path.subpath(1), true);
}

void Tial::VFS::MemoryDriver::removeDirectory(const Path &path) {
	LOGN2 << "Removing directory " << path;
	assert(path.absolute());
	if(!root->getNode(path.subpath(1))->directory)
		THROW Exceptions::ElementKindInvalid(path, "expected directory");
	root->removeNode(path.subpath(1));
}

std::shared_ptr<Tial::VFS::Driver::OpenFile> Tial::VFS::MemoryDriver::open(const Path &path) {
	LOGN2 << "Opening file " << path;
	assert(path.absolute());
	auto node = root->getNode(path.subpath(1));
	if(node->directory)
		THROW Exceptions::ElementKindInvalid(path, "expected file");
	return std::shared_ptr<MemoryOpenFile>(new MemoryOpenFile(node));
}

std::shared_ptr<Tial::VFS::Driver::MappedFile> Tial::VFS::MemoryDriver::map(const Path &path) {
	LOGN2 << "Mapping file " << path;
	assert(path.absolute());
	auto node = root->getNode(path.subpath(1));
	if(node->directory)
		THROW Exceptions::ElementKindInvalid(path, "expected file");
	if(!node->mapping)
		node->mapping.reset(new MemoryMappedFile(node));
	return node->mapping;
}
