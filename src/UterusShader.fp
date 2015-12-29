varying vec2 vTextureCoordinates;
varying vec3 vLightVector;
varying vec3 vEyeVector;
//varying vec3 vHalfVector;

uniform sampler2D bumpMap;
uniform sampler2D texture;

void main() {

	float distance = sqrt(dot(vLightVector, vLightVector));
	float attenuation = clamp(1.0 - 0.0005 * distance, 0.0, 1.0);

	vec4 color = texture2D(texture, vTextureCoordinates);

	vec3 normal = normalize(texture2D(bumpMap, vTextureCoordinates).rgb * 2.0f - 1.0f);
	
	//Diffuse
	//float diffuseCoefficient = max(vLightVector, normal), 0.0);
	float diffuseCoefficient = clamp(dot(normalize(vLightVector), normal), 0.0, 1.0);

	//Specular
	//float specularCoefficient = pow(max(dot(vHalfVector, normal), 0.0), 32.0);
	float specularCoefficient = pow(clamp(dot(reflect(-normalize(vLightVector), normal), normalize(vEyeVector)), 0.0, 1.0), 32);

	gl_FragColor = (gl_LightSource[0].ambient*color + gl_LightSource[0].diffuse*diffuseCoefficient*color + gl_LightSource[0].specular*specularCoefficient)*attenuation;	
}

