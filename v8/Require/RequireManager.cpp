#include "RequireManager.h"

namespace SMV8
{
	namespace Require
	{
		using namespace std;
		RequireManager::RequireManager(void)
		{
			providers.push_back(new Providers::CurrentDirectoryProvider());
			providers.push_back(new Providers::LocalPackageRepositoryProvider());
			providers.push_back(new Providers::RemotePackageRepositoryProvider("http://sourcemod-v8.angryzor.com/packages/"));
		}

		RequireManager::~RequireManager(void)
		{
			for(IDependencyProvider *provider: providers)
			{
				delete provider;
			}
		}

		string RequireManager::Require(const string& requirer, const string& path) const
		{
			for(IDependencyProvider *provider: providers)
			{
				if(provider->Provides(requirer, path + ".js"))
					return provider->Require(requirer, path + ".js");
				if(provider->Provides(requirer, path + ".coffee"))
					return provider->Require(requirer, path + ".coffee");
				if(provider->Provides(requirer, path + "/main.js"))
					return provider->Require(requirer, path + "/main.js");
				if(provider->Provides(requirer, path + "/main.coffee"))
					return provider->Require(requirer, path + "/main.coffee");
			}

			throw runtime_error("Dependency error: cannot resolve dependency '" + path + "'");
		}
	}
}