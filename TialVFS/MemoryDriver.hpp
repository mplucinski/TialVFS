#pragma once
#include "TialVFSExport.hpp"

#include <memory>
#include <unordered_map>

#include "Driver.hpp"

namespace Tial {
namespace VFS {

class TIALVFS_EXPORT MemoryDriver: public Driver {
protected:
	class MemoryMappedFile;

	class Node {
		bool directory = false;
		std::unordered_map<std::string, std::shared_ptr<Node>> elements;
		std::vector<uint8_t> data;
		std::shared_ptr<MemoryMappedFile> mapping;
	public:
		explicit Node(bool directory);
		FileEntry get(const Path &path);
		std::shared_ptr<Node> getNode(const Path &path);
		std::vector<FileEntry> listDirectory(const Path &path);
		void createNode(const Path &path, bool directory);
		void removeNode(const Path &path);

		friend class MemoryOpenFile;
		friend class MemoryDriver;
	};

	std::shared_ptr<Node> root = std::make_shared<Node>(true);

	class MemoryOpenFile: public Driver::OpenFile {
		std::shared_ptr<Node> node;

		explicit MemoryOpenFile(const std::shared_ptr<Node> &node);
	public:
		virtual size_t read(size_t pos, void *buffer, size_t bufferSize) override;
		virtual size_t write(size_t pos, const void *buffer, size_t bufferSize) override;
		virtual size_t size() override;

		friend class MemoryDriver;
	};

	class MemoryMappedFile: public Driver::MappedFile {
		std::weak_ptr<Node> node;

		explicit MemoryMappedFile(const std::weak_ptr<Node> &node);
	public:
		virtual void *get() override;
		virtual size_t size() override;
		virtual void resize(size_t size) override;

		friend class MemoryDriver;
	};

public:
	MemoryDriver(const std::string &name = "memory");
	virtual FileEntry get(const Path &path) override;
	virtual std::vector<FileEntry> listDirectory(const Path &path) override;
	virtual uintmax_t size(const Path &path) override;
	virtual void resize(const Path &path, uintmax_t size) override;
	virtual void createFile(const Path &path) override;
	virtual void removeFile(const Path &path) override;
	virtual void createDirectory(const Path &path) override;
	virtual void removeDirectory(const Path &path) override;
	virtual std::shared_ptr<OpenFile> open(const Path &path) override;
	virtual std::shared_ptr<MappedFile> map(const Path &path) override;
};

}
}
