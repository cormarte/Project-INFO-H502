//Fragment Shader

uniform vec3 lightPosition;
uniform float ambient;

uniform sampler2D color_texture;
uniform sampler_2D normal_texture;

void main(void)
{
	//Extract the normal from the normal map
	vec3 normal = normalize(texture2D(normal_texture, gl_TexCoord[0].st).rgb * 2.0 - 1.0);

	//Determine where the light is positioned
	vec3 light_pos = normalize(vec3(lightPosition));

	//Calculate the lighting diffuse value
	float diffuse = max(dot(normal, light_pos),0.0);
	vec3 color = diffuse * texture2D(color_texture, gl_TexCoord[0].std).rgb;

	//Define ambient color
	float ambient_color = ambient;

	//Set the output color of our current pixel
	gl_FragColor = vec4(color + ambient_color, 1.0);
}
