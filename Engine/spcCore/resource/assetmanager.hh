
#pragma once

#include <spcCore/coreexport.hh>
#include <spcCore/class.hh>
#include <spcCore/resource/iassetloader.hh>
#include <spcCore/resource/iassetloader.hh>
#include <map>
#include <vector>
#include <string>

namespace spc
{


class SPC_CORE_API AssetManager
{
  friend class CoreModule;
public:
  static void Set(AssetManager* manager);
  static AssetManager* Get();

  void RegisterLoader(iAssetLoader* loader);
  void RegisterLoader(iAssetLoaderSpc* loader);


  template<typename T>
  T* Get(const ResourceLocator& locator)
  {
    iObject* obj = Get(T::GetStaticClass(), locator);
    return obj ? obj->Query<T>() : nullptr;
  }
  iObject* Get(const Class* cls, const ResourceLocator& locator);

  template<typename T>
  T* Load(const ResourceLocator& locator)
  {
    iObject* obj = Load(T::GetStaticClass(), locator);
    return obj ? obj->Query<T>() : nullptr;
  }
  iObject* Load(const Class* cls, const ResourceLocator& locator);

  template<typename T>
  T* Load(const std::string& locator)
  {
    iObject* obj = Load(T::GetStaticClass(), ResourceLocator(locator));
    return obj ? obj->Query<T>() : nullptr;
  }
  iObject* Load(const Class* cls, const std::string& locator)
  {
    return Load(cls, ResourceLocator(locator));
  }

protected:
  AssetManager();

  iObject* LoadExt(const Class* cls, const ResourceLocator& locator);
  iObject* LoadSpc(const Class* cls, const ResourceLocator& locator);

  std::vector<iAssetLoader*> m_loaders;
  std::vector<iAssetLoaderSpc*> m_loadersSpc;


private:
  static AssetManager* s_assetManager;
};



}