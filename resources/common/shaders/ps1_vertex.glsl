#version 330 core
#pragma optimize(on)

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;

noperspective out vec3 CurColor;
out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
out float vAffine;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    CurColor = aColor;
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
    float dist = length(gl_Position);
    float affine = dist + ((gl_Position.w * 8.0) / dist) * 0.5;
    TexCoord *= affine;
    vAffine = affine;
    vec2 resolution = vec2(32, 24);
    gl_Position.xyz /= gl_Position.w * 0.5;
    gl_Position.xy = round(resolution * gl_Position.xy) / resolution;
    gl_Position.xyz *= gl_Position.w * 0.5;
}
