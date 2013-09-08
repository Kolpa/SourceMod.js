#include "SMJS_ConVar.h"
#include "SMJS_Plugin.h"


SIMPLE_WRAPPED_CLS_CPP(SMJS_ConVar, SMJS_SimpleWrapped);

FUNCTION_M(SMJS_ConVar::getName)
	GET_INTERNAL(SMJS_ConVar*, self);
	RETURN_SCOPED(v8::String::New(self->cv->GetName()));
END

void SMJS_ConVar::GetFlags(Local<String> property, const PropertyCallbackInfo<Value>& args){
	Local<Value> _intfld = args.This()->GetInternalField(0); 
	SMJS_ConVar *self = dynamic_cast<SMJS_ConVar*>((SMJS_BaseWrapped*)Handle<External>::Cast(_intfld)->Value());

	RETURN_INT(self->cv->GetFlags());
}

void SMJS_ConVar::SetFlags(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void>& args){
	Local<Value> _intfld = args.This()->GetInternalField(0); 
	SMJS_ConVar *self = dynamic_cast<SMJS_ConVar*>((SMJS_BaseWrapped*)Handle<External>::Cast(_intfld)->Value());

	auto newFlags = (int) value->NumberValue();

	int oldFlags = self->cv->GetFlags();
	if(((oldFlags ^ newFlags) & oldFlags) != 0) self->cv->RemoveFlags((oldFlags ^ newFlags) & oldFlags);
	if(((oldFlags ^ newFlags) & newFlags) != 0) self->cv->AddFlags((oldFlags ^ newFlags) & newFlags);
}

FUNCTION_M(SMJS_ConVar::getBool)
	GET_INTERNAL(SMJS_ConVar*, self);
	RETURN_SCOPED(v8::Boolean::New(self->cv->GetBool()));
END

FUNCTION_M(SMJS_ConVar::getInt)
	GET_INTERNAL(SMJS_ConVar*, self);
	RETURN_SCOPED(v8::Int32::New(self->cv->GetInt()));
END

FUNCTION_M(SMJS_ConVar::getNumber)
	GET_INTERNAL(SMJS_ConVar*, self);
	RETURN_SCOPED(v8::Number::New(self->cv->GetFloat()));
END

FUNCTION_M(SMJS_ConVar::getString)
	GET_INTERNAL(SMJS_ConVar*, self);
	RETURN_SCOPED(v8::String::New(self->cv->GetString()));
END

FUNCTION_M(SMJS_ConVar::setBool)
	GET_INTERNAL(SMJS_ConVar*, self);
	PBOL(value);
	self->cv->SetValue(value);
	RETURN_UNDEF;
END

FUNCTION_M(SMJS_ConVar::setInt)
	GET_INTERNAL(SMJS_ConVar*, self);
	PINT(value);
	self->cv->SetValue(value);
	RETURN_UNDEF;
END

FUNCTION_M(SMJS_ConVar::setNumber)
	GET_INTERNAL(SMJS_ConVar*, self);
	PNUM(value);
	self->cv->SetValue((float)value);
	RETURN_UNDEF;
END

FUNCTION_M(SMJS_ConVar::setString)
	GET_INTERNAL(SMJS_ConVar*, self);
	PSTR(value);
	self->cv->SetValue(*value);
	RETURN_UNDEF;
END