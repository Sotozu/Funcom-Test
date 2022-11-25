#pragma once 

/*
* Added some functions in sasMaths project to sm_vector.h so that vector operations are more intuitive.
* 
* Created withinRange function which is faster and more intuitive than previous implementation.
* 
* Enum has been typedef to be of size uint_fast8_t. This saves 3 bytes per instance.
* 
* Moveed _worldMin and _worldMax to sg_gameMng. Set them as static variables that we can access.
* 
* float distanceToBoid( const sgBoid& b ) const changed this to take in a reference.
* 
* Created a BoidData struct that holds the shared data from a type of boid.
* This will be referenced when performing calculations instead of each instance having this data.
*/


//NEW - Alex Soto
//Each instance does not require this information.
//We just need to apply this data

class sgGameMng;
namespace sgBoidType
{
    enum class Enum : uint_fast8_t
    {
        small = 0,
        big,
        count
    };

    struct BoidData
    {
        typedef uint_fast8_t BoidType;
        BoidData(
            const float vel,
            const float radius,
            const float proxTR,
            const float aligW,
            const float cohW,
            const float cohWOBT,
            const float sepW,
            const float sepWOBT,
            const Enum type
        ) :
          _vel{vel}
        , _radius{radius}
        , _proximityTestRange{proxTR}
        , _inverseTestRange{ 1.0f/proxTR }
        , _alignmentWeight{ aligW }
        , _cohesionWeight{cohW}
        , _cohesionWeightWithOtherBoidType{cohWOBT}
        , _separationWeight{sepW}
        , _separationWeightWithOtherBoidType{ sepWOBT }
        , _type{static_cast<uint_fast8_t>(type)}
        {}

        const float _vel;
        const float _radius;
        const float _proximityTestRange;
        const float _inverseTestRange;
        const float _alignmentWeight;
        const float _cohesionWeight;
        const float _cohesionWeightWithOtherBoidType;
        const float _separationWeight;
        const float _separationWeightWithOtherBoidType;
        const BoidType _type;
    };

    struct SmallBoidData : BoidData
    {

        SmallBoidData() : 
            BoidData(8.0f, 0.2f, 5.f, 2.5f, 1.f, 0.f, 1.8f, 4.f, Enum::small)
        {}  
    };

    struct LargeBoidData : BoidData
    {
        LargeBoidData() :
            BoidData(7.0f, 1.0f, 10.f, 0.f, 0.f, 0.5f, 2.f, 0.f, Enum::big)
        {}
    };
    /// <summary>
    /// You can make this a uint_fast8_t.
    /// Minimizes space allocated per instance.
    /// </summary>
}
sasALIGN16_PRE class sgBoid
{
    //NEW - Alex Soto
    //std::vector emplace_back function require that this class conform with copy and move constructor/operator.
    /*sasNO_COPY( sgBoid );
    sasMEM_OP( sgBoid );*/

public:
    typedef smVec3 smVec3_6[6];
    //NEW - Alex Soto
    typedef sgBoidType::BoidData BData;
    typedef uint_fast8_t BoidType;

    sgBoid( sgBoidType::Enum type );

    //NEW - Alex Soto
    //Creating a new constructor that takes in smVec3& of calculated random positions.
    //This is being done so that we can multithread the creation of the boids at the start of the program.
    sgBoid(sgBoidType::Enum type, smVec3& pos, smVec3& vel);
    sgBoid(sgBoidType::BoidData& data, smVec3& pos, smVec3& vel);

    ~sgBoid();
    
    void updateVelocity(const smVec3& worldMin, const smVec3& worldMax, const smVec3_6 worldEdgePoints, const smVec3_6 worldEdgeNormals);
    void updatePosition(const float deltaTime);
    void checkForCollisions(const smVec3& worldMin, const smVec3& worldMax);

    BoidType type() const 
    { 
       return _data._type;
       // return _type; 
    }
    const smVec3& pos() const { return _pos; }
    float radius() const 
    { 
        return _data._radius;
        //return _type == static_cast<uint_fast8_t>(sgBoidType::Enum::small) ? sgBoid::sbd._radius : sgBoid::lbd._radius;
    }
    const smVec3& velocity() const { return _vel; }

    float distanceToBoid( const sgBoid& b ) const;

    //NEW -Alex Soto
    bool withinRange( const sgBoid& b ) const;

    static sgBoidType::SmallBoidData sbd;
    static sgBoidType::LargeBoidData lbd;
private:
    smVec3 _pos;
    smVec3 _vel;
    const sgBoidType::BoidData& _data;
    void velocityCheckSameType();
    //Does not belong to individual instance of a boid. 
    //Saves space and is easily referencable when performing calculations.


} sasALIGN16_POST;
