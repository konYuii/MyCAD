#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec2 i_Position;
layout(location = 1) in vec2 i_tangent;
layout(location = 2) in vec2 i_normal;
layout(location = 0) out vec4 o_Color;



void main() {
    gl_Position = vec4(i_Position, 0, 1);
    gl_PointSize = 1;
    o_Color = vec4(1, 1, 1, 1);
}