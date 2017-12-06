//------------------------------------------------------------------------------
///
/// \file       objects.cpp 
/// \author     Qi WU
/// \version    1.0
/// \date       August, 2017
///
/// \brief Source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------

#include "lights.h"

//------------------------------------------------------------------------------

int GenLight::shadow_spp_min = 16;
int GenLight::shadow_spp_max = 64;
static bool useInverseSquareFalloff = true;

void DisableInverseSquareFalloff() { useInverseSquareFalloff = false; }

float InverseSquareFalloff(const Point3 &v)
{
  if (useInverseSquareFalloff) {
    return MIN(1.f, 1.f / length2(v));
  } else {
    return 1.f;
  }
}

float GenLight::Shadow(Ray ray, float t_max)
{
  HitInfo hInfo;
  hInfo.z = t_max;
  if (scene.TraceNodeShadow(scene.rootNode, ray, hInfo)) {
    return 0.0f;
  } else {
    return 1.0f;
  }
}
//------------------------------------------------------------------------------
Color3f PointLight::Illuminate(const Point3 &p, const Point3 &N) const
{
  if (size > 0.01f) {
    int spp = GenLight::shadow_spp_min, s = 0;
    float inshadow = 0.0f;
    while (s < spp) {
      const Point3 dir = position + rng->local().UniformBall(size) - p;
      Ray ray(p, dir);
      ray.Normalize();
      inshadow += (Shadow(ray, length(dir)) - inshadow) *
          InverseSquareFalloff(dir) /
          (float) (s + 1);
      s++;
      if (inshadow > 0.f && inshadow < 1.f) { spp = GenLight::shadow_spp_max; }
    };
    return inshadow * intensity;
  } else {
    const Point3 dir = position - p;
    Ray ray(p, dir);
    ray.Normalize();
    return Shadow(ray, length(dir)) *
        intensity *
        InverseSquareFalloff(dir);
  }
}
//------------------------------------------------------------------------------
DiffRay PointLight::RandomPhoton() const
{
  Point3 dir = rng->local().UniformSphere();
  return DiffRay(position, dir);
}
//------------------------------------------------------------------------------
