#pragma once 

#include "utils/sas_singleton.h"

class sasGui : public sasSingleton< sasGui >
{
  sasMEM_OP( sasGui );

public: 
  sasGui();
  ~sasGui();

  bool applyInput( const sasInput& pInput);	
  void render();
  void resizeWindow();
};

