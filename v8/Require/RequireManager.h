#ifndef _INCLUDE_V8_DEPENDENCYMANAGER_H_
#define _INCLUDE_V8_DEPENDENCYMANAGER_H_

#include "IDependencyProvider.h"
#include <vector>

namespace SMV8
{
	namespace Require
	{
		using namespace std;
		class RequireManager
		{
		public:
			RequireManager(void);
			virtual ~RequireManager(void);
			string Require(const string& requirer, const string& path) const;
		private:
			vector<IRequireProvider *> providers;
		};
	}
}

#endif // !_INCLUDE_V8_DEPENDENCYMANAGER_H_
