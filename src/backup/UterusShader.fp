varying vec2 vTextureCoordinates;
varying vec3 vLightVector;
varying vec3 vViewVector;

uniform sampler2D bumpMap;
uniform sampler2D texture;

void main() {

	gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}

