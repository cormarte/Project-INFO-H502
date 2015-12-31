varying vec2 vTextureCoordinates;
varying vec3 vLightVector;
varying vec3 vViewVector;

uniform sampler2D bumpMap;
uniform sampler2D texture;

void main() {


	//*************//
	// Attenuation //
	//*************//

	float distance = sqrt(dot(vLightVector, vLightVector));
	float attenuation = clamp(1.0 - 0.0005 * distance, 0.0, 1.0);


	//*********//
	// Vectors //
	//*********//

	vec3 N = normalize(texture2D(bumpMap, vTextureCoordinates).rgb * 2.0f - 1.0f);
	vec3 L = normalize(vLightVector);
	vec3 V = normalize(vViewVector);


	//*******//
	// Color //
	//*******//
	vec4 color = texture2D(texture, vTextureCoordinates);
	

	//****************//
	// Lighting model //
	//****************//

	//Ambient 
	vec4 ambient = gl_LightSource[0].ambient * color;

	//Diffuse
	vec4 diffuse = max(dot(N,L), 0.0) * gl_LightSource[0].diffuse * color; 
    
	//Specular
	vec4 specular = pow(max(dot(reflect(-L, N), V), 0.0), 32) * gl_LightSource[0].specular;


	//**************//
	// Output color //
	//**************//

	gl_FragColor = vec4(((ambient + diffuse + specular)*attenuation).rgb, 1.0);
}

