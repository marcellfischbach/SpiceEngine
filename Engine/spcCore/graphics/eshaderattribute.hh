
#pragma once

namespace spc
{


enum eShaderAttribute
{
  /* Matrices */
  eSA_ModelMatrix,
  eSA_ViewMatrix,
  eSA_ProjectionMatrix,
  eSA_ModelViewMatrix,
  eSA_ViewProjectionMatrix,
  eSA_ModelViewProjectionMatrix,
  eSA_ModelMatrixInv,
  eSA_ViewMatrixInv,
  eSA_ProjectionMatrixInv,
  eSA_ModelViewMatrixInv,
  eSA_ViewProjectionMatrixInv,
  eSA_ModelViewProjectionMatrixInv,

  /* Lighting */
  eSA_LightColor,
  eSA_LightVector,
  eSA_LightRange,
  eSA_LightCount

};

}