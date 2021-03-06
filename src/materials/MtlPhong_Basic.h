#pragma once

#include "scene/scene.h"

//------------------------------------------------------------------------------

class MtlPhong_Basic : public Material {
 public:
  MtlPhong_Basic();

  void SetDiffuse(Color3f dif) { diffuse.SetColor(dif); }

  void SetSpecular(Color3f spec) { specular.SetColor(spec); }

  void SetGlossiness(float gloss) { glossiness = gloss; }

  void SetEmission(Color3f e) { emission.SetColor(e); }

  void SetReflection(Color3f reflect) { reflection.SetColor(reflect); }

  void SetRefraction(Color3f refract) { refraction.SetColor(refract); }

  void SetAbsorption(Color3f absorp) { absorption = absorp; }

  void SetRefractionIndex(float _ior) { ior = _ior; }

  void SetDiffuseTexture(TextureMap *map) { diffuse.SetTexture(map); }

  void SetSpecularTexture(TextureMap *map) { specular.SetTexture(map); }

  void SetEmissionTexture(TextureMap *map) { emission.SetTexture(map); }

  void SetReflectionTexture(TextureMap *map) { reflection.SetTexture(map); }

  void SetRefractionTexture(TextureMap *map) { refraction.SetTexture(map); }

  void SetReflectionGlossiness(float gloss) { reflectionGlossiness = gloss; }

  void SetRefractionGlossiness(float gloss) { refractionGlossiness = gloss; }

  Color3f Shade(const DiffRay &ray, const DiffHitInfo &hInfo,
                const LightList &lights, int bounceCount) const override;

  // OpenGL Extensions
  void SetViewportMaterial(int subMtlID) const override;
 private:
  TexturedColor diffuse, specular, reflection, refraction, emission;
  float glossiness;
  Color3f absorption;
  float ior;  // index of refraction
  float reflectionGlossiness, refractionGlossiness;
};

//------------------------------------------------------------------------------
