uniform sampler2D environment;
uniform float reflection;

varying vec3 vNormal;
varying vec3 vLightVectors[3];
varying vec3 vViewVector;


struct material
{
    vec4 color;
	float ka, kd, ks;
	float waxiness;
	float shininess;
	float silhouetteness;
	float silhouettePow;
};

const material wax = material(vec4(0.964705, 0.670588, 0.556862, 1.0), 2.5f, 1.5f, 0.45f, 0.9f, 128.0f, 0.75f, 3.0f);
const material wax2 = material(vec4(0.964705, 0.670588, 0.556862, 1.0), 2.0f, 2.5f, 0.25f, 0.9f, 128.0f, 0.75f, 3.0f);
const material wax3 = material(vec4(0.964705, 0.670588, 0.556862, 1.0), 0.78f, 0.8f, 0.4f, 0.9f, 165.0f, 0.8f, 3.0f);
const material skin = material(vec4(0.964705, 0.670588, 0.556862, 1.0), 1.25f, 1.0f, 1.25f, 0.15f, 32.0f, 0.0f, 1.0f);
const material skin2 = material(vec4(0.964705, 0.670588, 0.556862, 1.0), 1.75f, 1.5f, 1.20f, 0.15f, 64.0f, 0.0f, 1.0f);
const material rubber = material(vec4(0.964705, 0.670588, 0.556862, 1.0), 2.15f, 1.0f, 1.0f, 0.25f, 128.0f, 0.5f, 1.0f);


void main() {


	//*********//
	// Vectors //
	//*********//

	vec3 N = normalize(vNormal);
	vec3 V = normalize(vViewVector);
	vec3 R = reflect(V, N);


	//**********//
	// Material //
	//**********//

	material material;
	material = wax3;

		
	//*******//
	// Color //
	//*******//

	//Sphere mapping
	vec2 textureCoordinates;
	float m = 2.0 * sqrt( R.x*R.x + R.y*R.y + (R.z+1.0)*(R.z+1.0) );
	textureCoordinates.s = R.x/m + 0.5;
	textureCoordinates.t = R.y/m + 0.5;

	vec4 environmentColor = texture2D(environment, textureCoordinates.st);

	material.color = mix(material.color, environmentColor, reflection);


	//************//
	// Silhouette //
	//************//

	// Gives a wax/silhouette border effect to diffuse component
	float silhouette = (1-material.silhouetteness*pow(abs(dot(V, N)), material.silhouettePow));


	//******************//
	// Light processing //
	//******************//

	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

	for (int i=0; i!=vLightVectors.length(); i++) {


		//****************//
		// Lighting model //
		//****************//

		vec3 L = normalize(vLightVectors[i]);

		//Ambient 
		vec4 ambient = gl_LightSource[0].ambient * material.color;

		//Diffuse
		vec4 diffuse = (material.waxiness + (1-material.waxiness)*max(dot(N,L), 0.0)) * gl_LightSource[0].diffuse * material.color; 
    
		//Specular
		vec4 specular = pow(max(dot(reflect(-L, N), V), 0.0), material.shininess) * gl_LightSource[0].specular * material.color;

		color += material.ka * ambient + material.kd * silhouette * diffuse + material.ks * specular;
	};


	//**************//
	// Output color //
	//**************//

	gl_FragColor = vec4(color.rgb, 1.0);
}
