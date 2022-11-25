#pragma once 

#include "utils/sas_stringid.h"
#include "../sas_rendertypes.h"

class sasShaderFlags 
{
  sasMEM_OP( sasShaderFlags );

private:
  typedef std::map< sasStringId::Key, std::string > Dictionnary; 
  struct Flag 
  {
    sasStringId::Key  key;
    size_t            shift;
    size_t            nBits;
    size_t            range;
  };  

public:
  sasShaderFlags( json_t* json );
  sasShaderFlags( const sasShaderFlags* global, const sasShaderFlags* flags );
  ~sasShaderFlags();

public:
  size_t      size() const { return _nFlags; }
  sasShaderID getOptionBitMask( const sasStringId& opt, size_t value = 1 );
  size_t      fillMacros( sasShaderID id, sasShaderMacro* macros, size_t capacity ) const;

private:  
  void createFlag( const char* name, size_t range, Flag& flag );
  void finalize();  
  void checkSize();
  void computeShifts();  
  void checkKeyUnicity();

private: 

  static Dictionnary  _dictionnary;
  Flag*               _flags;
  size_t              _nFlags;
};
