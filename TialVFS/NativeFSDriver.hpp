#pragma once
#include "TialVFSExport.hpp"

#include "Driver.hpp"
#include "Exception.hpp"
#include <TialUtility/TialUtility.hpp>

#include <unordered_map>

#include <boost/predef.h>

#if BOOST_OS_WINDOWS
#define NOMINMAX
#include <Windows.h>
#endif

#define TIAL_MODULE "Tial::VFS::NativeFSDriver"

namespace Tial {
namespace VFS {

class TIALVFS_EXPORT NativeFSDriver: public Driver {
	Utility::NativePath nativeDirectory;

	static std::string _prepareName(const std::string &name, const Utility::NativePath nativeDirectory);
	class NativeFileDescriptor {
	protected:
		std::shared_ptr<NativeFSDriver> driver;
		Utility::NativePath nativePath;
#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
		int fd = -1;
#elif BOOST_OS_WINDOWS
        HANDLE handle = INVALID_HANDLE_VALUE;
#else
#error "Platform not supported"
#endif

		NativeFileDescriptor(
			const std::shared_ptr<NativeFSDriver> &driver,
			const Utility::NativePath &nativePath,
			bool openAutomatically = true
		);
		virtual ~NativeFileDescriptor();
		virtual void descriptorOpen();
		virtual void descriptorClose();
		size_t size() const;

		friend class NativeFSDriver;
	};

	std::recursive_mutex openDescriptorsMutex;
	std::unordered_multimap<Utility::NativePath, std::weak_ptr<NativeFileDescriptor>> openDescriptors;

	void cleanupOpenDescriptors();
	uintmax_t sizeNative(const Path &path);
	void resizeNative(const Utility::NativePath &path, uintmax_t size);

	template<typename ExpectedType>
	std::shared_ptr<ExpectedType> descriptor(const Utility::NativePath &path) {
		std::unique_lock<std::recursive_mutex> lock(openDescriptorsMutex);
		assert(path.absolute());
		cleanupOpenDescriptors();

		std::shared_ptr<ExpectedType> result;

		auto its = openDescriptors.equal_range(path);
		for(auto it = its.first; it != its.second; ++it) {
			result = std::dynamic_pointer_cast<ExpectedType>(it->second.lock());
			if(result) {
				LOGN1 << "Found existing descriptor";
				break;
			}
		}

		if(!result) {
			result = std::make_shared<ExpectedType>(
				std::dynamic_pointer_cast<NativeFSDriver>(shared_from_this()), path
			);
			openDescriptors.insert(std::make_pair(path, result));
		}
		return result;
	}

	class NativeOpenFile: public NativeFileDescriptor, public Driver::OpenFile {
		void seek(size_t pos);
	public:
		explicit NativeOpenFile(const std::shared_ptr<NativeFSDriver> &driver, const Utility::NativePath &nativePath);
		virtual ~NativeOpenFile() override;
		virtual size_t read(size_t pos, void *buffer, size_t bufferSize) override;
		virtual size_t write(size_t pos, const void *buffer, size_t bufferSize) override;
		virtual size_t size() override;

		friend class NativeFSDriver;
	};

	class NativeMappedFile: public NativeFileDescriptor, public Driver::MappedFile {
	public:
        void *_ptr = nullptr;
#if (BOOST_OS_UNIX || BOOST_OS_MACOS)
		uintmax_t _size = 0;
#elif BOOST_OS_WINDOWS
        HANDLE mapping = nullptr;
#else
#error "Platform not supported"
#endif

		virtual void descriptorClose() override;
		virtual void descriptorOpen() override;
		explicit NativeMappedFile(const std::shared_ptr<NativeFSDriver> &driver, const Utility::NativePath &nativePath);
		virtual ~NativeMappedFile() override;
		virtual void *get() override;
		virtual size_t size() override;
		virtual void resize(size_t size) override;

		friend class NativeFSDriver;
	};

public:
	explicit NativeFSDriver(const Utility::NativePath &nativeDirectory, const std::string &name = std::string());
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

#undef TIAL_MODULE
