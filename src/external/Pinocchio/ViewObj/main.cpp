//
// Obj Viewer in OpenGL 2.1
// Anton Gerdelan
// 21 Dec 2014
//

#include "maths_funcs.h"

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <fstream>
#include <cstdlib>
#include <ctime>

#include "../Pinocchio/skeleton.h"
#include "../Pinocchio/utils.h"
#include "../Pinocchio/debugging.h"
#include "../Pinocchio/attachment.h"
#include "../Pinocchio/pinocchioApi.h"

#include "Model.h"

// This will generate a number from LO to HI, inclusive.
#define RANDOM_FLOAT(LO,HI) (static_cast <float> (LO) + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(static_cast <float> (HI) - static_cast <float> (LO)))))

using namespace Pinocchio;

AnimatedModel model;

//
// for parsing CL params
int my_argc;
char ** my_argv;

//
// dimensions of the window drawing surface
int gl_width = 800;
int gl_height = 800;

// shaders to use
char vs_file_name[PATH_MAX];
char fs_file_name[PATH_MAX];

// obj to load
char obj_file_name[PATH_MAX];

// texture file
char texture_file_name[PATH_MAX];

// built-in anti-aliasing to smooth jagged diagonal edges of polygons
int msaa_samples = 16;
// NOTE: if too high grainy crap appears on polygon edges

//
// check CL params for string. if found return argc value
// returns 0 if not present
// i stole this code from DOOM
int check_param (const char* s) {
	int i;

	for (i = 1; i < my_argc; i++) {
		if (!strcasecmp (s, my_argv[i])) {
			return i;
		}
	}

	return 0;
}

//
// Copy a shader from a plain text file into a character array
bool parse_file_into_str (const char* file_name, char** shader_str) {
	FILE* file;
	long sz;
	char line[2048];

	printf ("parsing %s\n", file_name);
	line[0] = '\0';

	file = fopen (file_name , "r");
	if (!file) {
		fprintf (stderr, "ERROR: opening file for reading: %s\n", file_name);
		return false;
	}

	// Get file size and allocate memory for string
	assert (0 == fseek (file, 0, SEEK_END));
	sz = ftell (file) + 1; // +1 for \0
	rewind (file);

	// +1 for line ending or sthng at end
	*shader_str = (char*)malloc (sz);
	*shader_str[0] = '\0';

	while (!feof (file)) {
		if (fgets (line, 2048, file)) {
			strcat (*shader_str, line);
		}
	}

	return true;
}

//
// take screenshot with F11
bool screencapture () {
	unsigned char* buffer = (unsigned char*)malloc (gl_width * gl_height * 3);
	glReadPixels (0, 0, gl_width, gl_height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	char name[1024];
	long int t = time (NULL);
	sprintf (name, "screenshot_%ld.png", t);
	unsigned char* last_row = buffer + (gl_width * 3 * (gl_height - 1));
	if (!stbi_write_png (name, gl_width, gl_height, 3, last_row, -3 * gl_width)) {
		fprintf (stderr, "ERROR: could not write screenshot file %s\n", name);
	}
	free (buffer);
	return true;
}

int main (int argc, char** argv) {
	GLFWwindow* window = NULL;
	const GLubyte* renderer;
	const GLubyte* version;
	GLuint shader_programme, normals_sp;
	int M_loc, V_loc, P_loc, time_loc;
	int normals_M_loc, normals_V_loc, normals_P_loc;
	int param = 0;
	float scalef = 1.0f;
	double prev;
	vec3 vtra = vec3 (0.0f, 0.0f, 0.0f);
	char win_title[PATH_MAX + 16];
	bool normals_mode = false;
	bool npressed = false;
	bool f11pressed = false;
	bool ppressed = false;
	int poly_mode = 0;

	// Seed the random number generator
	srand (static_cast <unsigned> (time(0)));

	my_argc = argc;
	my_argv = argv;

	param = check_param ("--help");
	if (param) {
		printf ("\nOpenGL .obj Viewer.\nAnton Gerdelan 21 Dec 2014 @capnramses\n\n");
		printf ("usage: ./viewer [-o FILE] [-t FILE] [-vs FILE] [-fs FILE]\n\n");
		printf ("--help\t\t\tthis text\n");
		printf ("-o FILE\t\t\t.obj to load\n");
		printf ("-sca FLOAT\t\tscale mesh uniformly by this factor\n");
		printf ("-tra FLOAT FLOAT FLOAT\ttranslate mesh by X Y Z\n");
		printf ("-tex FILE\t\timage to use as texture\n");
		printf ("-vs FILE\t\tvertex shader to use\n");
		printf ("-fs FILE\t\tfragment shader to use\n");
		printf ("\n");
		printf ("F11\t\t\tscreenshot\n");
		printf ("n\t\t\ttoggle normals visualisation\n");
		printf ("\n");
		return 0;
	}

	param = check_param ("-o");
	if (param && my_argc > param + 1) {
		strcpy (obj_file_name, argv[param + 1]);
	} else {
		strcpy (obj_file_name, "models/cube.obj");
	}

	param = check_param ("-vs");
	if (param && my_argc > param + 1) {
		strcpy (vs_file_name, argv[param + 1]);
	} else {
		strcpy (vs_file_name, "shaders/basic.vert");
	}

	param = check_param ("-fs");
	if (param && my_argc > param + 1) {
		strcpy (fs_file_name, argv[param + 1]);
	} else {
		strcpy (fs_file_name, "shaders/basic.frag");
	}

	param = check_param ("-sca");
	if (param && my_argc > param + 1) {
		scalef = atof (argv[param + 1]);
	}

	param = check_param ("-tra");
	if (param && my_argc > param + 3) {
		vtra.v[0] = atof (argv[param + 1]);
		vtra.v[1] = atof (argv[param + 2]);
		vtra.v[2] = atof (argv[param + 3]);
	}

	param = check_param ("-tex");
	if (param && my_argc > param + 1) {
		strcpy (texture_file_name, argv[param + 1]);
	} else {
		strcpy (texture_file_name, "textures/checkerboard.png");
	}

	//
	// Start OpenGL using helper libraries
	// --------------------------------------------------------------------------
	if (!glfwInit ()) {
		fprintf (stderr, "ERROR: could not start GLFW3\n");
		return 1;
	} 

	glfwWindowHint (GLFW_SAMPLES, msaa_samples);

	sprintf (win_title, "obj viewer: %s", obj_file_name);
	window = glfwCreateWindow (gl_width, gl_height, win_title, NULL, NULL);
	if (!window) {
		fprintf (stderr, "ERROR: opening OS window\n");
		return 1;
	}
	glfwMakeContextCurrent (window);

	glewExperimental = GL_TRUE;
	glewInit ();

	renderer = glGetString (GL_RENDERER);
	version = glGetString (GL_VERSION);
	printf ("Renderer: %s\n", renderer);
	printf ("OpenGL version supported %s\n", version);

	//
	// Prepare model
	// --------------------------------------------------------------------------

	const char * obj_filename = (char*) "objects/test.obj";
	model.loadObject(obj_filename, "paths/walk.txt");

	//
	// Load shaders from files
	// --------------------------------------------------------------------------

	char * vertex_shader_str = NULL; // Vertex shader code
	char * fragment_shader_str = NULL; // Fragment shader code
	GLuint vs = 0; // Vertex shader id
	GLuint fs = 0; // Fragment shader id

	// Load shader strings from text files
	assert (parse_file_into_str (vs_file_name, &vertex_shader_str));
	assert (parse_file_into_str (fs_file_name, &fragment_shader_str));
	vs = glCreateShader (GL_VERTEX_SHADER);
	fs = glCreateShader (GL_FRAGMENT_SHADER);
	glShaderSource (vs, 1, (const char**)&vertex_shader_str, NULL);
	glShaderSource (fs, 1, (const char**)&fragment_shader_str, NULL);

	// Free memory
	free (vertex_shader_str);
	vertex_shader_str = NULL;
	free (fragment_shader_str);
	fragment_shader_str = NULL;

	// Compile shaders
	glCompileShader (vs);
	glCompileShader (fs);
	shader_programme = glCreateProgram ();
	glAttachShader (shader_programme, fs);
	glAttachShader (shader_programme, vs);
	glBindAttribLocation (shader_programme, 0, "vp");
	glBindAttribLocation (shader_programme, 1, "vt");
	glBindAttribLocation (shader_programme, 2, "vn");
	glLinkProgram (shader_programme);
	M_loc = glGetUniformLocation (shader_programme, "M");
	V_loc = glGetUniformLocation (shader_programme, "V");
	P_loc = glGetUniformLocation (shader_programme, "P");

	// Attempt this. Won't use if < 0
	time_loc = glGetUniformLocation (shader_programme, "time");

	// Load shader strings from text files
	assert (parse_file_into_str ("shaders/normals.vert", &vertex_shader_str));
	assert (parse_file_into_str ("shaders/normals.frag", &fragment_shader_str));
	vs = glCreateShader (GL_VERTEX_SHADER);
	fs = glCreateShader (GL_FRAGMENT_SHADER);
	glShaderSource (vs, 1, (const char**)&vertex_shader_str, NULL);
	glShaderSource (fs, 1, (const char**)&fragment_shader_str, NULL);

	// Free memory
	free (vertex_shader_str);
	vertex_shader_str = NULL;
	free (fragment_shader_str);
	fragment_shader_str = NULL;

	// Compile shaders
	glCompileShader (vs);
	glCompileShader (fs);
	normals_sp = glCreateProgram ();
	glAttachShader (normals_sp, fs);
	glAttachShader (normals_sp, vs);
	glBindAttribLocation (normals_sp, 0, "vp");
	glBindAttribLocation (normals_sp, 1, "vt");
	glBindAttribLocation (normals_sp, 2, "vn");
	glLinkProgram (normals_sp);
	normals_M_loc = glGetUniformLocation (normals_sp, "M");
	normals_V_loc = glGetUniformLocation (normals_sp, "V");
	normals_P_loc = glGetUniformLocation (normals_sp, "P");

	//
	// Create some matrices
	// --------------------------------------------------------------------------

	mat4 M, V, P, S, T;
	vec3 cam_pos (0.0, 0.0, 5.0);
	vec3 targ_pos (0.0, 0.0, 0.0);
	vec3 up (0.0, 1.0, 0.0);

	T = translate(identity_mat4 (), vtra);
	S = scale(identity_mat4 (), vec3 (scalef, scalef, scalef));
	M = T * S;
	V = look_at (cam_pos, targ_pos, up);
	P = perspective (67.0f, (float)gl_width / (float)gl_height, 0.1, 1000.0);

	// Send matrix values to shader immediately
	glUseProgram (shader_programme);
	glUniformMatrix4fv (M_loc, 1, GL_FALSE, M.m);
	glUniformMatrix4fv (V_loc, 1, GL_FALSE, V.m);
	glUniformMatrix4fv (P_loc, 1, GL_FALSE, P.m);

	glUseProgram (normals_sp);
	glUniformMatrix4fv (normals_M_loc, 1, GL_FALSE, M.m);
	glUniformMatrix4fv (normals_V_loc, 1, GL_FALSE, V.m);
	glUniformMatrix4fv (normals_P_loc, 1, GL_FALSE, P.m);

	//
	// Create texture
	// --------------------------------------------------------------------------

	int x,y,n;
	unsigned char* data;
	GLuint tex;

	data = stbi_load (texture_file_name, &x, &y, &n, 4);
	if (!data) {
		fprintf (stderr, "ERROR: could not load image %s\n", texture_file_name);
		return 1;
	}
	printf ("loaded image with %ix%ipx and %i chans\n", x, y, n);

	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) { // NPOT Check
		fprintf (stderr, "WARNING: texture is not power-of-two dimensions %s\n",
			texture_file_name);
	}

	// Make upside-down copy for OpenGL

	unsigned char *imagePtr = &data[0];
	int halfTheHeightInPixels = y / 2;
	int heightInPixels = y;

	int numColorComponents = 4; // Assuming RGBA for 4 components per pixel.
	int widthInChars = x * numColorComponents; // Assuming each color component is an unsigned char.
	unsigned char * ptr_top = NULL;
	unsigned char * ptr_bottom = NULL;
	unsigned char temp = 0;
	for (int h = 0; h < halfTheHeightInPixels; h++) {
		ptr_top = imagePtr + h * widthInChars;
		ptr_bottom = imagePtr + (heightInPixels - h - 1) * widthInChars;
		for (int w = 0; w < widthInChars; w++) {
			// Swap the chars around.
			temp = *ptr_top;
			*ptr_top = *ptr_bottom;
			*ptr_bottom = temp;
			++ptr_top;
			++ptr_bottom;
		}
	}

	// Set up texture 0 in OpenGL

	glGenTextures (1, &tex);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, tex);
	glTexImage2D (
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		x,
		y,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		data
	);
	stbi_image_free(data);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//
	// Main loop
	// --------------------------------------------------------------------------


	//
	// Set up vertex buffers and vertex array object
	// --------------------------------------------------------------------------

	int point_count = 0;
	const GLfloat * vp = NULL; // array of vertex points (3 numbers per vertex)
	const GLfloat * vn = NULL; // array of vertex normals (3 numbers per vertex)
	const GLfloat * vt = NULL; // array of texture coordinates (2 numbers per vertex)

	int bones_count = 0;
	const GLfloat * bp = NULL; // array of bone line points (6 numbers per line)

	//
	// Start rendering
	// --------------------------------------------------------------------------
	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LESS);
	glClearColor (0.5, 0.5, 0.8, 1.0);

	glEnable (GL_CULL_FACE); // enable culling of faces
	glCullFace (GL_BACK);
	glFrontFace (GL_CCW);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDepthMask (GL_FALSE);

	float a = 0.0f;

	prev = glfwGetTime ();
	while (!glfwWindowShouldClose (window)) {
		double curr, elapsed;

		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport (0, 0, gl_width, gl_height);

		curr = glfwGetTime ();
		elapsed = curr - prev;
		prev = curr;

		a += sinf (elapsed * 50.0f);
		M = T * rotate_y_deg (S, a);


		// Copy points from the header file into our VBO on graphics hardware

		model.getModelShape(&point_count, &vp, &vn, &vt, &bones_count, &bp);

		GLuint points_vbo;
		glGenBuffers (1, &points_vbo);
		glBindBuffer (GL_ARRAY_BUFFER, points_vbo);
		glBufferData (GL_ARRAY_BUFFER, sizeof(float) * 3 * point_count, vp, GL_STATIC_DRAW);

		GLuint texcoord_vbo;
		glGenBuffers (1, &texcoord_vbo);
		glBindBuffer (GL_ARRAY_BUFFER, texcoord_vbo);
		glBufferData (GL_ARRAY_BUFFER, sizeof(float) * 2 * point_count, vt, GL_STATIC_DRAW);

		GLuint normals_vbo;
		glGenBuffers (1, &normals_vbo);
		glBindBuffer (GL_ARRAY_BUFFER, normals_vbo);
		glBufferData (GL_ARRAY_BUFFER, sizeof(float) * 3 * point_count, vn, GL_STATIC_DRAW);

		GLuint vao;
		glGenVertexArrays (1, &vao);
		glBindVertexArray (vao);

		glEnableVertexAttribArray (0);
		glBindBuffer (GL_ARRAY_BUFFER, points_vbo);
		glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		glEnableVertexAttribArray (1);
		glBindBuffer (GL_ARRAY_BUFFER, texcoord_vbo);
		glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		glEnableVertexAttribArray (2);
		glBindBuffer (GL_ARRAY_BUFFER, normals_vbo);
		glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		// Run previously compiled shader programs

		glBindTexture (GL_TEXTURE_2D, tex);

		if (normals_mode) {
			glUseProgram (normals_sp);
			glUniformMatrix4fv (normals_M_loc, 1, GL_FALSE, M.m);
		} else {
			glUseProgram (shader_programme);
			glUniformMatrix4fv (M_loc, 1, GL_FALSE, M.m);
			if (time_loc > 0) {
				glUniform1f (time_loc, (float)curr);
			}
		}

		glBindVertexArray (vao);
		glDrawArrays (GL_TRIANGLES, 0, point_count);

		// Draw skeleton bones on top

		glUseProgram (normals_sp);
		glUniformMatrix4fv (normals_M_loc, 1, GL_FALSE, M.m);

		glDisable (GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, 0); // Bind the default (empty) texture

		glLineWidth(5);
		glColor3d(0.5, 0.0, 0.0);
		glBegin(GL_LINES);
		for (int j = 0; j < bones_count; ++j) {
			glVertex3d(bp[j*6+0], bp[j*6+1], bp[j*6+2]);
			glVertex3d(bp[j*6+3], bp[j*6+4], bp[j*6+5]);
		}
		glEnd();

		glEnable (GL_DEPTH_TEST);

		// Swap buffers and poll events

		glfwPollEvents ();
		glfwSwapBuffers (window);

		if (GLFW_PRESS == glfwGetKey (window, GLFW_KEY_N)) {
			if (!npressed) {
				npressed = true;
				normals_mode = !normals_mode;
			}
		} else {
			npressed = false;
		}

		if (GLFW_PRESS == glfwGetKey (window, GLFW_KEY_P)) {
			if (!ppressed) {
				ppressed = true;
				poly_mode++;
				poly_mode = poly_mode % 3;

				if (0 == poly_mode) {
					glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
				} else if (1 == poly_mode) {
					glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
				} else {
					glPolygonMode (GL_FRONT_AND_BACK, GL_POINT);
				}
			}
		} else {
			ppressed = false;
		}

		if (GLFW_PRESS == glfwGetKey (window, GLFW_KEY_F11)) {
			if (!f11pressed) {
				f11pressed = true;
				screencapture ();
			}
		} else {
			f11pressed = false;
		}

		if (GLFW_PRESS == glfwGetKey (window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose (window, 1);
		}
	}


	return 0;
}
