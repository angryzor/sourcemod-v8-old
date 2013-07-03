#ifndef _INCLUDE_PLUGINSYSV8_H_
#define _INCLUDE_PLUGINSYSV8_H_

#include <v8.h>

class CPluginV8
{
public:
	CPluginV8(v8::Handle<v8::Context> context);
	virtual ~CPluginV8(void);
private:
	v8::Persistent<v8::Context> context;
};

#endif // !defined _INCLUDE_PLUGINSYSV8_H_
