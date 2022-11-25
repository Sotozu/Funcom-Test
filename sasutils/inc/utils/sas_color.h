#pragma once 

#include "sas_export.h"

struct sasColor
{
  float R,G,B,A;

  sasColor() {}
  sasColor( float r, float g, float b, float a = 1.f ) : R(r), G(g), B(b), A(a) {}

  operator const float*() const { return (const float*)this; }

  void set( float r, float g, float b, float a = 1.f ) {
    R=r; G=g; B=b; A=a;
  }

  static sasColor red() { return sasColor(1.f,0.f,0.f,1.f); }
  static sasColor green() { return sasColor(0,1.f,0.f,1.f); }
  static sasColor blue() { return sasColor(0.f,0.f,1.f,1.f); }
};

