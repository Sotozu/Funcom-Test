#pragma once 

#include "utils/sas_singleton.h"
#include "sg_boid.h"
#include <memory>
#include <mutex>
//NEW - Alex Soto
#include "Octree.h"


class sgBoidMng : public sasSingleton< sgBoidMng >
{
    sasNO_COPY( sgBoidMng );
    sasMEM_OP( sgBoidMng );

public:
    sgBoidMng( uint32_t smallBoidCount, uint32_t largeBoidCount );
    ~sgBoidMng();

    //NEW - Alexander Soto
    //Complicated format... this is simpler.
    std::vector<sgBoid>& boidArray() { return _boids; }
    //sgBoid** boidArray() { return &_boids[ 0 ]; }
    uint32_t boidCount() const { return static_cast< uint32_t >( _boids.size() ); }

    void stepSimulation();
    void flushRenderCommands();
    void syncWindowStep(); //happens when render and game threads are in sync

    //NEW - Alex Soto
    //Const and non const getters.
    unibn::Octree<sgBoid*>& GetOctree();
    const unibn::Octree<sgBoid*>& GetOctree() const;

private:
    //NEW - Alex Soto
    //Create octree here to hold pointers to our actual Boids
    unibn::Octree<sgBoid*> _oct;

    //NEW - Alex Soto
    //Now holds our boids in contigous memory
    std::vector<sgBoid> _boids;

    //New - AlexSoto
    //Will be used in octree
    std::vector<sgBoid*> _boidsPtr;

    void* _renderDataMemBlock[static_cast<uint_fast8_t>(sgBoidType::Enum::count) ][ 2 ];
    uint32_t _renderDataIndex; //We ping pong in between 2 buffers per boid type since game logic runs on different thread than render thread
    sasStringId _boidRenderObjectId[static_cast<uint_fast8_t>(sgBoidType::Enum::count) ];
    uint32_t _boidCount[static_cast<uint_fast8_t>(sgBoidType::Enum::count)]; //
    bool _renderDataInitialized;

    std::recursive_mutex _mutex;
};


