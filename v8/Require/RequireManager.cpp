#include "RequireManager.h"
#include "CurrentDirectoryProvider.h"
#include "PackageRepositoryProvider.h"

namespace SMV8
{
	namespace Require
	{
		using namespace std;
		RequireManager::RequireManager(ISourceMod *sm, ILibrarySys *libsys, DependencyManager *depMan)
			: sm(sm), libsys(libsys)
		{
			providers.push_back(new Providers::CurrentDirectoryProvider(sm));
			providers.push_back(new Providers::PackageRepositoryProvider(sm, depMan));
		}

		RequireManager::~RequireManager(void)
		{
			for(IRequireProvider *provider: providers)
			{
				delete provider;
			}
		}

		string RequireManager::Require(const SMV8Script& requirer, const string& path) const
		{
			for(IRequireProvider *provider: providers)
			{
				if(provider->Provides(requirer, path + ".coffee"))
					return provider->Require(requirer, path + ".coffee");
				if(provider->Provides(requirer, path + ".js"))
					return provider->Require(requirer, path + ".js");
				if(provider->Provides(requirer, path + "/main.coffee"))
					return provider->Require(requirer, path + "/main.coffee");
				if(provider->Provides(requirer, path + "/main.js"))
					return provider->Require(requirer, path + "/main.js");
			}

			throw runtime_error("Dependency error: cannot resolve dependency '" + path + "'");
		}
	}
}