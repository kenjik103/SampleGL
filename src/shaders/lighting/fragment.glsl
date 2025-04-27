#version 330 core 
struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

struct Light {
  vec3 lightPos;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};


in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

out vec4 FragColor;

void main() {
  vec3 ambient = light.ambient * material.ambient;

  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(light.lightPos - FragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = light.diffuse * (diff * material.diffuse);

  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectionDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectionDir), 0.0), material.shininess);
  vec3 specular = light.specular * (spec * material.specular);

  vec3 result = ambient + diffuse + specular;
  FragColor = vec4(result, 1.0f);
}
