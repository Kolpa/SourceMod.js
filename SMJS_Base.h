#ifndef _INCLUDE_SMJS_BASE_H_
#define _INCLUDE_SMJS_BASE_H_

class SMJS_Base {
public:
	virtual ~SMJS_Base(){}
};

#define WRAPPED_FUNC(name) proto->Set(v8::String::NewSymbol(#name), v8::FunctionTemplate::New(name));

#endif