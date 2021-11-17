#version 330 core
out vec4 FragColor;

in vec3 CurColor;
in vec2 TexCoord;
in vec3 Normal;  
in vec3 FragPos;  

uniform sampler2D TexData;
uniform int HasTex;
uniform vec3 viewPos; 
uniform vec3 lightPos; 
uniform vec3 lightColor;

uniform int objType;

void main()
{
    if (objType == 1) {
        FragColor = vec4(1.0);
    } else {
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * lightColor;

        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        float specularStrength = 0.5;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
        vec3 specular = specularStrength * spec * lightColor;  
        vec3 result = (ambient + diffuse + specular) * CurColor;

        if (HasTex != 0) {
            FragColor = (texture(TexData, TexCoord) + (vec4(result, 1.0) * 4 / 32)) * vec4(result, 1.0);
        } else {
            FragColor = vec4(result, 1.0);
        }
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
