#include <TialTesting/TialTesting.hpp>

#include <TialUtility/TialUtility.hpp>
#include <TialVFS/TialVFS.hpp>

#include <cstring>
#include <thread>
#include <boost/algorithm/string.hpp>

[[Tial::Testing::Typedef]] namespace Testing = Tial::Testing;
[[Tial::Testing::Typedef]] namespace Check = Tial::Testing::Check;

namespace [[Testing::Suite]] Tial {
namespace [[Testing::Suite]] VFS {
namespace [[Testing::Suite]] Test {

using namespace std::literals;

auto sortFiles = [](
	const std::shared_ptr<Tial::VFS::Object> &first,
	const std::shared_ptr<Tial::VFS::Object> &second
){
	return first->path() < second->path();
};

class MountPointWrapper {
private:
	std::shared_ptr<Tial::VFS::Root> _root;
	std::shared_ptr<Tial::VFS::Directory> mountPoint;
public:
	MountPointWrapper(
		std::shared_ptr<Tial::VFS::Root> root,
		std::shared_ptr<Tial::VFS::Directory> mountPoint
	): _root(root), mountPoint(mountPoint) {}

	std::shared_ptr<Tial::VFS::Root> root() {
		return _root;
	}

	std::shared_ptr<Tial::VFS::Directory> get() {
		return mountPoint;
	}

	Tial::VFS::Directory *operator->() {
		return mountPoint.get();
	}
};

template<typename Case>
class [[Testing::CaseBase]] VFS {
	static void driverTestCreateRemoveDirectories(MountPointWrapper root) {
		[[Check::NoThrow]] root->createDirectory("a");
		[[Check::NoThrow]] root->createDirectory("b");
		[[Check::Throw(Exceptions::ElementAlreadyExists)]] root->createDirectory("a");
		[[Check::Throw(Exceptions::ElementAlreadyExists)]] root->createDirectory("b");
		[[Check::NoThrow]] root->get<Tial::VFS::Directory>("a")->remove();
		[[Check::NoThrow]] root->get<Tial::VFS::Directory>("b")->remove();
	}

	static void driverTestCreateRemoveFiles(MountPointWrapper root) {
		[[Check::NoThrow]] root->createFile("a");
		[[Check::NoThrow]] root->createFile("b");
		[[Check::Throw(Exceptions::ElementAlreadyExists)]] root->createFile("a");
		[[Check::Throw(Exceptions::ElementAlreadyExists)]] root->createFile("b");
		[[Check::NoThrow]] root->get<Tial::VFS::File>("a")->remove();
		[[Check::NoThrow]] root->get<Tial::VFS::File>("b")->remove();
	}

	static void _driverTestListCreateSample(
		const std::shared_ptr<Tial::VFS::Directory> &root,
		int recursionLevel = 0,
		const std::string &namePrefix = std::string()
	) {
		auto content = [[Check::NoThrow]] root->content();
		std::sort(content.begin(), content.end(), sortFiles);
		[[Check::Verify]] content.size() == 0u;

		auto a = [[Check::NoThrow]] root->createDirectory(namePrefix+"a");
		content = [[Check::NoThrow]] root->content();
		std::sort(content.begin(), content.end(), sortFiles);
		[[Check::Verify]] content.size() == 1u;
		[[Check::Verify]] (content[0]->path()) == (root->path()/(namePrefix+"a"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[0]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[0]) != nullptr);

		auto b = [[Check::NoThrow]] root->createDirectory(namePrefix+"b");
		content = [[Check::NoThrow]] root->content();
		std::sort(content.begin(), content.end(), sortFiles);
		[[Check::Verify]] content.size() == 2u;
		[[Check::Verify]] (content[0]->path()) == (root->path()/(namePrefix+"a"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[0]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[0]) != nullptr);
		[[Check::Verify]] (content[1]->path()) == (root->path()/(namePrefix+"b"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[1]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[1]) != nullptr);

		auto c = [[Check::NoThrow]] root->createDirectory(namePrefix+"c");
		content = [[Check::NoThrow]] root->content();
		std::sort(content.begin(), content.end(), sortFiles);
		[[Check::Verify]] content.size() == 3u;
		[[Check::Verify]] (content[0]->path()) == (root->path()/(namePrefix+"a"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[0]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[0]) != nullptr);
		[[Check::Verify]] (content[1]->path()) == (root->path()/(namePrefix+"b"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[1]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[1]) != nullptr);
		[[Check::Verify]] (content[2]->path()) == (root->path()/(namePrefix+"c"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[2]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[2]) != nullptr);

		auto d = [[Check::NoThrow]] root->createDirectory(namePrefix+"d");
		content = [[Check::NoThrow]] root->content();
		std::sort(content.begin(), content.end(), sortFiles);
		[[Check::Verify]] content.size() == 4u;
		[[Check::Verify]] (content[0]->path()) == (root->path()/(namePrefix+"a"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[0]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[0]) != nullptr);
		[[Check::Verify]] (content[1]->path()) == (root->path()/(namePrefix+"b"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[1]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[1]) != nullptr);
		[[Check::Verify]] (content[2]->path()) == (root->path()/(namePrefix+"c"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[2]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[2]) != nullptr);
		[[Check::Verify]] (content[3]->path()) == (root->path()/(namePrefix+"d"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[3]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[3]) != nullptr);

		auto e = [[Check::NoThrow]] root->createFile(namePrefix+"e");
		content = [[Check::NoThrow]] root->content();
		std::sort(content.begin(), content.end(), sortFiles);
		[[Check::Verify]] content.size() == 5u;
		[[Check::Verify]] (content[0]->path()) == (root->path()/(namePrefix+"a"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[0]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[0]) != nullptr);
		[[Check::Verify]] (content[1]->path()) == (root->path()/(namePrefix+"b"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[1]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[1]) != nullptr);
		[[Check::Verify]] (content[2]->path()) == (root->path()/(namePrefix+"c"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[2]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[2]) != nullptr);
		[[Check::Verify]] (content[3]->path()) == (root->path()/(namePrefix+"d"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[3]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[3]) != nullptr);
		[[Check::Verify]] (content[4]->path()) == (root->path()/(namePrefix+"e"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[4]) != nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[4]) == nullptr);

		auto f = [[Check::NoThrow]] root->createFile(namePrefix+"f");
		content = [[Check::NoThrow]] root->content();
		std::sort(content.begin(), content.end(), sortFiles);
		[[Check::Verify]] content.size() == 6u;
		[[Check::Verify]] (content[0]->path()) == (root->path()/(namePrefix+"a"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[0]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[0]) != nullptr);
		[[Check::Verify]] (content[1]->path()) == (root->path()/(namePrefix+"b"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[1]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[1]) != nullptr);
		[[Check::Verify]] (content[2]->path()) == (root->path()/(namePrefix+"c"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[2]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[2]) != nullptr);
		[[Check::Verify]] (content[3]->path()) == (root->path()/(namePrefix+"d"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[3]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[3]) != nullptr);
		[[Check::Verify]] (content[4]->path()) == (root->path()/(namePrefix+"e"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[4]) != nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[4]) == nullptr);
		[[Check::Verify]] (content[5]->path()) == (root->path()/(namePrefix+"f"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[5]) != nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[5]) == nullptr);

		auto g = [[Check::NoThrow]] root->createFile(namePrefix+"g");
		content = [[Check::NoThrow]] root->content();
		std::sort(content.begin(), content.end(), sortFiles);
		[[Check::Verify]] content.size() == 7u;
		[[Check::Verify]] (content[0]->path()) == (root->path()/(namePrefix+"a"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[0]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[0]) != nullptr);
		[[Check::Verify]] (content[1]->path()) == (root->path()/(namePrefix+"b"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[1]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[1]) != nullptr);
		[[Check::Verify]] (content[2]->path()) == (root->path()/(namePrefix+"c"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[2]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[2]) != nullptr);
		[[Check::Verify]] (content[3]->path()) == (root->path()/(namePrefix+"d"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[3]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[3]) != nullptr);
		[[Check::Verify]] (content[4]->path()) == (root->path()/(namePrefix+"e"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[4]) != nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[4]) == nullptr);
		[[Check::Verify]] (content[5]->path()) == (root->path()/(namePrefix+"f"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[5]) != nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[5]) == nullptr);
		[[Check::Verify]] (content[6]->path()) == (root->path()/(namePrefix+"g"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[6]) != nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[6]) == nullptr);

		auto h = [[Check::NoThrow]] root->createFile(namePrefix+"h");
		content = [[Check::NoThrow]] root->content();
		std::sort(content.begin(), content.end(), sortFiles);
		[[Check::Verify]] content.size() == 8u;
		[[Check::Verify]] (content[0]->path()) == (root->path()/(namePrefix+"a"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[0]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[0]) != nullptr);
		[[Check::Verify]] (content[1]->path()) == (root->path()/(namePrefix+"b"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[1]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[1]) != nullptr);
		[[Check::Verify]] (content[2]->path()) == (root->path()/(namePrefix+"c"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[2]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[2]) != nullptr);
		[[Check::Verify]] (content[3]->path()) == (root->path()/(namePrefix+"d"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[3]) == nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[3]) != nullptr);
		[[Check::Verify]] (content[4]->path()) == (root->path()/(namePrefix+"e"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[4]) != nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[4]) == nullptr);
		[[Check::Verify]] (content[5]->path()) == (root->path()/(namePrefix+"f"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[5]) != nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[5]) == nullptr);
		[[Check::Verify]] (content[6]->path()) == (root->path()/(namePrefix+"g"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[6]) != nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[6]) == nullptr);
		[[Check::Verify]] (content[7]->path()) == (root->path()/(namePrefix+"h"));
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::File>(content[7]) != nullptr);
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(content[7]) == nullptr);

		auto i = [[Check::NoThrow]] content.cbegin();
		[[Check::Verify]] ((*i)->path()) == (root->path()/(namePrefix+"a"));
		[[Check::Verify]] i != content.cend();
		[[Check::NoThrow]] ++i;
		[[Check::Verify]] ((*i)->path()) == (root->path()/(namePrefix+"b"));
		[[Check::Verify]] i != content.cend();
		[[Check::NoThrow]] i++;
		[[Check::Verify]] ((*i)->path()) == (root->path()/(namePrefix+"c"));
		[[Check::Verify]] i != content.cend();
		[[Check::NoThrow]] ++i;
		[[Check::Verify]] ((*i)->path()) == (root->path()/(namePrefix+"d"));
		[[Check::Verify]] i != content.cend();
		[[Check::NoThrow]] i++;
		[[Check::Verify]] ((*i)->path()) == (root->path()/(namePrefix+"e"));
		[[Check::Verify]] i != content.cend();
		[[Check::NoThrow]] ++i;
		[[Check::Verify]] ((*i)->path()) == (root->path()/(namePrefix+"f"));
		[[Check::Verify]] i != content.cend();
		[[Check::NoThrow]] i++;
		[[Check::Verify]] ((*i)->path()) == (root->path()/(namePrefix+"g"));
		[[Check::Verify]] i != content.cend();
		[[Check::NoThrow]] ++i;
		[[Check::Verify]] ((*i)->path()) == (root->path()/(namePrefix+"h"));
		[[Check::Verify]] i != content.cend();
		[[Check::NoThrow]] i++;
		[[Check::Verify]] i == content.cend();

		[[Check::Verify]] (root->get<Tial::VFS::Object>(namePrefix+"a")->path()) == root->path()/(namePrefix+"a");
		[[Check::Verify]] (root->get<Tial::VFS::Object>(namePrefix+"b")->path()) == root->path()/(namePrefix+"b");
		[[Check::Verify]] (root->get<Tial::VFS::Object>(namePrefix+"c")->path()) == root->path()/(namePrefix+"c");
		[[Check::Verify]] (root->get<Tial::VFS::Object>(namePrefix+"d")->path()) == root->path()/(namePrefix+"d");
		[[Check::Verify]] (root->get<Tial::VFS::Object>(namePrefix+"e")->path()) == root->path()/(namePrefix+"e");
		[[Check::Verify]] (root->get<Tial::VFS::Object>(namePrefix+"f")->path()) == root->path()/(namePrefix+"f");
		[[Check::Verify]] (root->get<Tial::VFS::Object>(namePrefix+"g")->path()) == root->path()/(namePrefix+"g");
		[[Check::Verify]] (root->get<Tial::VFS::Object>(namePrefix+"h")->path()) == root->path()/(namePrefix+"h");
		[[Check::Throw(Exceptions::ElementNotFound)]] root->get<Tial::VFS::Object>(namePrefix+"i");

		if(recursionLevel > 0) {
			_driverTestListCreateSample(a, recursionLevel-1, namePrefix+"a");
			_driverTestListCreateSample(b, recursionLevel-1, namePrefix+"b");
			_driverTestListCreateSample(c, recursionLevel-1, namePrefix+"c");
			_driverTestListCreateSample(d, recursionLevel-1, namePrefix+"d");
		}
	}

	static void _driverTestListRemoveSample(
		const std::shared_ptr<Tial::VFS::Directory> &root
	) {
		[[Check::NoThrow]] root->get<Tial::VFS::Object>("a")->remove();
		[[Check::NoThrow]] root->get<Tial::VFS::Object>("b")->remove();
		[[Check::NoThrow]] root->get<Tial::VFS::Object>("c")->remove();
		[[Check::NoThrow]] root->get<Tial::VFS::Object>("d")->remove();
		[[Check::NoThrow]] root->get<Tial::VFS::Object>("e")->remove();
		[[Check::NoThrow]] root->get<Tial::VFS::Object>("f")->remove();
		[[Check::NoThrow]] root->get<Tial::VFS::Object>("g")->remove();
		[[Check::NoThrow]] root->get<Tial::VFS::Object>("h")->remove();
	}

	static void driverTestList(MountPointWrapper root) {
		_driverTestListCreateSample(root.get());
		_driverTestListRemoveSample(root.get());
	}

	static void driverTestListByWildcards(MountPointWrapper root) {
		auto china = [[Check::NoThrow]] root->createDirectory("China");
			[[Check::NoThrow]] china->createFile("Beijing");
			[[Check::NoThrow]] china->createFile("Nanjing");
			[[Check::NoThrow]] china->createFile("Dalian");
			[[Check::NoThrow]] china->createFile("Dandong");
			[[Check::NoThrow]] china->createFile("Danyang");
			[[Check::NoThrow]] china->createFile("Daqing");
			[[Check::NoThrow]] china->createFile("Datong");
			[[Check::NoThrow]] china->createFile("Dengzhou");
			[[Check::NoThrow]] china->createFile("Dezhou");
			[[Check::NoThrow]] china->createFile("Dingzhou");
			[[Check::NoThrow]] china->createFile("Dongguan");
			[[Check::NoThrow]] china->createFile("Dongying");
			[[Check::NoThrow]] china->createFile("Qujing");

		auto newZealand = [[Check::NoThrow]] root->createDirectory("New Zealand");
			[[Check::NoThrow]] newZealand->createFile("Auckland");

		auto unitedStates = [[Check::NoThrow]] root->createDirectory("United States");
			auto california = [[Check::NoThrow]] unitedStates->createDirectory("California");
				[[Check::NoThrow]] california->createFile("Oakland");
				[[Check::NoThrow]] california->createFile("San Francisco");
			auto florida = [[Check::NoThrow]] unitedStates->createDirectory("Florida");
				[[Check::NoThrow]] florida->createFile("Orlando");
			auto oregon = [[Check::NoThrow]] unitedStates->createDirectory("Oregon");
				[[Check::NoThrow]] oregon->createFile("Portland");

		auto files = [[Check::NoThrow]] root->getAll("United States");
		[[Check::Verify]] files.size() == 1u;
		[[Check::Verify]] (files.at(0)->path()) == (root->path()/"United States");
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(files.at(0))) != nullptr;
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Root>(files.at(0))) == nullptr;

		files = [[Check::NoThrow]] (root->getAll("United States/California"));
		[[Check::Verify]] files.size() == 1u;
		[[Check::Verify]] (files.at(0)->path()) == (root->path()/"United States/California");
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(files.at(0))) != nullptr;
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Root>(files.at(0))) == nullptr;

		files = [[Check::NoThrow]] (root->getAll("United States/California/Oakland"));
		[[Check::Verify]] files.size() == 1u;
		[[Check::Verify]] (files.at(0)->path()) == (root->path()/"United States/California/Oakland");
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Directory>(files.at(0))) == nullptr;
		[[Check::Verify]] (std::dynamic_pointer_cast<Tial::VFS::Root>(files.at(0))) == nullptr;

		files = [[Check::NoThrow]] (root->getAll("China/*jing"));
		[[Check::Verify]] files.size() == 3u;
		std::sort(files.begin(), files.end(), sortFiles);
		[[Check::Verify]] (files.at(0)->path()) == (root->path()/"China/Beijing");
		[[Check::Verify]] (files.at(1)->path()) == (root->path()/"China/Nanjing");
		[[Check::Verify]] (files.at(2)->path()) == (root->path()/"China/Qujing");

		files = [[Check::NoThrow]] (root->getAll("China/???jing"));
		[[Check::Verify]] files.size() == 2u;
		std::sort(files.begin(), files.end(), sortFiles);
		[[Check::Verify]] (files.at(0)->path()) == (root->path()/"China/Beijing");
		[[Check::Verify]] (files.at(1)->path()) == (root->path()/"China/Nanjing");

		files = [[Check::NoThrow]] (root->getAll("**/*land"));
		[[Check::Verify]] files.size() == 3u;
		std::sort(files.begin(), files.end(), sortFiles);
		[[Check::Verify]] (files.at(0)->path()) == (root->path()/"New Zealand/Auckland");
		[[Check::Verify]] (files.at(1)->path()) == (root->path()/"United States/California/Oakland");
		[[Check::Verify]] (files.at(2)->path()) == (root->path()/"United States/Oregon/Portland");

		for(auto i: china->content())
			[[Check::NoThrow]] i->remove();
		[[Check::NoThrow]] (china->remove());

		for(auto i: newZealand->content())
			[[Check::NoThrow]] i->remove();
		[[Check::NoThrow]] (newZealand->remove());

		for(auto i: unitedStates->content()) {
			for(auto j: std::dynamic_pointer_cast<Tial::VFS::Directory>(i)->content())
				[[Check::NoThrow]] j->remove();
			[[Check::NoThrow]] i->remove();
		}
		[[Check::NoThrow]] (unitedStates->remove());
	}

	static void driverTestInvalidateOnRemove(MountPointWrapper root) {
		[[Check::Verify]] (root->valid()) == Tial::VFS::Root::Validity::Invalid;
		[[Check::NoThrow]] root->createDirectory("Asia")->createDirectory("Indonesia");
		[[Check::Verify]] (root->valid()) == Tial::VFS::Root::Validity::Valid;

		// first, create simple structure
		auto asia1 = [[Check::NoThrow]] root->get<Tial::VFS::Directory>("Asia");
		[[Check::Verify]] (asia1->valid()) == Tial::VFS::Directory::Validity::Valid;
		auto content1 = [[Check::NoThrow]] asia1->content();
		[[Check::Verify]] (asia1->valid()) == Tial::VFS::Directory::Validity::Valid;
		std::sort(content1.begin(), content1.end(), sortFiles);
		[[Check::Verify]] content1.size() == 1u;
		auto indonesia1 = [[Check::NoThrow]] content1[0];
		[[Check::Verify]] (indonesia1->valid() == Tial::VFS::Directory::Validity::Invalid);
		[[Check::Verify]] (indonesia1->name() == "Indonesia");
		[[Check::Verify]] (indonesia1->valid() == Tial::VFS::Directory::Validity::Valid);

		// now, make it more complex, but using only temporary references
		[[Check::NoThrow]] root->get<Tial::VFS::Directory>("Asia")->createDirectory("East Timor");

		// reading now gives more entries
		[[Check::Verify]] (asia1->valid() == Tial::VFS::Directory::Validity::Valid);
		auto content2 = [[Check::NoThrow]] asia1->content();
		[[Check::Verify]] (asia1->valid() == Tial::VFS::Directory::Validity::Valid);
		std::sort(content2.begin(), content2.end(), sortFiles);
		[[Check::Verify]] content2.size() == 2u;
		[[Check::Verify]] (content2[0]->name()) == ("East Timor");
		[[Check::Verify]] (content2[1]->name()) == ("Indonesia");

		// but reading from old reference, gives more too
		[[Check::Verify]] (indonesia1->valid() == Tial::VFS::Directory::Validity::Valid);
		auto content3 = [[Check::NoThrow]] indonesia1->parent()->content();
		std::sort(content3.begin(), content3.end(), sortFiles);
		[[Check::Verify]] content3.size() == 2u;
		[[Check::Verify]] (content3[0]->name()) == ("East Timor");
		[[Check::Verify]] (content3[1]->name()) == ("Indonesia");

		// now, we remove what was referenced before
		[[Check::NoThrow]] root->get<Tial::VFS::Directory>("Asia")->get<Tial::VFS::Directory>("Indonesia")->remove();

		// and it disappears from existing references too
		[[Check::Verify]] (asia1->valid() == Tial::VFS::Directory::Validity::Invalid);
		auto content4 = [[Check::NoThrow]] asia1->content();
		[[Check::Verify]] (asia1->valid() == Tial::VFS::Directory::Validity::Valid);
		std::sort(content4.begin(), content4.end(), sortFiles);
		[[Check::Verify]] content4.size() == 1u;
		[[Check::Verify]] (content4[0]->name()) == ("East Timor");

		// and existing reference to removed does no longer work
		[[Check::Verify]] (indonesia1->valid() == Tial::VFS::Object::Validity::Broken);
		[[Check::Throw(Exceptions::ElementBroken)]] indonesia1->parent();
		[[Check::Throw(Exceptions::ElementBroken)]] indonesia1->name();
		[[Check::Throw(Exceptions::ElementBroken)]] indonesia1->path();

		// remove what's left
		[[Check::NoThrow]] root->get<Tial::VFS::Directory>("Asia/East Timor")->remove();
		[[Check::NoThrow]] root->get<Tial::VFS::Directory>("Asia")->remove();
	}

	class TestDriverInvalidable: public Tial::VFS::MemoryDriver {
	public:
		TestDriverInvalidable(): Tial::VFS::MemoryDriver("TestDriverInvalidable") {}

		void testDirectoryCreate() {
			root->createNode("foo", true);
			markInvalid("/");
		}

		void testDirectoryCreate2() {
			root->createNode("bar", true);
			markInvalid("/");
		}

		void testDirectoryRemove() {
			root->removeNode("foo");
			markBroken("/foo");
			markInvalid("/");
		}

		void testDirectoryRemove2() {
			root->removeNode("bar");
			markBroken("/bar");
			markInvalid("/");
		}
	};

	static void driverTestInvalidateOnDriverRequest(MountPointWrapper root) {
		// mount test driver
		[[Check::NoThrow]] root->unmount();
		auto testDriver = [[Check::NoThrow]] std::make_shared<TestDriverInvalidable>();
		[[Check::NoThrow]] root->mount(testDriver);

		// check invalidation signals that are coming from external sources
		[[Check::Verify]] (root->valid()) == Tial::VFS::Root::Validity::Invalid;
		auto content = [[Check::NoThrow]] root->content();
		[[Check::Verify]] (root->valid()) == Tial::VFS::Root::Validity::Valid;

		std::sort(content.begin(), content.end(), sortFiles);
		[[Check::Verify]] content.size() == 0u;

		[[Check::NoThrow]] testDriver->testDirectoryCreate();

		content = [[Check::NoThrow]] root->content();
		[[Check::NoThrow]] std::sort(content.begin(), content.end(), sortFiles);
		[[Check::Verify]] content.size() == 1u;
		auto foo = [[Check::NoThrow]] content[0];
		[[Check::Verify]] (foo->valid()) == Tial::VFS::Object::Validity::Invalid;
		[[Check::Verify]] (foo->name() == "foo");
		[[Check::Verify]] (foo->valid()) == Tial::VFS::Object::Validity::Valid;

		[[Check::NoThrow]] testDriver->testDirectoryCreate2();

		content = [[Check::NoThrow]] root->content();
		std::sort(content.begin(), content.end(), sortFiles);
		[[Check::Verify]] content.size() == 2u;
		auto bar = [[Check::NoThrow]] content[0];
		[[Check::Verify]] (bar->valid()) == Tial::VFS::Object::Validity::Invalid;
		[[Check::Verify]] (bar->name() == "bar");
		[[Check::Verify]] (bar->valid()) == Tial::VFS::Object::Validity::Valid;
		[[Check::Verify]] (content[1]->name()) == "foo";

		[[Check::NoThrow]] testDriver->testDirectoryRemove();

		content = [[Check::NoThrow]] root->content();
		std::sort(content.begin(), content.end(), sortFiles);
		[[Check::Verify]] content.size() == 1u;
		[[Check::Verify]] (content[0]->name()) == "bar";

		[[Check::Verify]] (foo->valid()) == Tial::VFS::Object::Validity::Broken;
		[[Check::Throw(Exceptions::ElementBroken)]] foo->parent();
		[[Check::Throw(Exceptions::ElementBroken)]] foo->name();
		[[Check::Throw(Exceptions::ElementBroken)]] foo->path();

		[[Check::NoThrow]] testDriver->testDirectoryRemove2();

		content = [[Check::NoThrow]] root->content();
		std::sort(content.begin(), content.end(), sortFiles);
		[[Check::Verify]] content.size() == 0u;

		[[Check::Verify]] (bar->valid()) == Tial::VFS::Object::Validity::Broken;
		[[Check::Throw(Exceptions::ElementBroken)]] bar->parent();
		[[Check::Throw(Exceptions::ElementBroken)]] bar->name();
		[[Check::Throw(Exceptions::ElementBroken)]] bar->path();
	}

	static void driverTestInvalidateOnParentRemoval(MountPointWrapper root) {
		//check invalidation on parent directory removal
		auto asia1 = [[Check::NoThrow]] root->createDirectory("Asia");
		auto indonesia1 = [[Check::NoThrow]] asia1->createDirectory("Indonesia");
		[[Check::Verify]] (indonesia1->valid()) == Tial::VFS::Object::Validity::Invalid;
		[[Check::Verify]] (indonesia1->content().size()) == 0u;
		[[Check::Verify]] (indonesia1->valid()) == Tial::VFS::Object::Validity::Valid;

		[[Check::NoThrow]] asia1->remove();

		auto asia2 = [[Check::NoThrow]] root->createDirectory("Asia");

		[[Check::Verify]] (indonesia1->valid()) == Tial::VFS::Object::Validity::Broken;
		[[Check::Throw(Exceptions::ElementBroken)]] indonesia1->parent();
		[[Check::Throw(Exceptions::ElementBroken)]] indonesia1->name();
		[[Check::Throw(Exceptions::ElementBroken)]] indonesia1->path();

		[[Check::NoThrow]] asia2->remove();
	}

	static void driverTestInvalidateOnUnmount(MountPointWrapper root) {
		auto memoryDriver = [[Check::NoThrow]] std::make_shared<Tial::VFS::MemoryDriver>();
		[[Check::Throw(Exceptions::AlreadyMounted)]] root->mount(memoryDriver);
		[[Check::NoThrow]] root->unmount();
		[[Check::NoThrow]] root->mount(memoryDriver);

		auto asia1 = [[Check::NoThrow]] root->createDirectory("Asia");
		auto indonesia1 = [[Check::NoThrow]] asia1->createDirectory("Indonesia");
		[[Check::Verify]] (indonesia1->valid()) == Tial::VFS::Object::Validity::Invalid;
		[[Check::Verify]] (indonesia1->content().size()) == 0u;
		[[Check::Verify]] (indonesia1->valid()) == Tial::VFS::Object::Validity::Valid;

		[[Check::Throw(Exceptions::NoMountPoint)]] indonesia1->unmount();
		[[Check::Throw(Exceptions::NoMountPoint)]] asia1->unmount();
		[[Check::NoThrow]] root->unmount();

		[[Check::Verify]] (asia1->valid()) == Tial::VFS::Object::Validity::Broken;
		[[Check::Throw(Exceptions::ElementBroken)]] asia1->parent();
		[[Check::Throw(Exceptions::ElementBroken)]] asia1->name();
		[[Check::Throw(Exceptions::ElementBroken)]] asia1->path();
		[[Check::Verify]] (indonesia1->valid()) == Tial::VFS::Object::Validity::Broken;
		[[Check::Throw(Exceptions::ElementBroken)]] indonesia1->parent();
		[[Check::Throw(Exceptions::ElementBroken)]] indonesia1->name();
		[[Check::Throw(Exceptions::ElementBroken)]] indonesia1->path();

		// remount does not recover an instance
		[[Check::NoThrow]] root->mount(memoryDriver);
		[[Check::Verify]] (asia1->valid()) == Tial::VFS::Object::Validity::Broken;
		[[Check::Throw(Exceptions::ElementBroken)]] asia1->parent();
		[[Check::Throw(Exceptions::ElementBroken)]] asia1->name();
		[[Check::Throw(Exceptions::ElementBroken)]] asia1->path();
		[[Check::Verify]] (indonesia1->valid()) == Tial::VFS::Object::Validity::Broken;
		[[Check::Throw(Exceptions::ElementBroken)]] indonesia1->parent();
		[[Check::Throw(Exceptions::ElementBroken)]] indonesia1->name();
		[[Check::Throw(Exceptions::ElementBroken)]] indonesia1->path();
	}

	static void verifyFileContent(const std::shared_ptr<Tial::VFS::File> &file, const std::string &expectedContent) {
		{ // check 1 - with stream
			auto stream = [[Check::NoThrow]] file->open();
			std::vector<char> string(file->size());
			stream.read(string.data(), string.size());
			[[Check::Verify]] std::string(string.begin(), string.end()) == expectedContent;
		}{ // check 2 - with mapping
			auto mapping = [[Check::NoThrow]] file->map();
			[[Check::Verify]] std::string(mapping.template as<char>(), mapping.size()) == expectedContent;
		}
	}

	static void driverTestOpenWriteRead(MountPointWrapper root, const std::vector<std::string> &filters) {
		for(const auto &filter: filters) {
			std::string fileName = "file"+(!filter.empty() ? "."+filter : "");

			// write
			root->createFile(fileName)->open() << "what is that...";
			verifyFileContent(root->get<Tial::VFS::File>(fileName), "what is that...");

			// overwrite
			root->get<Tial::VFS::File>(fileName)->open(8, std::ios_base::beg) << "this";
			verifyFileContent(root->get<Tial::VFS::File>(fileName), "what is this...");

			// append
			root->get<Tial::VFS::File>(fileName)->open(0, std::ios_base::end) << " I don't even";
			verifyFileContent(root->get<Tial::VFS::File>(fileName), "what is this... I don't even");

			// truncate
			{
				auto file = [[Check::NoThrow]] root->get<Tial::VFS::File>(fileName);
				[[Check::NoThrow]] (file->resize(file->size() - 13));
			}
			verifyFileContent(root->get<Tial::VFS::File>(fileName), "what is this...");

			root->get<Tial::VFS::File>(fileName)->remove();
		}
	}

	static void driverTestMapWriteRead(MountPointWrapper root, const std::vector<std::string> &filters) {
		for(const auto &filter: filters) {
			std::string fileName = "file" + (!filter.empty() ? "." + filter : "");

			// write
			{
				auto mapping = [[Check::NoThrow]] root->createFile(fileName)->map();
				[[Check::Verify]] mapping.size() == 0u;
				[[Check::NoThrow]] mapping.resize(15 * sizeof(char));
				[[Check::NoThrow]] memcpy(mapping.template as<char>(), "what is that...", 15 * sizeof(char));
			}
			verifyFileContent(root->get<Tial::VFS::File>(fileName), "what is that...");

			// overwrite
			{
				auto mapping = [[Check::NoThrow]] root->get<Tial::VFS::File>(fileName)->map();
				[[Check::Verify]] mapping.size() == 15 * sizeof(char);
				mapping.template as<char>()[10] = 'i';
				mapping.template as<char>()[11] = 's';
			}
			verifyFileContent(root->get<Tial::VFS::File>(fileName), "what is this...");

			// append
			{
				auto mapping = [[Check::NoThrow]] root->get<Tial::VFS::File>(fileName)->map();
				[[Check::Verify]] mapping.size() == 15 * sizeof(char);
				[[Check::NoThrow]] mapping.resize(mapping.size() + 13 * sizeof(char));
				[[Check::NoThrow]] memcpy(mapping.template as<char>() + 15 * sizeof(char), " I don't even", 13 * sizeof(char));
			}
			verifyFileContent(root->get<Tial::VFS::File>(fileName), "what is this... I don't even");

			// truncate
			{
				auto mapping = [[Check::NoThrow]] root->get<Tial::VFS::File>(fileName)->map();
				[[Check::Verify]] mapping.size() == 28 * sizeof(char);
				[[Check::NoThrow]] mapping.resize(mapping.size() - 13 * sizeof(char));
			}
			verifyFileContent(root->get<Tial::VFS::File>(fileName), "what is this...");

			root->get<Tial::VFS::File>(fileName)->remove();
		}
	}

	static void driverTestStreamFileObject(MountPointWrapper root, const std::vector<std::string> &filters) {
		for(const auto &filter: filters) {
			std::string fileName = "file" + (!filter.empty() ? "." + filter : "");

			// non-assigned object
			Tial::VFS::Stream stream;
			[[Check::Verify]] !stream.is_open();

			// create a file
			auto file = [[Check::NoThrow]] root->createFile(fileName);
			[[Check::Verify]] !stream.is_open();

			// assign stream object to file and write to it
			stream = [[Check::NoThrow]] file->open();
			[[Check::Verify]] stream.is_open();
			stream << "what is that...";
			stream.flush();
			verifyFileContent(file, "what is that...");

			// unassign stream
			stream = [[Check::NoThrow]] Tial::VFS::Stream();
			[[Check::Verify]] !stream.is_open();

			// reassign it to the same file
			stream = [[Check::NoThrow]] file->open();
			[[Check::Verify]] stream.is_open();

			// read data
			std::vector<char> string(file->size());
			stream.read(string.data(), string.size());
			[[Check::Verify]] std::string(string.begin(), string.end()) == "what is that...";

			file->remove();
		}
	}

	static void driverTestMappingFileObject(MountPointWrapper root, const std::vector<std::string> &filters) {
		for(const auto &filter: filters) {
			std::string fileName = "file" + (!filter.empty() ? "." + filter : "");

			// non-assigned object
			Tial::VFS::Mapping mapping;
			[[Check::Verify]] !mapping.assigned();
			[[Check::Verify]] !mapping;
			[[Check::Throw(Exceptions::UnassignedAccessor)]] mapping.size();
			[[Check::Throw(Exceptions::UnassignedAccessor)]] mapping.resize(13);
			[[Check::Throw(Exceptions::UnassignedAccessor)]] mapping.get();

			// create a file
			auto file = [[Check::NoThrow]] root->createFile(fileName);

			// assign mapping object to file and write to it
			mapping = [[Check::NoThrow]] file->map();
			[[Check::Verify]] mapping.assigned();
			[[Check::Verify]] mapping;
			[[Check::NoThrow]] mapping.resize(15);
			[[Check::NoThrow]] memcpy(mapping.as<char>(), "what is that...", 15 * sizeof(char));
			verifyFileContent(file, "what is that...");

			// unassign mapping
			mapping = [[Check::NoThrow]] Tial::VFS::Mapping();
			[[Check::Verify]] !mapping.assigned();
			[[Check::Verify]] !mapping;
			[[Check::Throw(Exceptions::UnassignedAccessor)]] mapping.size();
			[[Check::Throw(Exceptions::UnassignedAccessor)]] mapping.resize(13);
			[[Check::Throw(Exceptions::UnassignedAccessor)]] mapping.get();

			// reassign it to the same file
			mapping = [[Check::NoThrow]] file->map();

			// read data
			[[Check::Verify]] mapping.assigned();
			[[Check::Verify]] mapping;
			[[Check::Verify]] mapping.size() == 15u;
			[[Check::Verify]] std::string(mapping.as<char>(), mapping.size()) == "what is that...";

			file->remove();
		}
	}

	static void driverTestMultipleStreams(MountPointWrapper root, const std::vector<std::string> &filters) {
		for(const auto &filter: filters) {
			std::string fileName = "file" + (!filter.empty() ? "." + filter : "");

			auto file = [[Check::NoThrow]] root->createFile(fileName);
			Tial::VFS::Stream stream1 = [[Check::NoThrow]] file->open();
			Tial::VFS::Stream stream2 = [[Check::NoThrow]] file->open();

			stream1 << "what is that...";
			stream1.flush();
			verifyFileContent(file, "what is that...");

			stream2.seekg(0, std::ios_base::end);
			stream2 << " I don't even";
			stream2.flush();
			verifyFileContent(file, "what is that... I don't even");

			file->remove();
		}
	}

	static void driverTestMultipleMappings(MountPointWrapper root, const std::vector<std::string> &filters) {
		for(const auto &filter: filters) {
			std::string fileName = "file" + (!filter.empty() ? "." + filter : "");

			auto file = [[Check::NoThrow]] root->createFile(fileName);
			{
				auto map = [[Check::NoThrow]] file->map();
				[[Check::NoThrow]] map.resize(15);
				[[Check::Verify]] map.size() == 15u;
				memcpy(map.template as<char>(), "what is that...", 15 * sizeof(char));
				verifyFileContent(file, "what is that...");
			}
			verifyFileContent(file, "what is that...");
			{
				auto map = [[Check::NoThrow]] file->map();
				[[Check::Verify]] map.size() == 15u;
				[[Check::NoThrow]] map.resize(map.size() + 13);
				[[Check::Verify]] map.size() == 28u;
				memcpy(map.template as<char>() + 15u, " I don't even", 13 * sizeof(char));
			}
			verifyFileContent(file, "what is that... I don't even");

			[[Check::NoThrow]] file->resize(1);

			for(int i = 0; i < 10; ++i) {
				Testing::Thread first([&]() {
					for(int i = 0; i < 100; ++i) {
						{
							auto map = [[Check::NoThrow]] file->map();
							[[Check::NoThrow]] map.resize(0);
							std::this_thread::sleep_for(10us);
							[[Check::NoThrow]] map.resize(1);
							[[Check::NoThrow]] map.template as<char>()[0] = 'A';
							std::this_thread::sleep_for(20us);
							[[Check::Verify]] (map.template as<char>()[0]) == 'A';
						}
						std::this_thread::sleep_for(10us);
					}
				});

				Testing::Thread second([&]() {
					for(int i = 0; i < 100; ++i) {
						{
							auto map = [[Check::NoThrow]] file->map();
							[[Check::NoThrow]] map.resize(0);
							std::this_thread::sleep_for(10us);
							[[Check::NoThrow]] map.resize(1);
							[[Check::NoThrow]] map.template as<char>()[0] = 'B';
							std::this_thread::sleep_for(10us);
							[[Check::Verify]] (map.template as<char>()[0]) == 'B';
						}
						std::this_thread::sleep_for(10us);
					}
				});

				first("first");
				second("second");

				first.join();
				second.join();
			}

			file->remove();
		}
	}

	static void driverTestMutlipleStreamsMappings(MountPointWrapper root, const std::vector<std::string> &filters) {
		for(const auto &filter: filters) {
			std::string fileName = "file" + (!filter.empty() ? "." + filter : "");

			auto file = [[Check::NoThrow]] root->createFile(fileName);
			auto stream1 = [[Check::NoThrow]] file->open();
			auto stream2 = [[Check::NoThrow]] file->open();

			{
				auto map = [[Check::NoThrow]] file->map();
				[[Check::Verify]] map.assigned();
				[[Check::Verify]] map;
				[[Check::NoThrow]] map.resize(15);
				[[Check::Verify]] (file->size()) == 15u;
				[[Check::Verify]] (map.size()) == 15u;
				memcpy(map.template as<char>(), "what is that...", 15 * sizeof(char));
			}
			verifyFileContent(file, "what is that...");

			stream1.seekg(0, std::ios_base::end);
			stream1 << " I don't even";
			stream1.flush();
			verifyFileContent(file, "what is that... I don't even");

			{
				auto map = [[Check::NoThrow]] file->map();
				[[Check::NoThrow]] map.template as<char>()[10] = 'i';
				[[Check::NoThrow]] map.template as<char>()[11] = 's';
			}

			std::vector<char> string(file->size());
			stream2.read(string.data(), string.size());
			[[Check::Verify]] std::string(string.begin(), string.end()) == "what is this... I don't even";

			verifyFileContent(file, "what is this... I don't even");

			{
				auto map = [[Check::NoThrow]] file->map();
				[[Check::NoThrow]] map.resize(map.size() + 5 * sizeof(char));
				[[Check::NoThrow]] memcpy(map.template as<char>() + 28, " know", 5 * sizeof(char));
			}

			std::vector<char> string2(file->size());
			stream2.seekg(0, stream2.beg);
			stream2.read(string2.data(), string2.size());
			[[Check::Verify]] std::string(string2.begin(), string2.end()) == "what is this... I don't even know";

			file->remove();
		}
	}

	static void driverTestFilter(MountPointWrapper root, const std::string &filter, const std::string &encoded) {
		std::string fileName = "file" + (!filter.empty() ? "." + filter : "");
		{
			auto file = [[Check::NoThrow]] root->createFile(fileName);
			file.open() << "what is this?";

			{
				auto stream = [[Check::NoThrow]] file->open(0, std::ios_base::begin, Tial::VFS::File::OpenFlag::Raw);
				std::vector<char> string(file->size());
				stream.read(string.data(), string.size());
				[[Check::Verify]] std::string(string.begin(), string.end()) == encoded;
			}{
				auto map = [[Check::NoThrow]] file->map(Tial::VFS::File::MapFlag::Raw);
				[[Check::Verify]] std::experimental::string_view(map.template as<char>(), map.size()) == encoded;
			}

			file->remove();
		}{
			auto file = [[Check::NoThrow]] root->createFile(fileName);
			{
				auto map = [[Check::NoThrow]] file->map();
			}

			{
				auto stream = [[Check::NoThrow]] file->open(0, std::ios_base::begin, Tial::VFS::File::OpenFlag::Raw);
				std::vector<char> string(file->size());
				stream.read(string.data(), string.size());
				[[Check::Verify]] std::string(string.begin(), string.end()) == encoded;
			}{
				auto map = [[Check::NoThrow]] file->map();
				[[Check::Verify]] std::experimental::string_view(map.template as<char>(), map.size()) == encoded;
			}

			file->remove();
		}
	}

	static void driverTestComplexStructure(MountPointWrapper root) {
		_driverTestListCreateSample(root.get(), 3);

		{
			auto all = [[Check::NoThrow]] root->collect();
			for(auto &obj: all) {
				if(auto file = std::dynamic_pointer_cast<Tial::VFS::File>(obj)) {
					file->open() << boost::to_upper_copy(file->name());
				}
			}
		}{
			auto all = [[Check::NoThrow]] root->collect();
			for(auto &obj: all) {
				if(auto file = std::dynamic_pointer_cast<Tial::VFS::File>(obj)) {
					auto mapping = [[Check::NoThrow]] file->map();
					std::vector<char> data(mapping.size());
					[[Check::NoThrow]] memcpy(data.data(), mapping.template as<char>(), data.size());
					std::string string(data.begin(), data.end());
					[[Check::Verify]] string == boost::to_upper_copy(file->name());
					boost::to_lower(string);
					[[Check::NoThrow]] memcpy(mapping.template as<void>(), string.data(), mapping.size());
				}
			}
		}{
			auto all = [[Check::NoThrow]] root->collect();
			for(auto &obj: all) {
				if(auto file = std::dynamic_pointer_cast<Tial::VFS::File>(obj)) {
					auto open = [[Check::NoThrow]] file->open();
					std::vector<char> data(file->size());
					open.read(data.data(), data.size());
					std::string string(data.begin(), data.end());
					[[Check::Verify]] string == boost::to_lower_copy(file->name());
					boost::to_upper(string);
					open.seekg(0, open.beg);
					open.write(string.data(), string.size());
				}
			}
		}{
			auto all = [[Check::NoThrow]] root->collect();
			for(auto &obj: all) {
				if(auto file = std::dynamic_pointer_cast<Tial::VFS::File>(obj)) {
					auto mapping = [[Check::NoThrow]] file->map();
					std::vector<char> data(mapping.size());
					[[Check::NoThrow]] memcpy(data.data(), mapping.template as<void>(), data.size());
					std::string string(data.begin(), data.end());
					[[Check::Verify]] string == boost::to_upper_copy(file->name());
				}
			}
		}

		_driverTestListRemoveSample(root.get());
	}

	static void driverTests(std::function<MountPointWrapper()> initFunction) {
		Tial::Utility::Logger::setLoggingLevel(Tial::Utility::Logger::Level::Info, "Tial::Utility::Path");
		Tial::Utility::Logger::setLoggingLevel(Tial::Utility::Logger::Level::Info, "Tial::Utility::Wildcards");

		auto filters = {""s, "xz"s};

		driverTestCreateRemoveDirectories(initFunction());
		driverTestCreateRemoveFiles(initFunction());
		driverTestList(initFunction());
		driverTestListByWildcards(initFunction());
		driverTestInvalidateOnRemove(initFunction());
		driverTestInvalidateOnDriverRequest(initFunction());
		driverTestInvalidateOnParentRemoval(initFunction());
		driverTestInvalidateOnUnmount(initFunction());
		driverTestOpenWriteRead(initFunction(), filters);
		driverTestMapWriteRead(initFunction(), filters);
		driverTestStreamFileObject(initFunction(), filters);
		driverTestMappingFileObject(initFunction(), filters);
		driverTestMultipleStreams(initFunction(), filters);
		driverTestMultipleMappings(initFunction(), filters);
		driverTestMutlipleStreamsMappings(initFunction(), filters);
		driverTestFilter(initFunction(), "", "what is this?");
		dirverTestFilter(initFunction(), "xz", "<COMPRESSED>");

		driverTestComplexStructure(initFunction());
	}

	template<typename DriverClass, typename... Args>
	static MountPointWrapper driverTestInit(const Tial::VFS::Path &path, Args&&... args) {
		auto root = [[Check::NoThrow]] std::make_shared<Tial::VFS::Root>();
		std::shared_ptr<Tial::VFS::Directory> dir = root;
		if(!path.empty()) {
			[[Check::NoThrow]] dir->mount(std::make_shared<Tial::VFS::MemoryDriver>());
			for(auto i: path)
				dir = dir->createDirectory(i);
		}
		[[Check::NoThrow]] dir->mount(std::make_shared<DriverClass>(std::forward<Args>(args)...));
		return MountPointWrapper(root, dir);
	}
};

class [[Testing::Case]] MemoryDriver: public VFS<MemoryDriver> {
	void operator()() {
		driverTests(std::bind(driverTestInit<Tial::VFS::MemoryDriver>, ""));
		driverTests(std::bind(driverTestInit<Tial::VFS::MemoryDriver>, "mnt/test"));
	}
};

class [[Testing::Case]] NativeFsDriver: public VFS<NativeFsDriver> {
	void operator()() {
		driverTests(std::bind(
			driverTestInit<Tial::VFS::NativeFSDriver, const Tial::Utility::NativePath &>,
			"",
			Tial::Utility::NativeDirectory::current().path()/"testspace"
		));
		driverTests(std::bind(
			driverTestInit<Tial::VFS::NativeFSDriver, const Tial::Utility::NativePath &>,
			"mnt/test",
			Tial::Utility::NativeDirectory::current().path()/"testspace"
		));
		driverTests(std::bind(
			driverTestInit<Tial::VFS::NativeFSDriver, const Tial::Utility::NativePath &>,
			"",
			"testspace"
		));
	}
};

}
}
}
