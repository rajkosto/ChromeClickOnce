#pragma once

#include "WinApi.h"
#include <npapi/npapi.h>
#include <npapi/npfunctions.h>

template <class T>
class NPScriptableObject : public NPObject
{
public:
    // Default implementations to save having to implement not implemented functions
    virtual void Invalidate() { }
    virtual bool HasMethod(NPIdentifier name) { return false; }
    virtual bool Invoke(NPIdentifier method, const NPVariant* args, uint32_t argc, NPVariant* result) { return false; }
    virtual bool InvokeDefault(const NPVariant* args, uint32_t argc, NPVariant* result) { return false; }
    virtual bool HasProperty(NPIdentifier name) { return false; }
    virtual bool GetProperty(NPIdentifier name, NPVariant* result) { return false; }
    virtual bool SetProperty(NPIdentifier name, const NPVariant* value) { return false; }
    virtual bool RemoveProperty(NPIdentifier name) { return false; }
    virtual bool Enumerate(NPIdentifier **identifier, uint32_t *count) { return false; }
    virtual bool Construct(const NPVariant *args, uint32_t argc, NPVariant *result) { return false; }

    static NPObject* CreateInstance(NPP instance)
    {
        // I feel like there's a hack involving the new operator here somehow
        // but that might be more trouble than it's worth.
        return NPN_CreateObject(instance, &_npclass);
    }

protected:
    NPScriptableObject(NPP npp) { _npp = npp; }
    virtual ~NPScriptableObject() { }

    /////////////////////////////
    // Static NPObject methods //
    /////////////////////////////
    static NPObject* _Allocate(NPP npp, NPClass *aClass);
    static void _Deallocate(NPObject *npobj);

    static void _Invalidate(NPObject *npobj);
    static bool _HasMethod(NPObject *npobj, NPIdentifier name);
    static bool _Invoke(NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argc, NPVariant *result);
    static bool _InvokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argc, NPVariant *result);
    static bool _HasProperty(NPObject * npobj, NPIdentifier name);
    static bool _GetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result);
    static bool _SetProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value);
    static bool _RemoveProperty(NPObject *npobj, NPIdentifier name);
    static bool _Enumerate(NPObject *npobj, NPIdentifier **identifier, uint32_t *count);
    static bool _Construct(NPObject *npobj, const NPVariant *args, uint32_t argc, NPVariant *result);

    static NPClass _npclass;


    NPP _npp;
};

template<class T>
NPClass NPScriptableObject<T>::_npclass = {
    NP_CLASS_STRUCT_VERSION,
    NPScriptableObject::_Allocate,
    NPScriptableObject::_Deallocate,
    NPScriptableObject::_Invalidate,
    NPScriptableObject::_HasMethod,
    NPScriptableObject::_Invoke,
    NPScriptableObject::_InvokeDefault,
    NPScriptableObject::_HasProperty,
    NPScriptableObject::_GetProperty,
    NPScriptableObject::_SetProperty,
    NPScriptableObject::_RemoveProperty,
    NPScriptableObject::_Enumerate,
    NPScriptableObject::_Construct,
};

/*
 *   Static NPClass delegation methods
 */

template<class T>
NPObject* NPScriptableObject<T>::_Allocate(NPP npp, NPClass *aClass)
{
    return new T(npp);
}

template<class T>
void NPScriptableObject<T>::_Deallocate(NPObject *npobj)
{
    delete ((NPScriptableObject<T>*)npobj);
}

template<class T>
void NPScriptableObject<T>::_Invalidate(NPObject *npobj)
{
    ((NPScriptableObject<T>*)npobj)->Invalidate();
}

template<class T>
bool NPScriptableObject<T>::_HasMethod(NPObject *npobj, NPIdentifier name)
{
    return ((NPScriptableObject<T>*)npobj)->HasMethod(name);
}

template<class T>
bool NPScriptableObject<T>::_Invoke(NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argc, NPVariant *result)
{
    return ((NPScriptableObject<T>*)npobj)->Invoke(name, args, argc, result);
}

template<class T>
bool NPScriptableObject<T>::_InvokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argc, NPVariant *result)
{
    return ((NPScriptableObject<T>*)npobj)->InvokeDefault(args, argc, result);
}

template<class T>
bool NPScriptableObject<T>::_HasProperty(NPObject * npobj, NPIdentifier name)
{
    return ((NPScriptableObject<T>*)npobj)->HasProperty(name);
}

template<class T>
bool NPScriptableObject<T>::_GetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result)
{
    return ((NPScriptableObject<T>*)npobj)->GetProperty(name, result);
}

template<class T>
bool NPScriptableObject<T>::_SetProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
    return ((NPScriptableObject<T>*)npobj)->SetProperty(name, value);
}

template<class T>
bool NPScriptableObject<T>::_RemoveProperty(NPObject *npobj, NPIdentifier name)
{
   return ((NPScriptableObject<T>*)npobj)->RemoveProperty(name);
}

template<class T>
bool NPScriptableObject<T>::_Enumerate(NPObject *npobj, NPIdentifier **identifier, uint32_t *count)
{
    return ((NPScriptableObject<T>*)npobj)->Enumerate(identifier, count);
}

template<class T>
bool NPScriptableObject<T>::_Construct(NPObject *npobj, const NPVariant *args, uint32_t argc, NPVariant *result)
{
    return ((NPScriptableObject<T>*)npobj)->Construct(args, argc, result);
}
