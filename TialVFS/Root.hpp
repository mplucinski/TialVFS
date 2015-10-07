#pragma once
#include "TialVFSExport.hpp"

#include <unordered_map>

#include "Directory.hpp"
#include "Driver.hpp"

namespace Tial {
namespace VFS {

class TIALVFS_EXPORT Root: public Directory {
public:
	friend class Directory;

	Root();

	virtual Path path() const override;
	virtual std::shared_ptr<Root> root() override;
	virtual std::shared_ptr<Directory> parent() const override;
};

}
}
