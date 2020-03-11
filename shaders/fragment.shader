#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shiniess;
};
uniform Material material;

struct Light {
	vec3 position;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float cutOff;
	float outerCutOff;
	float constant;
	float linear;
	float quadratic;
};
uniform Light light;

void main()
{

	vec3 lightDir = normalize(light.position - FragPos);
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	if (theta > light.outerCutOff)
	{
		// attenuation
		float distance = length(light.position - FragPos);
		float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

		// ambient
		vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

		// diffuse
		vec3 norm = normalize(Normal);
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));

		// specular light
		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shiniess);
		vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));;

		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;

		diffuse *= intensity;
		specular *= intensity;

		vec3 result = (ambient + diffuse + specular);
		FragColor = vec4(result, 1.0);
	}

	// FragColor = vec4(light.ambient * vec3(texture(material.diffuse, TexCoords)), 1.0);

}
