#pragma once

#include "IRequireProvider.h"
#include <ISourceMod.h>

namespace SMV8
{
	namespace Require
	{
		namespace Providers
		{
			using namespace SourceMod;
			class PackageRepositoryProvider : public IRequireProvider
			{
			public:
				PackageRepositoryProvider(ISourceMod *sm);
				virtual ~PackageRepositoryProvider(void);
				virtual bool Provides(const std::string& requirer, const std::string& path) const;
				virtual std::string Require(const std::string& requirer, const std::string& path) const;
				std::string GetName() const;
			private:
				ISourceMod *sm;
				std::string directory;
			};
		}
	}
}
