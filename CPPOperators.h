/*
 * CPPOperators.h
 *
 * Created: 30/03/2020 10.42.14
 *  Author: Nicklas Grunert (@github.com/LordSyFo)
 */ 


#ifndef CPPOPERATORS_H_
#define CPPOPERATORS_H_

#include <stdlib.h>

/* Might be required for pure abstract classes and such (seems to work without atm)
extern "C" int __cxa_guard_acquire(__guard *);
extern "C" void __cxa_guard_release (__guard *);
extern "C" void __cxa_guard_abort (__guard *);

int __cxa_guard_acquire(__guard *g) {return !*(char *)(g);};
void __cxa_guard_release (__guard *g) {*(char *)g = 1;};
void __cxa_guard_abort (__guard *) {};
	
extern "C" void __cxa_pure_virtual(void);	
*/

__extension__ typedef int __guard __attribute__((mode (__DI__)));

void * operator new(size_t size);
void operator delete(void * ptr);
void * operator new[](size_t size);
void operator delete[](void * ptr);

int __cxa_guard_acquire(__guard *g) {return !*(char *)(g);};
void __cxa_guard_release (__guard *g) {*(char *)g = 1;};
void __cxa_guard_abort (__guard *) {};


void * operator new(size_t size)
{
	return malloc(size);
}

void operator delete(void * ptr)
{
	ptr != NULL ? free(ptr) : void(0);
}

void * operator new[](size_t size)
{
	return malloc(size);
}

void operator delete[](void * ptr)
{
	ptr != NULL ? free(ptr) : void(0);
}


#endif /* CPPOPERATORS_H_ */