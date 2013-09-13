// -----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
//                     Victor Grishchenko         <victor.grishchenko@gmail.com>
// ----------------------------------------------------------------------------
// Creation date     : 27-Sep-2008
// Last modification : 19-May-2009
// ----------------------------------------------------------------------------

#ifndef __K7_MACROS__
#define __K7_MACROS__

//#include "v8.h"
#include <stdarg.h>

using namespace v8;

// ----------------------------------------------------------------------------
//
// TYPE CONVERSION
//
// ----------------------------------------------------------------------------

/**
* These macros are just shorthand to creat V8/JavaScript values that can be
* passed to the V8 API or returned to the JavaScript environment */
#define JS_CONTEXT                     v8::Context::GetCurrent()
#define JS_GLOBAL                      JS_CONTEXT->Global()
#define JS_str(s)                      v8::String::New(s)
#define JS_str2(s,c)                   v8::String::New(s,c)
#define JS_int(s)                      v8::Integer::New(s)
#define JS_undefined                   v8::Undefined()
#define JS_null                        v8::Null()
#define JS_fn(f)                       v8::FunctionTemplate::New(f)->GetFunction()
#define JS_obj(o)                      v8::Object::New(o)
#define JS_bool(b)                     v8::Boolean::New(b)

#define JS_THROW(t, s)                 ThrowException(Exception::t(String::New(s)))
#define JS_ERROR(s)                    ThrowException(Exception::Error(String::New(s)))

// ----------------------------------------------------------------------------
//
// ARGUMENTS CONVERSION
//
// ----------------------------------------------------------------------------

/**
* This set of macro make it easy to declare a function that can be exported to
* the JavaScript environment.
* See the 'posix.cpp' module for examples. */
#define ARGC                           args.Length()
#define ARG_COUNT(c)                   if ( args.Length() != c ) { \
	THROW("Invalid number of arguments"); } 
#define ARG_BETWEEN(a,b)               if(args.Length() < a || args.Length() > b) THROW("Must have between " #a " and " #b " arguments"); 
#define ARG_int(n,c)                   int n=(int)(args[c]->Int32Value())
#define ARG_str(v,i)                   v8::String::AsciiValue v(args[i]);
#define ARG_utf8(v,i)                  v8::String::Utf8Value  v(args[i])
#define ARG_obj(v,i)                   v8::Local<v8::Object> v=args[i]->ToObject();
#define ARG_obj(v,i)                   v8::Local<v8::Object> v=args[i]->ToObject();
#define ARG_array(name, c) \
	if (!args[(c)]->IsArray()) { \
	/*std::ostringstream __k7_e;*/ \
	/*__k7_e << "Exception: argument error." << __func__ << " expects array for argument " << c << ".";*/ \
	/*return ThrowException(String::New(__k7_e.str().c_str()));*/ \
	} \
	Handle<Array> name = Handle<Array>::Cast(args[(c)]);
#define ARG_fn(name, c) \
	Handle<Function> name = Handle<Function>::Cast(args[(c)]); \
	if(name.IsEmpty()) THROW("Invalid argument #" #c ", must be a function")

#define THROW(str)              {v8::ThrowException(String::New(str));return;}
#define THROW_VERB(tmpl,...)    { char msg[1024]; snprintf(msg,1000,tmpl,__VA_ARGS__); THROW(msg); }
// performance freaks use this THROW_VERB :)
//#define THROW_VERB(tmpl,...)  THROW("Invocation error")

#define FUN_NAME            (*String::AsciiValue(args.Callee()->GetName()->ToString()))

#define PINT(n)             0; if (args.Length()<++_argn || !args[_argn-1]->IsInt32()) {\
	THROW_VERB("Argument %i of %s, must be an integer\n",_argn,FUN_NAME);} \
	int n=(int)(args[_argn-1]->Int32Value()); 0

#define PBOL(n)             0; if (args.Length()<++_argn || !args[_argn-1]->IsBoolean()) {\
	THROW_VERB("Argument %i of %s, must be a boolean\n",_argn,FUN_NAME);} \
	bool n=(bool)(args[_argn-1]->BooleanValue()); 0

#define PNUM(n)             0; if (args.Length()<++_argn || !args[_argn-1]->IsNumber()) {\
	THROW_VERB("Argument %i of %s, must be an number\n",_argn,FUN_NAME);} \
	double n=(args[_argn-1]->NumberValue()); 0

#define PNUM2(n)             0; if (args.Length()<++_argn || !args[_argn-1]->IsNumber()) {\
	THROW_VERB("Argument %i of %s, must be an number\n",_argn,FUN_NAME);} \
	n=(args[_argn-1]->NumberValue()); 0


#define PSTR(n)             0; if (args.Length()<++_argn) {\
	THROW_VERB("Argument %i of %s must be a string\n",_argn,FUN_NAME);} \
	v8::String::AsciiValue n(args[_argn-1]); 0

#define PUTF8(n)             0; if (args.Length()<++_argn) {\
	THROW_VERB("Argument %i of %s must be a string\n",_argn,FUN_NAME);} \
	v8::String::Utf8Value n(args[_argn-1]); 0

#define POBJ(n)             0; if (args.Length()<++_argn || !args[_argn-1]->IsObject()) {\
	THROW_VERB("Argument %i of %s, must be an object\n",_argn,FUN_NAME);} \
	v8::Handle<v8::Object> n (args[_argn-1]->ToObject()); 0

#define POBJ_NULLABLE(n)             0; if (args.Length()<++_argn || (!args[_argn-1]->IsObject() && !args[_argn-1]->IsNull())) {\
	THROW_VERB("Argument %i of %s, must be an object\n",_argn,FUN_NAME);} \
	v8::Handle<v8::Object> n; if(!args[_argn-1]->IsNull()) n = args[_argn-1]->ToObject(); 0

#define PFUN(n)             0; if (args.Length()<++_argn || !args[_argn-1]->IsFunction()) {\
	THROW_VERB("Argument %i of %s, must be a function\n",_argn,FUN_NAME);} \
	v8::Handle<v8::Function> n (v8::Function::Cast(*args[_argn-1])); 0

#define PWRAP(type,var,notNull)   0; if (args.Length()<++_argn || (!args[_argn-1]->IsExternal() && !args[_argn-1]->IsNull())) {\
	THROW_VERB("Argument %i of %s, must be an external\n",_argn,FUN_NAME);} \
	type var = dynamic_cast<type>((SMJS_Base*)v8::External::Cast(*args[_argn-1])->Value()); \
	if(notNull && var == NULL) THROW_VERB("Argument %i is of an invalid type\n",_argn,FUN_NAME);

#define PVEC_N(vec, n) v8::Handle<v8::Value> vec_tmp_##n = vec->Get(v8::String::NewSymbol(#n)); \
	if(vec_tmp_##n.IsEmpty() || !vec_tmp_##n->IsNumber()) THROW("Field " #n " in a vector must be a number"); \
	n = vec_tmp_##n->NumberValue();

#define PVEC(x, y, z) 0  ;if (args.Length()<=_argn) THROW_VERB("Argument %i of %s, must be a vector \n",_argn+1,FUN_NAME); \
	double x, y, z; \
	if(args[_argn]->IsNumber()) { \
		PNUM2(x); \
		PNUM2(y); \
		PNUM2(z); \
	}else{ \
		POBJ(vec_##x##y##z); \
		PVEC_N(vec_##x##y##z, x); \
		PVEC_N(vec_##x##y##z, y); \
		PVEC_N(vec_##x##y##z, z); \
	}

#define PENT(ent) 0; \
	POBJ_NULLABLE(ent##_tmp); \
	SMJS_Entity *ent = NULL; \
	if(!ent##_tmp.IsEmpty() && !ent_##tmp->IsNull() && !ent_##tmp->IsUndefined()){ \
		 \
		auto ent##_inte = ent##_tmp->GetInternalField(0); \
		if(ent##_inte.IsEmpty()){ \
			THROW("Invalid entity"); \
		} \
		 \
		ent = dynamic_cast<SMJS_Entity*>((SMJS_Base*) v8::Handle<v8::External>::Cast(ent##_inte)->Value()); \
		if(ent == NULL || !ent->IsValid()) THROW("Invalid other entity"); \
	}
	

// ----------------------------------------------------------------------------
//
// FUNCTIONS MACROS
//
// ----------------------------------------------------------------------------

#define FUNCTION_DECL(f)        static void f(const v8::FunctionCallbackInfo<v8::Value>&);
#define FUNCTION(f,...)         static void f(const v8::FunctionCallbackInfo<v8::Value>& args) { \
	v8::HandleScope handlescope; \
	int _argn=0;    \
	__VA_ARGS__;
#define FUNCTION_M(f,...)       void f(const v8::FunctionCallbackInfo<v8::Value>& args) { \
	v8::HandleScope handlescope; \
	int _argn=0;    \
	__VA_ARGS__;
#define FUNCTION_C(f)           static void f(const v8::FunctionCallbackInfo<v8::Value>& args) {
#define END                     }
#define JS_THIS                 args.This()
#define STUB                    return ThrowException(Exception::Error(String::New("Stub - Function not implemented")));
#define SET_INTERNAL(ptr)       args.This()->SetInternalField(0, External::New((void*)ptr));
#define GET_INTERNAL(type,var)  Local<Value> _intfld = args.This()->GetInternalField(0); \
	type var = dynamic_cast<type>((SMJS_Base*)Handle<External>::Cast(_intfld)->Value());
#define RETURN(x)				{args.GetReturnValue().Set(x); return;}
#define RETURN_SCOPED(x)        RETURN(x)
#define RETURN_BOOL(i)          RETURN(Boolean::New(i))
#define RETURN_INT(i)           RETURN(Integer::New(i))
#define RETURN_NUMBER(i)        RETURN(Number::New(i))
#define RETURN_STRING(i)        RETURN(String::New(i))
#define RETURN_WRAPPED(ptr)     RETURN(v8::External::New(ptr))
#define RETURN_UNDEF            RETURN(v8::Undefined())
#define RETURN_UNDEFINED		RETURN(v8::Undefined())
#define RETURN_NULL             RETURN(v8::Null())

// ----------------------------------------------------------------------------
//
// OBJECT MACROS
//
// ----------------------------------------------------------------------------

/**
* This set of macros allow you to declare a new object type, usually because
* you want to set internal data in this object. 
* See the 'posix.cpp' module for examples. */

#define OBJECT(name,fields,...) \
	v8::Handle<Object> name(__VA_ARGS__) { \
	v8::HandleScope handle_scope;\
	v8::Handle<v8::FunctionTemplate>   __class__  = v8::FunctionTemplate::New();\
	v8::Handle<v8::ObjectTemplate>     __object__ = __class__->InstanceTemplate();\
	__object__->SetInternalFieldCount(fields);\
	v8::Handle<v8::Object>               self = __object__->NewInstance();

#define OBJECT_COPY_SLOTS(from,to) \
{ \
	Handle<Array> keys = from->GetPropertyNames(); \
	for (int i = 0; i < keys->Length(); i ++) { \
	Handle<String> key = keys->Get(JS_int(i))->ToString(); \
	Handle<Value> val = from->Get(key); \
	to->Set(key, val); \
	} \
}

#define OBJECT_SET(o,s,v)   o->Set(JS_str(s), v)
#define OBJECT_GET(o,s)     o->Get(JS_str(s))
#define OBJECT_UNSET(o,s)   o->Set(JS_str(s), JS_undefined)
#define OBJECT_PROTOTYPE(o) o->GetPrototype();


#define INTERNAL(i,value) \
	self->SetInternalField(i, v8::External::New((void*)value));

#define EXTERNAL(type,name,object,indice) \
	type name = (type) (v8::Local<v8::External>::Cast(object->GetInternalField(indice))->Value());

// ----------------------------------------------------------------------------
//
// CLASS MACROS
//
// ----------------------------------------------------------------------------

#define CLASS(name)\
{ \
	v8::Handle<v8::String>            __class_name__ = v8::String::New(name); \
	v8::Handle<v8::FunctionTemplate>  __class__      = v8::FunctionTemplate::New(); \
	v8::Handle<v8::ObjectTemplate>    __object__     = __class__->InstanceTemplate(); \
	v8::Handle<v8::ObjectTemplate>    self           = __object__; \
	__class__->SetClassName(v8::String::New(name)); 
#define CONSTRUCTOR(c)       __class__->SetCallHandler(c);
#define DESTRUCTOR(d)        Persistent<Object> _weak_handle = Persistent<Object>::New(__object__); \
	_weak_handle.MakeWeak(NULL, d);
#define INTERNAL_FIELDS(i)   __object__->SetInternalFieldCount(i);
#define HAS_INTERNAL         __object__->SetInternalFieldCount(1);
#define END_CLASS            __module__->Set(__class_name__,__class__->GetFunction(),v8::PropertyAttribute(v8::ReadOnly|v8::DontDelete));}

// ----------------------------------------------------------------------------
//
// MODULE MACROS
//
// ----------------------------------------------------------------------------

#define K7_MODULE_CREATOR_T Handle<Object> (*moduleCreator)(Handle<Object> parent, const char* moduleName, const char* fullName)
#define MODULE \
	extern "C" v8::Handle<v8::Object> MODULE_STATIC (v8::Handle<v8::Object> __module__) {\
	v8::Handle<v8::Object> self = __module__;

#ifdef AS_PLUGIN
#define END_MODULE \
	return __module__; } \
	extern "C" Handle<Object> k7_module_init (Handle<Object> global, K7_MODULE_CREATOR_T) { \
	return MODULE_STATIC(moduleCreator(global, MODULE_NAME, NULL)); \
}
#else
#define END_MODULE \
	return __module__; }
#endif

// ----------------------------------------------------------------------------
//
// SCOPE AND BINDING MACROS
//
// ----------------------------------------------------------------------------

#define WITH_CLASS        { v8::Handle<v8::Object> self = __class__;
#define WITH_MODULE       { v8::Handle<v8::Object> self = __module__;
#define WITH_OBJECT       { v8::Handle<v8::Object> self = __object__;

#define SET(s,v)          self->Set(JS_str(s),v);
#define SET_int(s,v)      self->Set(JS_str(s), JS_int(v));
#define SET_str(s,v)      self->Set(JS_str(s), JS_str(v));

#define BIND(s,v)        {  Handle<Function> f = v8::FunctionTemplate::New(v)->GetFunction(); \
	f->SetName(JS_str(s));  \
	self->Set(JS_str(s),f); \
}
#define METHOD(s,v)       __object__->Set(JS_str(s),v8::FunctionTemplate::New(v)->GetFunction());
#define BIND_CONST(s,v)   self->Set(JS_str(s),v);
#define CLASS_METHOD(s,v) __class__->Set(JS_str(s),v8::FunctionTemplate::New(v)->GetFunction());

// ----------------------------------------------------------------------------
//
// API MACROS
//
// ----------------------------------------------------------------------------

/**
* These macros allow to declare a module initialization function and register
* FUNCTIONs in this module. */
#define LINK_TO(lib)
#define IMPORT(function)            extern "C" v8::Handle<v8::Object> function(v8::Handle<v8::Object> module);
#define LOAD(moduleName,function)   function(k7::module(global, moduleName, NULL));
#define EXEC(source)                k7::execute(source);
#define EVAL(source)                k7::eval(source);

#endif
// EOF - vim: ts=4 sw=4 et