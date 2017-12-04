//------------------------------------------------------------------------------
///
/// \file       materials.h
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    11.0
/// \date       November 11, 2015
///
/// \brief Example source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------
#ifndef _MATERIALS_H_INCLUDED_
#define _MATERIALS_H_INCLUDED_

#include "scene.h"

//------------------------------------------------------------------------------

Color3f Attenuation(const Color3f &absorption, const float l);

float LinearToSRGB(const float c);

//------------------------------------------------------------------------------

#include "materials/MtlBlinn_PathTracing.h"
#include "materials/MtlBlinn_MonteCarloGI.h"
#include "materials/MtlBlinn_Basic.h"
#include "materials/MtlPhong_Basic.h"

//------------------------------------------------------------------------------

using MtlBlinn = MtlBlinn_PathTracing;
//using MtlBlinn = MtlBlinn_MonteCarloGI;
//using MtlBlinn = MtlBlinn_Basic;
//using MtlBlinn = MtlPhong_Basic;

//------------------------------------------------------------------------------

class MultiMtl : public Material {
 public:
  virtual ~MultiMtl()
  {
    for (unsigned int i = 0; i < mtls.size(); i++) delete mtls[i];
  }

  virtual Color3f Shade(const DiffRay &ray, const DiffHitInfo &hInfo,
                      const LightList &lights, int bounceCount) const
  {
    return hInfo.c.mtlID < (int) mtls.size() ?
           mtls[hInfo.c.mtlID]->Shade(ray, hInfo, lights, bounceCount) :
           Color3f(1, 1, 1);
  }

  virtual void SetViewportMaterial(int subMtlID = 0) const
  {
    if (subMtlID < (int) mtls.size()) mtls[subMtlID]->SetViewportMaterial();
  }

  void AppendMaterial(Material *m) { mtls.push_back(m); }

 private:
  std::vector<Material *> mtls;
};

//------------------------------------------------------------------------------

#endif