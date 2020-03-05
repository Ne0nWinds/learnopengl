#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

struct Material {
	vec3 diffuse;
	vec3 specular;
	float ambient;
	float shiniess;
};
uniform Material material;

void main()
{
	// ambient
	vec3 ambient = material.ambient * material.diffuse;

	// diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	vec3 diffuse = material.diffuse * max(dot(norm, lightDir), 0.0) * lightColor;

	// specular light
	float specularStrength = 0.5;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shiniess);
	vec3 specular = specularStrength * spec * lightColor;

	vec3 result = (ambient + diffuse + specular);
	FragColor = vec4(result, 1.0);
}
