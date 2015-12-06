uniform float ambient;
uniform vec3 lightPosition;
uniform sampler2D bumpMap;
uniform sampler2D texture;

varying vec3 normal;

void main() {

	//Bump mapping
	/*float fOffset = 1.0f;
	vec4 s00 = texture2D(bumpMap, gl_TexCoord[0].st + vec2(-fOffset, -fOffset));
	vec4 s01 = texture2D(bumpMap, gl_TexCoord[0].st + vec2(0.0, -fOffset));
	vec4 s02 = texture2D(bumpMap, gl_TexCoord[0].st + vec2(fOffset, -fOffset));

	vec4 s10 = texture2D(bumpMap, gl_TexCoord[0].st + vec2(-fOffset, 0.0));
	vec4 s12 = texture2D(bumpMap, gl_TexCoord[0].st + vec2(fOffset, 0.0));

	vec4 s20 = texture2D(bumpMap, gl_TexCoord[0].st + vec2(-fOffset, fOffset));
	vec4 s21 = texture2D(bumpMap, gl_TexCoord[0].st + vec2(0.0, fOffset));
	vec4 s22 = texture2D(bumpMap, gl_TexCoord[0].st + vec2(fOffset, fOffset));

	vec4 sobelX = s00 + 2.0 * s10 + s20 - s02 - 2.0 * s12 - s22;
	vec4 sobelY = s00 + 2.0 * s01 + s02 - s20 - 2.0 * s21 - s22;

	float sx = dot(sobelX, vec4(normalize(lightPosition), 0.0));
	float sy = dot(sobelY, vec4(normalize(lightPosition), 0.0));

	vec3 n = normalize(vec3(sx, sy, 1.0));*/

	//Diffusion
	float diffuse = max(dot(normalize(vec3(normal)), normalize(vec3(lightPosition))), 0.0);
	vec3 color = diffuse * (texture2D(texture, gl_TexCoord[0].st).rgb) + (texture2D(texture, gl_TexCoord[0].st).rgb)/2.0;

	//Output color
	gl_FragColor = vec4(color + ambient, 1.0);
}
