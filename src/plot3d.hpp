
#ifndef PLOT3D_HPP_INCLUDED
#define PLOT3D_HPP_INCLUDED

#include "common.hpp"
#include "ddstxt.hpp"
#include "nif_file.hpp"

template< typename T, typename ColorType > class Plot3D : public T
{
 protected:
  struct Line
  {
    int     x;
    int     y;
    int     xEnd;
    int     yEnd;
    int     dx;
    int     dy;
    bool    verticalFlag;
    signed char xStep;
    signed char yStep;
    int     f;
    ColorType z;
    ColorType zStep;
    Line(int x0, int y0, ColorType z0, int x1, int y1, ColorType z1)
      : x(x0),
        y(y0),
        xEnd(x1),
        yEnd(y1),
        dx(x1 >= x0 ? (x1 - x0) : (x0 - x1)),
        dy(y1 >= y0 ? (y1 - y0) : (y0 - y1)),
        verticalFlag(dy > dx),
        xStep((signed char) (x1 > x0 ? 1 : (x1 < x0 ? -1 : 0))),
        yStep((signed char) (y1 > y0 ? 1 : (y1 < y0 ? -1 : 0))),
        f(verticalFlag ? dy : dx),
        z(z0),
        zStep(z1)
    {
      zStep -= z0;
      if (f)
        zStep *= (1.0f / float(f));
      f = f >> 1;
    }
    inline bool step()
    {
      z += zStep;
      if (verticalFlag)
      {
        f += dx;
        if (f >= dy)
        {
          f -= dy;
          x += int(xStep);
        }
        y += int(yStep);
        return (y != yEnd);
      }
      f += dy;
      if (f >= dx)
      {
        f -= dx;
        y += int(yStep);
      }
      x += int(xStep);
      return (x != xEnd);
    }
  };
  static inline void vertexSwap(int& x0, int& y0, ColorType& z0,
                                int& x1, int& y1, ColorType& z1)
  {
    {
      int     tmp = x0;
      x0 = x1;
      x1 = tmp;
      tmp = y0;
      y0 = y1;
      y1 = tmp;
    }
    ColorType tmp = z0;
    z0 = z1;
    z1 = tmp;
  }
 public:
#if 0
  // implemented in T
  inline void drawPixel(int x, int y, ColorType z);
#endif
  void drawLineH(int x0, int y0, ColorType z0, int x1, ColorType z1)
  {
    T::drawPixel(x0, y0, z0);
    if (x0 == x1)
    {
      T::drawPixel(x1, y0, z1);
      return;
    }
    int       xStep = (x1 > x0 ? 1 : -1);
    float     d = float(xStep) / float(x1 - x0);
    ColorType zStep(z1);
    zStep -= z0;
    zStep *= d;
    do
    {
      x0 += xStep;
      z0 += zStep;
      T::drawPixel(x0, y0, z0);
    }
    while (x0 != x1);
  }
  void drawLineV(int x0, int y0, ColorType z0, int y1, ColorType z1)
  {
    T::drawPixel(x0, y0, z0);
    if (y0 == y1)
    {
      T::drawPixel(x0, y1, z1);
      return;
    }
    int       yStep = (y1 > y0 ? 1 : -1);
    float     d = float(yStep) / float(y1 - y0);
    ColorType zStep(z1);
    zStep -= z0;
    zStep *= d;
    do
    {
      y0 += yStep;
      z0 += zStep;
      T::drawPixel(x0, y0, z0);
    }
    while (y0 != y1);
  }
  void drawLine(int x0, int y0, ColorType z0, int x1, int y1, ColorType z1)
  {
    if (y0 == y1)
    {
      drawLineH(x0, y0, z0, x1, z1);
      return;
    }
    if (x0 == x1)
    {
      drawLineV(x0, y0, z0, y1, z1);
      return;
    }
    Line    l(x0, y0, z0, x1, y1, z1);
    do
    {
      T::drawPixel(l.x, l.y, l.z);
    }
    while (l.step());
    T::drawPixel(x1, y1, z1);
  }
  void drawTriangle(int x0, int y0, ColorType z0,
                    int x1, int y1, ColorType z1,
                    int x2, int y2, ColorType z2)
  {
    T::drawPixel(x0, y0, z0);
    if (x0 == x1 && x0 == x2 && y0 == y1 && y0 == y2)
    {
      T::drawPixel(x1, y1, z1);
      T::drawPixel(x2, y2, z2);
      return;
    }
    // sort vertices by Y coordinate
    if (y0 > y1)
      vertexSwap(x0, y0, z0, x1, y1, z1);
    if (y1 > y2)
    {
      if (y0 > y2)
        vertexSwap(x0, y0, z0, x2, y2, z2);
      vertexSwap(x1, y1, z1, x2, y2, z2);
    }
    Line    l01(x0, y0, z0, x1, y1, z1);
    Line    l02(x0, y0, z0, x2, y2, z2);
    Line    l12(x1, y1, z1, x2, y2, z2);
    bool    continueFlag01 = (y1 > y0 || x1 != x0);
    bool    continueFlag02 = (y2 > y0 || x2 != x0);
    bool    continueFlag12 = (y2 > y1 || x2 != x1);
    do
    {
      if (continueFlag01 && l01.y == l02.y)
        drawLineH(l01.x, l01.y, l01.z, l02.x, l02.z);
      if (continueFlag12 && l12.y == l02.y)
        drawLineH(l12.x, l12.y, l12.z, l02.x, l02.z);
      int     prvY = l02.y;
      while (continueFlag02 && l02.y <= prvY)
      {
        continueFlag02 = l02.step();
        T::drawPixel(l02.x, l02.y, l02.z);
      }
      while (continueFlag01 && l01.y <= prvY)
      {
        continueFlag01 = l01.step();
        T::drawPixel(l01.x, l01.y, l01.z);
      }
      while (continueFlag12 && l12.y <= prvY)
      {
        continueFlag12 = l12.step();
        T::drawPixel(l12.x, l12.y, l12.z);
      }
    }
    while (continueFlag01 | continueFlag02 | continueFlag12);
  }
  void drawRectangle(int x0, int y0, int x1, int y1, ColorType z)
  {
    if (x0 > x1)
    {
      int     tmp = x0;
      x0 = x1;
      x1 = tmp;
    }
    if (y0 > y1)
    {
      int     tmp = y0;
      y0 = y1;
      y1 = tmp;
    }
    do
    {
      int     x = x0;
      do
      {
        T::drawPixel(x, y0, z);
      }
      while (++x <= x1);
    }
    while (++y0 <= y1);
  }
};

struct ColorV2
{
  // Z, light
  float   v0;
  float   v1;
  inline ColorV2& operator+=(const ColorV2& d)
  {
    v0 += d.v0;
    v1 += d.v1;
    return (*this);
  }
  inline ColorV2& operator-=(const ColorV2& d)
  {
    v0 -= d.v0;
    v1 -= d.v1;
    return (*this);
  }
  inline ColorV2& operator*=(float d)
  {
    v0 *= d;
    v1 *= d;
    return (*this);
  }
};

struct ColorV4 : public ColorV2
{
  // U, V
  float   v2;
  float   v3;
  inline ColorV4& operator+=(const ColorV4& d)
  {
    v0 += d.v0;
    v1 += d.v1;
    v2 += d.v2;
    v3 += d.v3;
    return (*this);
  }
  inline ColorV4& operator-=(const ColorV4& d)
  {
    v0 -= d.v0;
    v1 -= d.v1;
    v2 -= d.v2;
    v3 -= d.v3;
    return (*this);
  }
  inline ColorV4& operator*=(float d)
  {
    v0 *= d;
    v1 *= d;
    v2 *= d;
    v3 *= d;
    return (*this);
  }
};

struct ColorV6 : public ColorV4
{
  // normalX, normalY
  float   v4;
  float   v5;
  inline ColorV6& operator+=(const ColorV6& d)
  {
    v0 += d.v0;
    v1 += d.v1;
    v2 += d.v2;
    v3 += d.v3;
    v4 += d.v4;
    v5 += d.v5;
    return (*this);
  }
  inline ColorV6& operator-=(const ColorV6& d)
  {
    v0 -= d.v0;
    v1 -= d.v1;
    v2 -= d.v2;
    v3 -= d.v3;
    v4 -= d.v4;
    v5 -= d.v5;
    return (*this);
  }
  inline ColorV6& operator*=(float d)
  {
    v0 *= d;
    v1 *= d;
    v2 *= d;
    v3 *= d;
    v4 *= d;
    v5 *= d;
    return (*this);
  }
};

class Plot3D_TriShape : public NIFFile::NIFTriShape
{
 public:
  static inline unsigned int multiplyWithLight(unsigned int c,
                                               unsigned int alphaThreshold,
                                               int lightLevel)
  {
    if (c < alphaThreshold)
      return 0U;
    unsigned long long  tmp =
        (unsigned long long) (c & 0x000000FFU)
        | ((unsigned long long) (c & 0x0000FF00U) << 12)
        | ((unsigned long long) (c & 0x00FF0000U) << 24);
    tmp = (tmp * (unsigned int) lightLevel) + 0x0000800008000080ULL;
    tmp = tmp | ((((tmp >> 8) | (tmp >> 9)) & 0x0001000010000100ULL) * 0xFFU);
    return ((unsigned int) ((tmp >> 8) & 0x000000FFU)
            | (unsigned int) ((tmp >> 20) & 0x0000FF00U)
            | (unsigned int) ((tmp >> 32) & 0x00FF0000U) | 0xFF000000U);
  }
 private:
  class Plot3DTS_Base
  {
   public:
    unsigned int  *outBufRGBW;
    float   *outBufZ;
    int     width;
    int     height;
    float   mipLevel;
    unsigned int  alphaThreshold;
    const DDSTexture  *textureD;
    const DDSTexture  *textureN;
    float   mipLevel_n;
    float   lightX;
    float   lightY;
    float   lightZ;
    const float *lightingPolynomial;
  };
  class Plot3DTS_Water : public Plot3DTS_Base
  {
   public:
    // fill with water
    inline void drawPixel(int x, int y, const ColorV2& z);
  };
  class Plot3DTS_TextureN : public Plot3DTS_Base
  {
   public:
    // fill with solid color (1x1 texture)
    inline void drawPixel(int x, int y, const ColorV2& z);
  };
  class Plot3DTS_TextureB : public Plot3DTS_Base
  {
   public:
    // diffuse texture with bilinear filtering
    inline void drawPixel(int x, int y, const ColorV4& z);
  };
  class Plot3DTS_TextureT : public Plot3DTS_Base
  {
   public:
    // diffuse texture with trilinear filtering
    inline void drawPixel(int x, int y, const ColorV4& z);
  };
  class Plot3DTS_NormalsT : public Plot3DTS_Base
  {
   public:
    // diffuse + normal map with trilinear filtering
    void drawPixel(int x, int y, const ColorV6& z);
  };
 protected:
  static const float  defaultLightingPolynomial[6];
  unsigned int  *bufRGBW;
  float   *bufZ;
  int     width;
  int     height;
  std::vector< NIFFile::NIFVertex > vertexBuf;
  float   lightingPolynomial[6];
  bool transformVertexData(const NIFFile::NIFVertexTransform& modelTransform,
                           const NIFFile::NIFVertexTransform& viewTransform,
                           float& lightX, float& lightY, float& lightZ);
 public:
  // the alpha channel is 255 for solid geometry, 0 for air, and 1 to 254
  // for water with light level converted to RGB multiplier of (alpha - 1) / 64
  Plot3D_TriShape(unsigned int *outBufRGBW, float *outBufZ,
                  int imageWidth, int imageHeight,
                  const NIFFile::NIFTriShape& t);
  virtual ~Plot3D_TriShape();
  // set polynomial a[0..5] for mapping dot product (-1.0 to 1.0)
  // to RGB multiplier
  void setLightingFunction(const float *a);
  void getLightingFunction(float *a) const;
  static void getDefaultLightingFunction(float *a);
  // calculate RGB multiplier from normals and light direction
  static float calculateLighting(float normalX, float normalY, float normalZ,
                                 float lightX, float lightY, float lightZ,
                                 const float *a);
  void drawTriShape(const NIFFile::NIFVertexTransform& modelTransform,
                    const NIFFile::NIFVertexTransform& viewTransform,
                    float lightX, float lightY, float lightZ,
                    const DDSTexture *textureD,
                    const DDSTexture *textureN = (DDSTexture *) 0);
};

#endif

