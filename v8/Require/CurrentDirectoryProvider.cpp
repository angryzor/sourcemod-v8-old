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
			CurrentDirectoryProvider::CurrentDirectoryProvider(ISourceMod *sm)
				: sm(sm)
			{
			}

			CurrentDirectoryProvider::~CurrentDirectoryProvider(void)
			{
			}

			std::string CurrentDirectoryProvider::ResolvePath(const SMV8Script& requirer, const string& path) const
			{
				string reqpath = requirer.GetPath();
				string requirer_dir = reqpath.substr(0,reqpath.find_last_of('/') + 1);

				char fullpath[PLATFORM_MAX_PATH];
				sm->BuildPath(Path_SM, fullpath, sizeof(fullpath), (string("v8/") + requirer_dir + path).c_str());

				return fullpath;
			}

			bool CurrentDirectoryProvider::Provides(const SMV8Script& requirer, const string& path) const
			{
				ifstream ifs(ResolvePath(requirer, path));
				return ifs.is_open();
			}
			
			string CurrentDirectoryProvider::Require(const SMV8Script& requirer, const string& path) const
			{
				string fullpath = ResolvePath(requirer, path);

				ifstream ifs(ResolvePath(requirer, path));
				if(!ifs.is_open())
					throw logic_error("Can't open required path " + path + ". This should not happen.");

				return fullpath;
			}

			std::string CurrentDirectoryProvider::GetName() const
			{
				return "Current directory";
			}
		}
	}
}