varying vec3 vNormal;
varying vec3 vLightVector;
varying vec3 vViewVector;

uniform sampler2D environment;

void main() {


	//*********//
	// Vectors //
	//*********//

	vec3 N = normalize(vNormal);
	vec3 L = normalize(vLightVector);
	vec3 V = normalize(vViewVector);
	vec3 R = reflect(V, N);
	

	//*******//
	// Color //
	//*******//

	//vec4 color = vec4(0.9960, 0.7647, 0.6745, 1.0);
	vec4 babyColor = vec4(0.964705, 0.670588, 0.556862, 1.0);

	vec2 textureCoordinates;
	float m = 2.0 * sqrt( R.x*R.x + R.y*R.y + (R.z+1.0)*(R.z+1.0) );
	textureCoordinates.s = R.x/m + 0.5;
	textureCoordinates.t = R.y/m + 0.5;
	vec4 environmentColor = texture2D(environment, textureCoordinates.st);

	vec4 color = mix(babyColor, environmentColor, 0.35);


	//***********//
	// Materials //
	//***********//

	float waxiness;
	int shininess;
	float silhouetteness;
	int silhouettePow;
	float a, d, s;

	//Default
	/*waxiness = 0.15f;
	shininess = 32;
	a = 1.25f;
	d = 1.0f;
	s = 1.25f;
	silhouetteness = 0.0;
	silhouettePow = 1;*/

	//Default 2
	/*waxiness = 0.15f;
	shininess = 64;
	a = 1.75f;
	d = 1.5f;
	s = 1.0f;
	silhouetteness = 0.0;
	silhouettePow = 1;*/

	//Rubber
	/*waxiness = 0.25f;
	shininess = 128;
	a = 2.15f;
	d = 1.0f;
	s = 1.0f;
	silhouetteness = 0.5;
	silhouettePow = 1;*/

	//Wax
	/*waxiness = 0.85f;
	shininess = 64;
	a = 2.35f;
	d = 1.5f;
	s = 0.15f;
	silhouetteness = 0.8;
	silhouettePow = 2;*/

	//Wax 2
	waxiness = 0.9f;
	shininess = 64;
	a = 2.65f;
	d = 1.60f;
	s = 0.15f;
	silhouetteness = 0.75;
	silhouettePow = 3;


	//****************//
	// Lighting model //
	//****************//

	//Ambient 
	vec4 ambient = gl_LightSource[0].ambient * color;

	//Diffuse
	vec4 diffuse = (waxiness + (1-waxiness)*max(dot(N,L), 0.0)) * gl_LightSource[0].diffuse * color; 
    
	//Specular
	vec4 specular = pow(max(dot(reflect(-L, N), V), 0.0), shininess) * gl_LightSource[0].specular;


	//************//
	// Silhouette //
	//************//

	float silhouette = (1-silhouetteness*pow(abs(dot(V, N)), silhouettePow));


	//**************//
	// Output color //
	//**************//

	gl_FragColor = vec4((a * ambient + d * silhouette * diffuse + s * specular).rgb, 1.0);
}
