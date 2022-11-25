#pragma once

class sasCamera;

sasALIGN16_PRE class sasFrustum
{
public:
  sasFrustum(const smMat44& viewProjMtx);
  ~sasFrustum() {}

  smVec4 _planes[6];
} sasALIGN16_POST;

sasALIGN16_PRE class sasFrustumPoints
{
public:
  enum frustumPoints
  {
    farTopLeft = 0,
    farTopRight,
    farBottomLeft,
    farBottomRight,
    nearTopLeft, 
    nearTopRight,
    nearBottomLeft,
    nearBottomRight
  };

public:
  sasFrustumPoints( const sasCamera& cam, float width, float height );
  sasFrustumPoints( const smMat44& invProjView );
  sasFrustumPoints();
  ~sasFrustumPoints() {}

  smVec3 getPoint( frustumPoints pt ) const { return _points[ pt ]; }

  static sasFrustumPoints merge( const sasFrustumPoints& fa, const sasFrustumPoints& fb );

  smVec3  _points[8];

private:
  void ComputeFromInvPV( const smMat44& invPV );

} sasALIGN16_POST;


