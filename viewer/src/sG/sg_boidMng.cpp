#include "sg_pch.h"
#include <immintrin.h>
#include <iostream>
#include "sg_boidMng.h"
#include "sg_boid.h"
//Needed for world min and max
#include "sg_gameMng.h"

#include "utils/sas_timer.h"
#include "ThreadPool.h"

float getRandomizedFloatInRange(float minRange, float maxRange)
{
    float r = ((float)rand()) / (float)RAND_MAX;
    return r * (maxRange - minRange) + minRange;
}
//NEW - Alex Soto
//Multithreaded creation of boids
sgBoidMng::sgBoidMng( uint32_t smallBoidCount, uint32_t largeBoidCount )
    : _renderDataIndex( 0 )
    , _renderDataInitialized( false )
{
    const smVec3& worldMin = sgGameMng::singletonPtr()->GetWorldMin();
    const smVec3& worldMax = sgGameMng::singletonPtr()->GetWorldMax();

    _boidCount[ static_cast<uint_fast8_t>(sgBoidType::Enum::small) ] = smallBoidCount;

    _boidCount[static_cast<uint_fast8_t>(sgBoidType::Enum::big) ] = largeBoidCount;

    auto& tp = sgGameMng::singletonPtr()->GetThreadPool();

    _boids.reserve(smallBoidCount + largeBoidCount);
    _boidsPtr.reserve(smallBoidCount + largeBoidCount);
    std::size_t count{ 0 };
    for( uint32_t i = 0; i < smallBoidCount; i++ )
    {
        tp.DoWork([this, &count, &worldMin, &worldMax]()
            {
                smVec3 pos(getRandomizedFloatInRange(worldMin.X, worldMax.X), getRandomizedFloatInRange(worldMin.Y, worldMax.Y), getRandomizedFloatInRange(worldMin.Z, worldMax.Z));
                smVec3 vel(getRandomizedFloatInRange(-1.f, 1.f), getRandomizedFloatInRange(-0.5f, 0.5f), getRandomizedFloatInRange(-1.f, 1.f));

                {
                    std::lock_guard<std::recursive_mutex> lock(this->_mutex);
                    _boids.emplace_back(sgBoid::sbd, pos, vel);
                    _boidsPtr.push_back(&_boids[count]);
                    count++;
                }

            });
    }

    for( uint32_t i = 0; i < largeBoidCount; i++ )
    {
        tp.DoWork([this, &count, &worldMin, &worldMax, smallBoidCount]()
            {
                smVec3 pos(getRandomizedFloatInRange(worldMin.X, worldMax.X), getRandomizedFloatInRange(worldMin.Y, worldMax.Y), getRandomizedFloatInRange(worldMin.Z, worldMax.Z));
                smVec3 vel(getRandomizedFloatInRange(-1.f, 1.f), getRandomizedFloatInRange(-0.5f, 0.5f), getRandomizedFloatInRange(-1.f, 1.f));

                {
                    std::lock_guard<std::recursive_mutex> lock(this->_mutex);
                    _boids.emplace_back(sgBoid::lbd, pos, vel);
                    _boidsPtr.push_back(&_boids[count]);
                    count++;
                }

            });
    }
    tp.FinishAllThreads();
    tp.InitalizeThreads();

    for( uint32_t i = 0; i < 2; i++ )
    {
        _renderDataMemBlock[static_cast<uint_fast8_t>(sgBoidType::Enum::small) ][ i ] = sasMalloc( sizeof( smVec4 ) * 1000 );
        _renderDataMemBlock[ static_cast<uint_fast8_t>(sgBoidType::Enum::big) ][ i ] = sasMalloc( sizeof( smVec4 ) * 1000 );
    }

    _boidRenderObjectId[static_cast<uint_fast8_t>(sgBoidType::Enum::small) ] = sasStringId::build( "smallBoidMeshInst" );
    _boidRenderObjectId[static_cast<uint_fast8_t>(sgBoidType::Enum::big) ] = sasStringId::build( "bigBoidMeshInst" );
}

sgBoidMng::~sgBoidMng()
{
    sasDeleteModelInstance( _boidRenderObjectId[static_cast<uint_fast8_t>(sgBoidType::Enum::small) ] );
    sasDeleteModelInstance( _boidRenderObjectId[static_cast<uint_fast8_t>(sgBoidType::Enum::big) ] );

    for( uint32_t i = 0; i < 2; i++ )
    {
        sasFree( _renderDataMemBlock[static_cast<uint_fast8_t>(sgBoidType::Enum::small) ][ i ] );
        sasFree( _renderDataMemBlock[static_cast<uint_fast8_t>(sgBoidType::Enum::big) ][ i ] );
    }
}

void sgBoidMng::stepSimulation()
{

    auto& tp = sgGameMng::singletonPtr()->GetThreadPool();

    const smVec3& worldMin = sgGameMng::singletonPtr()->GetWorldMin();
    const smVec3& worldMax = sgGameMng::singletonPtr()->GetWorldMax();

    const sgGameMng::smVec3_6& worldEdgePoints = sgGameMng::singletonPtr()->GetWorldEdgePoints();
    const sgGameMng::smVec3_6& worldEdgeNormals = sgGameMng::singletonPtr()->GetWorldEdgeNormals();

    unibn::Octree<sgBoid*>& octree = sgBoidMng::singletonPtr()->GetOctree();
    std::vector<sgBoid>& boids = sgBoidMng::singletonPtr()->boidArray();
    float deltaTime = sasTimer::singletonPtr()->getElapsedTime();

    
    _oct.initialize(_boidsPtr);
    for (size_t i = 0; i < boids.size(); i++)
    {
        tp.DoWork([this, i, &boids, &worldEdgePoints, &worldEdgeNormals, &worldMin, &worldMax]()
            {
                boids[i].updateVelocity(worldMin, worldMax, worldEdgePoints, worldEdgeNormals);
            });
    }
    tp.FinishAllThreads();
    tp.InitalizeThreads();

    //To have same/similar behavior to original we need to iterate through these with no multithreading.
    for (size_t i = 0; i < boids.size(); i++)
    {
        boids[i].updatePosition(deltaTime);
        boids[i].checkForCollisions(worldMin, worldMax);
    }

}

void sgBoidMng::flushRenderCommands()
{
    smVec4* smallBoidRenderData = static_cast< smVec4* >( _renderDataMemBlock[static_cast<uint_fast8_t>(sgBoidType::Enum::small) ][ _renderDataIndex ] );
    smVec4* largeBoidRenderData = static_cast< smVec4* >( _renderDataMemBlock[static_cast<uint_fast8_t>(sgBoidType::Enum::big) ][ _renderDataIndex ] );
    for( size_t i = 0; i < _boids.size(); i++ )
    {
        if( _boids[ i ].type() == static_cast<uint_fast8_t>(sgBoidType::Enum::small ))
        {
            *smallBoidRenderData = smVec4( _boids[ i ].pos().X, _boids[ i ].pos().Y, _boids[ i ].pos().Z, _boids[ i ].radius() );
            smallBoidRenderData++;
        }
        else
        {
            *largeBoidRenderData = smVec4( _boids[ i ].pos().X, _boids[ i ].pos().Y, _boids[ i ].pos().Z, _boids[ i ].radius() );
            largeBoidRenderData++;
        }
    }
}

void sgBoidMng::syncWindowStep()
{ 
    _renderDataIndex = 1 - _renderDataIndex; 

    if( !_renderDataInitialized )
    {
        _renderDataInitialized = true;
        sasCreateBlobModelInstance( _boidRenderObjectId[ static_cast<uint_fast8_t>(sgBoidType::Enum::small) ] );
        sasCreateBlobModelInstance( _boidRenderObjectId[static_cast<uint_fast8_t>(sgBoidType::Enum::big) ] );
        sasSetInstanceTintColour( _boidRenderObjectId[static_cast<uint_fast8_t>(sgBoidType::Enum::small) ], smVec4( 1.f, 0.f, 0.5f, 1.f ) );
        sasSetInstanceTintColour( _boidRenderObjectId[static_cast<uint_fast8_t>(sgBoidType::Enum::big) ], smVec4( 0.f, 1.f, 1.f, 1.f ) );
    }

    sasSetModelInstanceData( _boidRenderObjectId[static_cast<uint_fast8_t>(sgBoidType::Enum::small) ], _renderDataMemBlock[static_cast<uint_fast8_t>(sgBoidType::Enum::small) ][ _renderDataIndex ], _boidCount[static_cast<uint_fast8_t>(sgBoidType::Enum::small) ] * sizeof( smVec4 ), _boidCount[static_cast<uint_fast8_t>(sgBoidType::Enum::small) ] );
    sasSetModelInstanceData( _boidRenderObjectId[static_cast<uint_fast8_t>(sgBoidType::Enum::big) ], _renderDataMemBlock[static_cast<uint_fast8_t>(sgBoidType::Enum::big) ][ _renderDataIndex ], _boidCount[static_cast<uint_fast8_t>(sgBoidType::Enum::big) ] * sizeof( smVec4 ), _boidCount[static_cast<uint_fast8_t>(sgBoidType::Enum::big) ] );
}

//Should make these inline
unibn::Octree<sgBoid*>& sgBoidMng::GetOctree()
{
    return _oct;
}
const unibn::Octree<sgBoid*>& sgBoidMng::GetOctree() const
{
    return _oct;
}
