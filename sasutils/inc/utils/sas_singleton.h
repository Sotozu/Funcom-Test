#pragma once 

template< typename T >
class sasSingleton
{
  sasNO_COPY(sasSingleton);

public:
  static bool isValid()     { return _singleton!=0; }
  static T& singleton()     { return *_singleton; }
  static T* singletonPtr()  { return _singleton; }

protected:
  sasSingleton()      { sasASSERT(!_singleton); _singleton = (T*)this; }
  ~sasSingleton()     { sasASSERT(_singleton); _singleton = 0; }

private:
  static T* _singleton;
};

template< typename T >
T* sasSingleton<T>::_singleton = 0;
