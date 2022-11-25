#pragma once 

struct sasVertexElement 
{
  enum Enum 
  {
    Position,
    Normal,
    Tangent,
    Uv0,
    Colour,
    BoneIndices,
    BoneWeights,
  };
};

struct sasVertexFormat 
{
  

  static const size_t POSITION_3F = 1;
  static const size_t NORMAL_3F   = 1;
  static const size_t UV_2F       = 1;
  static const size_t TANGENT_3F  = 1;
  static const size_t COLOUR_4F   = 1;
  static const size_t BONEINDICES = 1;
  static const size_t BONEWEIGHTS = 1;


  union 
  {
    struct 
    {
      size_t    position        :   1;
      size_t    normal          :   1;
      size_t    tangent         :   1;
      size_t    uv0             :   1;
      size_t    colour          :   1;
      size_t    boneIndices     :   1;
      size_t    boneWeights     :   1;
    };
    size_t flags;
  };  

  sasVertexFormat() :
    flags(0)
  {}

  size_t  size() const;
  size_t  elementSize( sasVertexElement::Enum e ) const;
  int     elementOffset( sasVertexElement::Enum e ) const;
};

inline bool operator<( sasVertexFormat lhs, sasVertexFormat rhs )
{
  return lhs.flags < rhs.flags;
}

class sasVertexReader
{
public:
  sasVertexReader( sasVertexFormat format, unsigned char* ptr );

public:
  bool readPosition( float result[3] );
  void next();

private:
  sasVertexFormat _format;
  unsigned char* _ptr;
  size_t _stride;

};