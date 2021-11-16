#version 330 core
out vec4 FragColor;

in vec3 CurColor;
in vec2 TexCoord;

// texture samplers
uniform sampler2D TexData;
uniform int TexData;

void main()
{
    if (HasTex) {
    	FragColor = texture(TexData, TexCoord) * vec4(CurColor, 1.0);
    } else {
    	FragColor = vec4(CurColor, 1.0);
    }
}
