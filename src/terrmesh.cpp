
#include "common.hpp"
#include "terrmesh.hpp"
#include "landtxt.hpp"

void TerrainMesh::calculateNormal(
    float& normalX, float& normalY, float& normalZ,
    float dx, float dy, float dz1, float dz2)
{
  float   tmp1 = 1.0f + (dz1 * dz1);
  float   tmp2 = 1.0f + (dz2 * dz2);
  float   tmp3 = float(std::sqrt(tmp2 / tmp1));
  float   dz = (dz1 * tmp3 + dz2) / (tmp3 + 1.0f);
  normalZ += 0.5f;
  normalX -= (dz * dx);
  normalY -= (dz * dy);
}

TerrainMesh::TerrainMesh()
  : NIFFile::NIFTriShape(),
    landTexture((DDSTexture *) 0)
{
}

TerrainMesh::~TerrainMesh()
{
  if (landTexture)
    delete landTexture;
}

void TerrainMesh::createMesh(
    const unsigned short *hmapData, int hmapWidth, int hmapHeight,
    const unsigned char *ltexData, int ltexWidth, int ltexHeight,
    int textureScale, int x0, int y0, int x1, int y1, int cellResolution,
    float xOffset, float yOffset, float zMin, float zMax)
{
  int     w = std::abs(x1 - x0) + 1;
  int     h = std::abs(y1 - y0) + 1;
  int     txtW = w << textureScale;
  int     txtH = h << textureScale;
  x0 = (x0 < x1 ? x0 : x1);
  y0 = (y0 < y1 ? y0 : y1);
  x1 = x0 + w - 1;
  y1 = y0 + h - 1;
  vertexCnt = size_t(w) * size_t(h);
  triangleCnt = (size_t(w - 1) * size_t(h - 1)) << 1;
  vertexDataBuf.resize(vertexCnt);
  triangleDataBuf.resize(triangleCnt);
  vertexData = &(vertexDataBuf.front());
  triangleData = &(triangleDataBuf.front());
  NIFFile::NIFVertex  *vertexPtr = &(vertexDataBuf.front());
  float   xyScale = 4096.0f / float(cellResolution);
  float   zScale = (zMax - zMin) / 65535.0f;
  float   normalScale1 = float(cellResolution) * (1.0f / 4096.0f);
  float   normalScale2 = float(cellResolution) * (0.7071068f / 4096.0f);
  for (int y = y1; y >= y0; y--)
  {
    for (int x = x0; x <= x1; x++, vertexPtr++)
    {
      static const int  xOffsTable[9] = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
      static const int  yOffsTable[9] = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };
      float   z[9];
      for (int i = 0; i < 9; i++)
      {
        int     xc = x + xOffsTable[i];
        int     yc = y + yOffsTable[i];
        xc = (xc > 0 ? (xc < (hmapWidth - 1) ? xc : (hmapWidth - 1)) : 0);
        yc = (yc > 0 ? (yc < (hmapHeight - 1) ? yc : (hmapHeight - 1)) : 0);
        z[i] = float(int(hmapData[size_t(yc) * size_t(hmapWidth) + size_t(xc)]))
               * zScale + zMin;
      }
      vertexPtr->x = xOffset + (float(x) * xyScale);
      vertexPtr->y = yOffset - (float(y) * xyScale);
      vertexPtr->z = z[4];
      float   normalX = 0.0f;
      float   normalY = 0.0f;
      float   normalZ = 0.0f;
      calculateNormal(normalX, normalY, normalZ, 1.0f, 0.0f,
                      (z[4] - z[3]) * normalScale1,     // -W
                      (z[5] - z[4]) * normalScale1);    // +E
      calculateNormal(normalX, normalY, normalZ, 0.0f, 1.0f,
                      (z[4] - z[7]) * normalScale1,     // -S
                      (z[1] - z[4]) * normalScale1);    // +N
      if ((x ^ y) & 1)
      {
        //    0 1 2
        // -1 +-+-+
        //    |\|/|
        //  0 +-+-+
        //    |/|\|
        //  1 +-+-+
        calculateNormal(normalX, normalY, normalZ, -0.7071068f, 0.7071068f,
                        (z[4] - z[8]) * normalScale2,   // -SE
                        (z[0] - z[4]) * normalScale2);  // +NW
        calculateNormal(normalX, normalY, normalZ, 0.7071068f, 0.7071068f,
                        (z[4] - z[6]) * normalScale2,   // -SW
                        (z[2] - z[4]) * normalScale2);  // +NE
      }
      float   tmp =
          (normalX * normalX) + (normalY * normalY) + (normalZ * normalZ);
      tmp = 1.0f / float(std::sqrt(tmp));
      vertexPtr->normalX = normalX * tmp;
      vertexPtr->normalY = normalY * tmp;
      vertexPtr->normalZ = normalZ * tmp;
    }
  }
}

void TerrainMesh::createMesh(
    const LandscapeData& landData,
    int textureScale, int x0, int y0, int x1, int y1, float textureMip,
    const DDSTexture * const *landTextures, size_t landTextureCnt)
{
  int     w = std::abs(x1 - x0) + 1;
  int     h = std::abs(y1 - y0) + 1;
  int     txtW = w << textureScale;
  int     txtH = h << textureScale;
  x0 = (x0 < x1 ? x0 : x1);
  y0 = (y0 < y1 ? y0 : y1);
  x1 = x0 + w - 1;
  y1 = y0 + h - 1;
  {
    textureBuf2.resize(size_t(txtW) * size_t(txtH) * 3U);
    LandscapeTexture  landTxt(landData, landTextures, landTextureCnt);
    landTxt.setMipLevel(textureMip);
    landTxt.renderTexture(&(textureBuf2.front()), textureScale, x0, y0, x1, y1);
  }
  hmapBuf.resize(size_t(w) * size_t(h));
  unsigned short  *dstPtr = &(hmapBuf.front());
  int     landWidth = landData.getImageWidth();
  int     landHeight = landData.getImageHeight();
  for (int y = y0; y <= y1; y++)
  {
    int     yc = (y > 0 ? (y < (landHeight - 1) ? y : (landHeight - 1)) : 0);
    const unsigned short  *srcPtr = landData.getHeightMap();
    srcPtr = srcPtr + (size_t(yc) * size_t(landWidth));
    for (int x = x0; x <= x1; x++, dstPtr++)
    {
      int     xc = (x > 0 ? (x < (landWidth - 1) ? x : (landWidth - 1)) : 0);
      *dstPtr = srcPtr[xc];
    }
  }
  int     cellResolution = landData.getCellResolution();
  float   xOffset = float(x0 - landData.getOriginX());
  float   yOffset = float(landData.getOriginY() - y0);
  xOffset = xOffset * (4096.0f / float(cellResolution));
  yOffset = yOffset * (4096.0f / float(cellResolution));
  createMesh(&(hmapBuf.front()), w, h, &(textureBuf2.front()), txtW, txtH,
             textureScale, 0, 0, w - 1, h - 1, cellResolution,
             xOffset, yOffset, landData.getZMin(), landData.getZMax());
}

