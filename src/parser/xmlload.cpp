//-----------------------------------------------------------------------------
///
/// \file       xmlload.cpp 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    11.0
/// \date       November 6, 2017
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-----------------------------------------------------------------------------

#include "xmlload.h"

#include "scene/scene.h"
#include "lights/lights.h"
#include "objects/objects.h"
#include "materials/materials.h"
#include "textures/texture.h"

#include <tinyxml/tinyxml.h>
#include <tiny_obj_loader.h>

//-----------------------------------------------------------------------------

#ifdef WIN32
#define COMPARE(a,b) (_stricmp(a,b)==0)
#else
#define COMPARE(a, b) (strcasecmp(a,b)==0)
#endif

bool silentmode = false;
#define PRINTF if(!silentmode) printf

//-----------------------------------------------------------------------------

void LoadScene(TiXmlElement *element);

void LoadNode(Node *node, TiXmlElement *element, int level = 0);

void LoadTransform(Transformation *trans, TiXmlElement *element, int level);

void LoadMaterial(TiXmlElement *element);

void LoadLight(TiXmlElement *element);

void ReadVector(TiXmlElement *element, Point3 &v);

void ReadColor(TiXmlElement *element, Color3f &c);

void ReadFloat(TiXmlElement *element, float &f, const char *name = "value");

TextureMap *ReadTexture(TiXmlElement *element);

Texture *ReadTexture(const char *filename);

//-----------------------------------------------------------------------------

void LoadSceneInSilentMode(bool flag) { silentmode = flag; }

//-----------------------------------------------------------------------------

struct NodeMtl {
  Node *node;
  const char *mtlName;
};

std::vector<NodeMtl> nodeMtlList;

//-----------------------------------------------------------------------------

int LoadScene(const char *filename)
{
  TiXmlDocument doc(filename);
  if (!doc.LoadFile()) {
    PRINTF("Failed to load the file \"%s\"\n", filename);
    return 0;
  }

  TiXmlElement *xml = doc.FirstChildElement("xml");
  if (!xml) {
    PRINTF("No \"xml\" tag found.\n");
    return 0;
  }

  TiXmlElement *scene = xml->FirstChildElement("scene");
  if (!scene) {
    PRINTF("No \"scene\" tag found.\n");
    return 0;
  }

  TiXmlElement *cam = xml->FirstChildElement("camera");
  if (!cam) {
    PRINTF("No \"camera\" tag found.\n");
    return 0;
  }

  nodeMtlList.clear();
  qaray::scene.rootNode.Init();
  qaray::scene.materials.DeleteAll();
  qaray::scene.lights.DeleteAll();
  qaray::scene.objList.Clear();
  qaray::scene.textureList.Clear();
  LoadScene(scene);

  qaray::scene.rootNode.ComputeChildBoundBox();

  // Assign materials
  int numNodes = nodeMtlList.size();
  for (int i = 0; i < numNodes; i++) {
    Material *mtl = qaray::scene.materials.Find(nodeMtlList[i].mtlName);
    if (mtl) nodeMtlList[i].node->SetMaterial(mtl);
  }
  nodeMtlList.clear();

  // Load Camera
  qaray::scene.camera.Init();
  qaray::scene.camera.dir += qaray::scene.camera.pos;
  TiXmlElement *camChild = cam->FirstChildElement();
  while (camChild) {
    if (COMPARE(camChild->Value(), "position"))
      ReadVector(camChild,
                 qaray::scene.camera.pos);
    else if (COMPARE(camChild->Value(), "target"))
      ReadVector(camChild,
                 qaray::scene.camera.dir);
    else if (COMPARE(camChild->Value(), "up"))
      ReadVector(camChild, qaray::scene.camera.up);
    else if (COMPARE(camChild->Value(), "fov"))
      ReadFloat(camChild, qaray::scene.camera.fovy);
    else if (COMPARE(camChild->Value(), "focaldist"))
      ReadFloat(camChild, qaray::scene.camera.focalDistance);
    else if (COMPARE(camChild->Value(), "dof"))
      ReadFloat(camChild, qaray::scene.camera.depthOfField);
    else if (COMPARE(camChild->Value(), "width"))
      camChild->QueryIntAttribute("value", &qaray::scene.camera.imgWidth);
    else if (COMPARE(camChild->Value(), "height"))
      camChild->QueryIntAttribute("value", &qaray::scene.camera.imgHeight);
    camChild = camChild->NextSiblingElement();
  }
  qaray::scene.camera.dir -= qaray::scene.camera.pos;
  qaray::scene.camera.dir = glm::normalize(qaray::scene.camera.dir);
  Point3 x = glm::cross(qaray::scene.camera.dir, qaray::scene.camera.up);
  qaray::scene.camera.up =
      glm::normalize(glm::cross(x, qaray::scene.camera.dir));

  // renderImage.Init( camera.imgWidth, camera.imgHeight );

  return 1;
}

//-----------------------------------------------------------------------------

void PrintIndent(int level)
{
  for (int i = 0; i < level; i++) PRINTF("   ");
}

//-----------------------------------------------------------------------------

void LoadScene(TiXmlElement *element)
{
  for (TiXmlElement *child = element->FirstChildElement();
       child != NULL; child = child->NextSiblingElement()) {

    if (COMPARE(child->Value(), "background")) {
      Color3f c(1, 1, 1);
      ReadColor(child, c);
      qaray::scene.background.SetColor(c);
      PRINTF("Background %f %f %f\n", c.r, c.g, c.b);
      qaray::scene.background.SetTexture(ReadTexture(child));
    } else if (COMPARE(child->Value(), "environment")) {
      Color3f c(1, 1, 1);
      ReadColor(child, c);
      qaray::scene.environment.SetColor(c);
      PRINTF("Environment %f %f %f\n", c.r, c.g, c.b);
      qaray::scene.environment.SetTexture(ReadTexture(child));
    } else if (COMPARE(child->Value(), "object")) {
      LoadNode(&qaray::scene.rootNode, child);
    } else if (COMPARE(child->Value(), "material")) {
      LoadMaterial(child);
    } else if (COMPARE(child->Value(), "light")) {
      LoadLight(child);
    }
  }
}

//-----------------------------------------------------------------------------
void LoadNode(Node *parent, TiXmlElement *element, int level)
{
  Node *node = new Node;
  parent->AppendChild(node);

  // name
  const char *name = element->Attribute("name");
  node->SetName(name);
  PrintIndent(level);
  PRINTF("object [");
  if (name) PRINTF("%s", name);
  PRINTF("]");

  // material
  const char *mtlName = element->Attribute("material");
  if (mtlName) {
    PRINTF(" <%s>", mtlName);
    NodeMtl nm;
    nm.node = node;
    nm.mtlName = mtlName;
    nodeMtlList.push_back(nm);
  }

  // type
  const char *type = element->Attribute("type");
  if (type) {
    if (COMPARE(type, "sphere")) {
      node->SetNodeObj(&theSphere);
      PRINTF(" - Sphere");
    } else if (COMPARE(type, "plane")) {
      node->SetNodeObj(&thePlane);
      PRINTF(" - Plane");
    } else if (COMPARE(type, "obj")) {
      PRINTF(" - OBJ");
      Object *obj = qaray::scene.objList.Find(name);
      if (obj == NULL) {// object is not on the list, so we should load it now
        TriObj *tobj = new TriObj;
        if (!tobj->Load(name, mtlName == NULL)) {
          PRINTF(" -- ERROR: Cannot load file \"%s.\"", name);
          delete tobj;
        } else {
          qaray::scene.objList.Append(tobj, name);// add to the list
          obj = tobj;
          // generate multi-material
          if (mtlName == NULL && tobj->NM() > 0) {
            if (qaray::scene.materials.Find(name) == NULL) {
              PRINTF("\n");
              PrintIndent(level);
              PRINTF(" - OBJ Multi-Material\n");
              MultiMtl *mm = new MultiMtl;
              for (unsigned int i = 0; i < tobj->NM(); i++) {
                MtlBlinn *m = new MtlBlinn;
                const auto &mtl = tobj->M(i);
                m->SetDiffuse(Color3f(mtl.diffuse[0], mtl.diffuse[1], mtl.diffuse[2]));
                m->SetSpecular(Color3f(mtl.specular[0], mtl.specular[1], mtl.specular[2]));
                m->SetGlossiness(mtl.shininess);
                m->SetRefractionIndex(mtl.ior);
                if (!mtl.diffuse_texname.empty()) {
                  auto* tex = new TextureMap(ReadTexture((tobj->GetDirectoryName() + mtl.diffuse_texname).c_str()));
                  m->SetDiffuseTexture(tex);
                }
                if (!mtl.specular_texname.empty()) {
                  auto* tex = new TextureMap(ReadTexture((tobj->GetDirectoryName() + mtl.specular_texname).c_str()));
                  m->SetDiffuseTexture(tex);
                }
                if (mtl.illum > 2 && mtl.illum <= 7) {
                  m->SetReflection(Color3f(mtl.specular[0], mtl.specular[1], mtl.specular[2]));
                  if (!mtl.specular_texname.empty()) {
                    auto *tex = new TextureMap(ReadTexture((tobj->GetDirectoryName() + mtl.specular_texname).c_str()));
                    m->SetReflectionTexture(tex);
                  }
                  float gloss = acosf(powf(2, 1 / mtl.shininess));
                  if (mtl.illum >= 6) {
                    m->SetRefraction(1.f - Color3f(mtl.transmittance[0], mtl.transmittance[1], mtl.transmittance[2]));
                  }
                }
                mm->AppendMaterial(m);
              }
              mm->SetName(name);
              qaray::scene.materials.push_back(mm);
              NodeMtl nm;
              nm.node = node;
              nm.mtlName = name;
              nodeMtlList.push_back(nm);
            }
          }
        }
      }
      node->SetNodeObj(obj);
    } else {
      PRINTF(" - UNKNOWN TYPE");
    }
  }
  PRINTF("\n");
  for (TiXmlElement *child = element->FirstChildElement();
       child != NULL; child = child->NextSiblingElement()) {
    if (COMPARE(child->Value(), "object")) {
      LoadNode(node, child, level + 1);
    }
  }
  LoadTransform(node, element, level);
}

//-----------------------------------------------------------------------------

void LoadTransform(Transformation *trans, TiXmlElement *element, int level)
{
  for (TiXmlElement *child = element->FirstChildElement();
       child != NULL; child = child->NextSiblingElement()) {
    if (COMPARE(child->Value(), "scale")) {
      Point3 s(1, 1, 1);
      ReadVector(child, s);
      trans->Scale(s.x, s.y, s.z);
      PrintIndent(level);
      PRINTF("   scale %f %f %f\n", s.x, s.y, s.z);
    } else if (COMPARE(child->Value(), "rotate")) {
      Point3 s(0, 0, 0);
      ReadVector(child, s);
      s = glm::normalize(s);
      float a;
      ReadFloat(child, a, "angle");
      trans->Rotate(s, a);
      PrintIndent(level);
      PRINTF("   rotate %f degrees around %f %f %f\n", a, s.x, s.y, s.z);
    } else if (COMPARE(child->Value(), "translate")) {
      Point3 t(0, 0, 0);
      ReadVector(child, t);
      trans->Translate(t);
      PrintIndent(level);
      PRINTF("   translate %f %f %f\n", t.x, t.y, t.z);
    }
  }
}

//-----------------------------------------------------------------------------

void LoadMaterial(TiXmlElement *element)
{
  Material *mtl = NULL;

  // name
  const char *name = element->Attribute("name");
  PRINTF("Material [");
  if (name) PRINTF("%s", name);
  PRINTF("]");

  // type
  const char *type = element->Attribute("type");
  if (type) {
    if (COMPARE(type, "blinn")) {
      PRINTF(" - Blinn\n");
      MtlBlinn *m = new MtlBlinn();
      mtl = m;
      for (TiXmlElement *child = element->FirstChildElement();
           child != NULL; child = child->NextSiblingElement()) {
        Color3f c(1, 1, 1);
        float f = 1;
        if (COMPARE(child->Value(), "diffuse")) {
          ReadColor(child, c);
          m->SetDiffuse(c);
          PRINTF("   diffuse %f %f %f\n", c.r, c.g, c.b);
          m->SetDiffuseTexture(ReadTexture(child));
        } else if (COMPARE(child->Value(), "specular")) {
          ReadColor(child, c);
          m->SetSpecular(c);
          PRINTF("   specular %f %f %f\n", c.r, c.g, c.b);
          m->SetSpecularTexture(ReadTexture(child));
        } else if (COMPARE(child->Value(), "glossiness")) {
          ReadFloat(child, f);
          m->SetGlossiness(f);
          PRINTF("   glossiness %f\n", f);
        } else if (COMPARE(child->Value(), "emission")) {
          ReadColor(child, c);
          m->SetEmission(c);
          PRINTF("   emission %f %f %f\n", c.r, c.g, c.b);
          m->SetEmissionTexture(ReadTexture(child));
        } else if (COMPARE(child->Value(), "reflection")) {
          ReadColor(child, c);
          m->SetReflection(c);
          PRINTF("   reflection %f %f %f\n", c.r, c.g, c.b);
          m->SetReflectionTexture(ReadTexture(child));
          f = 0;
          ReadFloat(child, f, "glossiness");
          m->SetReflectionGlossiness(f);
          if (f > 0) PRINTF(" (glossiness %f)\n", f);
        } else if (COMPARE(child->Value(), "refraction")) {
          ReadColor(child, c);
          m->SetRefraction(c);
          ReadFloat(child, f, "index");
          m->SetRefractionIndex(f);
          PRINTF("   refraction %f %f %f (index %f)\n", c.r, c.g, c.b, f);
          m->SetRefractionTexture(ReadTexture(child));
          f = 0;
          ReadFloat(child, f, "glossiness");
          m->SetRefractionGlossiness(f);
          if (f > 0) PRINTF(" (glossiness %f)\n", f);
        } else if (COMPARE(child->Value(), "absorption")) {
          ReadColor(child, c);
          m->SetAbsorption(c);
          PRINTF("   absorption %f %f %f\n", c.r, c.g, c.b);
        }
      }
    } else {
      PRINTF(" - UNKNOWN\n");
    }
  }

  if (mtl) {
    mtl->SetName(name);
    qaray::scene.materials.push_back(mtl);
  }
}

//-----------------------------------------------------------------------------

void LoadLight(TiXmlElement *element)
{
  Light *light = NULL;

  // name
  const char *name = element->Attribute("name");
  PRINTF("Light [");
  if (name) PRINTF("%s", name);
  PRINTF("]");

  // type
  const char *type = element->Attribute("type");
  if (type) {
    if (COMPARE(type, "ambient")) {
      PRINTF(" - Ambient\n");
      AmbientLight *l = new AmbientLight();
      light = l;
      for (TiXmlElement *child = element->FirstChildElement();
           child != NULL; child = child->NextSiblingElement()) {
        if (COMPARE(child->Value(), "intensity")) {
          Color3f c(1, 1, 1);
          ReadColor(child, c);
          l->SetIntensity(c);
          PRINTF("   intensity %f %f %f\n", c.r, c.g, c.b);
        }
      }
    } else if (COMPARE(type, "direct")) {
      PRINTF(" - Direct\n");
      DirectLight *l = new DirectLight();
      light = l;
      for (TiXmlElement *child = element->FirstChildElement();
           child != NULL; child = child->NextSiblingElement()) {
        if (COMPARE(child->Value(), "intensity")) {
          Color3f c(1, 1, 1);
          ReadColor(child, c);
          l->SetIntensity(c);
          PRINTF("   intensity %f %f %f\n", c.r, c.g, c.b);
        } else if (COMPARE(child->Value(), "direction")) {
          Point3 v(1, 1, 1);
          ReadVector(child, v);
          l->SetDirection(v);
          PRINTF("   direction %f %f %f\n", v.x, v.y, v.z);
        }
      }
    } else if (COMPARE(type, "point")) {
      PRINTF(" - Point\n");
      PointLight *l = new PointLight();
      light = l;
      for (TiXmlElement *child = element->FirstChildElement();
           child != NULL; child = child->NextSiblingElement()) {
        if (COMPARE(child->Value(), "intensity")) {
          Color3f c(1, 1, 1);
          ReadColor(child, c);
          l->SetIntensity(c);
          PRINTF("   intensity %f %f %f\n", c.r, c.g, c.b);
        } else if (COMPARE(child->Value(), "position")) {
          Point3 v(0, 0, 0);
          ReadVector(child, v);
          l->SetPosition(v);
          PRINTF("   position %f %f %f\n", v.x, v.y, v.z);
        } else if (COMPARE(child->Value(), "size")) {
          float f = 0;
          ReadFloat(child, f);
          l->SetSize(f);
          PRINTF("   size %f\n", f);
        }
      }
    }
    else if (COMPARE(type, "spot")) {
      PRINTF(" - Spot\n");
      SpotLight *l = new SpotLight();
      light = l;
      for (TiXmlElement *child = element->FirstChildElement();
           child != NULL; child = child->NextSiblingElement()) {
        if (COMPARE(child->Value(), "intensity")) {
          Color3f c(1, 1, 1);
          ReadColor(child, c);
//          float f = 0;
//          ReadFloat(child, f, "strength");
          l->SetIntensity(c);
          PRINTF("   intensity %f %f %f\n", c.r, c.g, c.b);
        } else if (COMPARE(child->Value(), "position")) {
          Point3 v(0, 0, 0);
          ReadVector(child, v);
          l->SetPosition(v);
          PRINTF("   position %f %f %f\n", v.x, v.y, v.z);
        } else if (COMPARE(child->Value(), "size")) {
          float f = 0;
          ReadFloat(child, f);
          l->SetSize(f);
          PRINTF("   size %f\n", f);
        } else if (COMPARE(child->Value(), "rotation")) {
          Point3 v(0, 0, 0);
          float f = 0;
          ReadVector(child, v);
          ReadFloat(child, f, "angle");
          l->SetRotation(f, v);
          PRINTF("   rotation axis %f %f %f angle %f\n", v.x, v.y, v.z, f);
        } else if (COMPARE(child->Value(), "angle")) {
          float f = 0;
          ReadFloat(child, f);
          l->SetAngle(f);
          PRINTF("   angle %f\n", f);
        } else if (COMPARE(child->Value(), "blend")) {
          float f = 0;
          ReadFloat(child, f);
          l->SetBlend(f);
          PRINTF("   blend %f\n", f);
        }
      }
    } else {
      PRINTF(" - UNKNOWN\n");
    }
  }

  if (light) {
    light->SetName(name);
    qaray::scene.lights.push_back(light);
  }

}

//-----------------------------------------------------------------------------

void ReadVector(TiXmlElement *element, Point3 &v)
{
  double x = (double) v.x;
  double y = (double) v.y;
  double z = (double) v.z;
  element->QueryDoubleAttribute("x", &x);
  element->QueryDoubleAttribute("y", &y);
  element->QueryDoubleAttribute("z", &z);
  v.x = (float) x;
  v.y = (float) y;
  v.z = (float) z;

  float f = 1;
  ReadFloat(element, f);
  v *= f;
}

//-----------------------------------------------------------------------------

void ReadColor(TiXmlElement *element, Color3f &c)
{
  double r = (double) c.r;
  double g = (double) c.g;
  double b = (double) c.b;
  element->QueryDoubleAttribute("r", &r);
  element->QueryDoubleAttribute("g", &g);
  element->QueryDoubleAttribute("b", &b);
  c.r = (float) r;
  c.g = (float) g;
  c.b = (float) b;

  float f = 1;
  ReadFloat(element, f);
  c *= f;
}

//-----------------------------------------------------------------------------

void ReadFloat(TiXmlElement *element, float &f, const char *name)
{
  double d = (double) f;
  element->QueryDoubleAttribute(name, &d);
  f = (float) d;
}

//-----------------------------------------------------------------------------
//-------------------------------------------------------------------------------

TextureMap *ReadTexture(TiXmlElement *element)
{
  const char *texName = element->Attribute("texture");
  if (texName == NULL) return NULL;

  Texture *tex = NULL;
  if (COMPARE(texName, "checkerboard")) {
    TextureChecker *ctex = new TextureChecker;
    tex = ctex;
    PRINTF("      Texture: Checker Board\n");
    for (TiXmlElement *child = element->FirstChildElement();
         child != NULL; child = child->NextSiblingElement()) {
      if (COMPARE(child->Value(), "color1")) {
        Color3f c(0, 0, 0);
        ReadColor(child, c);
        ctex->SetColor1(c);
        PRINTF("         color1 %f %f %f\n", c.r, c.g, c.b);
      } else if (COMPARE(child->Value(), "color2")) {
        Color3f c(0, 0, 0);
        ReadColor(child, c);
        ctex->SetColor2(c);
        PRINTF("         color2 %f %f %f\n", c.r, c.g, c.b);
      }
    }
    qaray::scene.textureList.Append(tex, texName);
  } else {
    tex = ReadTexture(texName);
  }

  TextureMap *map = new TextureMap(tex);
  LoadTransform(map, element, 1);
  return map;
}

//-------------------------------------------------------------------------------

Texture *ReadTexture(const char *texName)
{
  PRINTF("      Texture: File \"%s\"", texName);
  Texture *tex = qaray::scene.textureList.Find(texName);
  if (tex == NULL) {
    TextureFile *ftex = new TextureFile;
    tex = ftex;
    ftex->SetName(texName);
    if (!ftex->Load()) {
      PRINTF(" -- Error loading file!");
      delete tex;
      tex = NULL;
    } else {
      qaray::scene.textureList.Append(tex, texName);
    }
  }
  PRINTF("\n");

  return tex;
}

//-------------------------------------------------------------------------------
