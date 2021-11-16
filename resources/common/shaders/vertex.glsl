#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 CurColor;
out vec2 TexCoord;

void main()
{
	gl_Position = vec4(aPos, 1.0);
	CurColor = aColor;
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
