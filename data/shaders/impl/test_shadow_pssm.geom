#version 330
layout(triangles) in;
layout(triangle_strip, max_vertices = 9) out;

uniform mat4 spc_ShadowMapViewProjectionMatrix[3];


void main()
{
	{
		gl_Layer = 0;
		gl_Position = spc_ShadowMapViewProjectionMatrix[0] * gl_in[0].gl_Position;
		EmitVertex();
		gl_Position = spc_ShadowMapViewProjectionMatrix[0] * gl_in[1].gl_Position;
		EmitVertex();
		gl_Position = spc_ShadowMapViewProjectionMatrix[0] * gl_in[2].gl_Position;
		EmitVertex();
		EndPrimitive();
	}
	{
		gl_Layer = 1;
		gl_Position = spc_ShadowMapViewProjectionMatrix[1] * gl_in[0].gl_Position;
		EmitVertex();
		gl_Position = spc_ShadowMapViewProjectionMatrix[1] * gl_in[1].gl_Position;
		EmitVertex();
		gl_Position = spc_ShadowMapViewProjectionMatrix[1] * gl_in[2].gl_Position;
		EmitVertex();
		EndPrimitive();
	}
	{
		gl_Layer = 2;
		gl_Position = spc_ShadowMapViewProjectionMatrix[2] * gl_in[0].gl_Position;
		EmitVertex();
		gl_Position = spc_ShadowMapViewProjectionMatrix[2] * gl_in[1].gl_Position;
		EmitVertex();
		gl_Position = spc_ShadowMapViewProjectionMatrix[2] * gl_in[2].gl_Position;
		EmitVertex();
		EndPrimitive();
	}

}

