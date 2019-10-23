#version 120

varying vec3 n;

void main () {
	// shading darker based on angle to eye
	vec3 n_eye = normalize (n);

	gl_FragColor = vec4 (n_eye, 1.0);
}
