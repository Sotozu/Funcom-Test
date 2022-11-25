#include "su_pch.h"
#include "utils/sas_frustum.h"
#include "utils/sas_camera.h"

sasFrustum::sasFrustum(const smMat44& viewProjMtx)
{
  // Left clipping plane
  _planes[0].X = viewProjMtx.L3[0] + viewProjMtx.L0[0];
  _planes[0].Y = viewProjMtx.L3[1] + viewProjMtx.L0[1];
  _planes[0].Z = viewProjMtx.L3[2] + viewProjMtx.L0[2];
  _planes[0].W = viewProjMtx.L3[3] + viewProjMtx.L0[3];

  // Right clipping plane
  _planes[1].X = viewProjMtx.L3[0] - viewProjMtx.L0[0];
  _planes[1].Y = viewProjMtx.L3[1] - viewProjMtx.L0[1];
  _planes[1].Z = viewProjMtx.L3[2] - viewProjMtx.L0[2];
  _planes[1].W = viewProjMtx.L3[3] - viewProjMtx.L0[3];

  // Top clipping plane
  _planes[2].X = viewProjMtx.L3[0] - viewProjMtx.L1[0];
  _planes[2].Y = viewProjMtx.L3[1] - viewProjMtx.L1[1];
  _planes[2].Z = viewProjMtx.L3[2] - viewProjMtx.L1[2];
  _planes[2].W = viewProjMtx.L3[3] - viewProjMtx.L1[3];

  // Bottom clipping plane
  _planes[3].X = viewProjMtx.L3[0] + viewProjMtx.L1[0];
  _planes[3].Y = viewProjMtx.L3[1] + viewProjMtx.L1[1];
  _planes[3].Z = viewProjMtx.L3[2] + viewProjMtx.L1[2];
  _planes[3].W = viewProjMtx.L3[3] + viewProjMtx.L1[3];

  // Near clipping plane
  _planes[4].X = viewProjMtx.L3[0] + viewProjMtx.L2[0];
  _planes[4].Y = viewProjMtx.L3[1] + viewProjMtx.L2[1];
  _planes[4].Z = viewProjMtx.L3[2] + viewProjMtx.L2[2];
  _planes[4].W = viewProjMtx.L3[3] + viewProjMtx.L2[3];

  // Far clipping plane
  _planes[5].X = viewProjMtx.L3[0] - viewProjMtx.L2[0];
  _planes[5].Y = viewProjMtx.L3[1] - viewProjMtx.L2[1];
  _planes[5].Z = viewProjMtx.L3[2] - viewProjMtx.L2[2];
  _planes[5].W = viewProjMtx.L3[3] - viewProjMtx.L2[3];



  // Normalize planes
  _planes[0] /= smLength3(_planes[0]);
  _planes[1] /= smLength3(_planes[1]);
  _planes[2] /= smLength3(_planes[2]);
  _planes[3] /= smLength3(_planes[3]);
  _planes[4] /= smLength3(_planes[4]);
  _planes[5] /= smLength3(_planes[5]);

}


sasFrustumPoints::sasFrustumPoints( const sasCamera& cam, float width, float height )
{
#ifdef FRUSTUM_TRIGO_APPROACH
  //Retrieve frustum points
  float aspectRatio = width / height;

  float heightNear = 2.f * smATan( cam.FOV() / 2.f ) * cam.zNear();
  float widthNear = heightNear * aspectRatio;
  float heightFar = 2.f * smATan( cam.FOV() / 2.f ) * cam.zFar();
  float widthFar = heightFar * aspectRatio;

  float halfHeightNear = heightNear * 0.5f;
  float halfWidthNear = widthNear * 0.5f;
  float halfHeightFar = heightFar * 0.5f;
  float halfWidthFar = widthFar * 0.5f;

  smVec3 frustumPos = cam.position();
  smVec3 frustumDir = cam.front();
  smVec3 frustumUp = cam.up();
  smVec4 frustumRight;
  cam.getRight( frustumRight );

  smVec3 farCentre = frustumPos + frustumDir * cam.zFar();
  smVec3 nearCentre = frustumPos + frustumDir * cam.zNear();

  _points[ farTopLeft ]     =  farCentre + ( frustumUp * halfHeightFar ) - ( frustumRight * halfWidthFar ); 
  _points[ farTopRight ]    = farCentre + ( frustumUp * halfHeightFar ) + ( frustumRight * halfWidthFar ); 
  _points[ farBottomLeft ]  = farCentre - ( frustumUp * halfHeightFar ) - ( frustumRight * halfWidthFar ); 
  _points[ farBottomRight ] = farCentre - ( frustumUp * halfHeightFar ) + ( frustumRight * halfWidthFar ); 

  _points[ nearTopLeft ]    = nearCentre + ( frustumUp * halfHeightNear ) - ( frustumRight * halfWidthNear ); 
  _points[ nearTopRight ]   = nearCentre + ( frustumUp * halfHeightNear ) + ( frustumRight * halfWidthNear ); 
  _points[ nearBottomLeft ] = nearCentre - ( frustumUp * halfHeightNear ) - ( frustumRight * halfWidthNear ); 
  _points[ nearBottomRight ]= nearCentre - ( frustumUp * halfHeightNear ) + ( frustumRight * halfWidthNear ); 

#else //!FRUSTUM_TRIGO_APPROACH

  smMat44 view;
  cam.getViewMatrix( view );
  smMat44 proj;
  cam.getProjMatrix( width, height, proj );
  smMat44 invProjView;
  smMul( proj, view, invProjView );
  smInverse( invProjView, invProjView ); 

  ComputeFromInvPV( invProjView );
#endif //FRUSTUM_TRIGO_APPROACH
}

sasFrustumPoints::sasFrustumPoints( const smMat44& invProjView )
{
  ComputeFromInvPV( invProjView );
}

void sasFrustumPoints::ComputeFromInvPV( const smMat44& invProjView )
{
  static const smVec4 hCoords[ 8 ] = {  smVec4( -1.f, 1.f, 1.f, 1.f ),  //FTL
                                        smVec4( 1.f, 1.f, 1.f, 1.f ),   //FTR
                                        smVec4( -1.f, -1.f, 1.f, 1.f ), //FBL
                                        smVec4( 1.f, -1.f, 1.f, 1.f ),  //FBR
                                        smVec4( -1.f, 1.f, 0.f, 1.f ),  //NTL
                                        smVec4( 1.f, 1.f, 0.f, 1.f ),   //NTR
                                        smVec4( -1.f, -1.f, 0.f, 1.f ), //NBL
                                        smVec4( 1.f, -1.f, 0.f, 1.f ) };//NBR

  smVec4 pt;
  for( unsigned int i = 0; i < 8; i++ )
  {
    smMul( invProjView, hCoords[ i ], pt );
    _points[ i ] = pt / pt.W;
  }
}

sasFrustumPoints::sasFrustumPoints()
{
  memset( &_points[0], NULL, sizeof( _points ) );
}

sasFrustumPoints sasFrustumPoints::merge( const sasFrustumPoints& fa, const sasFrustumPoints& fb )
{
  sasFrustumPoints r;
  smVec3 centreA = fa._points[ nearTopLeft ] + ( ( fa._points[ farBottomRight ] - fa._points[ nearTopLeft ] ) * 0.5f );
  smVec3 centreB = fb._points[ nearTopLeft ] + ( ( fb._points[ farBottomRight ] - fb._points[ nearTopLeft ] ) * 0.5f );
  smVec3 centre = ( centreA + centreB ) * 0.5f;

  r._points[ farBottomLeft ] = ( smLengthSq3( fa._points[ farBottomLeft ] - centre ) < smLengthSq3( fb._points[ farBottomLeft ] - centre ) ) ? fb._points[ farBottomLeft ] : fa._points[ farBottomLeft ];
  r._points[ farBottomRight ] = ( smLengthSq3( fa._points[ farBottomRight ] - centre ) < smLengthSq3( fb._points[ farBottomRight ] - centre ) ) ? fb._points[ farBottomRight ] : fa._points[ farBottomRight ];
  r._points[ farTopLeft ] = ( smLengthSq3( fa._points[ farTopLeft ] - centre ) < smLengthSq3( fb._points[ farTopLeft ] - centre ) ) ? fb._points[ farTopLeft ] : fa._points[ farTopLeft ];
  r._points[ farTopRight ] = ( smLengthSq3( fa._points[ farTopRight ] - centre ) < smLengthSq3( fb._points[ farTopRight ] - centre ) ) ? fb._points[ farTopRight ] : fa._points[ farTopRight ];
  r._points[ nearBottomLeft ] = ( smLengthSq3( fa._points[ nearBottomLeft ] - centre ) < smLengthSq3( fb._points[ nearBottomLeft ] - centre ) ) ? fb._points[ nearBottomLeft ] : fa._points[ nearBottomLeft ];
  r._points[ nearBottomRight ] = ( smLengthSq3( fa._points[ nearBottomRight ] - centre ) < smLengthSq3( fb._points[ nearBottomRight ] - centre ) ) ? fb._points[ nearBottomRight ] : fa._points[ nearBottomRight ];
  r._points[ nearTopLeft ] = ( smLengthSq3( fa._points[ nearTopLeft ] - centre ) < smLengthSq3( fb._points[ nearTopLeft ] - centre ) ) ? fb._points[ nearTopLeft ] : fa._points[ nearTopLeft ];
  r._points[ nearTopRight ] = ( smLengthSq3( fa._points[ nearTopRight ] - centre ) < smLengthSq3( fb._points[ nearTopRight ] - centre ) ) ? fb._points[ nearTopRight ] : fa._points[ nearTopRight ];

  return r;
}
