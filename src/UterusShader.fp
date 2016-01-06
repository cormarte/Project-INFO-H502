uniform sampler2D bumpMap;
uniform sampler2D texture;

varying vec2 vTextureCoordinates;
varying vec3 vLightVectors[3];
varying vec3 vViewVector;


struct material 
{
    vec4 color;
	float ka, kd, ks;
	float shininess;
};

material tissue = material(vec4(1.0f, 1.0f, 1.0f,1.0f), 0.35f, 1.0f, 0.9f, 64.0f);


void main() {


	//*********//
	// Vectors //
	//*********//
	
	//Normal is extracted from the bump map and rescale between -1 and +1
	vec3 N = normalize(texture2D(bumpMap, vTextureCoordinates).rgb * 2.0f - 1.0f);
	vec3 V = normalize(vViewVector);


	//***********//
	// Materials //
	//***********//

	tissue.color = texture2D(texture, vTextureCoordinates);

	material material;
	material = tissue;


	//******************//
	// Light processing //
	//******************//

	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

	for (int i=0; i!=vLightVectors.length(); i++) {


		//*************//
		// Attenuation //
		//*************//

		//Light attenuation w.r.t. distance
		float distance = sqrt(dot(vLightVectors[i], vLightVectors[i]));
		float attenuation = clamp(1.0 - 0.001 * distance, 0.0, 1.0);


		//****************//
		// Lighting model //
		//****************//
		
		vec3 L = normalize(vLightVectors[i]);

		//Ambient 
		vec4 ambient = gl_LightSource[0].ambient * material.color;

		//Diffuse
		vec4 diffuse = max(dot(N,L), 0.0) * gl_LightSource[0].diffuse * material.color; 
    
		//Specular
		vec4 specular = pow(max(dot(reflect(-L, N), V), 0.0), material.shininess) * gl_LightSource[0].specular;

		color += material.ka * ambient + (material.kd * diffuse + material.ks * specular) * attenuation;
	};


	//**************//
	// Output color //
	//**************//

	gl_FragColor = vec4(color.rgb, 1.0);
}

