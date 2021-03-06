
#include <spcOpenGL/gl4/pipeline/forward/gl4forwardpipeline.hh>
#include <spcOpenGL/gl4/gl4device.hh>
#include <spcOpenGL/gl4/gl4directionallight.hh>
#include <spcOpenGL/gl4/gl4pointlight.hh>
#include <spcOpenGL/gl4/gl4rendertargetcube.hh>
#include <spcOpenGL/glerror.hh>
#include <spcCore/graphics/camera.hh>
#include <spcCore/graphics/projector.hh>
#include <spcCore/graphics/scene/gfxscene.hh>
#include <spcCore/graphics/irendertarget2d.hh>
#include <spcCore/graphics/irendertargetcube.hh>
#include <spcCore/graphics/isampler.hh>
#include <spcCore/math/clipper/boxclipper.hh>
#include <spcCore/math/clipper/multiplaneclipper.hh>
#include <spcCore/math/clipper/sphereclipper.hh>
#include <spcCore/settings.hh>
#include <algorithm>
#include <GL/glew.h>

namespace spc::opengl
{

const int MaxLights = 4;
const float MinLightInfluence = 0.0f;

GL4ForwardPipeline::GL4ForwardPipeline()
    : m_frame(0)
      , m_device(nullptr)
      , m_scene(nullptr)
      , m_target(nullptr)
{
  SPC_CLASS_GEN_CONSTR;

}

void GL4ForwardPipeline::Initialize()
{
  Settings settings(ResourceLocator("file:///config/graphics.spc"));
  m_pointLightRenderer.Initialize(settings);
  m_directionalLightRenderer.Initialize(settings);

}

GL4ForwardPipeline::~GL4ForwardPipeline() noexcept
{

}


void GL4ForwardPipeline::Render(iRenderTarget2D *target, const Camera &camera, const Projector &projector, iDevice *device, GfxScene *scene)
{
  SPC_GL_ERROR();
  ++m_frame;
  m_device = device;
  m_camera = camera;
  m_projector = projector;
  m_scene = scene;
  m_target = target;

  m_pointLightRenderer.SetDevice(device);
  m_pointLightRenderer.SetScene(scene);
  m_directionalLightRenderer.SetDevice(device);
  m_directionalLightRenderer.SetScene(scene);


//  BoxClipper clipper(Vector3f(-1000.0f, -1000.0f, -1000.0f), Vector3f(1000.0f, 1000.0f, 1000.0f));


  MultiPlaneClipper clipper(camera, projector);

  m_pointLightRenderer.Clear();
  m_directionalLightRenderer.Clear();
  SPC_GL_ERROR();


  // get all global lights from the scene....
  // global lights are always along the final renderlights 
  const GfxLight *finalRenderLights[MaxLights] = {};
  Size finalRenderLightOffset = 0;
  scene->ScanLights(&clipper, GfxScene::eSM_Global,
                    [this, &finalRenderLights, &finalRenderLightOffset](GfxLight *light) {

                      if (finalRenderLightOffset >= MaxLights)
                      {
                        return false;
                      }
                      finalRenderLights[finalRenderLightOffset++] = light;
                      CollectShadowLights(light);
                      return true;
                    });

  SPC_GL_ERROR();

  //
  // collect the "normal" static and dynamic lights
  m_dynamicLights.clear();
  m_staticLights.clear();
  m_staticLightsNew.clear();
  scene->ScanLights(&clipper, GfxScene::eSM_Dynamic | GfxScene::eSM_Static,
                    [this](GfxLight *light) {
                      LightScanned(light);
                      CollectShadowLights(light);
                      return true;
                    });

  SPC_GL_ERROR();

  //
  // Render up to MasLights shadow maps
  RenderShadowMaps();

  camera.Bind(device);
  projector.Bind(device);

  SPC_GL_ERROR();

  // 
  // and finaly render all visible objects
  device->SetRenderTarget(m_target);
  device->Clear(true, spc::Color4f(0.0f, 0.0, 0.0, 1.0f), true, 1.0f, true, 0);
  //printf("Render - start\n");
  scene->ScanMeshes(&clipper, GfxScene::eSM_Dynamic | GfxScene::eSM_Static,
                    [this, &finalRenderLights, &finalRenderLightOffset](GfxMesh *mesh) {
                      RenderMesh(mesh, finalRenderLights, finalRenderLightOffset);
                    }
  );
  //printf("Render - done\n");

  SPC_GL_ERROR();


  // for debuging purpose
  iTexture2DArray *debugColor = m_directionalLightRenderer.GetColorTexture();
  if (debugColor && false)
  {
    Size size = target->GetWidth() / 3;


    m_device->SetViewport(0, 0, static_cast<UInt16>(size), static_cast<UInt16>(size));
    m_device->RenderFullscreen(debugColor, 0);

    /*
    m_device->SetViewport(size, 0, size, size);
    m_device->RenderFullscreen(debugColor, 1);

    m_device->SetViewport(size * 2, 0, size, size);
    m_device->RenderFullscreen(debugColor, 2);
    */


  }


  //scene->Render(device, spc::eRP_Forward);
  m_device = nullptr;
  m_camera = camera;
  m_projector = projector;
  m_scene = nullptr;
  m_target = nullptr;

  SPC_GL_ERROR();

}


void GL4ForwardPipeline::LightScanned(GfxLight *light)
{
  if (light->IsStatic())
  {
    m_staticLights.push_back(light);
    if (light->GetFrame() == 0)
    {
      m_staticLightsNew.push_back(light);
      light->SetFrame(m_frame);
    }
  }
  else
  {
    m_dynamicLights.push_back(light);
  }
}


void GL4ForwardPipeline::RenderMesh(GfxMesh *mesh, const GfxLight **lights, Size offset)
{
  // printf("  RenderMesh - start\n");
  if (mesh->IsStatic())
  {
    if (mesh->IsLightingDirty())
    {
      mesh->ClearLights();
      AppendLights(mesh, m_staticLights);
      mesh->SetLightingDirty(false);
    }
    else
    {
      AppendLights(mesh, m_staticLightsNew);
    }

    auto dynamicLights = CalcMeshLightInfluences(mesh, m_dynamicLights, true);
    Size numLights = AssignLights(
        mesh->GetLights(),
        dynamicLights,
        lights,
        offset);
    SPC_GL_ERROR();
    mesh->RenderForward(m_device, eRP_Forward, lights, numLights);
    SPC_GL_ERROR();
  }
  else
  {
    auto staticLights = CalcMeshLightInfluences(mesh, m_staticLights, true);
    auto dynamicLights = CalcMeshLightInfluences(mesh, m_dynamicLights, true);
    Size numLights = AssignLights(
        staticLights,
        dynamicLights,
        lights,
        offset);
    SPC_GL_ERROR();
    mesh->RenderForward(m_device, eRP_Forward, lights, numLights);
    SPC_GL_ERROR();
  }

  //printf("  RenderMesh - done\n");
}

void GL4ForwardPipeline::CollectShadowLights(GfxLight *light)
{
  if (!light)
  {
    return;
  }
  iLight *lght = light->GetLight();
  if (!lght->IsCastShadow())
  {
    return;
  }
  switch (lght->GetType())
  {
    case eLT_Point:
    {
      GL4PointLight *pointLight = static_cast<GL4PointLight *>(light->GetLight());
      m_pointLightRenderer.Add(pointLight);
    }
      break;
    case eLT_Directional:
    {
      GL4DirectionalLight *directionalLight = static_cast<GL4DirectionalLight *>(light->GetLight());
      m_directionalLightRenderer.Add(directionalLight);
    }
      break;
  }
}


void GL4ForwardPipeline::RenderShadowMaps()
{
  m_device->ClearShadowMaps();

  Size i = m_directionalLightRenderer.RenderShadowMaps(MaxLights, m_camera, m_projector);
  m_pointLightRenderer.RenderShadowMaps(MaxLights - i);

}


Size GL4ForwardPipeline::AssignLights(
    const std::vector<GfxMesh::Light> &static_lights,
    const std::vector<GfxMesh::Light> &dynamic_lights,
    const GfxLight **lights,
    Size offset)
{
  for (Size s = 0, d = 0, sn = static_lights.size(), dn = dynamic_lights.size();
       offset < MaxLights && (s < sn || d < dn); offset++)
  {
    if (s < sn && d < dn)
    {
      if (static_lights[s].Influence >= dynamic_lights[d].Influence)
      {
        lights[offset] = static_lights[s++].Light;
      }
      else
      {
        lights[offset] = dynamic_lights[d++].Light;
      }
    }
    else if (s < sn)
    {
      lights[offset] = static_lights[s++].Light;
    }
    else if (d < dn)
    {
      lights[offset] = dynamic_lights[d++].Light;
    }
    else
    {
      break;
    }
  }

  return offset;
}


float GL4ForwardPipeline::CalcMeshLightInfluence(const GfxLight *light, const GfxMesh *mesh) const
{
  if (!light || !mesh)
  {
    return 0.0f;
  }

  float halfSize = mesh->GetMesh()->GetBoundingBox().GetDiagonal() / 2.0f;
  // TODO: Take the power of the light from the light ... currently there is no power in the light
  float lightPower = 1.0f;
  float lightDistanceFactor = 0.0f;
  switch (light->GetLight()->GetType())
  {
    case eLT_Directional:
      lightDistanceFactor = 1.0f;
      break;
    case eLT_Point:
      auto pointLight = light->GetLight()->Query<iPointLight>();
      Vector3f lightPos = pointLight->GetPosition();
      Vector3f meshPos = mesh->GetModelMatrix().GetTranslation();
      Vector3f delta = lightPos - meshPos;
      float distance = delta.Length();
      float overlap = pointLight->GetRange() + halfSize - distance;
      if (overlap > 0.0f)
      {
        lightDistanceFactor = overlap / pointLight->GetRange();
      }
      break;
  }
  return lightPower * lightDistanceFactor;
}


std::vector<GfxMesh::Light> GL4ForwardPipeline::CalcMeshLightInfluences(const GfxMesh *mesh, const std::vector<GfxLight *> &lights, bool sorted) const
{
  std::vector<GfxMesh::Light> influences;
  for (GfxLight *light : lights)
  {
    float influence = CalcMeshLightInfluence(light, mesh);
    if (influence <= MinLightInfluence)
    {
      continue;
    }
    GfxMesh::Light l{};
    l.Light = light;
    l.Influence = influence;
    influences.push_back(l);
  }

  if (sorted)
  {
    std::sort(influences.begin(), influences.end(), [](GfxMesh::Light &l0, GfxMesh::Light &l1) { return l0.Influence > l1.Influence; });
  }


  return influences;
}

void GL4ForwardPipeline::AppendLights(GfxMesh *mesh, const std::vector<GfxLight *> &lights) const
{
  if (lights.empty())
  {
    return;
  }

  auto meshLights = CalcMeshLightInfluences(mesh, lights, false);
  for (auto &meshLight : meshLights)
  {
    mesh->AddLight(meshLight.Light, meshLight.Influence);
  }
  mesh->SortAndLimitLights(MaxLights);
}

}