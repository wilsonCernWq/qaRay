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

#ifndef QARAY_SETUP_H
#define QARAY_SETUP_H
#pragma once

#include <cassert>
#include <string>
#include <iostream>
#include <vector>
#include <atomic>

#define debug(x) (std::cout << #x << " " << (x) << std::endl)
#define TEXTURE_SAMPLE_COUNT 32

#define HIT_NONE           1<<0
#define HIT_FRONT          1<<1
#define HIT_BACK           1<<2
#define HIT_FRONT_AND_BACK (HIT_FRONT|HIT_BACK)

#define BIGFLOAT 1.0e30f

namespace qaray {
class Box;
class Camera;
class DiffRay;
class DiffHitInfo;
class HitInfo;
class Light;
class ItemBase;
class Material;
class MaterialList;
class Node;
class Object;
class Ray;
class Sampler;
class Texture;
class TextureMap;
class TexturedColor;
class Transformation;
template<class T>
class ItemList;
template<class T>
class ItemFileList;
typedef ItemList<Light> LightList;
typedef ItemFileList<Object> ObjFileList;
};

#endif //QARAY_SETUP_H
