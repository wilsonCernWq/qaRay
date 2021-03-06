#include "lights/lights.h"
#include "materials/materials.h"
#include "MtlBlinn_Basic.h"

//------------------------------------------------------------------------------

MtlBlinn_Basic::MtlBlinn_Basic() :
    diffuse(0.5f, 0.5f, 0.5f),
    specular(0.7f, 0.7f, 0.7f),
    glossiness(20.0f),
    reflection(0, 0, 0),
    refraction(0, 0, 0),
    absorption(0, 0, 0),
    ior(1),
    reflectionGlossiness(0),
    refractionGlossiness(0)
{
  DisableInverseSquareFalloff();
}

//------------------------------------------------------------------------------

const float glossy_threshold = 0.001f;
const float total_reflection_threshold = 1.01f;
const float refraction_angle_threshold = 0.01f;
const float reflection_angle_threshold = 0.01f;
const float refraction_color_threshold = 0.01f;
const float reflection_color_threshold = 0.01f;

Color3f MtlBlinn_Basic::Shade(const DiffRay &ray,
                            const DiffHitInfo &hInfo,
                            const LightList &lights,
                            int bounceCount)
const
{
  // input parameters
  Color3f color(0.f, 0.f, 0.f);
  const auto
      N = glm::normalize(hInfo.c.N);   // surface normal in world coordinate
  const auto V = glm::normalize(-ray.c.dir); // ray incoming direction
  const auto Vx = glm::normalize(-ray.x.dir); // diff ray incoming direction
  const auto Vy = glm::normalize(-ray.y.dir); // diff ray incoming direction
  const auto
      p = hInfo.c.p;                  // surface position in world coordinate
  const auto px = ray.x.p + ray.x.dir * hInfo.x.z;
  const auto py = ray.y.p + ray.y.dir * hInfo.y.z;

  // coordinate
  const auto X = glm::normalize(glm::cross(glm::cross(N, V), N)); // Vx
  const auto Y = glm::normalize(N * glm::dot(N, V));            // Vy

  // index of refraction
  const float nIOR = hInfo.c.hasFrontHit ? 1.f / ior : ior;

  // refraction and reflection
  float cosI, sinI;
  Point3 tDir, rDir, txDir, rxDir, tyDir, ryDir;
  do {
    // jitter normal
    Point3 tjN = N, rjN = N;
    if (refractionGlossiness > glossy_threshold) {
      tjN = glm::normalize(N + rng->local().UniformBall(refractionGlossiness));
    }
    if (reflectionGlossiness > glossy_threshold) {
      rjN = glm::normalize(N + rng->local().UniformBall(reflectionGlossiness));
    }
    // incidence angle & refraction angle
    cosI = glm::dot(tjN, V);
    sinI = SQRT(1 - cosI * cosI);
    const float cosIx = glm::dot(tjN, Vx);
    const float sinIx = SQRT(1 - cosIx * cosIx);
    const float cosIy = glm::dot(tjN, Vy);
    const float sinIy = SQRT(1 - cosIy * cosIy);
    const float sinO = MAX(0.f, MIN(1.f, sinI * nIOR));
    const float cosO = SQRT(1.f - sinO * sinO);
    const float sinOx = MAX(0.f, MIN(1.f, sinIx * nIOR));
    const float cosOx = SQRT(1.f - sinOx * sinOx);
    const float sinOy = MAX(0.f, MIN(1.f, sinIy * nIOR));
    const float cosOy = SQRT(1.f - sinOy * sinOy);

    // ray directions
    tDir = -X * sinO - Y * cosO;
    txDir = -X * sinOx - Y * cosOx;
    tyDir = -X * sinOy - Y * cosOy;
    rDir = 2.f * rjN * (glm::dot(rjN, V)) - V;
    rxDir = 2.f * rjN * (glm::dot(rjN, Vx)) - Vx;
    ryDir = 2.f * rjN * (glm::dot(rjN, Vy)) - Vy;

    // loop early termination
    if (refractionGlossiness > glossy_threshold ||
        reflectionGlossiness > glossy_threshold) { break; }
  } while ((dot(tDir, Y) > refraction_angle_threshold) ||
      (dot(txDir, Y) > refraction_angle_threshold) ||
      (dot(tyDir, Y) > refraction_angle_threshold) ||
      (dot(rDir, Y) < -reflection_angle_threshold) ||
      (dot(rxDir, Y) < -reflection_angle_threshold) ||
      (dot(ryDir, Y) < -reflection_angle_threshold));

  // reflection and transmission coefficients  
  const float C0 = (nIOR - 1.f) * (nIOR - 1.f) / ((nIOR + 1.f) * (nIOR + 1.f));
  const float rC = C0 + (1.f - C0) * POW(1.f - ABS(cosI), 5.f);
  const float tC = 1.f - rC;
  assert(rC <= 1.f);
  assert(tC <= 1.f);

  // reflection and transmission colors
  const bool totReflection = (nIOR * sinI) > total_reflection_threshold;
  const Color3f sampleRefraction =
      hInfo.c.hasTexture ?
      refraction.Sample(hInfo.c.uvw, hInfo.c.duvw) : refraction.GetColor();
  const Color3f sampleReflection =
      hInfo.c.hasTexture ?
      reflection.Sample(hInfo.c.uvw, hInfo.c.duvw) : reflection.GetColor();
  const auto tK = totReflection ? Color3f(0.f) : sampleRefraction * tC;
  const auto rK = totReflection ?
                  (sampleReflection + sampleRefraction) :
                  (sampleReflection + sampleRefraction * rC);

  //!--- refraction ---
  if (bounceCount > 0 &&
      (tK.x > refraction_color_threshold ||
          tK.y > refraction_color_threshold ||
          tK.z > refraction_color_threshold)) {
    DiffRay tRay(p, tDir, px, txDir, py, tyDir);
    DiffHitInfo tHit;
    tHit.c.z = BIGFLOAT;
    tRay.Normalize();
    if (scene.TraceNodeNormal(scene.rootNode, tRay, tHit)) {
      const auto K = tK * (tHit.c.hasFrontHit ?
                           Color3f(1.f) :
                           Attenuation(absorption, tHit.c.z));
      const auto *tMtl = tHit.c.node->GetMaterial();
      color += K * tMtl->Shade(tRay, tHit, lights, bounceCount - 1);
    } else {
      color += tK * scene.environment.SampleEnvironment(tRay.c.dir);
    }
  }

  //!--- reflection ---
  if (bounceCount > 0 &&
      (rK.x > reflection_color_threshold ||
          rK.y > reflection_color_threshold ||
          rK.z > reflection_color_threshold)) {
    DiffRay rRay(p, rDir, px, rxDir, py, ryDir);
    DiffHitInfo rHit;
    rRay.Normalize();
    rHit.c.z = BIGFLOAT;
    if (scene.TraceNodeNormal(scene.rootNode, rRay, rHit)) {
      const auto K = rK * (rHit.c.hasFrontHit ?
                           Color3f(1.f) :
                           Attenuation(absorption, rHit.c.z));
      const auto *rMtl = rHit.c.node->GetMaterial();
      color += K * rMtl->Shade(rRay, rHit, lights, bounceCount - 1);
    } else {
      color += rK * scene.environment.SampleEnvironment(rRay.c.dir);
    }
  }

  //!--- normal shading ---
  const Color3f sampleDiffuse =
      hInfo.c.hasTexture ?
      diffuse.Sample(hInfo.c.uvw, hInfo.c.duvw) :
      diffuse.GetColor();
  const Color3f sampleSpecular =
      hInfo.c.hasTexture ?
      specular.Sample(hInfo.c.uvw, hInfo.c.duvw) :
      specular.GetColor();
  if (hInfo.c.hasFrontHit) {
    for (auto &light : lights) {
      auto Intensity = light->Illuminate(p, N);
      if (light->IsAmbient()) {
        color += sampleDiffuse * Intensity;
      } else {
        auto L = glm::normalize(-light->Direction(p));
        auto H = glm::normalize(V + L);
        auto cosNL = MAX(0.f, dot(N, L));
        auto cosNH = MAX(0.f, dot(N, H));
        color += sampleDiffuse * Intensity * cosNL;
        color += sampleSpecular * Intensity * POW(cosNH, glossiness) * cosNL;
      }
    }
  }
  return color;

}

//------------------------------------------------------------------------------
