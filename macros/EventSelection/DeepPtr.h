#ifndef DEEPPTR_H
#define DEEPPTR_H
#include <iostream>
#include <memory>

template <typename T>
class DeepPtr
{
public:
    DeepPtr( std::nullptr_t value  = nullptr) :myPtr( nullptr ) {}

    DeepPtr( const T& value ) :myPtr( new T{ value } ) {}
 
    DeepPtr( T* value ) :myPtr( value ) {}

    DeepPtr( const DeepPtr& other ) noexcept
    :myPtr( nullptr )
    {
        if ( other )
        {
            myPtr = std::unique_ptr<T>{ new T{ *other } };
        }
    }

    DeepPtr( DeepPtr&& other )
    :myPtr( nullptr )
    {
        if ( other )
            myPtr = std::move(other);
    }

    DeepPtr& operator=( const DeepPtr& other )
    {
        DeepPtr temp{ other };
        swap( *this, temp );
        return *this;
    }

    DeepPtr& operator=( DeepPtr&& other ) noexcept
    {
        swap( *this, other );
        return *this;
    }

    static void swap( DeepPtr& left, DeepPtr& right ) { std::swap( left.myPtr, right.myPtr ); }

    T& operator*() { return *myPtr; }

    const T& operator*() const { return *myPtr; }

    T* const operator->() { return myPtr.operator->(); }

    const T* const operator->() const { return myPtr.operator->(); }

    const T* const get() const { return myPtr.get(); }

    explicit operator bool() const { return (bool)myPtr; }

private:
    std::unique_ptr<T> myPtr;
};

#endif
