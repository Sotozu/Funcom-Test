#pragma once
#include "utils/sas_singleton.h"
#include "utils/sas_stringid.h"

class sasConstantBuffer;
class sasRenderTarget;
class sasStructuredBuffer;

namespace sasEditorQueryState
{
  enum Enum
  {
    Issued = 0,
    Resolved,
    Inactive,
  };
}

class sasEditor : public sasSingleton< sasEditor >
{
  sasNO_COPY( sasEditor );  
  sasMEM_OP( sasEditor );

public:
  sasEditor();
  ~sasEditor();

  void setEnabled( bool state );
  bool enabled() const          { return _enabled; }
  
  //update methods for editor
  void step();
  void frameInitialize();
  void frameEnd();
  void runComputeJobs();

  /*
    interface for new gamecode side editor
  */
  void kickMeshPickingTask( uint32_t x, uint32_t y );
  void kickBonePickTaskForInstance( uint32_t x, uint32_t y, sasStringId modelId, uint32_t meshId );
  const sasPickedMeshData& pickedMeshData() const;
  bool areMeshPickingResultsReady() const;
  const sasPickedBoneData& pickedBoneData() const;
  bool areBonePickingResultsReady() const;


  sasConstantBuffer*    _constantBuffer;
  sasStructuredBuffer*  _readbackBuffer;
  sasStructuredBuffer*  _stagingBuffer;

  sasRenderTarget*    _meshIdBuffer;

  float         _lastPickingTimestamp;
  unsigned int  _pickedMeshInstanceId;
  bool          _pickMeshThisFrame;
  bool          _readbackRequired;

  bool          _enabled;

  // query data for the game side editor
  sasPickedMeshData           _pickedMeshData;
  sasEditorQueryState::Enum   _pickedMeshDataState;
  sasPickedBoneData           _pickedBoneData;
  sasEditorQueryState::Enum   _pickedBoneDataState;
  uint32_t                    _queryX;
  uint32_t                    _queryY;
  sasStringId                 _queryModelId;
  uint32_t                    _queryMeshId;
};
