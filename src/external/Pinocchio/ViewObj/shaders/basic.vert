#version 120

attribute vec3 vp; // points
attribute vec2 vt; // tex coords
attribute vec3 vn; // normals

uniform mat4 M, V, P;

varying vec2 st;
varying vec3 n, p;

void main () {
	st = vt;
	n = vec3 (V * M * vec4 (vn, 0.0));
	p = vec3 (V * M * vec4 (vp, 1.0));
	gl_Position = P * V * M * vec4 (vp, 1.0);
}
