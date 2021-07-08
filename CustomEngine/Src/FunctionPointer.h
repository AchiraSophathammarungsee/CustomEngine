#ifndef FUNCTIONPOINTER_H
#define FUNCTIONPOINTER_H
#include "Log.h"

//template<typename C, typename T, typename R>
//class FunctionPointer
//{
//public:
//  FunctionPointer(R(*funcptr)(T), C* instance = nullptr) : FunctionPtr(funcptr), ClassInstance(instance) {};
//  ~FunctionPointer() {};
//
//  R Call(T param)
//  {
//    if (ClassInstance)
//    {
//      return ClassInstance->FunctionPtr(param);
//    }
//    else
//    {
//      return FunctionPtr(param);
//    }
//  };
//
//private:
//  C* ClassInstance;
//  R (*FunctionPtr)(T);
//};

//template <typename C, typename R>
//struct InstanceFuncPtr
//{
//public:
//  InstanceFuncPtr() : Instance(nullptr), Function(nullptr) {};
//  InstanceFuncPtr(C* objinstance, R(C::* funcptr)()) : Instance(objinstance), Function(funcptr) {};
//  ~InstanceFuncPtr() {};
//
//  bool IsValid() { return Instance && Function; };
//
//  R Call()
//  {
//    if (IsValid())
//      return this->*Function();
//
//    Log::Write("Attempt to call invalid instance function pointer!", "Function Pointer", Log::Error);
//    return R();
//  };
//
//private:
//  C* Instance;
//  R(C::* Function)();
//};

#endif