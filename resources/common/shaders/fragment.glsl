#version 330 core
out vec4 FragColor;

in vec3 CurColor;
in vec2 TexCoord;

// texture samplers
uniform sampler2D TexData;

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	FragColor = texture(TexData, TexCoord) * vec4(CurColor, 1.0);
}
