#include "globalvar.h"
#include "MtlBlinn_MonteCarloGI.h"

//------------------------------------------------------------------------------

MtlBlinn_MonteCarloGI::MtlBlinn_MonteCarloGI() :
  diffuse(0.5f,0.5f,0.5f),
  specular(0.7f,0.7f,0.7f),
  glossiness(20.0f),
  emission(0,0,0),
  reflection(0,0,0),
  refraction(0,0,0),
  absorption(0,0,0),
  ior(1),
  reflectionGlossiness(0),
  refractionGlossiness(0) 
{}

//------------------------------------------------------------------------------

//static std::atomic<int> idx_halton(1);
const float glossy_threshold = 0.001f;
const float total_reflection_threshold = 1.001f;
const float refraction_angle_threshold = 0.001f;
const float reflection_angle_threshold = 0.001f;
const float refraction_color_threshold = 0.001f;
const float reflection_color_threshold = 0.001f;
const float diffuse_color_threshold    = 0.001f;

//------------------------------------------------------------------------------

Color MtlBlinn_MonteCarloGI::Shade(const DiffRay &ray,
				   const DiffHitInfo &hInfo,
				   const LightList &lights,
				   int bounceCount)
  const
{
  // input parameters
  Color color = hInfo.c.hasTexture ?
    emission.Sample(hInfo.c.uvw, hInfo.c.duvw) : emission.GetColor();
  const auto N = glm::normalize(hInfo.c.N);   // surface normal in world coordinate
  const auto V  = glm::normalize(-ray.c.dir); // ray incoming direction
  const auto Vx = glm::normalize(-ray.x.dir); // diff ray incoming direction
  const auto Vy = glm::normalize(-ray.y.dir); // diff ray incoming direction
  const auto p  = hInfo.c.p;                  // surface position in world coordinate
  const auto px = ray.x.p + ray.x.dir * hInfo.x.z;
  const auto py = ray.y.p + ray.y.dir * hInfo.y.z;

  // coordinate
  const auto Y = glm::dot(N, V) > 0.f ? N : -N;    // Vy
  const auto Z = glm::cross(V, Y);
  const auto X = glm::normalize(glm::cross(Y, Z)); // Vx

  // index of refraction
  const float nIOR = hInfo.c.hasFrontHit ? 1.f / ior : ior;
  
  // refraction and reflection
  float cosI, sinI;
  Point3 tDir, rDir, txDir, rxDir, tyDir, ryDir;
  do {    
    // jitter normal
    Point3 tjN = N, rjN = N;
    if (refractionGlossiness > glossy_threshold) 
    {
      const float r1 = rng->Get();
      const float r2 = rng->Get();
      const float r3 = rng->Get();	  
      tjN = glm::normalize(N + GetCirclePoint(r1, r2, r3, refractionGlossiness));
    }
    if (reflectionGlossiness > glossy_threshold) 
    {
      const float r1 = rng->Get();
      const float r2 = rng->Get();
      const float r3 = rng->Get();
      rjN = glm::normalize(N + GetCirclePoint(r1, r2, r3, refractionGlossiness));
    }
    
    // incidence angle & refraction angle
    cosI  = glm::dot(tjN,V);
    sinI  = SQRT(1 - cosI * cosI);
    const float cosIx = glm::dot(tjN,Vx);
    const float sinIx = SQRT(1 - cosIx * cosIx);
    const float cosIy = glm::dot(tjN,Vy);
    const float sinIy = SQRT(1 - cosIy * cosIy);
    const float sinO  = MAX(0.f, MIN(1.f, sinI  * nIOR));
    const float cosO  = SQRT(1.f - sinO * sinO);  
    const float sinOx = MAX(0.f, MIN(1.f, sinIx * nIOR));
    const float cosOx = SQRT(1.f - sinOx * sinOx);
    const float sinOy = MAX(0.f, MIN(1.f, sinIy * nIOR));
    const float cosOy = SQRT(1.f - sinOy * sinOy);

    // ray directions
    tDir  = -X * sinO  - Y * cosO;
    txDir = -X * sinOx - Y * cosOx;
    tyDir = -X * sinOy - Y * cosOy;
    rDir  = 2.f*rjN*(glm::dot(rjN,V ))-V;
    rxDir = 2.f*rjN*(glm::dot(rjN,Vx))-Vx;
    ryDir = 2.f*rjN*(glm::dot(rjN,Vy))-Vy;

    // loop early termination
    if (refractionGlossiness > glossy_threshold ||
	reflectionGlossiness > glossy_threshold)
    { break; }
  }
  while ((glm::dot(tDir, Y) >  refraction_angle_threshold) ||
	 (glm::dot(txDir,Y) >  refraction_angle_threshold) ||
	 (glm::dot(tyDir,Y) >  refraction_angle_threshold) ||
	 (glm::dot(rDir, Y) < -reflection_angle_threshold) ||
	 (glm::dot(rxDir,Y) < -reflection_angle_threshold) ||
	 (glm::dot(ryDir,Y) < -reflection_angle_threshold));
    
  // reflection and transmission coefficients  
  const float C0 = (nIOR - 1.f) * (nIOR - 1.f) / ((nIOR + 1.f) * (nIOR + 1.f));
  const float rC = C0 + (1.f - C0) * POW(1.f - ABS(cosI), 5.f);
  const float tC = 1.f - rC;
  assert(rC <= 1.f); assert(tC <= 1.f);

  // reflection and transmission colors
  const bool totReflection = (nIOR * sinI) > total_reflection_threshold;
  const Color sampleRefraction =
    hInfo.c.hasTexture ?
    refraction.Sample(hInfo.c.uvw, hInfo.c.duvw) : refraction.GetColor();
  const Color sampleReflection =
    hInfo.c.hasTexture ?
    reflection.Sample(hInfo.c.uvw, hInfo.c.duvw) : reflection.GetColor();
  const auto tK = totReflection ? Color(0.f) : sampleRefraction * tC;
  const auto rK = totReflection ?
    (sampleReflection + sampleRefraction) :
    (sampleReflection + sampleRefraction * rC);

  //!--- refraction ---
  if (bounceCount > 0 && 
      (tK.x > refraction_color_threshold ||
       tK.y > refraction_color_threshold ||
       tK.z > refraction_color_threshold)) 
  {
    DiffRay tRay(p, tDir, px, txDir, py, tyDir);
    DiffHitInfo tHit;
    tHit.c.z = BIGFLOAT;
    tRay.Normalize();
    if (TraceNodeNormal(rootNode, tRay, tHit)) {
      const auto K = tK * (tHit.c.hasFrontHit ?
                           Color(1.f) :
                           Attenuation(absorption, tHit.c.z));
      const auto* tMtl = tHit.c.node->GetMaterial();
      color += K * tMtl->Shade(tRay, tHit, lights, bounceCount - 1);
    }
    else {
      color += tK * environment.SampleEnvironment(tRay.c.dir);
    }
  }

  //!--- reflection ---
  if (bounceCount > 0 && 
      (rK.x > reflection_color_threshold ||
       rK.y > reflection_color_threshold ||
       rK.z > reflection_color_threshold))  
  {
    DiffRay rRay(p, rDir, px, rxDir, py, ryDir);
    DiffHitInfo rHit; 
    rRay.Normalize();
    rHit.c.z = BIGFLOAT;
    if (TraceNodeNormal(rootNode, rRay, rHit)) {
      const auto K = rK * (rHit.c.hasFrontHit ?
                           Color(1.f) :
                           Attenuation(absorption, rHit.c.z));
      const auto* rMtl = rHit.c.node->GetMaterial();
      color += K * rMtl->Shade(rRay, rHit, lights, bounceCount - 1);
    }
    else {
      color += rK * environment.SampleEnvironment(rRay.c.dir);
    }
  }

  //!--- normal shading ---
  const Color sampleDiffuse  =
    hInfo.c.hasTexture ?
    diffuse.Sample(hInfo.c.uvw, hInfo.c.duvw) :
    diffuse.GetColor();
  const Color sampleSpecular =
    hInfo.c.hasTexture ?
    specular.Sample(hInfo.c.uvw, hInfo.c.duvw) :
    specular.GetColor();
  const int numSampleMC =
    ((Material::maxBounce - Material::maxBounceMC - bounceCount >= 0) ?
     1 : Material::maxMCSample);
  if (hInfo.c.hasFrontHit) {
    // Directional Lights
    Color directShadecolor = Color(0.f);
    const float normCoeDI = 1.f;     
    for (auto& light : lights) {
      if (light->IsAmbient()) {
        // color += sampleDiffuse * Intensity;
      } else {
	auto Intensity = light->Illuminate(p, N) * normCoeDI;
	auto L = glm::normalize(-light->Direction(p));
	auto H = glm::normalize(V + L);
	auto cosNL = MAX(0.f, glm::dot(N,L));
	auto cosNH = MAX(0.f, glm::dot(N,H));
	directShadecolor += (sampleDiffuse * cosNL + sampleSpecular * POW(cosNH , glossiness)) * 
	  Intensity;
      }
    }
    // Monte Carlo GI
    Color indirectShadecolor = Color(0.f);
    const float normCoeGI = 0.5f / numSampleMC;
    if (bounceCount > 0)
    {
      for (int i = 0; i < numSampleMC; ++i) 
      {
        //-- Sampling Hemisphere
	//-- Method 1
        // Point3 mc_coe;
        // do {
        //   mc_coe.x = (2.f * rng->Get() - 1.f);
        //   mc_coe.y = (2.f * rng->Get() - 1.f);
        //   mc_coe.z = rng->Get();
        // } while (glm::length(mc_coe) < 0.001f || glm::length(mc_coe) > 0.999f);
	// mc_coe = glm::normalize(mc_coe);
	//
	//-- Method 2
	Point3 mc_coe = glm::normalize(CosWeightedSampleHemiSphere(rng->Get(), rng->Get()));
	//
	//-- Method 3 (the idx_halton can go out of limit)
	// Point3 mc_coe = 
	//   glm::normalize(CosWeightedSampleHemiSphere(Halton(idx_halton,2), Halton(idx_halton,3)));
	// ++idx_halton;
	
	//-- Compute local coordinate frame
	//-- Method 1
	// Point3 new_z = Y;
	// auto new_zx = ABS(glm::dot(Point3(1.f,0.f,0.f), new_z));
	// auto new_zy = ABS(glm::dot(Point3(0.f,1.f,0.f), new_z));
	// auto new_zz = ABS(glm::dot(Point3(0.f,0.f,1.f), new_z));
	// Point3 new_y = (new_zx < new_zy && new_zx < new_zz) ?
	//   glm::normalize(glm::cross(new_z, Point3(1.f,0.f,0.f))) :
	//   (new_zy < new_zz ? glm::normalize(glm::cross(new_z, Point3(0.f,1.f,0.f))) : 
	//                      glm::normalize(glm::cross(new_z, Point3(0.f,0.f,1.f))));
	// Point3 new_x = glm::normalize(glm::cross(new_y, new_z));
	//
	//-- Method 2
	Point3 new_x, new_y, new_z = N;
	if (ABS(new_z.x) > ABS(new_z.y)) { 
	  new_y = glm::normalize(Point3(new_z.z, 0, -new_z.x));
	}
	else {
	  new_y = glm::normalize(Point3(0, -new_z.z, new_z.y));
	}
	new_x = glm::normalize(glm::cross(new_y, new_z));

        // generate ray
        Point3 dirMC = mc_coe.x * new_x + mc_coe.y * new_y + mc_coe.z * new_z;
        DiffRay rayMC(p, dirMC);
        DiffHitInfo hitMC;
        hitMC.c.z = BIGFLOAT;
        rayMC.Normalize();
        Color Intensity;
        if (TraceNodeNormal(rootNode, rayMC, hitMC)) {
          Intensity =
    	    hitMC.c.node->GetMaterial()->Shade(rayMC, hitMC, lights, bounceCount - 1);
        } else {
          Intensity = environment.SampleEnvironment(rayMC.c.dir);
        }
	auto H = glm::normalize(V + dirMC);
	auto cosNL = MAX(0.f, glm::dot(N, dirMC));
	auto cosNH = MAX(0.f, glm::dot(N,H));
        indirectShadecolor += Intensity * 
	  (cosNL * sampleSpecular * POW(cosNH , glossiness) + sampleDiffuse);
      }
      indirectShadecolor *= normCoeGI;
    }
    color += (indirectShadecolor + directShadecolor);
  }
  //!--- process color ---
  color.r = LinearToSRGB(MAX(0.f, MIN(1.f, color.r)));
  color.g = LinearToSRGB(MAX(0.f, MIN(1.f, color.g)));
  color.b = LinearToSRGB(MAX(0.f, MIN(1.f, color.b)));

  if (Material::gamma != 1.f) {
    color.r = POW(color.r,1.f/Material::gamma);
    color.g = POW(color.g,1.f/Material::gamma);
    color.b = POW(color.b,1.f/Material::gamma);
  }
  return color;
}

//------------------------------------------------------------------------------