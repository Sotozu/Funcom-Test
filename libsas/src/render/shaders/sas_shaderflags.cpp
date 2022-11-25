#include "sas_pch.h"
#include "sas_shaderflags.h"
#include "../sas_rendertypes.h"

sasShaderFlags::Dictionnary sasShaderFlags::_dictionnary;

sasShaderFlags::sasShaderFlags( json_t* json )
  : _flags(0)
  , _nFlags(0)
{
  sasSTATIC_ASSERT( sasALIGNED16( sizeof(Flag) ) );  

  std::vector< Flag > flags;
  flags.reserve( 32 );

  // parse flags
  const char *key;
  json_t *value;
  void *iter = json_object_iter( json );
  while(iter)
  {
    key = json_object_iter_key(iter);
    value = json_object_iter_value(iter);    
    sasASSERT( json_is_integer(value) );
    int range = json_integer_value( value );
    sasASSERT( range > 0 );
    flags.resize( flags.size() + 1 );
    createFlag( key, range, *flags.rbegin() );
    iter = json_object_iter_next( json, iter);
  }  
  _nFlags = flags.size();
  if( _nFlags )
  {
    size_t copySize = _nFlags*sizeof(Flag);
    _flags = (Flag*)sasMalloc( copySize );    
    memcpy( _flags, &flags[0], copySize );    
    finalize();
  }  
}

sasShaderFlags::sasShaderFlags( const sasShaderFlags* global, const sasShaderFlags* local )
  : _flags(0)
  , _nFlags(0)
{
  _nFlags = global->_nFlags + local->_nFlags;
  if( _nFlags )
  {
    size_t copySize = _nFlags*sizeof(Flag);
    _flags = (Flag*)sasMalloc( copySize );
    size_t i = 0;
    while( i<global->_nFlags )
    {
      const Flag& flag = global->_flags[ i ];
      _flags[i].key = flag.key;
      _flags[i].nBits = flag.nBits;
      _flags[i].shift = 0;
      _flags[i].range = flag.range;
      ++i;
    }
    while( i < _nFlags )
    {
      const Flag& flag = local->_flags[ i - global->_nFlags ];
      _flags[i].key = flag.key;
      _flags[i].nBits = flag.nBits;
      _flags[i].shift = 0;
      _flags[i].range = flag.range;
      ++i;
    }
    finalize();
  }  
}

sasShaderFlags::~sasShaderFlags()
{
  if( _flags )
    sasFree( _flags );
}

sasShaderID sasShaderFlags::getOptionBitMask( const sasStringId& opt, size_t value )
{
  sasShaderID mask;

  for( size_t i=0; i<_nFlags; i++ )
  {
    if( _flags[i].key == opt )
    {
      sasASSERT( value <= _flags[i].range );
      mask.flags = value;      
      mask.flags <<= _flags[i].shift;
      break;
    }
  }

  return mask;
}

size_t sasShaderFlags::fillMacros( sasShaderID id, sasShaderMacro* macros, size_t capacity ) const
{
  size_t n = 0;
  for( size_t i=0; i<_nFlags; i++ )
  {
    sasShaderID mask;
    mask.flags = ( 1 << _flags[i].nBits ) - 1;
    mask.flags <<= _flags[i].shift;
    if( mask.flags & id.flags )
    {      
      sasShaderID value; 
      value.flags = id.flags & mask.flags;
      value.flags >>= _flags[i].shift;
      sasASSERT( n < capacity );
      macros[ n ].define = _dictionnary[ _flags[i].key ].c_str();
      int ivalue = (int)value.flags;
      sprintf( macros[n].value, "%9d", ivalue);
      ++n;
    }
  }
  return n;
}

void sasShaderFlags::createFlag( const char* name, size_t range, Flag& flag )
{
  // add name to dictionnary
  sasStringId key = sasStringId::build( name );
  sasASSERT( key.isValid() );  
  _dictionnary[ key ] = name;

  // compute number of bits
  size_t nBits = 1;
  while( (size_t)((1<<nBits)-1) < range )
    ++nBits;
  // add flags  
  flag.key    = key;
  flag.nBits  = nBits;
  flag.shift  = 0;
  flag.range  = range;
}

void sasShaderFlags::finalize()
{  
  checkSize();
  computeShifts();  
  checkKeyUnicity();
}

void sasShaderFlags::checkSize()
{
  size_t nBits = 0;
  for( size_t i=0; i<_nFlags; i++ )
  {
    nBits += _flags[i].nBits;
  }
  sasASSERT( nBits <= sasShaderID::NBITS_SHADERFLAGS );
}

void sasShaderFlags::computeShifts()
{
  size_t offset = _flags[0].nBits;
  for( size_t i=1; i<_nFlags; i++ )
  {
    _flags[i].shift = offset;
    offset += _flags[i].nBits;
  }
}

void sasShaderFlags::checkKeyUnicity()
{
  if( _nFlags )
  {
    sasStringId::Key refKey = _flags[0].key;
    for( size_t i=1; i<_nFlags; i++ )
    {
      sasStringId::Key curKey = _flags[i].key;
      sasASSERT( refKey != curKey );
      refKey = curKey;
    }
  }  
}
