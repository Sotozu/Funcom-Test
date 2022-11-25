#pragma once

#include "utils/sas_singleton.h"

class sasRenderTarget;
class sasDepthStencilTarget;
struct sasRenderTargetDesc;
struct sasDepthStencilTargetDesc;

class sasRenderTargetPool : public sasSingleton< sasRenderTargetPool >
{
  sasNO_COPY( sasRenderTargetPool );
  sasMEM_OP( sasRenderTargetPool );

public:
  sasRenderTargetPool();
  ~sasRenderTargetPool();

  sasRenderTarget*        RequestAndLock( const sasRenderTargetDesc& rtDesc );
  sasDepthStencilTarget*  RequestAndLock( const sasDepthStencilTargetDesc& dstDesc );
  void                    Unlock( sasRenderTarget* rt );
  void                    Unlock( sasDepthStencilTarget* dst );

  void                    Clear();

private:

  template< typename T >
  struct Entry 
  {
    uint32_t  hash;
    bool      locked;
    T         object;

    bool operator<( const Entry& other ) const { return hash < other.hash; }
  };
  typedef Entry<sasRenderTarget*> EntryRT;
  typedef Entry<sasDepthStencilTarget*> EntryDST;
  typedef std::vector<EntryRT> RenderTargetPool;
  typedef std::vector<EntryDST> DepthBufferPool;

  RenderTargetPool _rtPool;
  DepthBufferPool  _dstPool;
};
