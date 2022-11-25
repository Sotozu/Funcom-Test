#pragma once 

class sasStringId
{
public: //! types
  typedef uint32_t Key;
  enum { Invalid = 0 };

public: //! methods
  bool isValid( ) const { return _hash != Invalid; }
  operator Key() const { return _hash; }
  const char* string() const;

public: // operators  
  bool operator==( const sasStringId& other ) const { return _hash == other._hash; }
  bool operator<( const sasStringId& other ) const { return _hash < other._hash; }

public: //! ctor/dtor
  static sasStringId build( const char* str );
  static sasStringId invalid();

  sasStringId( ) : _hash( Invalid ) {}		

  sasStringId( const char (&str)[1] )
  {    
    size_t unused = sizeof(str[0]);
    Key hash0 = 5381;    
    _hash = hash0;
    setDebugString( str );
  }	
  sasStringId( const char (&str)[2] )
  {
    Key hash0 = 5381;
    Key hash1 = ((hash0<<5)+hash0) + str[0];    
    _hash = hash1;
    setDebugString( str );
  }
  sasStringId( const char (&str)[3] )
  {
    Key hash0 = 5381;
    Key hash1 = ((hash0<<5)+hash0) + str[0];
    Key hash2 = ((hash1<<5)+hash1) + str[1];    
    _hash = hash2;
    setDebugString( str );
  }
  sasStringId( const char (&str)[4] )
  {
    Key hash0 = 5381;
    Key hash1 = ((hash0<<5)+hash0) + str[0];
    Key hash2 = ((hash1<<5)+hash1) + str[1];
    Key hash3 = ((hash2<<5)+hash2) + str[2];    
    _hash = hash3;
    setDebugString( str );
  }	
  sasStringId( const char (&str)[5] )
  {
    Key hash0 = 5381;
    Key hash1 = ((hash0<<5)+hash0) + str[0];
    Key hash2 = ((hash1<<5)+hash1) + str[1];
    Key hash3 = ((hash2<<5)+hash2) + str[2];
    Key hash4 = ((hash3<<5)+hash3) + str[3];    
    _hash = hash4;
    setDebugString( str );
  }	
  sasStringId( const char (&str)[6] )
  {
    Key hash0 = 5381;
    Key hash1 = ((hash0<<5)+hash0) + str[0];
    Key hash2 = ((hash1<<5)+hash1) + str[1];
    Key hash3 = ((hash2<<5)+hash2) + str[2];
    Key hash4 = ((hash3<<5)+hash3) + str[3];
    Key hash5 = ((hash4<<5)+hash4) + str[4];    
    _hash = hash5;
    setDebugString( str );
  }	
  sasStringId( const char (&str)[7] )
  {
    Key hash0 = 5381;
    Key hash1 = ((hash0<<5)+hash0) + str[0];
    Key hash2 = ((hash1<<5)+hash1) + str[1];
    Key hash3 = ((hash2<<5)+hash2) + str[2];
    Key hash4 = ((hash3<<5)+hash3) + str[3];
    Key hash5 = ((hash4<<5)+hash4) + str[4];
    Key hash6 = ((hash5<<5)+hash5) + str[5];    
    _hash = hash6;
    setDebugString( str );
  }
  sasStringId( const char (&str)[8] )
  {
    Key hash0 = 5381;
    Key hash1 = ((hash0<<5)+hash0) + str[0];
    Key hash2 = ((hash1<<5)+hash1) + str[1];
    Key hash3 = ((hash2<<5)+hash2) + str[2];
    Key hash4 = ((hash3<<5)+hash3) + str[3];
    Key hash5 = ((hash4<<5)+hash4) + str[4];
    Key hash6 = ((hash5<<5)+hash5) + str[5];
    Key hash7 = ((hash6<<5)+hash6) + str[6];        
    _hash = hash7;
    setDebugString( str );
  }
  sasStringId( const char (&str)[9] )
  {
    Key hash0 = 5381;
    Key hash1 = ((hash0<<5)+hash0) + str[0];
    Key hash2 = ((hash1<<5)+hash1) + str[1];
    Key hash3 = ((hash2<<5)+hash2) + str[2];
    Key hash4 = ((hash3<<5)+hash3) + str[3];
    Key hash5 = ((hash4<<5)+hash4) + str[4];
    Key hash6 = ((hash5<<5)+hash5) + str[5];
    Key hash7 = ((hash6<<5)+hash6) + str[6];    
    Key hash8 = ((hash7<<5)+hash7) + str[7];    
    _hash = hash8;
    setDebugString( str );
  }
  sasStringId( const char (&str)[10] )
  {
    Key hash0 = 5381;
    Key hash1 = ((hash0<<5)+hash0) + str[0];
    Key hash2 = ((hash1<<5)+hash1) + str[1];
    Key hash3 = ((hash2<<5)+hash2) + str[2];
    Key hash4 = ((hash3<<5)+hash3) + str[3];
    Key hash5 = ((hash4<<5)+hash4) + str[4];
    Key hash6 = ((hash5<<5)+hash5) + str[5];
    Key hash7 = ((hash6<<5)+hash6) + str[6];    
    Key hash8 = ((hash7<<5)+hash7) + str[7];
    Key hash9 = ((hash8<<5)+hash8) + str[8];    
    _hash = hash9;
    setDebugString( str );
  }
  sasStringId( const char (&str)[11] )
  {
    Key hash0 = 5381;
    Key hash1 = ((hash0<<5)+hash0) + str[0];
    Key hash2 = ((hash1<<5)+hash1) + str[1];
    Key hash3 = ((hash2<<5)+hash2) + str[2];
    Key hash4 = ((hash3<<5)+hash3) + str[3];
    Key hash5 = ((hash4<<5)+hash4) + str[4];
    Key hash6 = ((hash5<<5)+hash5) + str[5];
    Key hash7 = ((hash6<<5)+hash6) + str[6];    
    Key hash8 = ((hash7<<5)+hash7) + str[7];
    Key hash9 = ((hash8<<5)+hash8) + str[8];
    Key hash10 = ((hash9<<5)+hash9) + str[9];    
    _hash = hash10;
    setDebugString( str );
  }
  sasStringId( const char (&str)[12] )
  {
    Key hash0 = 5381;
    Key hash1 = ((hash0<<5)+hash0) + str[0];
    Key hash2 = ((hash1<<5)+hash1) + str[1];
    Key hash3 = ((hash2<<5)+hash2) + str[2];
    Key hash4 = ((hash3<<5)+hash3) + str[3];
    Key hash5 = ((hash4<<5)+hash4) + str[4];
    Key hash6 = ((hash5<<5)+hash5) + str[5];
    Key hash7 = ((hash6<<5)+hash6) + str[6];    
    Key hash8 = ((hash7<<5)+hash7) + str[7];
    Key hash9 = ((hash8<<5)+hash8) + str[8];
    Key hash10 = ((hash9<<5)+hash9) + str[9];
    Key hash11 = ((hash10<<5)+hash10) + str[10];    
    _hash = hash11;
    setDebugString( str );
  }
  sasStringId( const char (&str)[13] )
  {
    Key hash0 = 5381;
    Key hash1 = ((hash0<<5)+hash0) + str[0];
    Key hash2 = ((hash1<<5)+hash1) + str[1];
    Key hash3 = ((hash2<<5)+hash2) + str[2];
    Key hash4 = ((hash3<<5)+hash3) + str[3];
    Key hash5 = ((hash4<<5)+hash4) + str[4];
    Key hash6 = ((hash5<<5)+hash5) + str[5];
    Key hash7 = ((hash6<<5)+hash6) + str[6];    
    Key hash8 = ((hash7<<5)+hash7) + str[7];
    Key hash9 = ((hash8<<5)+hash8) + str[8];
    Key hash10 = ((hash9<<5)+hash9) + str[9];
    Key hash11 = ((hash10<<5)+hash10) + str[10];
    Key hash12 = ((hash11<<5)+hash11) + str[11];    
    _hash = hash12;
    setDebugString( str );
  }
  sasStringId( const char (&str)[14] )
  {
    Key hash0 = 5381;
    Key hash1 = ((hash0<<5)+hash0) + str[0];
    Key hash2 = ((hash1<<5)+hash1) + str[1];
    Key hash3 = ((hash2<<5)+hash2) + str[2];
    Key hash4 = ((hash3<<5)+hash3) + str[3];
    Key hash5 = ((hash4<<5)+hash4) + str[4];
    Key hash6 = ((hash5<<5)+hash5) + str[5];
    Key hash7 = ((hash6<<5)+hash6) + str[6];    
    Key hash8 = ((hash7<<5)+hash7) + str[7];
    Key hash9 = ((hash8<<5)+hash8) + str[8];
    Key hash10 = ((hash9<<5)+hash9) + str[9];
    Key hash11 = ((hash10<<5)+hash10) + str[10];
    Key hash12 = ((hash11<<5)+hash11) + str[11];
    Key hash13 = ((hash12<<5)+hash12) + str[12];    
    _hash = hash13;
    setDebugString( str );
  }
  sasStringId( const char (&str)[15] )
  {
    Key hash0 = 5381;
    Key hash1 = ((hash0<<5)+hash0) + str[0];
    Key hash2 = ((hash1<<5)+hash1) + str[1];
    Key hash3 = ((hash2<<5)+hash2) + str[2];
    Key hash4 = ((hash3<<5)+hash3) + str[3];
    Key hash5 = ((hash4<<5)+hash4) + str[4];
    Key hash6 = ((hash5<<5)+hash5) + str[5];
    Key hash7 = ((hash6<<5)+hash6) + str[6];    
    Key hash8 = ((hash7<<5)+hash7) + str[7];
    Key hash9 = ((hash8<<5)+hash8) + str[8];
    Key hash10 = ((hash9<<5)+hash9) + str[9];
    Key hash11 = ((hash10<<5)+hash10) + str[10];
    Key hash12 = ((hash11<<5)+hash11) + str[11];
    Key hash13 = ((hash12<<5)+hash12) + str[12];
    Key hash14 = ((hash13<<5)+hash13) + str[13];        
    _hash = hash14;
    setDebugString( str );
  }
  sasStringId( const char (&str)[16] )
  {
    Key hash0 = 5381;
    Key hash1 = ((hash0<<5)+hash0) + str[0];
    Key hash2 = ((hash1<<5)+hash1) + str[1];
    Key hash3 = ((hash2<<5)+hash2) + str[2];
    Key hash4 = ((hash3<<5)+hash3) + str[3];
    Key hash5 = ((hash4<<5)+hash4) + str[4];
    Key hash6 = ((hash5<<5)+hash5) + str[5];
    Key hash7 = ((hash6<<5)+hash6) + str[6];    
    Key hash8 = ((hash7<<5)+hash7) + str[7];
    Key hash9 = ((hash8<<5)+hash8) + str[8];
    Key hash10 = ((hash9<<5)+hash9) + str[9];
    Key hash11 = ((hash10<<5)+hash10) + str[10];
    Key hash12 = ((hash11<<5)+hash11) + str[11];
    Key hash13 = ((hash12<<5)+hash12) + str[12];
    Key hash14 = ((hash13<<5)+hash13) + str[13];    
    Key hash15 = ((hash14<<5)+hash14) + str[14];    
    _hash = hash15;
    setDebugString( str );
  }
  sasStringId( const char (&str)[17] )
  {
    Key hash0 = 5381;
    Key hash1 = ((hash0<<5)+hash0) + str[0];
    Key hash2 = ((hash1<<5)+hash1) + str[1];
    Key hash3 = ((hash2<<5)+hash2) + str[2];
    Key hash4 = ((hash3<<5)+hash3) + str[3];
    Key hash5 = ((hash4<<5)+hash4) + str[4];
    Key hash6 = ((hash5<<5)+hash5) + str[5];
    Key hash7 = ((hash6<<5)+hash6) + str[6];    
    Key hash8 = ((hash7<<5)+hash7) + str[7];
    Key hash9 = ((hash8<<5)+hash8) + str[8];
    Key hash10 = ((hash9<<5)+hash9) + str[9];
    Key hash11 = ((hash10<<5)+hash10) + str[10];
    Key hash12 = ((hash11<<5)+hash11) + str[11];
    Key hash13 = ((hash12<<5)+hash12) + str[12];
    Key hash14 = ((hash13<<5)+hash13) + str[13];    
    Key hash15 = ((hash14<<5)+hash14) + str[14];
    Key hash16 = ((hash15<<5)+hash15) + str[15];    
    _hash = hash16;    
    setDebugString( str );
  }

private: // inner methods
  enum { DEBUG_STRING_LEN = 32 };

  void setDebugString( const char* str )
  {
#ifdef sasDEBUG
    strncpy( _str, str, DEBUG_STRING_LEN );
    _str[DEBUG_STRING_LEN-1] = 0;
#endif
  }

private: // members
  Key _hash;

#ifdef sasDEBUG
  char _str[ DEBUG_STRING_LEN ];
#endif
};
