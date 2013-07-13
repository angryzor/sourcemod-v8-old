#include "PackageRepositoryProvider.h"
#include <sm_platform.h>
#include <fstream>


namespace SMV8
{
	namespace Require
	{
		namespace Providers
		{
			using namespace std;
			PackageRepositoryProvider::PackageRepositoryProvider(ISourceMod *sm)
				: sm(sm), directory(directory)
			{
			}

			PackageRepositoryProvider::~PackageRepositoryProvider(void)
			{
			}

			bool PackageRepositoryProvider::Provides(const string& requirer, const string& path) const
			{
				char fullpath[PLATFORM_MAX_PATH];
				sm->BuildPath(Path_SM, fullpath, sizeof(fullpath), (string("v8/packages/") + path).c_str());

				ifstream ifs(fullpath);
				return ifs;
			}
			
			string PackageRepositoryProvider::Require(const string& requirer, const string& path) const
			{
				string requirer_dir = requirer.substr(0,requirer.find_last_of('/') + 1);

				char fullpath[PLATFORM_MAX_PATH];
				sm->BuildPath(Path_SM, fullpath, sizeof(fullpath), (string("v8/packages/") + requirer_dir + path).c_str());

				return string(fullpath);
			}

			std::string PackageRepositoryProvider::GetName() const
			{
				return "Package repository";
			}
		}
	}
}
