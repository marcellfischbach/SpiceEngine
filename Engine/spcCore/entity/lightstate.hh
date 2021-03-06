
#pragma  once


#include <spcCore/coreexport.hh>
#include <spcCore/entity/spatialstate.hh>
#include <spcCore/graphics/elighttype.hh>
#include <spcCore/math/color4f.hh>
#include <spcCore/math/vector3f.hh>

namespace spc
{

struct iLight;
struct iPointLight;
struct iDirectionalLight;
class GfxLight;
class World;

SPC_CLASS()
class SPC_CORE_API LightState : public SPC_SUPER(SpatialState)
{
  SPC_CLASS_GEN_OBJECT;

public:
  LightState(const std::string &name);
  ~LightState() override;

  void SetType(eLightType type);
  SPC_NODISCARD eLightType GetType() const;

  void SetCastShadow(bool castShadow);
  SPC_NODISCARD bool IsCastShadow() const;

  void SetShadowMapBias(float bias);
  SPC_NODISCARD float GetShadowMapBias() const;

  void SetColor(const Color4f &color);
  SPC_NODISCARD const Color4f &GetColor() const;

  void SetRange(float range);
  SPC_NODISCARD float GetRange() const;

  SPC_NODISCARD void SetSplits(float split0, float split1, float split2);
  SPC_NODISCARD SPC_NODISCARD float GetSplit0() const;
  SPC_NODISCARD SPC_NODISCARD float GetSplit1() const;
  SPC_NODISCARD SPC_NODISCARD float GetSplit2() const;

  void OnAttachedToWorld(World * world) override;
  void OnDetachedFromWorld(World * world) override;

protected:
  void TransformationUpdatedPreChildren() override;

private:
  iLight *CreateLight();
  void AddToScene(World *world);
  void RemoveFromScene(World * world);

  void UpdateValues();


  iLight* m_light;
  iPointLight* m_pointLight;
  iDirectionalLight* m_directionalLight;

  GfxLight *m_gfxLight;

  eLightType m_lightType;

  bool m_castShadow;
  float m_shadowBias;
  Color4f m_color;
  float m_range;
  Vector3f m_splits;
};

}

