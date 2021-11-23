#version 330
#pragma optimize(on)

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;

out vec3 CurColor;
out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
flat out int fIs2D;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int is2D;

void main() {
    fIs2D = is2D;
    CurColor = aColor;
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    FragPos = vec3(model * vec4(aPos, 1.0));
    if (fIs2D == 0) {
        Normal = mat3(transpose(inverse(model))) * aNormal;
        gl_Position = projection * view * vec4(FragPos, 1.0);
    } else {
        gl_Position = vec4(FragPos, 1.0);
    }
}
