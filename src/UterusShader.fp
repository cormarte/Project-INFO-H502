uniform sampler2D bumpMap;
uniform sampler2D texture;

varying vec2 vTextureCoordinates;
varying vec3 vLightVector;
varying vec3 vViewVector;


struct material {

    vec4 color;
	float ka, kd, ks;
	float shininess;
} tissue;


void main() {


	//*************//
	// Attenuation //
	//*************//

	//Light attenuation w.r.t. distance
	float distance = sqrt(dot(vLightVector, vLightVector));
	float attenuation = clamp(1.0 - 0.0005 * distance, 0.0, 1.0);


	//*********//
	// Vectors //
	//*********//

	//Normal is extracted from the bump map and rescale between -1 and +1
	vec3 N = normalize(texture2D(bumpMap, vTextureCoordinates).rgb * 2.0f - 1.0f);
	vec3 L = normalize(vLightVector);
	vec3 V = normalize(vViewVector);


	//***********//
	// Materials //
	//***********//

	tissue.color = texture2D(texture, vTextureCoordinates);
	tissue.ka = 1.0f;
	tissue.kd = 1.0f;
	tissue.ks = 1.0f;
	tissue.shininess = 32.0f;

	material material;
	material = tissue;
	

	//****************//
	// Lighting model //
	//****************//

	//Ambient 
	vec4 ambient = gl_LightSource[0].ambient * material.color;

	//Diffuse
	vec4 diffuse = max(dot(N,L), 0.0) * gl_LightSource[0].diffuse * material.color; 
    
	//Specular
	vec4 specular = pow(max(dot(reflect(-L, N), V), 0.0), material.shininess) * gl_LightSource[0].specular;


	//**************//
	// Output color //
	//**************//

	gl_FragColor = vec4((material.ka * ambient + (material.kd * diffuse + material.ks * specular) * attenuation).rgb, 1.0);
}

