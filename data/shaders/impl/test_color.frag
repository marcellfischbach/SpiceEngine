#version 330

layout(location = 0) out vec4 spc_FragColor;

uniform sampler2D spc_Diffuse;
uniform vec4 spc_Color;


in vec4 color;
in vec2 texCoord;
in vec3 world_position;
in vec3 world_normal;
in vec3 camera_space_position;

#include <../common/lighting.glsl>


void main()
{
    vec3 norm = normalize(world_normal);
    vec4 frag_color = calc_lights(world_position, norm, camera_space_position);
    frag_color *= color;
    frag_color *= texture(spc_Diffuse, texCoord * 3);
//    frag_color = vec4(norm * 0.5 + 0.5, 1.0);
    spc_FragColor = frag_color;
}

