
#ifndef NIF_VIEW_HPP_INCLUDED
#define NIF_VIEW_HPP_INCLUDED

#include "common.hpp"
#include "ba2file.hpp"
#include "bgsmfile.hpp"
#include "nif_file.hpp"
#include "ddstxt.hpp"
#include "plot3d.hpp"
#include "rndrbase.hpp"
#include "sdlvideo.hpp"

#include <thread>
#include <mutex>

class Renderer : protected Renderer_Base
{
 protected:
  const BA2File&  ba2File;
  ESMFile *esmFile;
  std::vector< Plot3D_TriShape * >  renderers;
  std::vector< std::string >  threadErrMsg;
  std::vector< int >  viewOffsetY;
  std::vector< NIFFile::NIFTriShape > meshData;
  TextureCache  textureSet;
  std::vector< std::vector< unsigned char > > threadFileBuffers;
  int     threadCnt;
  float   lightX, lightY, lightZ;
  NIFFile *nifFile;
  NIFFile::NIFVertexTransform modelTransform;
  NIFFile::NIFVertexTransform viewTransform;
  unsigned int  materialSwapTable[8];
  MaterialSwaps materialSwaps;
  DDSTexture  defaultTexture;
  std::string defaultEnvMap;
  std::string waterTexture;
  static void threadFunction(Renderer *p, size_t n);
  const DDSTexture *loadTexture(const std::string& texturePath,
                                size_t threadNum = 0);
  void setDefaultTextures();
 public:
  float   modelRotationX, modelRotationY, modelRotationZ;
  float   viewRotationX, viewRotationY, viewRotationZ;
  float   viewOffsX, viewOffsY, viewOffsZ, viewScale;
  float   lightRotationY, lightRotationZ;
  FloatVector4  lightColor;
  FloatVector4  envColor;
  float   rgbScale;
  float   reflZScale;
  float   waterEnvMapLevel;
  std::uint32_t waterColor;
  int     defaultEnvMapNum;     // 0 to 7
  int     debugMode;            // 0 to 5
  Renderer(const BA2File& archiveFiles, ESMFile *esmFilePtr = (ESMFile *) 0);
  virtual ~Renderer();
  void loadModel(const std::string& fileName);
  void renderModel(std::uint32_t *outBufRGBA, float *outBufZ,
                   int imageWidth, int imageHeight);
  // if formID & 0x80000000 is set, formID & 0xFF is interpreted as color swap
  void addMaterialSwap(unsigned int formID);
  void clearMaterialSwaps();
  void setWaterColor(unsigned int watrFormID);
  void renderModelToFile(const char *outFileName,
                         int imageWidth, int imageHeight);
  void viewModels(SDLDisplay& display,
                  const std::vector< std::string >& nifFileNames);
};

#endif

