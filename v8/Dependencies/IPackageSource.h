#pragma once

#include <string>

namespace SMV8
{
	namespace Dependencies
	{
		class IPackageSource
		{
		public:
			virtual void RetrievePackage(const std::string& package, const std::string& destination) const = 0;
		};
	}
}