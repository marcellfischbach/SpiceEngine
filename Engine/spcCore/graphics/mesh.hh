
#pragma once

#include <spcCore/coreexport.hh>
#include <spcCore/class.hh>
#include <spcCore/graphics/irendermesh.hh>
#include <spcCore/graphics/material/imaterial.hh>
#include <spcCore/types.hh>


#include <string>
#include <vector>


namespace spc
{





SPC_CLASS()
class SPC_CORE_API Mesh : public SPC_SUPER(iObject)
{
  SPC_CLASS_GEN_OBJECT;
public:
  Mesh();
  virtual ~Mesh();



  class SubMesh
  {
    friend class Mesh;
  private:
    SubMesh();

  public:
    ~SubMesh();
    void SetMesh(iRenderMesh* mesh);
    iRenderMesh* GetMesh();
    const iRenderMesh* GetMesh() const;

    void SetMaterialSlotIdx(Size materialSlotIdx);
    Size GetMaterialSlotIdx() const;


  private:
    iRenderMesh* m_mesh;

    Size m_materialSlotIdx;

  };


  class MaterialSlot
  {
    friend class Mesh;
  private:
    MaterialSlot(const std::string &name, iMaterial *defaultMaterial);
  public:
    ~MaterialSlot();
    iMaterial* GetDefaultMaterial() const;


    const std::string& GetName() const;

  private:
    iMaterial* m_defaultMaterial;
    std::string m_name;
  };


  Size AddMaterialSlot(const std::string& name, iMaterial* defaultMaterial = nullptr);
  Size GetNumberOfMaterialSlots() const;
  const MaterialSlot& GetMaterialSlot(Size idx) const;


  void AddSubMesh(iRenderMesh* mesh, Size materialSlotIdx);


private:

  std::vector<MaterialSlot> m_materialSlots;
  std::vector<SubMesh> m_subMeshes;


};


}