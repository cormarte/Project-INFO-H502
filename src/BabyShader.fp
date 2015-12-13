uniform float ambient;
uniform vec3 lightPosition;

varying vec3 normal;

void main() {
	
	//Diffusion
	float diffuse = max(dot(normalize(vec3(normal)), normalize(vec3(lightPosition))), 0.0);
    vec3 color = diffuse * vec3(0.9960/2.0, 0.7647/2.0, 0.6745/2.0) + vec3(0.9960/2.0, 0.7647/2.0, 0.6745/2.0);

	//Output color
	gl_FragColor = vec4(color, 1.0);
}
