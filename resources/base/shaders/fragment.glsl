#version 330
#pragma optimize(on)

out vec4 FragColor;

struct light_struct {
    int type;
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 direction;
    float range;
    vec3 tcorner;
    vec3 bcorner;
};

struct material_struct {
    float shine;
    float resis;
};

in vec3 CurColor;
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
flat in int fIs2D;

uniform sampler2D TexData;
uniform int HasTex;
uniform vec3 viewPos;

uniform light_struct light[64];
uniform material_struct material;
uniform int maxlightindex;

vec3 v3zero = vec3(0, 0, 0);
vec3 v3one = vec3(1, 1, 1);

bool bcheck(vec3 v, vec3 tcorner, vec3 bcorner) {
    if (tcorner == v3zero && bcorner == v3zero) return true;
    return (step(bcorner, v) - step(tcorner, v)) == v3one;
}

vec3 calcLight(int i) {
    if (bcheck(FragPos, light[i].tcorner, light[i].bcorner)) {
        float dist = 1 / mix(abs(distance(light[i].position, FragPos) + 1), 1, light[i].range);
        vec3 viewDir = normalize(viewPos - FragPos);
        if (light[i].type == 2) {
            return (light[i].ambient + light[i].diffuse * max(dot(mix(Normal, normalize(Normal), 1), normalize(-light[i].direction)), 0.0)) * CurColor;
        } else {
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(light[i].position - FragPos);
            return (light[i].ambient +
                max(dot(norm, lightDir), 0) * light[i].diffuse +
                ((material.shine / 256) * pow(max(dot(viewDir, reflect(-lightDir, norm)), 0), material.shine) * light[i].specular * dist)) * CurColor * dist;
        }
    } else {
        return vec3(0, 0, 0);
    }
}

void main() {
    if (fIs2D == 0) {
        vec3 result = vec3(0.0, 0.0, 0.0);
        for (int i = 0; i < maxlightindex; ++i) {
            if (light[i].type != 0) result += calcLight(i);
        }
        result = mix(result, CurColor, material.resis);
        FragColor = vec4(result, 1.0);
        if (HasTex != 0) {
            FragColor *= (texture(TexData, TexCoord) + (material.shine / 4096));
        }
    } else {
        FragColor = vec4(CurColor, 1.0);
        if (HasTex != 0) {
            FragColor *= texture(TexData, TexCoord);
        }
    }
}
