#include <sas_main.h>
#include <string>
#include <sasMaths.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glew.h>
#include <GL/glfw.h>
#include <AntTweakBar.h>

#ifdef __APPLE__
  #include <mach/mach_time.h>
#endif

// ============================================================================

void                mainLoop();
bool                initialize();
void                update();
void                shutdown();
bool                loadScene();

// ============================================================================

struct Global
{
  sasResource*  scene;
  sasCamera*    camera;
  int           mouseX;
  int           mouseY;

  Global()
  {    
    scene = 0;
    camera = 0;
    mouseX = mouseY = -1;
  }
} G;


// ============================================================================

// Callback function called by GLFW when a mouse button is clicked
void GLFWCALL OnMouseButton(int glfwButton, int glfwAction)
{
    if( !TwEventMouseButtonGLFW(glfwButton, glfwAction) )   // Send event to AntTweakBar
    {
        // Event has not been handled by AntTweakBar
        // Do something if needed.
    }
}


// Callback function called by GLFW when mouse has moved
void GLFWCALL OnMousePos(int mouseX, int mouseY)
{
    if( !TwEventMousePosGLFW(mouseX, mouseY) )  // Send event to AntTweakBar
    {
        // Event has not been handled by AntTweakBar
        // Do something if needed.
    }
}


// Callback function called by GLFW on mouse wheel event
void GLFWCALL OnMouseWheel(int pos)
{
    if( !TwEventMouseWheelGLFW(pos) )   // Send event to AntTweakBar
    {
        // Event has not been handled by AntTweakBar
        // Do something if needed.
    }
}
// Callback function called by GLFW on key event
void GLFWCALL OnKey(int glfwKey, int glfwAction)
{
    if( !TwEventKeyGLFW(glfwKey, glfwAction) )  // Send event to AntTweakBar
    {
      if( glfwAction == GLFW_PRESS )
      {
        if( glfwKey==GLFW_KEY_ESC ) // Want to quit?
        {
          glfwCloseWindow();          
        }
        else if( glfwKey == GLFW_KEY_SPACE )
        {
          sasReloadShaders();
        }
      }
    }
}


// Callback function called by GLFW on char event
void GLFWCALL OnChar(int glfwChar, int glfwAction)
{
    if( !TwEventCharGLFW(glfwChar, glfwAction) )    // Send event to AntTweakBar
    {
        // Event has not been handled by AntTweakBar
        // Do something if needed.
    }
}


// Callback function called by GLFW when window size changes
void GLFWCALL OnWindowSize(int width, int height)
{    
    // Send the new window size to AntTweakBar
    TwWindowSize(width, height);
}

int main( int argc, char** argv )
{
  if( glfwInit() == GL_FALSE )
  {
    printf("Couldn't initialize GLFW\n");
    return -1;
  }

  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
  glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   GLFWvidmode mode;
   glfwGetDesktopMode(&mode);
   if( glfwOpenWindow(1024, 768, mode.RedBits, mode.GreenBits, mode.BlueBits, 0, 0, 0, GLFW_WINDOW) == GL_FALSE )
   {
     printf("Couldn't open window.\n");
     glfwTerminate();
     return -1;
   }

   glfwSetWindowSizeCallback(OnWindowSize);
   glfwSetMouseButtonCallback(OnMouseButton);
   glfwSetMousePosCallback(OnMousePos);
   glfwSetMouseWheelCallback(OnMouseWheel);
   glfwSetKeyCallback(OnKey);
   glfwSetCharCallback(OnChar);

   sasConfig config;
   config.shaderFolder = sas_SHADERDIR;
   config.resourcePath = sas_RSCDIR;

  if( sasInitialize( config ) )
  {
    if( initialize() )
    {
      mainLoop();
      shutdown();
    }      
    sasShutdown();
  }

  glfwTerminate();

  return 0;
}


void mainLoop()
{
  bool shouldExit = false;

  while( !shouldExit )
  {
    glClear( GL_COLOR_BUFFER_BIT );
    update();
    glfwSwapBuffers();
    shouldExit = glfwGetKey( GLFW_KEY_ESC ) || !glfwGetWindowParam( GLFW_OPENED );
#ifndef _WIN32
    sleep(0);
#else
    Sleep(0);
#endif
  }
}

bool initialize()
{ 
  if (!loadScene())
  {
    shutdown();
    return false;
  }

  return true;
}

void update()
{
#ifdef __APPLE__
  mach_timebase_info_data_t info;
  mach_timebase_info(&info);
  double nano = 1e-9 * ( (double) info.numer) / ((double) info.denom);
    
  static uint64_t lastFrameTime = mach_absolute_time();
  uint64_t currentTime = mach_absolute_time();
  
  double dt = (currentTime - lastFrameTime) * nano;
  lastFrameTime = currentTime;
#endif

#ifdef _WIN32
  // update delta time
  float dt = 0.f;
  {
    LARGE_INTEGER freq;
    LARGE_INTEGER cur;
    QueryPerformanceFrequency( &freq );
    QueryPerformanceCounter( &cur );
    static LARGE_INTEGER prev = cur;
    dt = (float)(cur.QuadPart-prev.QuadPart) / (float)freq.QuadPart;
    prev = cur;
  }
#endif

  // update camera pos
  if( G.camera )
  {
    smVec4 pos    = sasCameraPosition( G.camera );
    smVec4 front  = sasCameraFront( G.camera );
    smVec4 right  = sasCameraRight( G.camera );
    smVec4 dpos( 0.f );
    static float rotUp = 45.f;
    static float rotRight = 0.f;

    if( glfwGetKey('W') )    
      dpos += front * 5.f;
    if( glfwGetKey('S') )
      dpos -= front * 5.f;
    if( glfwGetKey('A') )
      dpos -= right * 5.f;
    if( glfwGetKey('D') )  
      dpos += right * 5.f;
    if( glfwGetKey(GLFW_KEY_LEFT) )
      rotUp -= 100.f*dt;
    if( glfwGetKey(GLFW_KEY_RIGHT) )
      rotUp += 100.f*dt;
    if( glfwGetKey(GLFW_KEY_UP) )
      rotRight += 100.f*dt;
    if( glfwGetKey(GLFW_KEY_DOWN) )
      rotRight -= 100.f*dt;

    pos += dt * dpos;
    rotRight = smNormalizeAngle( rotRight );   
    rotUp = smNormalizeAngle( rotUp );
    
    smVec4 lookAt( 0,0,-1,0 );
    smMat44 mRotRight;
    smMat44 mRotUp;
    smMat44 mRotFinal;
    smGetRotIMatrix( rotRight, mRotRight );
    smGetRotJMatrix( rotUp, mRotUp );
    smMul3( mRotUp, mRotRight, mRotFinal );
    smMul3( mRotFinal, lookAt, lookAt );
    smAdd3( pos, lookAt, lookAt );    

    sasCameraSetPosition( pos, G.camera );
    sasCameraLookAt( lookAt, smVec4(0,1,0,0), G.camera );
  }

  sasUpdate();  
}

void shutdown()
{ 
  if( G.camera )
    sasDestroyCamera( G.camera );
  if( G.scene )
    sasUnloadResource( G.scene );
}

bool loadScene()
{
  std::string scenePath = sas_RSCDIR;
  scenePath += "sponza.dae";
  G.camera = sasCreateCamera();
  sasCameraSetPosition( smVec4(-9.38f, 7.58f, 0.27f, 0.0f), G.camera );
  //sasCameraLookAt( smVec4(0,12,-1,1), smVec4(0,1,0,0), G.camera );
  sasCameraLookAt( smVec4(0,0,0,1), smVec4(0,1,0,0), G.camera );
  G.scene = sasLoadResource( scenePath.c_str() );  
  return true;
}
