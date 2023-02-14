#ifndef SPIDERJS_INTERFACE_DEF_H
#define SPIDERJS_INTERFACE_DEF_H

/*
** This file was machine generated by idltool 51.8.
** Do not edit
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef EXEC_EXEC_H
#include <exec/exec.h>
#endif
#ifndef EXEC_INTERFACES_H
#include <exec/interfaces.h>
#endif

#ifndef LIBRARIES_SPIDER_H
#include <libraries/spider.h>
#endif

struct SpiderJSIFace
{
        struct InterfaceData Data;

        ULONG APICALL (*Obtain)(struct SpiderJSIFace *Self);
        ULONG APICALL (*Release)(struct SpiderJSIFace *Self);
        void APICALL (*Expunge)(struct SpiderJSIFace *Self);
        struct Interface * APICALL (*Clone)(struct SpiderJSIFace *Self);
        int64 APICALL (*JS_Now)(struct SpiderJSIFace *Self);
        jsval APICALL (*JS_GetNaNValue)(struct SpiderJSIFace *Self, JSContext * cx);
        jsval APICALL (*JS_GetNegativeInfinityValue)(struct SpiderJSIFace *Self, JSContext * cx);
        jsval APICALL (*JS_GetPositiveInfinityValue)(struct SpiderJSIFace *Self, JSContext * cx);
        jsval APICALL (*JS_GetEmptyStringValue)(struct SpiderJSIFace *Self, JSContext * cx);
        JSBool APICALL (*JS_ConvertArgumentsA)(struct SpiderJSIFace *Self, JSContext * cx, LONG argc, jsval * argv, const char * format, ULONG * args);
        JSBool APICALL (*JS_ConvertArguments)(struct SpiderJSIFace *Self, JSContext * cx, LONG argc, jsval * argv, const char * format, ...);
        jsval * APICALL (*JS_PushArgumentsA)(struct SpiderJSIFace *Self, JSContext * cx, void ** markp, const char * format, ULONG * args);
        jsval * APICALL (*JS_PushArguments)(struct SpiderJSIFace *Self, JSContext * cx, void ** markp, const char * format, ...);
        void APICALL (*JS_PopArguments)(struct SpiderJSIFace *Self, JSContext * cx, void * mark);
        JSBool APICALL (*JS_ConvertValue)(struct SpiderJSIFace *Self, JSContext * cx, LONG v, LONG type, jsval * vp);
        JSBool APICALL (*JS_ValueToObject)(struct SpiderJSIFace *Self, JSContext * cx, LONG v);
        JSFunction * APICALL (*JS_ValueToFunction)(struct SpiderJSIFace *Self, JSContext * cx, LONG v);
        JSFunction * APICALL (*JS_ValueToConstructor)(struct SpiderJSIFace *Self, JSContext * cx, LONG v);
        JSString * APICALL (*JS_ValueToString)(struct SpiderJSIFace *Self, JSContext * cx, LONG v);
        JSBool APICALL (*JS_ValueToNumber)(struct SpiderJSIFace *Self, JSContext * cx, LONG v, jsdouble * dp);
        JSBool APICALL (*JS_ValueToECMAInt32)(struct SpiderJSIFace *Self, JSContext * cx, LONG v, int32 * ip);
        JSBool APICALL (*JS_ValueToECMAUint32)(struct SpiderJSIFace *Self, JSContext * cx, LONG v, uint32 * ip);
        JSBool APICALL (*JS_ValueToInt32)(struct SpiderJSIFace *Self, JSContext * cx, LONG v, int32 * ip);
        JSBool APICALL (*JS_ValueToUint16)(struct SpiderJSIFace *Self, JSContext * cx, LONG v, uint16 * ip);
        JSBool APICALL (*JS_ValueToBoolean)(struct SpiderJSIFace *Self, JSContext * cx, LONG v, JSBool * bp);
        JSType APICALL (*JS_TypeOfValue)(struct SpiderJSIFace *Self, JSContext * cx, LONG v);
        const char * APICALL (*JS_GetTypeName)(struct SpiderJSIFace *Self, JSContext * cx, LONG type);
        JSRuntime * APICALL (*JS_NewRuntime)(struct SpiderJSIFace *Self, uint32 maxbytes);
        void APICALL (*JS_DestroyRuntime)(struct SpiderJSIFace *Self, JSRuntime * rt);
        void APICALL (*JS_ShutDown)(struct SpiderJSIFace *Self);
        void APICALL (*JS_Lock)(struct SpiderJSIFace *Self, JSRuntime * rt);
        void APICALL (*JS_Unlock)(struct SpiderJSIFace *Self, JSRuntime * rt);
        JSContext * APICALL (*JS_NewContext)(struct SpiderJSIFace *Self, JSRuntime * rt, ULONG stackChunkSize);
        void APICALL (*JS_DestroyContext)(struct SpiderJSIFace *Self, JSContext * cx);
        void APICALL (*JS_DestroyContextNoGC)(struct SpiderJSIFace *Self, JSContext * cx);
        void APICALL (*JS_DestroyContextMaybeGC)(struct SpiderJSIFace *Self, JSContext * cx);
        void * APICALL (*JS_GetContextPrivate)(struct SpiderJSIFace *Self, JSContext * cx);
        void APICALL (*JS_SetContextPrivate)(struct SpiderJSIFace *Self, JSContext * cx, void * data);
        JSRuntime * APICALL (*JS_GetRuntime)(struct SpiderJSIFace *Self, JSContext * cx);
        JSContext * APICALL (*JS_ContextIterator)(struct SpiderJSIFace *Self, JSRuntime * rt, JSContext ** iterp);
        JSVersion APICALL (*JS_GetVersion)(struct SpiderJSIFace *Self, JSContext * cx);
        JSVersion APICALL (*JS_SetVersion)(struct SpiderJSIFace *Self, JSContext * cx, LONG version);
        const char * APICALL (*JS_VersionToString)(struct SpiderJSIFace *Self, LONG version);
        JSVersion APICALL (*JS_StringToVersion)(struct SpiderJSIFace *Self, const char * string);
        uint32 APICALL (*JS_GetOptions)(struct SpiderJSIFace *Self, JSContext * cx);
        uint32 APICALL (*JS_SetOptions)(struct SpiderJSIFace *Self, JSContext * cx, uint32 options);
        uint32 APICALL (*JS_ToggleOptions)(struct SpiderJSIFace *Self, JSContext * cx, uint32 options);
        const char * APICALL (*JS_GetImplementationVersion)(struct SpiderJSIFace *Self);
        JSObject * APICALL (*JS_GetGlobalObject)(struct SpiderJSIFace *Self, JSContext * cx);
        void APICALL (*JS_SetGlobalObject)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj);
        JSBool APICALL (*JS_InitStandardClasses)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj);
        JSBool APICALL (*JS_ResolveStandardClass)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, LONG id, JSBool * resolved);
        JSBool APICALL (*JS_EnumerateStandardClasses)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj);
        JSIdArray * APICALL (*JS_EnumerateResolvedStandardClasses)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSIdArray * ida);
        JSObject * APICALL (*JS_GetScopeChain)(struct SpiderJSIFace *Self, JSContext * cx);
        void * APICALL (*JS_malloc)(struct SpiderJSIFace *Self, JSContext * cx, ULONG nbytes);
        void * APICALL (*JS_realloc)(struct SpiderJSIFace *Self, JSContext * cx, void * p, ULONG nbytes);
        void APICALL (*JS_free)(struct SpiderJSIFace *Self, JSContext * cx, void * p);
        char * APICALL (*JS_strdup)(struct SpiderJSIFace *Self, JSContext * cx, const char * s);
        jsdouble * APICALL (*JS_NewDouble)(struct SpiderJSIFace *Self, JSContext * cx, LONG d);
        JSBool APICALL (*JS_NewDoubleValue)(struct SpiderJSIFace *Self, JSContext * cx, LONG d, jsval * rval);
        JSBool APICALL (*JS_NewNumberValue)(struct SpiderJSIFace *Self, JSContext * cx, LONG d, jsval * rval);
        JSBool APICALL (*JS_AddRoot)(struct SpiderJSIFace *Self, JSContext * cx, void * rp);
        JSBool APICALL (*JS_AddNamedRoot)(struct SpiderJSIFace *Self, JSContext * cx, void * rp, const char * name);
        JSBool APICALL (*JS_AddNamedRootRT)(struct SpiderJSIFace *Self, JSRuntime * rt, void * rp, const char * name);
        JSBool APICALL (*JS_RemoveRoot)(struct SpiderJSIFace *Self, JSContext * cx, void * rp);
        JSBool APICALL (*JS_RemoveRootRT)(struct SpiderJSIFace *Self, JSRuntime * rt, void * rp);
        void APICALL (*JS_ClearNewbornRoots)(struct SpiderJSIFace *Self, JSContext * cx);
        JSBool APICALL (*JS_EnterLocalRootScope)(struct SpiderJSIFace *Self, JSContext * cx);
        void APICALL (*JS_LeaveLocalRootScope)(struct SpiderJSIFace *Self, JSContext * cx);
        void APICALL (*JS_ForgetLocalRoot)(struct SpiderJSIFace *Self, JSContext * cx, void * thing);
        uint32 APICALL (*JS_MapGCRoots)(struct SpiderJSIFace *Self, JSRuntime * rt, LONG map, void * data);
        JSBool APICALL (*JS_LockGCThing)(struct SpiderJSIFace *Self, JSContext * cx, void * thing);
        JSBool APICALL (*JS_LockGCThingRT)(struct SpiderJSIFace *Self, JSRuntime * rt, void * thing);
        JSBool APICALL (*JS_UnlockGCThing)(struct SpiderJSIFace *Self, JSContext * cx, void * thing);
        JSBool APICALL (*JS_UnlockGCThingRT)(struct SpiderJSIFace *Self, JSRuntime * rt, void * thing);
        void APICALL (*JS_MarkGCThing)(struct SpiderJSIFace *Self, JSContext * cx, void * thing, const char * name, void * arg);
        void APICALL (*JS_GC)(struct SpiderJSIFace *Self, JSContext * cx);
        void APICALL (*JS_MaybeGC)(struct SpiderJSIFace *Self, JSContext * cx);
        JSGCCallback APICALL (*JS_SetGCCallback)(struct SpiderJSIFace *Self, JSContext * cx, LONG cb);
        JSGCCallback APICALL (*JS_SetGCCallbackRT)(struct SpiderJSIFace *Self, JSRuntime * rt, LONG cb);
        JSBool APICALL (*JS_IsAboutToBeFinalized)(struct SpiderJSIFace *Self, JSContext * cx, void * thing);
        void APICALL (*JS_SetGCParameter)(struct SpiderJSIFace *Self, JSRuntime * rt, LONG key, uint32 value);
        intN APICALL (*JS_AddExtenalStringFinalizer)(struct SpiderJSIFace *Self, LONG finalizer);
        intN APICALL (*JS_RemoveExternalStringFinalizer)(struct SpiderJSIFace *Self, LONG finalizer);
        JSString * APICALL (*JS_NewExternalString)(struct SpiderJSIFace *Self, JSContext * cx, jschar * chars, ULONG length, LONG type);
        intN APICALL (*JS_GetExternalStringGCType)(struct SpiderJSIFace *Self, JSRuntime * rt, JSString * str);
        void APICALL (*JS_SetThreadStackLimit)(struct SpiderJSIFace *Self, JSContext * cx, LONG limitAddr);
        void APICALL (*JS_DestroyIdArray)(struct SpiderJSIFace *Self, JSContext * cx, JSIdArray * ida);
        JSBool APICALL (*JS_ValueToId)(struct SpiderJSIFace *Self, JSContext * cx, LONG v);
        JSBool APICALL (*JS_IdToValue)(struct SpiderJSIFace *Self, JSContext * cx, LONG id);
        JSBool APICALL (*JS_PropertyStub)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, LONG id, jsval * vp);
        JSBool APICALL (*JS_EnumerateStub)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj);
        JSBool APICALL (*JS_ResolveStub)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, LONG id);
        JSBool APICALL (*JS_ConvertStub)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, LONG type, jsval * vp);
        void APICALL (*JS_FinalizeStub)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj);
        JSObject * APICALL (*JS_InitClass)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSObject * parent_proto, JSClass * clasp, LONG constructor, LONG nargs, JSPropertySpec * ps, JSFunctionSpec * fs, JSPropertySpec * static_ps, JSFunctionSpec * static_fs);
        JSClass * APICALL (*JS_GetClass)(struct SpiderJSIFace *Self, JSObject * obj);
        JSBool APICALL (*JS_InstanceOf)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSClass * clasp, jsval * argv);
        JSBool APICALL (*JS_HasInstance)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, LONG v, JSBool * bp);
        void * APICALL (*JS_GetPrivate)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj);
        JSBool APICALL (*JS_SetPrivate)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, void * data);
        void * APICALL (*JS_GetInstancePrivate)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSClass * clasp, jsval * argv);
        JSObject * APICALL (*JS_GetPrototype)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj);
        JSBool APICALL (*JS_SetPrototype)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSObject * proto);
        JSObject * APICALL (*JS_GetParent)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj);
        JSBool APICALL (*JS_SetParent)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSObject * parent);
        JSObject * APICALL (*JS_GetConstructor)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * proto);
        JSBool APICALL (*JS_GetObjectId)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, jsid * idp);
        JSObject * APICALL (*JS_NewObject)(struct SpiderJSIFace *Self, JSContext * cx, JSClass * clasp, JSObject * proto, JSObject * parent);
        JSBool APICALL (*JS_SealObject)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, LONG deep);
        JSObject * APICALL (*JS_ConstructObject)(struct SpiderJSIFace *Self, JSContext * cx, JSClass * clasp, JSObject * proto, JSObject * parent);
        JSObject * APICALL (*JS_ConstructObjectWithArguments)(struct SpiderJSIFace *Self, JSContext * cx, JSClass * clasp, JSObject * proto, JSObject * parent, LONG argc, jsval * argv);
        JSObject * APICALL (*JS_DefineObject)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, JSClass * clasp, JSObject * proto, LONG attrs);
        JSBool APICALL (*JS_DefineConstDoubles)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSConstDoubleSpec * cds);
        JSBool APICALL (*JS_DefineProperties)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSPropertySpec * ps);
        JSBool APICALL (*JS_DefineProperty)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, LONG value, LONG getter, LONG setter, LONG attrs);
        JSBool APICALL (*JS_GetPropertyAttributes)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, uintN * attrsp, JSBool * foundp);
        JSBool APICALL (*JS_GetPropertyAttrsGetterAndSetter)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, uintN * attrsp, JSBool * foundp, JSPropertyOp * getterp, JSPropertyOp * setterp);
        JSBool APICALL (*JS_SetPropertyAttributes)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, LONG attrs, JSBool * foundp);
        JSBool APICALL (*JS_DefinePropertyWithTinyId)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, LONG tinyid, LONG value, LONG getter, LONG setter, LONG attrs);
        JSBool APICALL (*JS_AliasProperty)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, const char * alias);
        JSBool APICALL (*JS_HasProperty)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, JSBool * foundp);
        JSBool APICALL (*JS_LookupProperty)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, jsval * vp);
        JSBool APICALL (*JS_LookupPropertyWithFlags)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, LONG flags, jsval * vp);
        JSBool APICALL (*JS_GetProperty)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, jsval * vp);
        JSBool APICALL (*JS_GetMethod)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, JSObject ** objp, jsval * vp);
        JSBool APICALL (*JS_SetProperty)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, jsval * vp);
        JSBool APICALL (*JS_DeleteProperty)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name);
        JSBool APICALL (*JS_DeleteProperty2)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, jsval * rval);
        JSBool APICALL (*JS_DefineUCProperty)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, jschar * name, ULONG namelen, LONG value, LONG getter, LONG setter, LONG attrs);
        JSBool APICALL (*JS_GetUCPropertyAttributes)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, jschar * name, ULONG namelen, uintN * attrsp, JSBool * foundp);
        JSBool APICALL (*JS_GetUCPropertyAttrsGetterAndSetter)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, jschar * name, ULONG namelen, uintN * attrsp, JSBool * foundp, JSPropertyOp * getterp, JSPropertyOp * setterp);
        JSBool APICALL (*JS_SetUCPropertyAttributes)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, jschar * name, ULONG namelen, LONG attrs, JSBool * foundp);
        JSBool APICALL (*JS_DefineUCPropertyWithTinyId)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, jschar * name, ULONG namelen, LONG tinyid, LONG value, LONG getter, LONG setter, LONG attrs);
        JSBool APICALL (*JS_HasUCProperty)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, jschar * name, ULONG namelen, JSBool * vp);
        JSBool APICALL (*JS_LookupUCProperty)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, jschar * name, ULONG namelen, jsval * vp);
        JSBool APICALL (*JS_GetUCProperty)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, jschar * name, ULONG namelen, jsval * vp);
        JSBool APICALL (*JS_SetUCProperty)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, jschar * name, ULONG namelen, jsval * vp);
        JSBool APICALL (*JS_DeleteUCProperty2)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, jschar * name, ULONG namelen, jsval * rval);
        JSObject * APICALL (*JS_NewArrayObject)(struct SpiderJSIFace *Self, JSContext * cx, LONG length, jsval * vector);
        JSBool APICALL (*JS_IsArrayObject)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj);
        JSBool APICALL (*JS_GetArrayLength)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, jsuint * lengthp);
        JSBool APICALL (*JS_SetArrayLength)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, LONG length);
        JSBool APICALL (*JS_HasArrayLength)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, jsuint * lengthp);
        JSBool APICALL (*JS_DefineElement)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, LONG index, LONG value, LONG getter, LONG setter, LONG attrs);
        JSBool APICALL (*JS_AliasElement)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, LONG alias);
        JSBool APICALL (*JS_HasElement)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, LONG index, JSBool * foundp);
        JSBool APICALL (*JS_LookupElement)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, LONG index, jsval * vp);
        JSBool APICALL (*JS_GetElement)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, LONG index, jsval * vp);
        JSBool APICALL (*JS_SetElement)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, LONG index, jsval * vp);
        JSBool APICALL (*JS_DeleteElement)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, LONG index);
        JSBool APICALL (*JS_DeleteElement2)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, LONG index, jsval * rval);
        void APICALL (*JS_ClearScope)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj);
        JSIdArray * APICALL (*JS_Enumerate)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj);
        JSObject * APICALL (*JS_NewPropertyIterator)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj);
        JSBool APICALL (*JS_NextProperty)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * iterobj, jsid * idp);
        JSBool APICALL (*JS_CheckAccess)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, LONG id, LONG mode, jsval * vp, uintN * attrsp);
        JSCheckAccessOp APICALL (*JS_SetCheckObjectAccessCallback)(struct SpiderJSIFace *Self, JSRuntime * rt, LONG acb);
        JSBool APICALL (*JS_GetReservedSlot)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, uint32 index, jsval * vp);
        JSBool APICALL (*JS_SetReservedSlot)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, uint32 index, LONG v);
        JSPrincipalsTranscoder APICALL (*JS_SetPrincipalsTranscoder)(struct SpiderJSIFace *Self, JSRuntime * rt, LONG px);
        JSObjectPrincipalsFinder APICALL (*JS_SetObjectPrincipalsFinder)(struct SpiderJSIFace *Self, JSRuntime * rt, LONG fop);
        JSFunction * APICALL (*JS_NewFunction)(struct SpiderJSIFace *Self, JSContext * cx, LONG call, LONG nargs, LONG flags, JSObject * parent, const char * name);
        JSObject * APICALL (*JS_GetFunctionObject)(struct SpiderJSIFace *Self, JSFunction * fun);
        const char * APICALL (*JS_GetFunctionName)(struct SpiderJSIFace *Self, JSFunction * fun);
        JSString * APICALL (*JS_GetFunctionId)(struct SpiderJSIFace *Self, JSFunction * fun);
        uintN APICALL (*JS_GetFunctionFlags)(struct SpiderJSIFace *Self, JSFunction * fun);
        uint16 APICALL (*JS_GetFunctionArity)(struct SpiderJSIFace *Self, JSFunction * fun);
        JSBool APICALL (*JS_ObjectIsFunction)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj);
        JSBool APICALL (*JS_DefineFunctions)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSFunctionSpec * fs);
        JSFunction * APICALL (*JS_DefineFunction)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, LONG call, LONG nargs, LONG attrs);
        JSFunction * APICALL (*JS_DefineUCFunction)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, jschar * name, ULONG namelen, LONG call, LONG nargs, LONG attrs);
        JSObject * APICALL (*JS_CloneFunctionObject)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * funobj, JSObject * parent);
        JSBool APICALL (*JS_BufferIsCompilableUnit)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * bytes, ULONG length);
        JSScript * APICALL (*JS_CompileScript)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * bytes, ULONG length, const char * filename, LONG lineno);
        JSScript * APICALL (*JS_CompileScriptForPrincipals)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSPrincipals * principals, const char * bytes, ULONG length, const char * filename, LONG lineno);
        JSScript * APICALL (*JS_CompileUCScript)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, jschar * chars, ULONG length, const char * filename, LONG lineno);
        JSScript * APICALL (*JS_CompileUCScriptForPrincipals)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSPrincipals * principals, jschar * chars, ULONG length, const char * filename, LONG lineno);
        JSScript * APICALL (*JS_CompileFile)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * filename);
        JSScript * APICALL (*JS_CompileFileHandle)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * filename, FILE * fh);
        JSScript * APICALL (*JS_CompileFileHandleForPrincipals)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * filename, FILE * fh, JSPrincipals * principals);
        JSObject * APICALL (*JS_NewScriptObject)(struct SpiderJSIFace *Self, JSContext * cx, JSScript * script);
        JSObject * APICALL (*JS_GetScriptObject)(struct SpiderJSIFace *Self, JSScript * script);
        void APICALL (*JS_DestroyScript)(struct SpiderJSIFace *Self, JSContext * cx, JSScript * script);
        JSFunction * APICALL (*JS_CompileFunction)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, LONG nargs, const char ** argnames, const char * bytes, ULONG length, const char * filename, LONG lineno);
        JSFunction * APICALL (*JS_CompileFunctionForPrincipals)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSPrincipals * principals, const char * name, LONG nargs, const char ** argnames, const char * bytes, ULONG length, const char * filename, LONG lineno);
        JSFunction * APICALL (*JS_CompileUCFunction)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, LONG nargs, const char ** argnames, jschar * chars, ULONG length, const char * filename, LONG lineno);
        JSFunction * APICALL (*JS_CompileUCFunctionForPrincipals)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSPrincipals * principals, const char * name, LONG nargs, const char ** argnames, jschar * chars, ULONG length, const char * filename, LONG lineno);
        JSString * APICALL (*JS_DecompileScript)(struct SpiderJSIFace *Self, JSContext * cx, JSScript * script, const char * name, LONG indent);
        JSString * APICALL (*JS_DecompileFunction)(struct SpiderJSIFace *Self, JSContext * cx, JSFunction * fun, LONG indent);
        JSString * APICALL (*JS_DecompileFunctionBody)(struct SpiderJSIFace *Self, JSContext * cx, JSFunction * fun, LONG indent);
        JSBool APICALL (*JS_ExecuteScript)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSScript * script, jsval * rval);
        JSBool APICALL (*JS_ExecuteScriptPart)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSScript * script, LONG part);
        JSBool APICALL (*JS_EvaluateScript)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * bytes, LONG length, const char * filename, LONG lineno, jsval * rval);
        JSBool APICALL (*JS_EvaluateScriptForPrincipals)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSPrincipals * principals, const char * bytes, LONG length, const char * filename, LONG lineno, jsval * rval);
        JSBool APICALL (*JS_EvaluateUCScript)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, jschar * chars, LONG length, const char * filename, LONG lineno, jsval * rval);
        JSBool APICALL (*JS_EvaluateUCScriptForPrincipals)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSPrincipals * principals, jschar * chars, LONG length, const char * filename, LONG lineno, jsval * rval);
        JSBool APICALL (*JS_CallFunction)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, JSFunction * fun, LONG argc, jsval * argv, jsval * rval);
        JSBool APICALL (*JS_CallFunctionName)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, const char * name, LONG argc, jsval * argv, jsval * rval);
        JSBool APICALL (*JS_CallFunctionValue)(struct SpiderJSIFace *Self, JSContext * cx, JSObject * obj, LONG fval, LONG argc, jsval * argv, jsval * rval);
        JSBranchCallback APICALL (*JS_SetBranchCallback)(struct SpiderJSIFace *Self, JSContext * cx, LONG cb);
        JSBool APICALL (*JS_IsRunning)(struct SpiderJSIFace *Self, JSContext * cx);
        JSBool APICALL (*JS_IsConstructing)(struct SpiderJSIFace *Self, JSContext * cx);
        JSBool APICALL (*JS_IsAssigning)(struct SpiderJSIFace *Self, JSContext * cx);
        void APICALL (*JS_SetCallReturnValue2)(struct SpiderJSIFace *Self, JSContext * cx, LONG v);
        JSString * APICALL (*JS_NewString)(struct SpiderJSIFace *Self, JSContext * cx, char * bytes, ULONG length);
        JSString * APICALL (*JS_NewStringCopyN)(struct SpiderJSIFace *Self, JSContext * cx, const char * s, ULONG n);
        JSString * APICALL (*JS_NewStringCopyZ)(struct SpiderJSIFace *Self, JSContext * cx, const char * s);
        JSString * APICALL (*JS_InternString)(struct SpiderJSIFace *Self, JSContext * cx, const char * s);
        JSString * APICALL (*JS_NewUCString)(struct SpiderJSIFace *Self, JSContext * cx, jschar * chars, ULONG length);
        JSString * APICALL (*JS_NewUCStringCopyN)(struct SpiderJSIFace *Self, JSContext * cx, jschar * s, ULONG n);
        JSString * APICALL (*JS_NewUCStringCopyZ)(struct SpiderJSIFace *Self, JSContext * cx, jschar * s);
        JSString * APICALL (*JS_InternUCStringN)(struct SpiderJSIFace *Self, JSContext * cx, jschar * s, ULONG length);
        JSString * APICALL (*JS_InternUCString)(struct SpiderJSIFace *Self, JSContext * cx, jschar * s);
        char * APICALL (*JS_GetStringBytes)(struct SpiderJSIFace *Self, JSString * str);
        jschar * APICALL (*JS_GetStringChars)(struct SpiderJSIFace *Self, JSString * str);
        size_t APICALL (*JS_GetStringLength)(struct SpiderJSIFace *Self, JSString * str);
        intN APICALL (*JS_CompareStrings)(struct SpiderJSIFace *Self, JSString * str1, JSString * str2);
        JSString * APICALL (*JS_NewGrowableString)(struct SpiderJSIFace *Self, JSContext * cx, jschar * chars, ULONG length);
        JSString * APICALL (*JS_NewDependentString)(struct SpiderJSIFace *Self, JSContext * cx, JSString * str, ULONG start, ULONG length);
        JSString * APICALL (*JS_ConcatStrings)(struct SpiderJSIFace *Self, JSContext * cx, JSString * left, JSString * right);
        jschar * APICALL (*JS_UndependString)(struct SpiderJSIFace *Self, JSContext * cx, JSString * str);
        JSBool APICALL (*JS_MakeStringImmutable)(struct SpiderJSIFace *Self, JSContext * cx, JSString * str);
        JSBool APICALL (*JS_StringsAreUTF8)(struct SpiderJSIFace *Self);
        JSBool APICALL (*JS_EncodeCharacters)(struct SpiderJSIFace *Self, JSContext * cx, jschar * src, ULONG srclen, char * dst, size_t * dstlenp);
        JSBool APICALL (*JS_DecodeBytes)(struct SpiderJSIFace *Self, JSContext * cx, const char * src, ULONG srclen, jschar * dst, size_t * dstlenp);
        void APICALL (*JS_SetLocaleCallbacks)(struct SpiderJSIFace *Self, JSContext * cx, JSLocaleCallbacks * callbacks);
        JSLocaleCallbacks * APICALL (*JS_GetLocaleCallbacks)(struct SpiderJSIFace *Self, JSContext * cx);
        void APICALL (*JS_ReportErrorA)(struct SpiderJSIFace *Self, JSContext * cx, const char * format, ULONG * args);
        void APICALL (*JS_ReportError)(struct SpiderJSIFace *Self, JSContext * cx, const char * format, ...);
        void APICALL (*JS_ReportErrorNumberA)(struct SpiderJSIFace *Self, JSContext * cx, LONG errorCallback, void * userRef, LONG errorNumber, ULONG * args);
        void APICALL (*JS_ReportErrorNumber)(struct SpiderJSIFace *Self, JSContext * cx, LONG errorCallback, void * userRef, LONG errorNumber, ...);
        void APICALL (*JS_ReportErrorNumberUCA)(struct SpiderJSIFace *Self, JSContext * cx, LONG errorCallback, void * userRef, LONG errorNumber, ULONG * args);
        void APICALL (*JS_ReportErrorNumberUC)(struct SpiderJSIFace *Self, JSContext * cx, LONG errorCallback, void * userRef, LONG errorNumber, ...);
        JSBool APICALL (*JS_ReportWarningA)(struct SpiderJSIFace *Self, JSContext * cx, const char * format, ULONG * args);
        JSBool APICALL (*JS_ReportWarning)(struct SpiderJSIFace *Self, JSContext * cx, const char * format, ...);
        JSBool APICALL (*JS_ReportErrorFlagsAndNumberA)(struct SpiderJSIFace *Self, JSContext * cx, LONG flags, LONG errorCallback, void * userRef, LONG errorNumber, ULONG * args);
        JSBool APICALL (*JS_ReportErrorFlagsAndNumber)(struct SpiderJSIFace *Self, JSContext * cx, LONG flags, LONG errorCallback, void * userRef, LONG errorNumber, ...);
        JSBool APICALL (*JS_ReportErrorFlagsAndNumberUCA)(struct SpiderJSIFace *Self, JSContext * cx, LONG flags, LONG errorCallback, void * userRef, LONG errorNumber, ULONG * args);
        JSBool APICALL (*JS_ReportErrorFlagsAndNumberUC)(struct SpiderJSIFace *Self, JSContext * cx, LONG flags, LONG errorCallback, void * userRef, LONG errorNumber, ...);
        void APICALL (*JS_ReportOutOfMemory)(struct SpiderJSIFace *Self, JSContext * cx);
        JSErrorReporter APICALL (*JS_SetErrorReporter)(struct SpiderJSIFace *Self, JSContext * cx, LONG er);
        JSObject * APICALL (*JS_NewRegExpObject)(struct SpiderJSIFace *Self, JSContext * cx, char * bytes, ULONG length, LONG flags);
        JSObject * APICALL (*JS_NewUCRegExpObject)(struct SpiderJSIFace *Self, JSContext * cx, jschar * chars, ULONG length, LONG flags);
        void APICALL (*JS_SetRegExpInput)(struct SpiderJSIFace *Self, JSContext * cx, JSString * input, LONG multiline);
        void APICALL (*JS_ClearRegExpStatics)(struct SpiderJSIFace *Self, JSContext * cx);
        void APICALL (*JS_ClearRegExpRoots)(struct SpiderJSIFace *Self, JSContext * cx);
        JSBool APICALL (*JS_IsExceptionPending)(struct SpiderJSIFace *Self, JSContext * cx);
        JSBool APICALL (*JS_GetPendingException)(struct SpiderJSIFace *Self, JSContext * cx, jsval * vp);
        void APICALL (*JS_SetPendingException)(struct SpiderJSIFace *Self, JSContext * cx, LONG v);
        void APICALL (*JS_ClearPendingException)(struct SpiderJSIFace *Self, JSContext * cx);
        JSBool APICALL (*JS_ReportPendingException)(struct SpiderJSIFace *Self, JSContext * cx);
        JSExceptionState * APICALL (*JS_SaveExceptionState)(struct SpiderJSIFace *Self, JSContext * cx);
        void APICALL (*JS_RestoreExceptionState)(struct SpiderJSIFace *Self, JSContext * cx, JSExceptionState * state);
        void APICALL (*JS_DropExceptionState)(struct SpiderJSIFace *Self, JSContext * cx, JSExceptionState * state);
        JSErrorReport * APICALL (*JS_ErrorFromException)(struct SpiderJSIFace *Self, JSContext * cx, LONG v);
        JSBool APICALL (*JS_ThrowReportedError)(struct SpiderJSIFace *Self, JSContext * cx, const char * message, JSErrorReport * reportp);
};

#endif /* SPIDERJS_INTERFACE_DEF_H */