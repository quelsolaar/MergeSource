#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <objc/runtime.h>
#include <objc/message.h>

typedef struct 
{
    const char*     pName;
    const char*     pSignature;
    void*           pFunctionPointer;
} c_ocoa_class_definition_method_t;

typedef struct
{
    Class                               pObjcClass;
    const char*                         pName;
    c_ocoa_class_definition_method_t*   pMethods;
    unsigned int                        methodCount;
    unsigned int                        methodCapacity;
    unsigned char                       isFinished;
} c_ocoa_class_definition_t;

c_ocoa_class_definition_t* c_ocoa_create_class_definition( const char* pClassName )
{
    //FK: Add custom memory allocation strategy?
    const size_t initialMethodCapacity = 8;
    c_ocoa_class_definition_method_t* pMethodsArray = (c_ocoa_class_definition_method_t*)malloc( sizeof( c_ocoa_class_definition_method_t ) * initialMethodCapacity );

    c_ocoa_class_definition_t* pClassDefinition = (c_ocoa_class_definition_t*)malloc( sizeof( c_ocoa_class_definition_t* ) );
    if( pClassDefinition == NULL || pMethodsArray == NULL )
    {
        free( pClassDefinition );
        free( pMethodsArray );

        printf("Error: Out of memory while calling 'cocoa_create_class_definition'.\n");
        return NULL;
    }

    pClassDefinition->isFinished        = 0;
    pClassDefinition->pObjcClass        = NULL;
    pClassDefinition->methodCapacity    = initialMethodCapacity;
    pClassDefinition->methodCount       = 0;
    pClassDefinition->pMethods          = pMethodsArray;
    pClassDefinition->pName             = pClassName;
    
    return pClassDefinition;
}

int c_ocoa_add_class_definition_method( c_ocoa_class_definition_t* pClassDefinition, const char* pMethodName, void* pFunctionPointer, const char* pObjcFunctionSignature )
{
    if( pClassDefinition->isFinished )
    {
        printf("Warning: Can't add more methods to class '%s' because 'cocoa_finish_class_definition' has already been called.\n", pClassDefinition->pName );
        return 0;
    }

    const size_t newMethodCount = pClassDefinition->methodCount + 1;
    if( newMethodCount == pClassDefinition->methodCapacity )
    {
        const size_t newMethodCapacity = pClassDefinition->methodCapacity * 2;
        const size_t newMethodArraySizeInBytes = sizeof( c_ocoa_class_definition_method_t ) * newMethodCapacity;
        c_ocoa_class_definition_method_t* pNewMethodsArray = (c_ocoa_class_definition_method_t*)realloc( pClassDefinition->pMethods, newMethodArraySizeInBytes );

        if( pNewMethodsArray == NULL )
        {
            printf("Error: Could not expand methods array of c-ocoa class definition '%s'.\n", pClassDefinition->pName );
            return 0;
        }

        pClassDefinition->pMethods          = pNewMethodsArray;
        pClassDefinition->methodCapacity    = newMethodCapacity;
    }

    const unsigned int methodIndex = pClassDefinition->methodCount++;
    pClassDefinition->pMethods[ methodIndex ].pName             = pMethodName;
    pClassDefinition->pMethods[ methodIndex ].pFunctionPointer  = pFunctionPointer;
    pClassDefinition->pMethods[ methodIndex ].pSignature        = pObjcFunctionSignature;

    return 1;
}

int c_ocoa_finish_class_definition( c_ocoa_class_definition_t* pClassDefinition )
{
    if( pClassDefinition->isFinished )
    {
        printf("Warning: Class '%s' already finished.\n", pClassDefinition->pName);
        return 0;
    }

    Class pNSObjectClass = objc_getClass("NSObject");
    if( pNSObjectClass == NULL )
    {
        printf("Error: 'objc_getClass(\"NSObject\")' returned NULL - please check what Frameworks you're linking against.\n");
        return 0;
    }

    Class pCustomClass = objc_allocateClassPair( pNSObjectClass, pClassDefinition->pName, 0);
    if( pCustomClass == NULL )
    {
        printf("Error: Couldn't create new class for c_ocoa class definition '%s'.\n", pClassDefinition->pName);
        return 0;
    }

    pClassDefinition->pObjcClass = pCustomClass;
    pClassDefinition->isFinished = 1;

    for( unsigned int methodIndex = 0; methodIndex < pClassDefinition->methodCount; ++methodIndex )
    {
        const c_ocoa_class_definition_method_t* pMethodDefinition = pClassDefinition->pMethods + methodIndex;
        SEL pMethodSelectorName = sel_registerName( pMethodDefinition->pName );
        BOOL methodAdded = class_addMethod( pCustomClass, pMethodSelectorName, (IMP)pMethodDefinition->pFunctionPointer, pMethodDefinition->pSignature );
        if( !methodAdded )
        {
            printf("Error: Couldn't register method '%s' for c_ocoa class definition '%s'.\n", pMethodDefinition->pName, pClassDefinition->pName );
            continue;
        }
    }

    objc_registerClassPair( pCustomClass );

    return 1;
}

void* c_ocoa_alloc_object_of_class( const c_ocoa_class_definition_t* pClassDefinition )
{
    if( !pClassDefinition->isFinished )
    {
        printf("Warning: Can't alloc object of class '%s' because the class isn't finished yet.\n", pClassDefinition->pName);
        return NULL;
    }
    
    SEL allocSelector = sel_registerName("alloc");
    return ((id (*)(id, SEL))objc_msgSend)(pClassDefinition->pObjcClass, allocSelector);
}