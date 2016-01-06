uniform sampler2D environment;
uniform float reflection;

varying vec3 vNormal;
varying vec3 vLightVector;
varying vec3 vViewVector;


struct material {

    vec4 color;
	float ka, kd, ks;
	float waxiness;
	float shininess;
	float silhouetteness;
	float silhouettePow;
} wax, wax2, skin, skin2, rubber;


void main() {


	//*********//
	// Vectors //
	//*********//

	vec3 N = normalize(vNormal);
	vec3 L = normalize(vLightVector);
	vec3 V = normalize(vViewVector);
	vec3 R = reflect(V, N);


	//***********//
	// Materials //
	//***********//

	wax.color = vec4(0.964705, 0.670588, 0.556862, 1.0);
	wax.ka = 2.35f;
	wax.kd = 1.5f;
	wax.ks = 0.15f;
	wax.waxiness = 0.15f;
	wax.shininess = 64.0f;
	wax.silhouetteness = 0.8f;
	wax.silhouettePow = 2.0f;

	wax2.color = vec4(0.964705, 0.670588, 0.556862, 1.0);
	wax2.ka = 2.5f;
	wax2.kd = 1.5f;
	wax2.ks = 0.45f;
	wax2.waxiness = 0.9f;
	wax2.shininess = 128.0f;
	wax2.silhouetteness = 0.75f;
	wax2.silhouettePow = 3.0f;
	
	skin.color = vec4(0.964705, 0.670588, 0.556862, 1.0);
	skin.ka = 1.25f;
	skin.kd = 1.0f;
	skin.ks = 1.25f;
	skin.waxiness = 0.15f;
	skin.shininess = 32.0f;
	skin.silhouetteness = 0.0f;
	skin.silhouettePow = 1.0f;

	skin2.color = vec4(0.964705, 0.670588, 0.556862, 1.0);
	skin2.ka = 1.75f;
	skin2.kd = 1.5f;
	skin2.ks = 1.20f;
	skin2.waxiness = 0.15f;
	skin2.shininess = 64.0f;
	skin2.silhouetteness = 0.0f;
	skin2.silhouettePow = 1.0f;

	rubber.color = vec4(0.964705, 0.670588, 0.556862, 1.0);
	rubber.ka = 2.15f;
	rubber.kd = 1.0f;
	rubber.ks = 1.0f;
	rubber.waxiness = 0.25f;
	rubber.shininess = 128.0f;
	rubber.silhouetteness = 0.5f;
	rubber.silhouettePow = 1.0f;

	material material;
	material = wax2;

		
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


	//****************//
	// Lighting model //
	//****************//

	//Ambient 
	vec4 ambient = gl_LightSource[0].ambient * material.color;

	//Diffuse
	vec4 diffuse = (material.waxiness + (1-material.waxiness)*max(dot(N,L), 0.0)) * gl_LightSource[0].diffuse * material.color; 
    
	//Specular
	vec4 specular = pow(max(dot(reflect(-L, N), V), 0.0), material.shininess) * gl_LightSource[0].specular * material.color;


	//************//
	// Silhouette //
	//************//

	// Gives a wax/silhouette border effect to diffuse component
	float silhouette = (1-material.silhouetteness*pow(abs(dot(V, N)), material.silhouettePow));


	//**************//
	// Output color //
	//**************//

	gl_FragColor = vec4((material.ka * ambient + material.kd * silhouette * diffuse + material.ks * specular).rgb, 1.0);
}
