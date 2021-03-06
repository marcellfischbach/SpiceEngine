
#include <spcLauncher/window/sdlinputsystem.hh>
#include <spcLauncher/window/sdlkeyboard.hh>
#include <spcLauncher/window/sdlmouse.hh>
#include <spcLauncher/launchermodule.hh>
#include <spcCore/coremodule.hh>
#include <spcCore/settings.hh>
#include <spcCore/entity/camerastate.hh>
#include <spcCore/entity/entity.hh>
#include <spcCore/entity/entitystate.hh>
#include <spcCore/entity/lightstate.hh>
#include <spcCore/entity/spatialstate.hh>
#include <spcCore/entity/staticmeshstate.hh>
#include <spcCore/entity/world.hh>
#include <spcCore/input/input.hh>
#include <spcCore/math/math.hh>
#include <spcCore/objectregistry.hh>
#include <spcCore/graphics/camera.hh>
#include <spcCore/graphics/idevice.hh>
#include <spcCore/graphics/image.hh>
#include <spcCore/graphics/ipointlight.hh>
#include <spcCore/graphics/irendermesh.hh>
#include <spcCore/graphics/irenderpipeline.hh>
#include <spcCore/graphics/irendertarget2d.hh>
#include <spcCore/graphics/isampler.hh>
#include <spcCore/graphics/samplers.hh>
#include <spcCore/graphics/mesh.hh>
#include <spcCore/graphics/projector.hh>
#include <spcCore/graphics/shading/ishader.hh>
#include <spcCore/graphics/shading/ishaderattribute.hh>
#include <spcCore/graphics/material/material.hh>
#include <spcCore/graphics/material/materialinstance.hh>
#include <spcCore/graphics/scene/gfxscene.hh>
#include <spcCore/graphics/scene/gfxmesh.hh>
#include <spcCore/resource/assetmanager.hh>
#include <spcCore/resource/vfs.hh>
#include <spcCore/resource/resourcelocator.hh>

#include <spcAssimpLoader/assimploadermodule.hh>
#include <spcOpenGL/openglmodule.hh>
#include <GL/glew.h>

#include <spcImgLoader/imgloadermodule.hh>

#include <iostream>
#include <SDL.h>
#include <regex>
#include <string>

spc::SDLKeyboard keyboard;
spc::SDLMouse mouse;


void UpdateEvents()
{
  keyboard.Update();
  mouse.Update();
  SDL_Event evt;
  while (SDL_PollEvent(&evt))
  {
    switch (evt.type)
    {
      case SDL_KEYDOWN:
        keyboard.Update(evt.key.keysym.scancode, true);
        break;
      case SDL_KEYUP:
        keyboard.Update(evt.key.keysym.scancode, false);
        break;
      case SDL_MOUSEBUTTONDOWN:
        mouse.Update(evt.button.button, true);
        break;
      case SDL_MOUSEBUTTONUP:
        mouse.Update(evt.button.button, false);
        break;
      case SDL_MOUSEWHEEL:
        mouse.Update(evt.wheel.y, evt.wheel.x);
        break;
      case SDL_MOUSEMOTION:
        mouse.Update(evt.motion.x, evt.motion.y, evt.motion.xrel, evt.motion.yrel);
        break;

    }
  }

}

std::vector<std::string> split(const std::string &string)
{
  std::vector<std::string> res;
  size_t offset = 0;
  size_t idx = 0;
  while ((idx = string.find('\n', offset)) != std::string::npos)
  {
    std::string part = string.substr(offset, idx - offset);
    res.push_back(part);
    offset = idx + 1;
  }
  std::string part = string.substr(offset, string.length() - offset);
  res.push_back(part);

  return res;
}

std::string merge(const std::vector<std::string> &lines)
{
  std::string res;
  for (const std::string &str : lines)
  {
    res += str + "\n";
  }
  return res;
}

bool register_modules(int argc, char **argv)
{

  if (!spc::LauncherModule::Register(argc, argv))
  {
    printf("Unable to register launcher\n");
    return false;
  }
  if (!spc::CoreModule::Register(argc, argv))
  {
    printf("Unable to register core\n");
    return false;
  }
  if (!spc::opengl::OpenGLModule::Register(argc, argv))
  {
    printf("Unable to register opengl\n");
    return false;
  }
  if (!spc::assimp::AssimpLoaderModule::Register(argc, argv))
  {
    printf("Unable to register assimp loader\n");
    return false;
  }
  if (!spc::img::ImgLoaderModule::Register(argc, argv))
  {
    printf("Unable to register png loader\n");
    return false;
  }

  return true;
}

SDL_Window *wnd;
SDL_GLContext context;


bool initialize_modules(int argc, char **argv)
{
  spc::VFS::Get()->SetBasePath("D:\\DEV\\SpiceEngine\\data");
  spc::Settings settings("file:///config/display.spc");

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  Uint32 flags = SDL_WINDOW_OPENGL;
  // flags |= SDL_WINDOW_BORDERLESS;
  std::string title = settings.GetText("title");
  spc::Vector2i res = settings.GetVector2i("resolution");
  spc::Vector2i pos = settings.GetVector2i("pos");
  std::string viewMode = settings.GetText("viewmode", "windowed");
  if (viewMode == "viewMode")
  {
    flags |= SDL_WINDOW_FULLSCREEN;
  }
  else if (viewMode == "desktop")
  {
    flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  }
  else if (viewMode != "windowed")
  {
    printf("Illegal viewmode: '%s'. Defaulting to 'windowed'\n", viewMode.c_str());
  }

  bool vsync = settings.GetBool("vsync");
  wnd = SDL_CreateWindow(title.c_str(),
                         pos.x, pos.y,
                         res.x, res.y, flags);
  //  wnd = SDL_CreateWindow("Spice", 0, 0, 1920, 1080, flags);
  context = SDL_GL_CreateContext(wnd);
  SDL_GL_SetSwapInterval(vsync ? 1 : 0);

  SDL_ShowWindow(wnd);
  SDL_GL_MakeCurrent(wnd, context);

  if (!spc::CoreModule::Initialize(argc, argv))
  {
    printf("Unable to initialize core\n");
    return false;
  }
  if (!spc::opengl::OpenGLModule::Initialize(argc, argv))
  {
    printf("Unable to initialize opengl\n");
    return false;
  }
  if (!spc::assimp::AssimpLoaderModule::Initialize(argc, argv))
  {
    printf("Unable to initialize assimp loader\n");
    return false;
  }
  if (!spc::img::ImgLoaderModule::Initialize(argc, argv))
  {
    printf("Unable to initialize png loader\n");
    return false;
  }
  return true;
}

spc::iRenderMesh *create_plane_mesh(float nx, float ny)
{
  //
  // create a render mesh
  spc::iRenderMeshGenerator *generator = spc::ObjectRegistry::Get<spc::iRenderMeshGeneratorFactory>()->Create();
  std::vector<spc::Vector3f> positions;
  positions.push_back(spc::Vector3f(-40.0f, 0.0f, -40.0f));
  positions.push_back(spc::Vector3f(-40.0f, 0.0f, 40.0f));
  positions.push_back(spc::Vector3f(40.0f, 0.0f, -40.0f));
  positions.push_back(spc::Vector3f(40.0f, 0.0f, 40.0f));
  std::vector<spc::Vector3f> normals;
  normals.push_back(spc::Vector3f(0.0f, 1.0f, 0.0f));
  normals.push_back(spc::Vector3f(0.0f, 1.0f, 0.0f));
  normals.push_back(spc::Vector3f(0.0f, 1.0f, 0.0f));
  normals.push_back(spc::Vector3f(0.0f, 1.0f, 0.0f));
  std::vector<spc::Vector2f> uv;
  uv.push_back(spc::Vector2f(0.0f, 0.0f));
  uv.push_back(spc::Vector2f(0.0f, ny));
  uv.push_back(spc::Vector2f(nx, 0.0f));
  uv.push_back(spc::Vector2f(nx, ny));
  std::vector<spc::UInt32> indices;
  indices.push_back(0);
  indices.push_back(1);
  indices.push_back(3);
  indices.push_back(0);
  indices.push_back(3);
  indices.push_back(2);
  std::vector<spc::Color4f> colors;
  spc::Color4f color(1.0f, 1.0f, 1.0f, 1.0f);
  colors.push_back(color);
  colors.push_back(color);
  colors.push_back(color);
  colors.push_back(color);
  generator->SetVertices(positions);
  generator->SetNormals(normals);
  generator->SetIndices(indices);
  generator->SetColors(colors);
  generator->SetUV0(uv);
  spc::iRenderMesh *renderMesh = generator->Generate();
  generator->Release();
  return renderMesh;
}

void debug(spc::SpatialState *state, int indent)
{
  if (!state)
  {
    return;
  }
  for (int i = 0; i < indent; i++)
  {
    printf("  ");
  }
  printf("%s [%s]\n",
         state->GetName().c_str(),
         state->GetEntity() ? state->GetEntity()->GetName().c_str() : "n/a"
  );
  for (spc::Size i = 0, in = state->GetNumberOfChildren(); i < in; i++)
  {
    debug(state->GetChild(i), indent + 1);
  }
}


void create_suzannes_plain(spc::Mesh *suzanneMesh, spc::World *world)
{
  size_t num = 20;
  for (size_t i = 0; i < num; i++)
  {
    float x = -40.0f + (float) i / (float) num * 80.0f;
    for (size_t j = 0; j < num; j++)
    {
      float y = -40.0f + (float) j / (float) num * 80.0f;

      spc::Entity *suzanneEntity = new spc::Entity("Entity1");
      spc::StaticMeshState *meshState1 = new spc::StaticMeshState("StaticMesh1");
      meshState1->SetTransform(spc::Transform(spc::Vector3f(x, 0, y)));
      meshState1->SetMesh(suzanneMesh);
      meshState1->SetStatic(true);
      suzanneEntity->Attach(meshState1);
      world->Attach(suzanneEntity);
    }
  }
}


void create_suzanne_batch(spc::Mesh *suzanneMesh, int a, int b, size_t numI, size_t numJ, size_t maxI, size_t maxJ, spc::World *world)
{
  auto generator = spc::ObjectRegistry::Get<spc::iRenderMeshBatchGeneratorFactory>()->Create();

  for (size_t ni = 0, i = a * numI; ni < numI; ni++, i++)
  {
    float x = -40.0f + (float) i / (float) maxI * 80.0f;
    for (size_t nj = 0, j = b * numJ; nj < numJ; nj++, j++)
    {
      float y = -40.0f + (float) j / (float) maxJ * 80.0f;
      spc::Matrix4f mat;
      mat.SetTranslation(spc::Vector3f(x, 0, y));
      generator->Add(suzanneMesh->GetSubMesh(0).GetMesh(), mat);
    }
  }

  spc::iRenderMesh *batchedRM = generator->Generate();

  spc::Mesh *suzyMesh = new spc::Mesh();
  for (size_t i = 0; i < suzanneMesh->GetNumberOfMaterialSlots(); i++)
  {
    suzyMesh->AddMaterialSlot(suzanneMesh->GetMaterialSlot(i).GetName(), suzanneMesh->GetMaterialSlot(i).GetDefaultMaterial());
  }
  suzyMesh->AddSubMesh(batchedRM, 0);

  spc::Entity *suzanneEntity = new spc::Entity("Entity1");
  spc::StaticMeshState *meshState1 = new spc::StaticMeshState("StaticMesh1");
  meshState1->SetTransform(spc::Transform(spc::Vector3f(0, 0, 0)));
  meshState1->SetMesh(suzyMesh);
  meshState1->SetStatic(true);
  suzanneEntity->Attach(meshState1);
  world->Attach(suzanneEntity);

  generator->Release();
}

void create_suzannes_batched(spc::Mesh *suzanneMesh, spc::World *world)
{
  for (int a = 0; a < 10; a++)
  {
    for (int b = 0; b < 10; b++)
    {
      create_suzanne_batch(suzanneMesh, a, b, 2, 2, 20, 20, world);
    }
  }
}

#include <regex>
#include <sstream>

int main(int argc, char **argv)
{

  if (!register_modules(argc, argv))
  {
    return -1;
  }

  if (!initialize_modules(argc, argv))
  {
    return -1;
  }

  spc::iDevice *device = spc::ObjectRegistry::Get<spc::iDevice>();

  spc::iShader *forwardShader = spc::AssetManager::Get()->Load<spc::iShader>(spc::ResourceLocator("/shaders/test_color_program.spc"));
  spc::iShader *shadowCubeShader = spc::AssetManager::Get()->Load<spc::iShader>(spc::ResourceLocator("/shaders/test_shadow_point_program.spc"));
  spc::iShader *shadowPSSMShader = spc::AssetManager::Get()->Load<spc::iShader>(spc::ResourceLocator("/shaders/test_shadow_pssm_program.spc"));
  spc::iShader *shadowShader = spc::AssetManager::Get()->Load<spc::iShader>(spc::ResourceLocator("/shaders/test_shadow_program.spc"));


  spc::iSampler *sampler = spc::AssetManager::Get()->Load<spc::iSampler>(spc::ResourceLocator("sampler_default.spc"));

  spc::Image *image = spc::AssetManager::Get()->Load<spc::Image>(spc::ResourceLocator("GrassGreenTexture0003.jpg"));
  if (!image)
  {
    image = spc::AssetManager::Get()->Load<spc::Image>(spc::ResourceLocator("snowflake_64.png"));
  }

  if (image)
  {
    image->GenerateMipMaps(spc::Image::eMipMapProcedure::eMMP_Linear4x4);
  }

  spc::iTexture2D::Descriptor desc = {};
  desc.Format = image->GetPixelFormat();
  desc.Width = image->GetWidth();
  desc.Height = image->GetHeight();
  desc.MipMaps = true;
  spc::iTexture2D *texture = device->CreateTexture(desc);
  texture->Data(image);

  spc::Material *material = new spc::Material();
  material->SetShader(spc::eRP_Forward, forwardShader);
  material->SetShader(spc::eRP_Shadow, shadowShader);
  material->SetShader(spc::eRP_ShadowCube, shadowCubeShader);
  material->SetShader(spc::eRP_ShadowPSSM, shadowPSSMShader);

  material->RegisterAttribute("Diffuse");
  material->RegisterAttribute("Color");
  material->Set(material->IndexOf("Diffuse"), texture);
  material->Set(material->IndexOf("Color"), spc::Color4f(1, 1, 1, 1));

  spc::MaterialInstance *instance = new spc::MaterialInstance();
  instance->SetMaterial(material);
  instance->Set(instance->IndexOf("Color"), spc::Color4f(0, 0, 1, 1));


  spc::iRenderMesh *renderMesh = create_plane_mesh(8, 8);
  spc::Mesh *mesh = new spc::Mesh();
  mesh->AddMaterialSlot("Default", instance);
  mesh->AddSubMesh(renderMesh, 0);


  spc::World *world = new spc::World();

  int wnd_width, wnd_height;
  SDL_GetWindowSize(wnd, &wnd_width, &wnd_height);

  spc::Settings settings("file:///config/display.spc");
  spc::Vector2i resolution = settings.GetVector2i("resolution", spc::Vector2i(wnd_width, wnd_height));
  int width = resolution.x;
  int height = resolution.y;

  float aspect = (float) wnd_height / (float) wnd_width;



  spc::Mesh *suzanneMesh = spc::AssetManager::Get()->Load<spc::Mesh>(spc::ResourceLocator("file:///suzanne.fbx"));
  spc::Mesh *cube = spc::AssetManager::Get()->Load<spc::Mesh>(spc::ResourceLocator("cube.fbx"));
  suzanneMesh->SetDefaultMaterial(0, material);
  cube->SetDefaultMaterial(0, material);

  spc::Entity *entity0 = new spc::Entity("Entity0");
  spc::StaticMeshState *meshState0 = new spc::StaticMeshState("StaticMesh0");

  meshState0->SetTransform(spc::Transform(spc::Vector3f(0, 0, 0)));
  meshState0->SetMesh(mesh);
  meshState0->SetStatic(true);
  entity0->Attach(meshState0);
  world->Attach(entity0);


  create_suzannes_plain(suzanneMesh, world);
//  create_suzannes_batched(suzanneMesh, world);

  spc::Entity *lightEntity = new spc::Entity("Light_0");
  spc::LightState *lightState = new spc::LightState("LightState");
  lightEntity->Attach(lightState);
  lightState->SetType(spc::eLT_Point);
  lightState->SetColor(spc::Color4f(1.0f, 1.0f, 1.0f, 1.0f) * 1.0f);
  lightState->SetRange(50);
  lightState->SetStatic(true);
  lightState->SetCastShadow(true);
  lightState->SetTransform(spc::Transform(spc::Vector3f(5.0f, 5.0f, 5.0f)));
  world->Attach(lightEntity);

  /*
  lightEntity = new spc::Entity("Light_0");
  lightState = new spc::LightState("LightState");
  lightEntity->Attach(lightState);
  lightState->SetType(spc::eLT_Point);
  lightState->SetColor(spc::Color4f(1.0f, 1.0f, 1.0f, 1.0f) * 0.5f);
  lightState->SetRange(25.0f);
  lightState->SetStatic(true);
  lightState->SetCastShadow(true);
  lightEntity->GetRoot()->GetTransform()
    .SetTranslation(spc::Vector3f(5.0f, 5.0f, 5.0f))
    .Finish();
  world->Attach(lightEntity);
  */

  spc::Entity *sunEntity = new spc::Entity("Sun");
  spc::LightState *sunLightState = new spc::LightState("SunLight");
  sunEntity->Attach(sunLightState);
  sunLightState->SetType(spc::eLT_Directional);
  sunLightState->SetColor(spc::Color4f(1.0f, 1.0f, 1.0f, 1.0f) * 1.0f);
  sunLightState->SetSplits(25.0f, 50.0f, 100.0f);
  sunLightState->SetShadowMapBias(0.003f);
  sunLightState->SetStatic(true);
  sunLightState->SetCastShadow(false);
  sunLightState->SetTransform(sunLightState->GetTransform()
          //.SetRotation(spc::Quaternion::FromAxisAngle(spc::Vector3f(1.0f, 0.0f, 0.0f), spc::spcDeg2Rad(-45.0f)))
                                  .SetRotation(spc::Quaternion::FromAxisAngle(spc::Vector3f(1.0f, 1.0f, 1.0f).Normalize(), spc::spcDeg2Rad(-45.0f)))
  );

  world->Attach(sunEntity);

  spc::Entity* cameraEntity = new spc::Entity("Camera");
  spc::CameraState* cameraState = new spc::CameraState();
  cameraEntity->Attach(cameraState);
  world->Attach(cameraEntity);



  spc::iSampler *colorSampler = device->CreateSampler();
  colorSampler->SetFilterMode(spc::eFM_MinMagNearest);

  spc::iSampler *depthSampler = device->CreateSampler();
  depthSampler->SetFilterMode(spc::eFM_MinMagNearest);

  spc::iTexture2D::Descriptor rt_col_desc = {};
  rt_col_desc.Width = width;
  rt_col_desc.Height = height;
  rt_col_desc.Format = spc::ePF_RGBA;
  rt_col_desc.MipMaps = false;
  spc::iTexture2D *color_texture = device->CreateTexture(rt_col_desc);
  color_texture->SetSampler(colorSampler);

  spc::iTexture2D::Descriptor rt_dpth_desc = {};
  rt_dpth_desc.Width = width;
  rt_dpth_desc.Height = height;
  rt_dpth_desc.Format = spc::ePF_Depth;
  rt_dpth_desc.MipMaps = false;
  spc::iTexture2D *depth_texture = device->CreateTexture(rt_dpth_desc);
  depth_texture->SetSampler(depthSampler);


  spc::iRenderTarget2D::Descriptor rt_desc = {};
  rt_desc.Width = width;
  rt_desc.Height = height;

  spc::iRenderTarget2D *renderTarget = device->CreateRenderTarget(rt_desc);
  renderTarget->AddColorTexture(color_texture);
  renderTarget->SetDepthBuffer(spc::ePF_Depth);
  if (!renderTarget->Compile())
  {
    printf("Unable to compile render target: %s\n", renderTarget->GetCompileLog().c_str());
    return 0;
  }
  else
  {
    printf("Render target complete\n");
  }

  spc::Quaternion myRot = spc::Quaternion::FromAxisAngle(spc::Vector3f(1, 2, 3).Normalize(), 1.234f);
  printf("Quaternion: %.2f %.2f %.2f %.2f\n", myRot.x, myRot.y, myRot.z, myRot.w);
  spc::Matrix3f myMat = myRot.ToMatrix3();
  myRot = spc::Quaternion::FromMatrix(myMat);
  printf("Quaternion: %.2f %.2f %.2f %.2f\n", myRot.x, myRot.y, myRot.z, myRot.w);


  spc::iRenderPipeline *renderPipeline = spc::ObjectRegistry::Get<spc::iRenderPipeline>();

  std::string title = spc::Settings("display.spc").GetText("title");
  float rot = 0.0f;
  float entRot = 0.0f;

  spc::UInt32 nextSec = SDL_GetTicks() + 1000;
  spc::UInt32 frames = 0;
  spc::UInt32 lastTime = SDL_GetTicks();


  bool useCs = true;
  bool anim = true;
#if _DEBUG
  spc::Size numDrawCallsPerSec = 0;
  spc::Size numTrianglesPerSec = 0;
#endif
  while (true)
  {
#if _DEBUG
    device->ResetDebug();
#endif
    Uint32 time = SDL_GetTicks();
    if (time > nextSec)
    {
      nextSec += 1000;
      char buffer[1024];
#if _DEBUG
      sprintf_s<1024>(buffer, "%s  %d FPS  #%llu calls (%llu triangles)", title.c_str(), frames, numDrawCallsPerSec, numTrianglesPerSec);
      numDrawCallsPerSec = 0;
      numTrianglesPerSec = 0;
#else
      sprintf_s<1024>(buffer, "%s  %d FPS", title.c_str(), frames);
#endif
      SDL_SetWindowTitle(wnd, buffer);
      printf("%s\n", buffer);
      fflush(stdout);
      frames = 0;

    }
    else
    {
      frames++;
    }
    spc::UInt32 deltaTime = time - lastTime;
    lastTime = time;

    SDL_GL_MakeCurrent(wnd, context);
    UpdateEvents();

    if (spc::Input::IsKeyPressed(spc::Key::eK_Escape))
    {
      break;
    }

    if (spc::Input::IsKeyPressed(spc::Key::eK_A))
    {
      anim = !anim;
    }
    if (spc::Input::IsKeyPressed(spc::Key::eK_Space))
    {
      useCs = !useCs;
    }

    /*
    entityX->GetRoot()->GetTransform()
      .SetRotation(spc::Quaternion::FromAxisAngle(spc::Vector3f(0.0f, 1.0f, 0.0f), entRot * 2))
      .Finish();

    entityZ->GetRoot()->GetTransform()
      .SetRotation(spc::Quaternion::FromAxisAngle(spc::Vector3f(0.0f, 1.0f, 0.0f), entRot / 2.0f))
      .Finish();
    */
    if (anim)
    {
      entRot += 0.003f;
    }

//    lightEntity->GetRoot()->SetTransform(spc::Transform(spc::Vector3f(spc::spcSin(entRot) * 5.0f, 5.0f, spc::spcCos(entRot) * 5.0f)));
    /*
      suzanneEntity->GetRoot()->SetTransform(spc::Transform(
          spc::Vector3f(spc::spcCos(entRot * 3.5f) * 5.0f, 0.0f, spc::spcSin(entRot * 3.5f) * 5.0f),
          spc::Quaternion::FromAxisAngle(spc::Vector3f(0, 1, 0), entRot * 3.5f - (float) M_PI / 2.0f)
      ));
      */

    float dist = 10.0f;
    cameraEntity->GetRoot()->LookAt(
      spc::Vector3f(spc::spcCos(entRot + (float)M_PI / 2.0f + 0.2f) * dist, dist, spc::spcSin(entRot + (float)M_PI / 2.0f + 0.2f) * dist),
      spc::Vector3f(0.0f, 0.0f, 0.0f)
    );

    //rot += 0.005f;

    world->Update((float) deltaTime / 1000.0f);


    cameraState->Update(renderTarget->GetWidth(), renderTarget->GetHeight());
    renderPipeline->Render(renderTarget, cameraState->GetCamera(), cameraState->GetProjector(), device, world->GetScene());


    device->SetRenderTarget(nullptr);
    device->SetViewport(0, 0, wnd_width, wnd_height);
    glDisable(GL_DEPTH_TEST);
    //device->Clear(true, spc::Color4f(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f, true, 0);
    device->RenderFullscreen(color_texture);
    glEnable(GL_DEPTH_TEST);

#if _DEBUG
    numDrawCallsPerSec += device->GetNumberOfDrawCalls();
    numTrianglesPerSec += device->GetNumberOfTriangles();
#endif

    SDL_GL_SwapWindow(wnd);

  }


  return 0;
}

