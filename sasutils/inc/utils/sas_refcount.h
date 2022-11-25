#pragma once 

class sasRefCounter
{
public:
  sasRefCounter()      :_refCount( 1 )  {}
  virtual ~sasRefCounter()              {}

  long incRefCount()          { return ( ++_refCount ); }
  void releaseRefCount()      
  { 
    sasASSERT( _refCount > 0 ); 
    --_refCount; 
    if( _refCount <= 0 )
      delete this;
  }

  long refCount() const     { return _refCount; }

private:
  mutable long  _refCount;
};


