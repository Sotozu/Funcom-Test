#pragma once

class sasInstance;

class sasDrawList
{
  sasNO_COPY( sasDrawList );
  sasMEM_OP( sasDrawList );

public:
  sasDrawList()
  {
    _size = 0;
    memset(_drawList, 0, sizeof(_drawList));
  }

  static const unsigned int kMaxDrawListSize = 1024;
  sasInstance*   _drawList[ kMaxDrawListSize ];
  unsigned int  _size;
};

