#include "extension.h"
#include "SMJS_SimpleWrapped.h"
#include "SMJS_Plugin.h"

v8::Persistent<FunctionTemplate> SMJS_SimpleWrapped::temp;

void SimpleDecWrappedRefCount(Isolate* isolate, v8::Persistent<v8::Value> *object, SMJS_SimpleWrapped *parameter){
	SMJS_SimpleWrapped *wrapped = (SMJS_SimpleWrapped*) parameter;
	wrapped->DecRef();

	object->Dispose();
    object->Clear();
}

SMJS_SimpleWrapped::SMJS_SimpleWrapped(SMJS_Plugin *pl) : plugin(pl){
	refCount = 0;
	destroying = false;
	plugin->RegisterDestroyCallback(this);
}

void SMJS_SimpleWrapped::Destroy(){
	if(refCount == 0){
		delete this;
	}else{
		destroying = true;
		wrapper.MakeWeak<v8::Value, SMJS_SimpleWrapped>(plugin->GetIsolate(), this, SimpleDecWrappedRefCount);
	}
}