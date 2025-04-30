#version 330 core 
out vec4 FragColor;

struct Material {
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};

struct DirLight{
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};
uniform DirLight dirLight;
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewPos);

struct PointLight{
  vec3 position;
  
  float constant;
  float linear;
  float quadratic;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights [NR_POINT_LIGHTS];
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;

void main() {
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);

  vec3 result = CalcDirLight(dirLight, norm, viewDir);
  for (int i = 0; i < NR_POINT_LIGHTS; i++){
    result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
  }

  FragColor = vec4(result, 1.0f);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
  vec3 lightDir = normalize(-light.direction);

  vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * light.diffuse * 
    texture(material.diffuse, TexCoords).rgb;

  vec3 reflectedDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectedDir), 0.0), material.shininess);
  vec3 specular = spec * light.specular 
    * texture(material.specular, TexCoords).rgb;
  
  return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
  vec3 lightDir = normalize(light.position - fragPos);

  vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * light.diffuse * 
    texture(material.diffuse, TexCoords).rgb;

  vec3 reflectedDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectedDir), 0.0), material.shininess);
  vec3 specular = spec * light.specular 
    * texture(material.specular, TexCoords).rgb;

  float distance = length(light.position - fragPos);
  float attenuation = 1.0f / (light.constant * distance
                            + light.linear * distance 
                            + light.quadratic * distance * distance);
  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;

  return ambient + diffuse + specular;
  
}
