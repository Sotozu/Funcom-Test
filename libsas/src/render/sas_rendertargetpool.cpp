#include "sas_pch.h"
#include "sas_rendertargetpool.h"
#include "utils/sas_hash.h"
#include "sas_rendertarget.h"
#include "sas_depthbuffer.h"

sasRenderTargetPool::sasRenderTargetPool()
{
  _rtPool.reserve( 48 );
  _dstPool.reserve( 8 );
}

sasRenderTargetPool::~sasRenderTargetPool()
{
  Clear();
}

sasRenderTarget* sasRenderTargetPool::RequestAndLock( const sasRenderTargetDesc& rtDesc)
{
  uint32_t descHash = sasHasher::fnv_32_buf( &rtDesc, sizeof( sasRenderTargetDesc ) );
  for( size_t i = 0; i < _rtPool.size(); i++)
  {
    if((_rtPool[i].hash == descHash) && !_rtPool[i].locked)
    {
      _rtPool[i].locked = true;
      return _rtPool[i].object;
    }
  }

  sasDebugOutput( "RTPool: creating new render target. %d\n", descHash );
  //sasDebugOutput( "desc: %d %d %d %d %d %d %d\n", rtDesc.width, rtDesc.height, rtDesc.depth, rtDesc.mips, rtDesc.needUAV ? 1 : 0, static_cast< int >( rtDesc.type ), static_cast< int >( rtDesc.format ) );
  //sasDebugOutput( "RTPool: new size %d\n", _rtPool.size() );

  EntryRT e;
  e.hash = descHash;
  e.locked = true;
  e.object = sasNew sasRenderTarget( rtDesc );
  _rtPool.push_back(e);
  return e.object;
}

sasDepthStencilTarget* sasRenderTargetPool::RequestAndLock( const sasDepthStencilTargetDesc& dstDesc)
{
  uint32_t descHash = sasHasher::fnv_32_buf( &dstDesc, sizeof( sasDepthStencilTargetDesc ) );
  for( size_t i = 0; i < _dstPool.size(); i++)
  {
    if((_dstPool[i].hash == descHash) && !_dstPool[i].locked)
    {
      _dstPool[i].locked = true;
      return _dstPool[i].object;
    }
  }

  sasDebugOutput( "RTPool: creating new depth buffer. %d\n", descHash );  

  EntryDST e;
  e.hash = descHash;
  e.locked = true;
  e.object = sasNew sasDepthStencilTarget( dstDesc );
  _dstPool.push_back(e);
  return e.object;
}

void sasRenderTargetPool::Unlock( sasRenderTarget* rt )
{
  for( RenderTargetPool::iterator it = _rtPool.begin(); it != _rtPool.end(); it++ )
  {
    if( it->object == rt )
    {
      it->locked = false;
      return;
    }
  }

  sasASSERT( false );

}

void sasRenderTargetPool::Unlock( sasDepthStencilTarget* dst )
{
  DepthBufferPool::iterator p = _dstPool.begin();
  for( size_t i=0; i<_dstPool.size(); i++, ++p )
  {
    if( (*p).object == dst )
      break;
  }
  sasASSERT( p != _dstPool.end() );
  (*p).locked = false;
}

void sasRenderTargetPool::Clear()
{
  for( size_t i=0; i<_dstPool.size(); i++ )
  {
    sasDelete _dstPool[ i ].object;
  }
  for( size_t i=0; i<_rtPool.size(); i++ )
  {
    sasDelete _rtPool[ i ].object;
  }
  _rtPool.clear();
  _dstPool.clear();
}
