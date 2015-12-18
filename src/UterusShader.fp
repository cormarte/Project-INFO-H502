varying vec3 vLightVector;
varying vec3 vEyeVector;
varying vec2 vTextureCoordinates;

uniform vec3 ambient;
uniform vec3 lightPosition;
uniform sampler2D bumpMap;
uniform sampler2D texture;

void main() {

	vec3 color = texture2D(texture, vTextureCoordinates).rgb;

	vec3 bump = normalize(texture2D(bumpMap, vTextureCoordinates).rgb * 2.0f - 1.0f);

	float diffuse = max(dot(vLightVector, bump), 0.0);
	
	gl_FragColor = vec4(ambient*color + (vec3(1.0f)-ambient)*diffuse*color, 1.0);
}
