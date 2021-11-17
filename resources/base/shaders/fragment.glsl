#version 330 core

out vec4 FragColor;

struct light_struct {
    int type;
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
    vec3 direction;
    float range;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
};

struct material_struct {
    sampler2D diffuse;
    sampler2D specular;
    float shine;
};

in vec3 CurColor;
in vec2 TexCoord;
in vec3 Normal;  
in vec3 FragPos;  

uniform sampler2D TexData;
uniform int HasTex;
uniform vec3 viewPos; 
uniform vec3 lightPos; 
uniform vec3 lightColor;
uniform int lightmaxct;

uniform light_struct light[256];
uniform material_struct material;

vec3 calcLight(int i)
{
    vec3 ambient = light[i].ambient;
    vec3 norm = normalize(Normal);
    float dist = 1 / mix(abs(distance(light[i].position, FragPos) + 1), 1, light[i].range);
    vec3 lightDir = normalize(light[i].position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light[i].diffuse;
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shine);
    vec3 specular = specularStrength * spec * light[i].specular * (material.shine / 256);
    return (ambient + diffuse + specular) * CurColor * dist;
}

void main()
{
    vec3 result;
    result.r = 0;
    result.g = 0;
    result.b = 0;
    for (int i = 0; i < lightmaxct; ++i) {
        if (light[i].type != 0) result += calcLight(i);
    }
    FragColor = vec4(result, 1.0);
    if (HasTex != 0) {
        FragColor *= (texture(TexData, TexCoord) + (material.shine / 4096));
    }
}

#if 1 && 0
#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
  
uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
      
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
#endif
