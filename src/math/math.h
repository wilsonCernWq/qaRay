///--------------------------------------------------------------------------//
///                                                                          //
/// Copyright(c) 2017-2018, Qi WU (University of Utah)                       //
///                                                                          //
/// Redistribution and use in source and binary forms, with or without       //
/// modification, are permitted provided that the following conditions are   //
/// met:                                                                     //
///  - Redistributions of source code must retain the above copyright        //
///    notice, this list of conditions and the following disclaimer.         //
///  - Redistributions in binary form must reproduce the above copyright     //
///    notice, this list of conditions and the following disclaimer in the   //
///    documentation and/or other materials provided with the distribution.  //
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS  //
/// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED    //
/// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A          //
/// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT       //
/// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,   //
/// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT         //
/// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,    //
/// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    //
/// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT      //
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE    //
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.     //
///                                                                          //
///--------------------------------------------------------------------------//
#ifndef QARAY_MATH_H
#define QARAY_MATH_H
#pragma once
//-----------------------------------------------------------------------------
#ifndef _USE_MATH_DEFINES
# define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <algorithm>
#include "scalar.h"
#include "vector.h"
//-----------------------------------------------------------------------------
#ifndef USE_GLM
# error "Ray-packet project requires GLM"
#else
# include <glm/glm.hpp>
# include <glm/gtc/matrix_transform.hpp>
# include <glm/gtc/matrix_access.hpp>
# include <glm/gtc/type_ptr.hpp>
# include <glm/gtx/norm.hpp>
#endif
//-----------------------------------------------------------------------------
typedef bool          qaBOOL;
typedef unsigned char qaUCHAR;
typedef unsigned int  qaUINT;
typedef int           qaINT;
typedef float         qaFLOAT;
typedef double        qaDOUBLE;
//-----------------------------------------------------------------------------
namespace qaray
{
const float PI = static_cast<float>(M_PI);
}
//-----------------------------------------------------------------------------
#define DEFINE_TYPE(TYPE, N, T)                         \
  namespace qaray                                       \
  {                                                     \
    typedef glm::tvec##N<TYPE> vec##N##T;               \
    typedef glm::tmat##N##x##N<TYPE> mat##N##T;         \
    struct bbox##N##T { vec##N##T upper, lower; };      \
    struct affine##N##T { mat##N##T l; vec##N##T p; };  \
  };
#define DEFINE_ALL_DEC(type, t) \
  DEFINE_TYPE(type, 2, t)       \
  DEFINE_TYPE(type, 3, t)       \
  DEFINE_TYPE(type, 4, t)
DEFINE_ALL_DEC(unsigned char, c);
DEFINE_ALL_DEC(int, i);
DEFINE_ALL_DEC(float, f);
DEFINE_ALL_DEC(double, d);
//-----------------------------------------------------------------------------
namespace qaray {
typedef vec3f Color3f;
typedef vec3c Color3c;
typedef vec4f Color4f;
typedef vec4c Color4c;
typedef qaray::vec2f Point2;
typedef qaray::vec3f Point3;
typedef qaray::vec4f Point4;
typedef qaray::mat3f Matrix3;
typedef qaray::mat4f Matrix4;
//-----------------------------------------------------------------------------
inline Color3f ToColor(const Color3c &c)
{
  return Color3f(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f);
}
//-----------------------------------------------------------------------------
template<typename T>
qaFLOAT length(const T& v) { return glm::length(v); }
template<typename T>
qaFLOAT length2(const T &v) { return glm::length2(v); }
template<typename T>
qaFLOAT dot(const T &a, const T &b) { return glm::dot(a, b); }
template<typename T>
T cross(const T &a, const T &b) { return glm::cross(a, b); }
};
//-----------------------------------------------------------------------------
using namespace qaray;
//-----------------------------------------------------------------------------
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define CLAMP(X, L, U) MIN(U, MAX(L, X))
#define ABS(x)    ((x) > 0 ? (x) : -(x))
#define POW(x, y) (std::pow(x, y))
#define SQRT(x)   (std::sqrt(x))
#define CEIL(x)   (std::ceil(x))
#define FLOOR(x)  (std::floor(x))
#define SIN(x)    (std::sin(x))
#define COS(x)    (std::cos(x))
#define BIGFLOAT 1.0e30f
//-----------------------------------------------------------------------------
#endif //QARAY_MATH_H