#ifndef _INCLUDE_CURRENTDIRECTORYPROVIDER_H_
#define _INCLUDE_CURRENTDIRECTORYPROVIDER_H_

#include "IRequireProvider.h"
#include <ISourceMod.h>

namespace SMV8
{
	namespace Require
	{
		namespace Providers
		{
			using namespace SourceMod;
			class CurrentDirectoryProvider : public IRequireProvider
			{
			public:
				CurrentDirectoryProvider(ISourceMod *sm);
				virtual ~CurrentDirectoryProvider(void);
				virtual bool Provides(const std::string& requirer, const std::string& path) const;
				virtual std::string Require(const std::string& requirer, const std::string& path) const;
				std::string GetName() const;
			private:
				ISourceMod *sm;
			};
		}
	}
}

#endif