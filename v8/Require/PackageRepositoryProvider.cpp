#include "PackageRepositoryProvider.h"
#include <sm_platform.h>
#include <fstream>

namespace SMV8
{
	namespace Require
	{
		namespace Providers
		{
			using namespace SourceMod;
			using namespace std;
			PackageRepositoryProvider::PackageRepositoryProvider(ISourceMod *sm, DependencyManager *depMan)
				: sm(sm), depMan(depMan)
			{
			}

			PackageRepositoryProvider::~PackageRepositoryProvider(void)
			{
			}

			std::string PackageRepositoryProvider::ResolvePath(const SMV8Script& requirer, const string& path) const
			{
				string package = requirer.GetVersionedPackage();
				string target_dir = package_dir + depMan->ResolvePath(package,path);

				char fullpath[PLATFORM_MAX_PATH];
				sm->BuildPath(Path_SM, fullpath, sizeof(fullpath), target_dir.c_str());

				return fullpath;
			}

			bool PackageRepositoryProvider::Provides(const SMV8Script& requirer, const string& path) const
			{
				if(!requirer.IsInPackage())
					return false;

				ifstream ifs(ResolvePath(requirer, path));
				return ifs.is_open();
			}
			
			string PackageRepositoryProvider::Require(const SMV8Script& requirer, const string& path) const
			{
				if(!requirer.IsInPackage())
					return false;

				string fullpath(ResolvePath(requirer, path));
				ifstream ifs(fullpath);
				if(!ifs.is_open())
					throw logic_error("Can't open required path " + path + ". This should not happen.");

				return fullpath;
			}

			std::string PackageRepositoryProvider::GetName() const
			{
				return "Package repository";
			}
		}
	}
}
