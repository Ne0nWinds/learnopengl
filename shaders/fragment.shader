#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;
uniform float alpha;

void main()
{
	// vanilla color
	// fragmentColor = vec4(outColor, 1.0);

	// finding color from texture
	// FragColor = texture(texture1, TexCoord);

	// overlaying color onto texture
	FragColor = texture(texture1, TexCoord);

	// overlay textures
	// FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), alpha);
}
