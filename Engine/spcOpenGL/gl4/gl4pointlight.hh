

#pragma once

#include <spcOpenGL/openglexport.hh>
#include <spcCore/graphics/ipointlight.hh>

namespace spc::opengl
{


SPC_CLASS()
class SPC_OGL_API GL4PointLight : public SPC_SUPER(iPointLight)
{
  SPC_CLASS_GEN_OBJECT;
public:
  GL4PointLight();
  virtual ~GL4PointLight();

  eLightType GetType() const
  {
    return eLT_Point;
  }


  void SetChangeMode(eLightChangeMode changeMode) override;
  eLightChangeMode GetChangeMode() const override;

  void SetCastShadow(bool castShadow) override;
  bool IsCastShadow() const;

  void SetShadowMapBias(float shadowBias) override;
  float GetShadowMapBias() const;

  void SetColor(const Color4f & color) override;
  const Color4f& GetColor() const override;

  void SetIntensity(float intensity);
  float GetIntensity() const;

  void SetPosition(const Vector3f & position) override;
  const Vector3f& GetPosition() const override;

  void SetRange(float range) override;
  float GetRange() const override;

private:

  eLightChangeMode m_changeMode;
  bool m_castShadow;
  float m_shadowBias;
  Color4f m_color;
  float m_intensity;

  Vector3f m_position;
  float m_range;

};

}