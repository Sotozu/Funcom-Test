#include "sas_pch.h"
#include "sas_vertexformat.h"

size_t sasVertexFormat::size() const
{
  size_t result = 0;
  result += elementSize( sasVertexElement::Position );
  result += elementSize( sasVertexElement::Normal );
  result += elementSize( sasVertexElement::Tangent );
  result += elementSize( sasVertexElement::Uv0 );
  result += elementSize( sasVertexElement::Colour );
  result += elementSize( sasVertexElement::BoneIndices );
  result += elementSize( sasVertexElement::BoneWeights );
  return result;
}

size_t sasVertexFormat::elementSize( sasVertexElement::Enum e ) const
{
  size_t result = 0;
  if( e == sasVertexElement::Position )
  {
    if( position == POSITION_3F )
      result = 12;
  }
  else if( e == sasVertexElement::Normal )
  {
    if( normal == NORMAL_3F )
      result = 12;
  }
  else if( e == sasVertexElement::Tangent )
  {
    if( tangent == TANGENT_3F )
      result = 12;
  }
  else if( e == sasVertexElement::Uv0 )
  {
    if( uv0 == UV_2F )
      result = 8;
  }
  else if( e == sasVertexElement::Colour )
  {
    if( colour == COLOUR_4F )
      result = 16;
  }
  else if( e == sasVertexElement::BoneIndices )
  {
    if( boneIndices == BONEINDICES )
      result = 16;
  }
  else if( e == sasVertexElement::BoneWeights )
  {
    if( boneWeights == BONEWEIGHTS )
      result = 16;
  }
  return result;
}

int sasVertexFormat::elementOffset( sasVertexElement::Enum e ) const
{
  size_t offset = 0;
  if( position )
  {
    if( e == sasVertexElement::Position )
      return offset;
    offset += elementSize( sasVertexElement::Position );    
  }
  if( normal )
  {
    if( e == sasVertexElement::Normal )
      return offset;    
    offset += elementSize( sasVertexElement::Normal );
  }
  if( tangent )
  {
    if( e == sasVertexElement::Tangent )
      return offset;    
    offset += elementSize( sasVertexElement::Normal );
  }
  if( uv0 )
  {
    if( e == sasVertexElement::Uv0 )
      return offset;    
    offset += elementSize( sasVertexElement::Uv0 );
  }
  if( colour )
  {
    if( e == sasVertexElement::Colour )
      return offset;    
    offset += elementSize( sasVertexElement::Colour );
  }
  return -1;
}

sasVertexReader::sasVertexReader( sasVertexFormat format, unsigned char* ptr )
  : _format(format)
  , _ptr( ptr )
{
  _stride = format.size();
}

bool sasVertexReader::readPosition( float result[3] )
{
  int offset = _format.elementOffset( sasVertexElement::Position );
  if( offset < 0 )
    return false;

  float* pfloat = (float*)(_ptr+offset );
  result[0] = pfloat[0];
  result[1] = pfloat[1];
  result[2] = pfloat[2];

  return true;
}

void sasVertexReader::next()
{
  _ptr += _stride;
}
