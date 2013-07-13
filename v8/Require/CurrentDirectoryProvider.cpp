#include "CurrentDirectoryProvider.h"
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
			CurrentDirectoryProvider::CurrentDirectoryProvider(ISourceMod *sm) : sm(sm)
			{
			}

			CurrentDirectoryProvider::~CurrentDirectoryProvider(void)
			{
			}

			bool CurrentDirectoryProvider::Provides(const string& requirer, const string& path) const
			{
				string requirer_dir = requirer.substr(0,requirer.find_last_of('/') + 1);

				char fullpath[PLATFORM_MAX_PATH];
				sm->BuildPath(Path_SM, fullpath, sizeof(fullpath), (string("v8/") + requirer_dir + path).c_str());

				ifstream ifs(fullpath);
				return ifs;
			}
			
			string CurrentDirectoryProvider::Require(const string& requirer, const string& path) const
			{
				string requirer_dir = requirer.substr(0,requirer.find_last_of('/') + 1);

				char fullpath[PLATFORM_MAX_PATH];
				sm->BuildPath(Path_SM, fullpath, sizeof(fullpath), (string("v8/") + requirer_dir + path).c_str());

				return string(fullpath);
			}

			std::string CurrentDirectoryProvider::GetName() const
			{
				return "Current directory";
			}
		}
	}
}