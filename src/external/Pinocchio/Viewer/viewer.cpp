/**
 * From the OpenGL Programming wikibook: http://en.wikibooks.org/wiki/OpenGL_Programming
 * This file is in the public domain.
 * Contributors: Sylvain Beucler
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

/* Use glew.h instead of gl.h to get all the GL prototypes declared */
#include <GL/glew.h>

/* Using the GLUT library for the base windowing setup */
#include <GL/glut.h>

/* GLM */
// #define GLM_MESSAGES
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_utils.h"

#define GROUND_SIZE 20

#include "model.h"

int screen_width=800, screen_height=600;
GLuint program;
GLint attribute_v_coord = -1;
GLint attribute_v_normal = -1;
GLint uniform_m = -1, uniform_v = -1, uniform_p = -1;
GLint uniform_m_3x3_inv_transp = -1, uniform_v_inv = -1;
bool compute_arcball;
int last_mx = 0, last_my = 0, cur_mx = 0, cur_my = 0;
int arcball_on = false;

AnimatedModel model;

using namespace std;

enum MODES { MODE_OBJECT, MODE_CAMERA, MODE_LIGHT, MODE_LAST } view_mode = MODE_LIGHT;
int rotY_direction = 0, rotX_direction = 0, transZ_direction = 0, strife = 0;
float speed_factor = 1;
glm::mat4 transforms[MODE_LAST];
int last_ticks = 0;

static unsigned int fps_start = glutGet(GLUT_ELAPSED_TIME);
static unsigned int fps_frames = 0;

class ObjectMesh {
    private:
        GLuint vbo_vertices, vbo_normals, ibo_elements;
    public:
        vector<glm::vec4> vertices;
        vector<glm::vec3> normals;
        vector<GLushort> elements;
        glm::mat4 object2world;

        ObjectMesh() : vbo_vertices(0), vbo_normals(0), ibo_elements(0), object2world(glm::mat4(1)) {}
        ~ObjectMesh() {
            if (vbo_vertices != 0) {
                glDeleteBuffers(1, &vbo_vertices);
            }
            if (vbo_normals != 0) {
                glDeleteBuffers(1, &vbo_normals);
            }
            if (ibo_elements != 0) {
                glDeleteBuffers(1, &ibo_elements);
            }
        }

        /**
         * Store object vertices, normals and/or elements in graphic card
         * buffers
         */
        void upload() {
            if (this->vertices.size() > 0) {
                glGenBuffers(1, &this->vbo_vertices);
                glBindBuffer(GL_ARRAY_BUFFER, this->vbo_vertices);
                glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(this->vertices[0]),
                    this->vertices.data(), GL_STATIC_DRAW);
            }

            if (this->normals.size() > 0) {
                glGenBuffers(1, &this->vbo_normals);
                glBindBuffer(GL_ARRAY_BUFFER, this->vbo_normals);
                glBufferData(GL_ARRAY_BUFFER, this->normals.size() * sizeof(this->normals[0]),
                    this->normals.data(), GL_STATIC_DRAW);
            }

            if (this->elements.size() > 0) {
                glGenBuffers(1, &this->ibo_elements);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo_elements);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->elements.size() * sizeof(this->elements[0]),
                    this->elements.data(), GL_STATIC_DRAW);
            }
        }

        /**
         * Draw the object
         */
        void draw() {
            if (this->vbo_vertices != 0) {
                glEnableVertexAttribArray(attribute_v_coord);
                glBindBuffer(GL_ARRAY_BUFFER, this->vbo_vertices);
                glVertexAttribPointer(
                    attribute_v_coord,            // attribute
                    4,                            // number of elements per vertex, here (x,y,z,w)
                    GL_FLOAT,                     // the type of each element
                    GL_FALSE,                     // take our values as-is
                    0,                            // no extra data between each position
                    0                             // offset of first element
                    );
            }

            if (this->vbo_normals != 0) {
                glEnableVertexAttribArray(attribute_v_normal);
                glBindBuffer(GL_ARRAY_BUFFER, this->vbo_normals);
                glVertexAttribPointer(
                    attribute_v_normal,           // attribute
                    3,                            // number of elements per vertex, here (x,y,z)
                    GL_FLOAT,                     // the type of each element
                    GL_FALSE,                     // take our values as-is
                    0,                            // no extra data between each position
                    0                             // offset of first element
                    );
            }

            /* Apply object's transformation matrix */
            glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(this->object2world));
            /* Transform normal vectors with transpose of inverse of upper left 3x3 model matrix (ex-gl_NormalMatrix): */
            glm::mat3 m_3x3_inv_transp = glm::transpose(glm::inverse(glm::mat3(this->object2world)));
            glUniformMatrix3fv(uniform_m_3x3_inv_transp, 1, GL_FALSE, glm::value_ptr(m_3x3_inv_transp));

            /* Push each element in buffer_vertices to the vertex shader */
            if (this->ibo_elements != 0) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo_elements);
                int size;  glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
                glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
            } else {
                glDrawArrays(GL_TRIANGLES, 0, this->vertices.size());
            }

            if (this->vbo_normals != 0) {
                glDisableVertexAttribArray(attribute_v_normal);
            }

            if (this->vbo_vertices != 0) {
                glDisableVertexAttribArray(attribute_v_coord);
            }
        }

        /**
         * Draw object bounding box
         */
        void draw_bbox() {
            if (this->vertices.size() == 0) {
                return;
            }

            // Cube 1x1x1, centered on origin
            GLfloat vertices[] = {
                -0.5, -0.5, -0.5, 1.0,
                0.5, -0.5, -0.5, 1.0,
                0.5,  0.5, -0.5, 1.0,
                -0.5,  0.5, -0.5, 1.0,
                -0.5, -0.5,  0.5, 1.0,
                0.5, -0.5,  0.5, 1.0,
                0.5,  0.5,  0.5, 1.0,
                -0.5,  0.5,  0.5, 1.0,
            };
            GLuint vbo_vertices;
            glGenBuffers(1, &vbo_vertices);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            GLushort elements[] = {
                0, 1, 2, 3,
                4, 5, 6, 7,
                0, 4, 1, 5, 2, 6, 3, 7
            };
            GLuint ibo_elements;
            glGenBuffers(1, &ibo_elements);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            GLfloat min_x, max_x, min_y, max_y, min_z, max_z;
            min_x = max_x = this->vertices[0].x;
            min_y = max_y = this->vertices[0].y;
            min_z = max_z = this->vertices[0].z;
            for (unsigned int i = 0; i < this->vertices.size(); i++) {
                if (this->vertices[i].x < min_x) { min_x = this->vertices[i].x; }
                if (this->vertices[i].x > max_x) { max_x = this->vertices[i].x; }
                if (this->vertices[i].y < min_y) { min_y = this->vertices[i].y; }
                if (this->vertices[i].y > max_y) { max_y = this->vertices[i].y; }
                if (this->vertices[i].z < min_z) { min_z = this->vertices[i].z; }
                if (this->vertices[i].z > max_z) { max_z = this->vertices[i].z; }
            }
            glm::vec3 size = glm::vec3(max_x-min_x, max_y-min_y, max_z-min_z);
            glm::vec3 center = glm::vec3((min_x+max_x)/2, (min_y+max_y)/2, (min_z+max_z)/2);
            glm::mat4 transform = glm::scale(glm::mat4(1), size) * glm::translate(glm::mat4(1), center);

            /* Apply object's transformation matrix */
            glm::mat4 m = this->object2world * transform;
            glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(m));

            glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
            glEnableVertexAttribArray(attribute_v_coord);
            glVertexAttribPointer(
                attribute_v_coord,                // attribute
                4,                                // number of elements per vertex, here (x,y,z,w)
                GL_FLOAT,                         // the type of each element
                GL_FALSE,                         // take our values as-is
                0,                                // no extra data between each position
                0                                 // offset of first element
                );

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
            glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
            glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*)(4*sizeof(GLushort)));
            glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8*sizeof(GLushort)));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            glDisableVertexAttribArray(attribute_v_coord);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glDeleteBuffers(1, &vbo_vertices);
            glDeleteBuffers(1, &ibo_elements);
        }
};

ObjectMesh ground, main_object, light_bbox;

void load_obj(const char* filename, ObjectMesh* mesh) {
    ifstream in(filename, ios::in);
    if (!in) { cerr << "Cannot open " << filename << endl; exit(1); }
    vector<int> nb_seen;

    string line;
    while (getline(in, line)) {
        if (line.substr(0,2) == "v ") {
            istringstream s(line.substr(2));
            glm::vec4 v; s >> v.x; s >> v.y; s >> v.z; v.w = 1.0;
            mesh->vertices.push_back(v);
        }
        else if (line.substr(0,2) == "f ") {
            istringstream s(line.substr(2));
            GLushort a,b,c;
            s >> a; s >> b; s >> c;
            a--; b--; c--;
            mesh->elements.push_back(a); mesh->elements.push_back(b); mesh->elements.push_back(c);
        }
        else if (line[0] == '#') { /* ignoring this line */ }
        else { /* ignoring this line */ }
    }

    mesh->normals.resize(mesh->vertices.size(), glm::vec3(0.0, 0.0, 0.0));
    nb_seen.resize(mesh->vertices.size(), 0);
    for (unsigned int i = 0; i < mesh->elements.size(); i+=3) {
        GLushort ia = mesh->elements[i];
        GLushort ib = mesh->elements[i+1];
        GLushort ic = mesh->elements[i+2];
        glm::vec3 normal = glm::normalize(glm::cross(
            glm::vec3(mesh->vertices[ib]) - glm::vec3(mesh->vertices[ia]),
            glm::vec3(mesh->vertices[ic]) - glm::vec3(mesh->vertices[ia])));

        int v[3];  v[0] = ia;  v[1] = ib;  v[2] = ic;
        for (int j = 0; j < 3; j++) {
            GLushort cur_v = v[j];
            nb_seen[cur_v]++;
            if (nb_seen[cur_v] == 1) {
                mesh->normals[cur_v] = normal;
            } else { // average
                mesh->normals[cur_v].x = mesh->normals[cur_v].x * (1.0 - 1.0/nb_seen[cur_v]) + normal.x * 1.0/nb_seen[cur_v];
                mesh->normals[cur_v].y = mesh->normals[cur_v].y * (1.0 - 1.0/nb_seen[cur_v]) + normal.y * 1.0/nb_seen[cur_v];
                mesh->normals[cur_v].z = mesh->normals[cur_v].z * (1.0 - 1.0/nb_seen[cur_v]) + normal.z * 1.0/nb_seen[cur_v];
                mesh->normals[cur_v] = glm::normalize(mesh->normals[cur_v]);
            }
        }
    }
}


int init_resources(char* model_filename, char* vshader_filename, char* fshader_filename) {
    load_obj(model_filename, &main_object);
    // mesh position initialized in init_view()

    for (int i = -GROUND_SIZE/2; i < GROUND_SIZE/2; i++) {
        for (int j = -GROUND_SIZE/2; j < GROUND_SIZE/2; j++) {
            ground.vertices.push_back(glm::vec4(i,   0.0,  j+1, 1.0));
            ground.vertices.push_back(glm::vec4(i+1, 0.0,  j+1, 1.0));
            ground.vertices.push_back(glm::vec4(i,   0.0,  j,   1.0));
            ground.vertices.push_back(glm::vec4(i,   0.0,  j,   1.0));
            ground.vertices.push_back(glm::vec4(i+1, 0.0,  j+1, 1.0));
            ground.vertices.push_back(glm::vec4(i+1, 0.0,  j,   1.0));
            for (unsigned int k = 0; k < 6; k++)
                ground.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
        }
    }

    glm::vec3 light_position = glm::vec3(0.0,  1.0,  2.0);
    light_bbox.vertices.push_back(glm::vec4(-0.1, -0.1, -0.1, 0.0));
    light_bbox.vertices.push_back(glm::vec4( 0.1, -0.1, -0.1, 0.0));
    light_bbox.vertices.push_back(glm::vec4( 0.1,  0.1, -0.1, 0.0));
    light_bbox.vertices.push_back(glm::vec4(-0.1,  0.1, -0.1, 0.0));
    light_bbox.vertices.push_back(glm::vec4(-0.1, -0.1,  0.1, 0.0));
    light_bbox.vertices.push_back(glm::vec4( 0.1, -0.1,  0.1, 0.0));
    light_bbox.vertices.push_back(glm::vec4( 0.1,  0.1,  0.1, 0.0));
    light_bbox.vertices.push_back(glm::vec4(-0.1,  0.1,  0.1, 0.0));
    light_bbox.object2world = glm::translate(glm::mat4(1), light_position);

    main_object.upload();
    ground.upload();
    light_bbox.upload();

    /* Compile and link shaders */
    GLint link_ok = GL_FALSE;
    GLint validate_ok = GL_FALSE;

    GLuint vs, fs;
    if ((vs = create_shader(vshader_filename, GL_VERTEX_SHADER))   == 0) return 0;
    if ((fs = create_shader(fshader_filename, GL_FRAGMENT_SHADER)) == 0) return 0;

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        fprintf(stderr, "glLinkProgram:");
        print_log(program);
        return 0;
    }
    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &validate_ok);
    if (!validate_ok) {
        fprintf(stderr, "glValidateProgram:");
        print_log(program);
    }

    const char* attribute_name;
    attribute_name = "v_coord";
    attribute_v_coord = glGetAttribLocation(program, attribute_name);
    if (attribute_v_coord == -1) {
        fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
        return 0;
    }
    attribute_name = "v_normal";
    attribute_v_normal = glGetAttribLocation(program, attribute_name);
    if (attribute_v_normal == -1) {
        fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
        return 0;
    }
    const char* uniform_name;
    uniform_name = "m";
    uniform_m = glGetUniformLocation(program, uniform_name);
    if (uniform_m == -1) {
        fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
        return 0;
    }
    uniform_name = "v";
    uniform_v = glGetUniformLocation(program, uniform_name);
    if (uniform_v == -1) {
        fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
        return 0;
    }
    uniform_name = "p";
    uniform_p = glGetUniformLocation(program, uniform_name);
    if (uniform_p == -1) {
        fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
        return 0;
    }
    uniform_name = "m_3x3_inv_transp";
    uniform_m_3x3_inv_transp = glGetUniformLocation(program, uniform_name);
    if (uniform_m_3x3_inv_transp == -1) {
        fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
        return 0;
    }
    uniform_name = "v_inv";
    uniform_v_inv = glGetUniformLocation(program, uniform_name);
    if (uniform_v_inv == -1) {
        fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
        return 0;
    }

    fps_start = glutGet(GLUT_ELAPSED_TIME);

    return 1;
}


void init_view() {
    main_object.object2world = glm::mat4(1);
    transforms[MODE_CAMERA] = glm::lookAt(
        glm::vec3(0.0, 0.0, 4.0),   // Eye
        glm::vec3(0.0, 0.0, 0.0),   // Direction
        glm::vec3(0.0, 1.0, 0.0));  // Up
}


void onSpecial(int key, int x, int y) {
    int modifiers = glutGetModifiers();
    if ((modifiers & GLUT_ACTIVE_ALT) == GLUT_ACTIVE_ALT) {
        strife = 1;
    } else {
        strife = 0;
    }

    if ((modifiers & GLUT_ACTIVE_SHIFT) == GLUT_ACTIVE_SHIFT) {
        speed_factor = 0.1;
    } else {
        speed_factor = 1;
    }

    switch (key) {
        case GLUT_KEY_F1:
            view_mode = MODE_OBJECT;
            break;
        case GLUT_KEY_F2:
            view_mode = MODE_CAMERA;
            break;
        case GLUT_KEY_F3:
            view_mode = MODE_LIGHT;
            break;
        case GLUT_KEY_LEFT:
            rotY_direction = 1;
            break;
        case GLUT_KEY_RIGHT:
            rotY_direction = -1;
            break;
        case GLUT_KEY_UP:
            transZ_direction = 1;
            break;
        case GLUT_KEY_DOWN:
            transZ_direction = -1;
            break;
        case GLUT_KEY_PAGE_UP:
            rotX_direction = -1;
            break;
        case GLUT_KEY_PAGE_DOWN:
            rotX_direction = 1;
            break;
        case GLUT_KEY_HOME:
            init_view();
            break;
    }
}


void onSpecialUp(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
        case GLUT_KEY_RIGHT:
            rotY_direction = 0;
            break;
        case GLUT_KEY_UP:
        case GLUT_KEY_DOWN:
            transZ_direction = 0;
            break;
        case GLUT_KEY_PAGE_UP:
        case GLUT_KEY_PAGE_DOWN:
            rotX_direction = 0;
            break;
    }
}


/**
 * Get a normalized vector from the center of the virtual ball O to a
 * point P on the virtual ball surface, such that P is aligned on
 * screen's (X,Y) coordinates.  If (X,Y) is too far away from the
 * sphere, return the nearest point on the virtual ball surface.
 */
glm::vec3 get_arcball_vector(int x, int y) {
    glm::vec3 P = glm::vec3(1.0*x/screen_width*2 - 1.0,
        1.0*y/screen_height*2 - 1.0,
        0);
    P.y = -P.y;
    float OP_squared = P.x * P.x + P.y * P.y;
    if (OP_squared <= 1*1) {
        P.z = sqrt(1*1 - OP_squared);             // Pythagore
    } else {
        P = glm::normalize(P);                    // nearest point
    }
    return P;
}


void logic() {
    /* FPS count */
    fps_frames++;
    int delta_time = glutGet(GLUT_ELAPSED_TIME) - fps_start;
    if (delta_time > 1000) {
        cout << 1000.0 * fps_frames / delta_time << endl;
        fps_frames = 0;
        fps_start = glutGet(GLUT_ELAPSED_TIME);
    }


    /* Handle keyboard-based transformations */
    int delta_t = glutGet(GLUT_ELAPSED_TIME) - last_ticks;
    last_ticks = glutGet(GLUT_ELAPSED_TIME);

    float delta_transZ = transZ_direction * delta_t / 1000.0 * 5 * speed_factor; // 5 units per second
    float delta_transX = 0, delta_transY = 0, delta_rotY = 0, delta_rotX = 0;
    if (strife) {
        delta_transX = rotY_direction * delta_t / 1000.0 * 3 * speed_factor; // 3 units per second
        delta_transY = rotX_direction * delta_t / 1000.0 * 3 * speed_factor; // 3 units per second
    } else {
        delta_rotY =  rotY_direction * delta_t / 1000.0 * 120 * speed_factor; // 120° per second
        delta_rotX = -rotX_direction * delta_t / 1000.0 * 120 * speed_factor; // 120° per second
    }


    if (view_mode == MODE_OBJECT) {

        main_object.object2world = glm::rotate(main_object.object2world, glm::radians(delta_rotY), glm::vec3(0.0, 1.0, 0.0));
        main_object.object2world = glm::rotate(main_object.object2world, glm::radians(delta_rotX), glm::vec3(1.0, 0.0, 0.0));
        main_object.object2world = glm::translate(main_object.object2world, glm::vec3(0.0, 0.0, delta_transZ));

    } else if (view_mode == MODE_CAMERA) {

        // Camera is reverse-facing, so reverse Z translation and X rotation.
        // Plus, the View matrix is the inverse of the camera2world (it's
        // world->camera), so we'll reverse the transformations.
        // Alternatively, imagine that you transform the world, instead of positioning the camera.

        if (strife) {
            transforms[MODE_CAMERA] = glm::translate(glm::mat4(1.0), glm::vec3(delta_transX, 0.0, 0.0)) * transforms[MODE_CAMERA];
        } else {
            glm::vec3 y_axis_world = glm::mat3(transforms[MODE_CAMERA]) * glm::vec3(0.0, 1.0, 0.0);
            transforms[MODE_CAMERA] = glm::rotate(glm::mat4(1.0), glm::radians(-delta_rotY), y_axis_world) * transforms[MODE_CAMERA];
        }

        if (strife) {
            transforms[MODE_CAMERA] = glm::translate(glm::mat4(1.0), glm::vec3(0.0, delta_transY, 0.0)) * transforms[MODE_CAMERA];
        } else {
            transforms[MODE_CAMERA] = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, delta_transZ)) * transforms[MODE_CAMERA];
        }

        transforms[MODE_CAMERA] = glm::rotate(glm::mat4(1.0), glm::radians(delta_rotX), glm::vec3(1.0, 0.0, 0.0)) * transforms[MODE_CAMERA];

    }


    /* Handle arcball */
    if (cur_mx != last_mx || cur_my != last_my) {
        glm::vec3 va = get_arcball_vector(last_mx, last_my);
        glm::vec3 vb = get_arcball_vector( cur_mx,  cur_my);
        float angle = acos(min(1.0f, glm::dot(va, vb)));
        glm::vec3 axis_in_camera_coord = glm::cross(va, vb);
        glm::mat3 camera2object = glm::inverse(glm::mat3(transforms[MODE_CAMERA]) * glm::mat3(main_object.object2world));
        glm::vec3 axis_in_object_coord = camera2object * axis_in_camera_coord;
        main_object.object2world = glm::rotate(main_object.object2world, angle, axis_in_object_coord);
        last_mx = cur_mx;
        last_my = cur_my;
    }


    // Model
    // Set in onDisplay() - cf. main_object.object2world

    // View
    glm::mat4 world2camera = transforms[MODE_CAMERA];

    // Projection
    glm::mat4 camera2screen = glm::perspective(45.0f, 1.0f*screen_width/screen_height, 0.1f, 100.0f);

    glUseProgram(program);
    glUniformMatrix4fv(uniform_v, 1, GL_FALSE, glm::value_ptr(world2camera));
    glUniformMatrix4fv(uniform_p, 1, GL_FALSE, glm::value_ptr(camera2screen));

    glm::mat4 v_inv = glm::inverse(world2camera);
    glUniformMatrix4fv(uniform_v_inv, 1, GL_FALSE, glm::value_ptr(v_inv));

    glutPostRedisplay();
}


void draw() {
    glClearColor(0.45, 0.45, 0.45, 1.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);

    main_object.draw();
    ground.draw();

    model.drawModel();

    light_bbox.draw_bbox();
}


void onDisplay() {
    logic();
    draw();
    glutSwapBuffers();
}


void onMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        arcball_on = true;
        last_mx = cur_mx = x;
        last_my = cur_my = y;
    } else {
        arcball_on = false;
    }
}


void onMotion(int x, int y) {
    if (arcball_on) { // if left button is pressed
        cur_mx = x;
        cur_my = y;
    }
}


void onReshape(int width, int height) {
    screen_width = width;
    screen_height = height;
    glViewport(0, 0, screen_width, screen_height);
}


void free_resources() {
    glDeleteProgram(program);
}


int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA|GLUT_ALPHA|GLUT_DOUBLE|GLUT_DEPTH);
    glutInitWindowSize(screen_width, screen_height);
    glutCreateWindow("OBJ viewer");

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
        return 1;
    }

    if (!GLEW_VERSION_2_0) {
        fprintf(stderr, "Error: your graphic card does not support OpenGL 2.0\n");
        return 1;
    }

    char* obj_filename = (char*) "objects/test.obj";
    char* v_shader_filename = (char*) "shaders/phong-shading.v.glsl";
    char* f_shader_filename = (char*) "shaders/phong-shading.f.glsl";
    if (argc != 4) {
        fprintf(stderr, "Usage: %s model.obj vertex_shader.v.glsl fragment_shader.f.glsl\n", argv[0]);
    } else {
        obj_filename = argv[1];
        v_shader_filename = argv[2];
        f_shader_filename = argv[3];
    }

    model.loadObject(obj_filename, "paths/walk.txt");

    if (init_resources(obj_filename, v_shader_filename, f_shader_filename)) {
        init_view();
        glutDisplayFunc(onDisplay);
        glutSpecialFunc(onSpecial);
        glutSpecialUpFunc(onSpecialUp);
        glutMouseFunc(onMouse);
        glutMotionFunc(onMotion);
        glutReshapeFunc(onReshape);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        //glDepthFunc(GL_LEQUAL);
        //glDepthRange(1, 0);
        last_ticks = glutGet(GLUT_ELAPSED_TIME);
        glutMainLoop();
    }

    free_resources();
    return 0;
}
