#ifndef _INCLUDE_PLUGINSYSV8_H_
#define _INCLUDE_PLUGINSYSV8_H_


class CPluginV8
{
public:
	CPluginV8(Isolate* isolate, v8::Handle<v8::Context> context);
	virtual ~CPluginV8(void);
private:
	Handle<Value> BuildObjects();

};

#endif // !defined _INCLUDE_PLUGINSYSV8_H_
