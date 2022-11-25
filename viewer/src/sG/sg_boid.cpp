#include "sg_pch.h"

//Intrinsics
//Using AVX2
#include <immintrin.h>
#include <math.h>
#include "sg_boid.h"
#include "sg_boidMng.h"
//NEW - Alex Soto
//Singleton access of world win and max
#include "sg_gameMng.h"
#include "utils/sas_timer.h"


//initalize static structs for describing data
sgBoidType::SmallBoidData sgBoid::sbd;
sgBoidType::LargeBoidData sgBoid::lbd;


//We can have the calculations/function calls for getting a randomizedfloatInRange be done in a multithreaded fashion.
//Passing in these values by reference in the structure that contains them using an emplaceback.
//worldMin and worldMax are global variables not needed here.
//Only piece of data that really matters is the current velocity of the boid and it's type.
sgBoid::sgBoid( sgBoidType::Enum type ) : 
    _data((type == sgBoidType::Enum::small) ? static_cast<sgBoidType::BoidData>(sgBoid::sbd) : static_cast<sgBoidType::BoidData>(sgBoid::lbd))
{
    smNormalize3( _vel, _vel );
    _vel *= _data._vel;

}


sgBoid::sgBoid(sgBoidType::Enum type, smVec3& pos, smVec3& vel) : 
  _pos(pos)
, _vel(vel)
, _data((type == sgBoidType::Enum::small) ? static_cast<sgBoidType::BoidData>(sgBoid::sbd) : static_cast<sgBoidType::BoidData>(sgBoid::lbd))
{
    smNormalize3(_vel, _vel);
    _vel *= _data._vel;
}

sgBoid::sgBoid(sgBoidType::BoidData& data, smVec3& pos, smVec3& vel) :
_pos(pos)
, _vel(vel)
, _data(data)
{
    smNormalize3(_vel, _vel);
    _vel *= _data._vel;
}


sgBoid::~sgBoid()
{}

void sgBoid::updateVelocity(const smVec3& worldMin, const smVec3& worldMax, const sgGameMng::smVec3_6 worldEdgePoints, const sgGameMng::smVec3_6 worldEdgeNormals)
{
#pragma region NEW VERSION
std::vector<sgBoid>& boids = sgBoidMng::singletonPtr()->boidArray();
unibn::Octree<sgBoid*>& octree = sgBoidMng::singletonPtr()->GetOctree();
std::vector<uint32_t> neighborsIndex;
octree.radiusNeighbors<unibn::L2Distance<sgBoid*>>(this, _data._proximityTestRange, neighborsIndex);

float nearbyBoidWeight = 0.f;
uint32_t numNearbyBoids = 0;

float nearbyBoidWeightOther = 0.f;
uint32_t numNearbyBoidsOther = 0;

//0
//alignment with other boids of same type
smVec3 nearbyFlockVel(0.f);
smVec3 alignmentForce(0.f);

//3
//separation from other boids of same type
smVec3 posToNearbyFlock(0.f);
smVec3 separationForce(0.f);


//1
//cohesion with other boids of same type
smVec3 nearbyFlockPos(0.f);
smVec3 cohesionForce(0.f);

//2
//cohesion with other boids of different type
smVec3 nearbyOtherTypeFlockPos(0.f);
smVec3 cohesionForceWithOtherType(0.f);


//4
//separation from other boids of different type
smVec3 posToNearbyFlockOfOtherType(0.f);
smVec3 separationForceWithOtherType(0.f);

//0
for (auto i : neighborsIndex)
{
    if (&boids[i] != this)
    {
        if ((distanceToBoid(boids[i]) < _data._proximityTestRange) && (boids[i].type() == type()))
        {
            float impactWeight = 1.f - (distanceToBoid(boids[i]) / _data._proximityTestRange);
            nearbyFlockVel += boids[i].velocity() * impactWeight;

            nearbyBoidWeight += impactWeight;
            numNearbyBoids++;

            posToNearbyFlock += (boids[i].pos() - pos()) * impactWeight;

            nearbyFlockPos += boids[i].pos();
        }
        else if ((distanceToBoid(boids[i]) < _data._proximityTestRange) && (boids[i].type() != type()))
        {
            nearbyOtherTypeFlockPos += boids[i].pos();
            numNearbyBoidsOther++;

            float impactWeight = 1.f - (distanceToBoid(boids[i]) / _data._proximityTestRange);
            posToNearbyFlockOfOtherType += (boids[i].pos() - pos()) * impactWeight;
            nearbyBoidWeightOther += impactWeight;

        }
    }
}
//REQUIRED SAME 1
if (nearbyBoidWeight > 0)
{
    nearbyFlockVel /= static_cast<float>(numNearbyBoids);
    smNormalize3(nearbyFlockVel, alignmentForce);
    alignmentForce *= _data._alignmentWeight;

    posToNearbyFlock /= static_cast<float>(numNearbyBoids);
    posToNearbyFlock *= -1.f;
    smNormalize3(posToNearbyFlock, separationForce);
    separationForce *= _data._separationWeight;

}
//REQUIRED SAME 2
if (numNearbyBoids > 0)
{
    nearbyFlockPos /= static_cast<float>(numNearbyBoids);
    smVec3 posToAvgFlockPos = nearbyFlockPos - _pos;
    smNormalize3(posToAvgFlockPos, cohesionForce);
    cohesionForce *= _data._cohesionWeight;
}

//REQUIRED OTHER 1
if (numNearbyBoidsOther > 0)
{
    nearbyOtherTypeFlockPos /= static_cast<float>(numNearbyBoidsOther);
    smVec3 posToAvgFlockPos = nearbyOtherTypeFlockPos - _pos;
    smNormalize3(posToAvgFlockPos, cohesionForceWithOtherType);
    cohesionForceWithOtherType *= _data._cohesionWeightWithOtherBoidType;
}

if (nearbyBoidWeightOther > 0)
{
    posToNearbyFlockOfOtherType /= static_cast<float>(numNearbyBoidsOther);
    posToNearbyFlockOfOtherType *= -1.f;
    smNormalize3(posToNearbyFlockOfOtherType, separationForceWithOtherType);
    separationForceWithOtherType *= _data._separationWeightWithOtherBoidType;
}

//separation from edges
smVec3 posToNearbyEdges(0.f);
float nearbyEdgeWeight = 0.f;
uint32_t numNearbyEdges = 0;
smVec3 separationForceWithEdge(0.f);

for (uint32_t i = 0; i < 6; i++)
{
    float distToPlane = smDot3(_pos - worldEdgePoints[i], worldEdgeNormals[i]);
    if (distToPlane < _data._proximityTestRange)
    {
        float impactWeight = 1.f - (distToPlane / _data._proximityTestRange);
        posToNearbyEdges += distToPlane * worldEdgeNormals[i] * impactWeight;
        nearbyEdgeWeight += impactWeight;
        numNearbyEdges++;
    }
}
if (nearbyEdgeWeight > 0)
{
    posToNearbyEdges /= static_cast<float>(numNearbyEdges);
    smNormalize3(posToNearbyEdges, separationForceWithEdge);
    separationForceWithEdge *= _data._separationWeightWithOtherBoidType * 1.f;
}

_vel += alignmentForce + separationForce + separationForceWithOtherType + cohesionForce + cohesionForceWithOtherType + separationForceWithEdge;
smNormalize3(_vel, _vel);
_vel *= _data._vel;
#pragma endregion
}

void sgBoid::updatePosition(const float deltaTime)
{
    _pos += _vel * deltaTime;
}

void sgBoid::checkForCollisions(const smVec3& worldMin, const smVec3& worldMax)
{

 

    std::vector<sgBoid>& boids = sgBoidMng::singletonPtr()->boidArray();
    unibn::Octree<sgBoid*>& octree = sgBoidMng::singletonPtr()->GetOctree();

    std::vector<uint32_t> neighborsIndex;
    float boidCollisionRadius = _data._radius + sgBoid::lbd._radius; //Covers all possible cases

    octree.radiusNeighbors<unibn::L2Distance<sgBoid*>>(this, boidCollisionRadius, neighborsIndex);

    for (auto i : neighborsIndex)
    {
        if (&boids[i] != this)
        {
            smVec3 posToBoid = boids[i].pos() - _pos;
            float bcrSquared = pow(_data._radius + boids[i].radius(), 2.0f);
            if (posToBoid.magnitudeSquared() < bcrSquared)
            {
                smVec3 unitPosToBoid;
                smNormalize3(posToBoid, unitPosToBoid);
                smVec3 offset = (unitPosToBoid * (_data._radius + boids[i].radius())) - posToBoid;
                _pos -= offset;
            }
        }

    }

    const float epsilon = 0.05f;
    if (_pos.X < worldMin.X)
    {
        _pos.X = worldMin.X + epsilon;
    }

    if (_pos.Y < worldMin.Y)
    {
        _pos.Y = worldMin.Y + epsilon;
    }

    if (_pos.Z < worldMin.Z)
    {
        _pos.Z = worldMin.Z + epsilon;
    }

    if (_pos.X > worldMax.X)
    {
        _pos.X = worldMax.X - epsilon;
    }

    if (_pos.Y > worldMax.Y)
    {
        _pos.Y = worldMax.Y - epsilon;
    }

    if (_pos.Z > worldMax.Z)
    {
        _pos.Z = worldMax.Z - epsilon;
    }

}

float sgBoid::distanceToBoid( const sgBoid& b ) const
{
    smVec3 posToBoid = b.pos() - _pos;
    return smLength3( posToBoid );
}

//NEW - Alexander Soto
//This is faster and more intuitive implementation that previously found.
bool sgBoid::withinRange(const sgBoid& b) const
{
    smVec3 posToBoid = (b.pos() - _pos);
    float ms = posToBoid.magnitudeSquared();
    return ((_data._proximityTestRange * _data._proximityTestRange) > ms);
}


void sgBoid::velocityCheckSameType()
{

}
