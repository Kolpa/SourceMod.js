
#ifndef _INCLUDE_SMJS_H_
#define _INCLUDE_SMJS_H_

#ifdef WIN32
	#pragma comment(lib, "psapi.lib")
	#pragma comment(lib, "iphlpapi.lib")
#endif

__pragma(warning(disable:4005) )
#include <stdint.h>
#define HAVE_STDINT_
#define HAVE_STDINT_H
#include "platform.h"
#define V8STDINT_H_

#include "v8.h"
#include "k7v8macros.h"
#include "uv.h"

#include <string>

__pragma(warning(default:4005) )

//#include "sp_file_headers.h"

#define SMJS_API_VERSION 6

typedef int PLUGIN_ID;

class SMJS_Plugin;
class SMJS_BaseWrapped;
class SMJS_Module;

class IPluginDestroyedHandler {
public:
	virtual void OnPluginDestroyed(SMJS_Plugin *plugin) = 0;
};

void SMJS_Init();
void SMJS_Ping();

void SMJS_Pause();
void SMJS_Resume();

char* SMJS_FileToString(const char *file, const char* dir = NULL);

extern v8::Isolate *mainIsolate;

extern const char *scriptSMStr;
extern v8::ScriptData *scriptSMData;

extern const char *scriptDotaStr;
extern v8::ScriptData *scriptDotaData;

extern uv_loop_t *uvLoop;


#endif