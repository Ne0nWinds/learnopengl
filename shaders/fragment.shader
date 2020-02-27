#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
	// for vanilla color
	// fragmentColor = vec4(outColor, 1.0);

	// for finding color from texture
    FragColor = texture(ourTexture, TexCoord);
}
