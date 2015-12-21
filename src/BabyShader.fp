varying vec3 vNormal;
varying vec3 vLightVector;
varying vec3 vEyeVector;

void main() {
	
	vec4 color = vec4(0.9960, 0.7647, 0.6745, 1.0);

	//Diffuse
	float diffuseCoefficient = clamp(dot(normalize(vNormal), normalize(vLightVector)), 0.0, 1.0);
    
	//Specular
	float waxiness = 0.1f;
	float specularCoefficient = waxiness + (1-waxiness)*pow(clamp(dot(reflect(-normalize(vLightVector), normalize(vNormal)), normalize(vEyeVector)), 0.0, 1.0), 32);

	//Output color
	gl_FragColor = gl_LightSource[0].ambient*color + gl_LightSource[0].diffuse*diffuseCoefficient*color + gl_LightSource[0].specular*specularCoefficient;
}
