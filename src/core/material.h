///--------------------------------------------------------------------------//
///                                                                          //
/// Created by Qi WU on 11/24/17.                                             //
/// Copyright (c) 2017 University of Utah. All rights reserved.             //
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

#ifndef QARAY_MATERIAL_H
#define QARAY_MATERIAL_H
#pragma once

#include "core/setup.h"
#include "core/items.h"

namespace qaray {
class Material : public ItemBase {
 public:
  static int maxBounce;
 public:
  //
  // The main method that handles the shading by calling all the lights in the
  // list
  //
  // ray: incoming ray,
  // hInfo: hit information for the point that is being shaded,
  // lights: the light list,
  // bounceCount: permitted number of additional bounces for reflection and
  //              refraction.
  virtual Color3f Shade(const DiffRay &ray, const DiffHitInfo &hInfo,
                        const LightList &lights, int bounceCount) const = 0;
  // OpenGL Extensions
  virtual void SetViewportMaterial(int subMtlID) const {}
  // Photon Extensions
  // if this method returns true, the photon will be stored
  virtual bool IsPhotonSurface(int subMtlID) const
  {
    return true;
  }
  // if this method returns true, a new photon with the given direction and
  // color will be traced
  virtual bool RandomPhotonBounce(DiffRay &r, Color3f &c,
                                  const DiffHitInfo &hInfo) const
  { return false; }
};
class MaterialList : public ItemList<Material> {
 public:
  Material *Find(const char *name);
};
}
#endif //QARAY_MATERIAL_H
