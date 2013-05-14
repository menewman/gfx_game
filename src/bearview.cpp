// Source file for the scene file viewer
//# define cygwin         // comment out to compile in cygwin
                          // without glew and openAL

////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////

# ifndef cygwin
#   include "cos426_opengl.h"
# else
#   include "cos426_opengl_cyg.h"
# endif

#include "R3/R3.h"
#include "R3Scene.h"
#include "particle.h"
#include "Bear.h"
#include "Prey.h"
#include "Hunter.h"
#include <errno.h>
#include <sstream>
#include <iostream>
#include <fstream>
#define R3Rgb R2Pixel
#define PI 3.14159265
#define TOLERANCE 0.0001
#define BOUND 500
#define NUM_BUFFERS 1
#define NUM_SOURCES 1
#define NUM_ENVIRONMENTS 1

////////////////////////////////////////////////////////////
// GLOBAL CONSTANTS
////////////////////////////////////////////////////////////

static const double VIDEO_FRAME_DELAY = 1./25.; // 25 FPS

////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
////////////////////////////////////////////////////////////

// Program arguments

static char *input_scene_name = NULL;
static char *output_image_name = NULL;
static const char *video_prefix = "./video-frames/";
static int integration_type = EULER_INTEGRATION;

// Display variables

static R3Scene *scene = NULL;
static R3Camera camera;
static Bear player;
static vector<Prey> prey_list;
static vector<Hunter> hunter_list;
//static vector<Bearpaw> bearpaw_list;
#ifndef cygwin
static vector<ALuint> source_list;
#endif
static int show_faces = 1;
static int show_edges = 0;
static int show_bboxes = 0;
static int show_lights = 0;
static int show_camera = 0;
static int show_particles = 1;
static int show_particle_springs = 1;
static int show_particle_sources_and_sinks = 1;
static int save_image = 0;
static int save_video = 0;
static int num_frames_to_record = -1;
static int quit = 0;
static int move_forward = 0;
static int move_backward = 0;
static int move_left = 0;
static int move_right = 0;
static int move_jump = 0;
static int turn_left = 0;
static int turn_right = 0;
static int swiped = 0;
static double sprint = 1;
static int menu = 0;

// GLUT variables

static int GLUTwindow = 0;
static int GLUTwindow_height = 512;
static int GLUTwindow_width = 512;
static int GLUTmouse[2] = { 0, 0 };
static int GLUTbutton[3] = { 0, 0, 0 };
static int GLUTmodifiers = 0;

// Framebuffer parameters

unsigned int fbo; // The frame buffer object
unsigned int fbo_depth; // The depth buffer for the frame buffer object
unsigned int fbo_texture; // The texture object to write our frame buffer object to
int window_width = 500; // The width of our window
int window_height = 500; // The height of our window

// GLUT command list

enum {
    DISPLAY_FACE_TOGGLE_COMMAND,
    DISPLAY_EDGE_TOGGLE_COMMAND,
    DISPLAY_BBOXES_TOGGLE_COMMAND,
    DISPLAY_LIGHTS_TOGGLE_COMMAND,
    DISPLAY_CAMERA_TOGGLE_COMMAND,
    DISPLAY_PARTICLES_TOGGLE_COMMAND,
    DISPLAY_PARTICLE_SPRINGS_TOGGLE_COMMAND,
    DISPLAY_PARTICLE_SOURCES_AND_SINKS_TOGGLE_COMMAND,
    SAVE_IMAGE_COMMAND,
    SAVE_VIDEO_COMMAND,
    QUIT_COMMAND,
};


////////////////////////////////////////////////////////////
// TIMER CODE
////////////////////////////////////////////////////////////

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/time.h>
#endif

static double GetTime(void)
{
#ifdef _WIN32
    // Return number of seconds since start of execution
    static int first = 1;
    static LARGE_INTEGER timefreq;
    static LARGE_INTEGER start_timevalue;

    // Check if this is the first time
    if (first) {
        // Initialize first time
        QueryPerformanceFrequency(&timefreq);
        QueryPerformanceCounter(&start_timevalue);
        first = 0;
        return 0;
    }
    else {
        // Return time since start
        LARGE_INTEGER current_timevalue;
        QueryPerformanceCounter(&current_timevalue);
        return ((double) current_timevalue.QuadPart -
                (double) start_timevalue.QuadPart) /
            (double) timefreq.QuadPart;
    }
#else
    // Return number of seconds since start of execution
    static int first = 1;
    static struct timeval start_timevalue;

    // Check if this is the first time
    if (first) {
        // Initialize first time
        gettimeofday(&start_timevalue, NULL);
        first = 0;
        return 0;
    }
    else {
        // Return time since start
        struct timeval current_timevalue;
        gettimeofday(&current_timevalue, NULL);
        int secs = current_timevalue.tv_sec - start_timevalue.tv_sec;
        int usecs = current_timevalue.tv_usec - start_timevalue.tv_usec;
        return (double) (secs + 1.0E-6F * usecs);
    }
#endif
}



////////////////////////////////////////////////////////////
// SCENE DRAWING CODE
////////////////////////////////////////////////////////////

void DrawTextBox()
{
    int XSize = glutGet(GLUT_WINDOW_WIDTH);
    int YSize = glutGet(GLUT_WINDOW_HEIGHT);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho (0, XSize, YSize, 0, 0, 1);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glColor3d(1, 1, 1);
    glBegin(GL_QUADS);
    // Top left corner of the screen is (0, 0)
    glVertex2f(150, YSize - 7);     // Bottom Right
    glVertex2f(7, YSize - 7);       //Bottom Left
    glVertex2f(7, YSize - 50);          //Top Left
    glVertex2f(150, YSize - 50);       //Top Right
    glEnd();

    glColor3d(0, 0, 0);
    glRasterPos2i(10, YSize - 25);

    const char *menuString = "Menu";
    for (int i = 0; i < 4; i++) {
      glutBitmapCharacter(GLUT_BITMAP_8_BY_13, menuString[i]);
    }

    if (menu == 1) {
        glColor3d(1, 1, 1);
        glBegin(GL_QUADS);
        glVertex2f(150, YSize - 50);     // Bottom Right
        glVertex2f(7, YSize - 50);       //Bottom Left
        glVertex2f(7, YSize - 100);          //Top Left
        glVertex2f(150, YSize - 100);       //Top Right
        glEnd();

        glLineWidth(4);
        glColor3d(0, 0, 0);
        glBegin(GL_LINES);
        glVertex2f(150, YSize - 50);
        glVertex2f(7, YSize - 50);
        glEnd();

        char *quitString = (char *)"Quit";
        glRasterPos2i(10, YSize - 68);
	for (int i = 0; i < 4; i++) {
	  glutBitmapCharacter(GLUT_BITMAP_8_BY_13, quitString[i]);
	}

        glColor3d(1, 1, 1);
        glBegin(GL_QUADS);
        glVertex2f(150, YSize - 100);     // Bottom Right
        glVertex2f(7, YSize - 100);       //Bottom Left
        glVertex2f(7, YSize - 150);          //Top Left
        glVertex2f(150, YSize - 150);       //Top Right
        glEnd();

        glLineWidth(4);
        glColor3d(0, 0, 0);
        glBegin(GL_LINES);
        glVertex2f(150, YSize - 100);
        glVertex2f(7, YSize - 100);
        glEnd();

	glRasterPos2i(10, YSize - 118);

	// convert player health to string for display
	std::ostringstream strs;
	strs << player.health;
	std::string str = strs.str();

	std::string health_str = "health: ";
	std::string display_str = health_str + str;

	for(std::string::size_type i = 0; i < display_str.size(); ++i) {
	  glutBitmapCharacter(GLUT_BITMAP_8_BY_13, display_str[i]);
	}

    }
    glEnable(GL_DEPTH_TEST);
}


void DrawShape(R3Shape *shape)
{
    // Check shape type
    if (shape->type == R3_BOX_SHAPE) shape->box->Draw();
    else if (shape->type == R3_SPHERE_SHAPE) shape->sphere->Draw();
    else if (shape->type == R3_CYLINDER_SHAPE) shape->cylinder->Draw();
    else if (shape->type == R3_CONE_SHAPE) shape->cone->Draw();
    else if (shape->type == R3_MESH_SHAPE) shape->mesh->Draw();
    else if (shape->type == R3_SEGMENT_SHAPE) shape->segment->Draw();
    else if (shape->type == R3_CIRCLE_SHAPE) shape->circle->Draw();
    else fprintf(stderr, "Unrecognized shape type: %d\n", shape->type);
}



void LoadMatrix(R3Matrix *matrix)
{
    // Multiply matrix by top of stack
    // Take transpose of matrix because OpenGL represents vectors with
    // column-vectors and R3 represents them with row-vectors
    R3Matrix m = matrix->Transpose();
    glMultMatrixd((double *) &m);
}



void LoadMaterial(R3Material *material)
{
    GLfloat c[4];

    // Check if same as current
    static R3Material *current_material = NULL;
    if (material == current_material) return;
    current_material = material;

    // Compute "opacity"
    double opacity = 1 - material->kt.Luminance();

    // Load ambient
    c[0] = material->ka[0];
    c[1] = material->ka[1];
    c[2] = material->ka[2];
    c[3] = opacity;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

    // Load diffuse
    c[0] = material->kd[0];
    c[1] = material->kd[1];
    c[2] = material->kd[2];
    c[3] = opacity;
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

    // Load specular
    c[0] = material->ks[0];
    c[1] = material->ks[1];
    c[2] = material->ks[2];
    c[3] = opacity;
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

    // Load emission
    c[0] = material->emission.Red();
    c[1] = material->emission.Green();
    c[2] = material->emission.Blue();
    c[3] = opacity;
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

    // Load shininess
    c[0] = material->shininess;
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, c[0]);

    // Load texture
    if (material->texture) {
        if (material->texture_index <= 0) {
            // Create texture in OpenGL
            GLuint texture_index;
            glGenTextures(1, &texture_index);
            material->texture_index = (int) texture_index;
            glBindTexture(GL_TEXTURE_2D, material->texture_index);
            R2Image *image = material->texture;
            int npixels = image->NPixels();
            R2Pixel *pixels = image->Pixels();
            GLfloat *buffer = new GLfloat [ 4 * npixels ];
            R2Pixel *pixelsp = pixels;
            GLfloat *bufferp = buffer;
            for (int j = 0; j < npixels; j++) {
                *(bufferp++) = pixelsp->Red();
                *(bufferp++) = pixelsp->Green();
                *(bufferp++) = pixelsp->Blue();
                *(bufferp++) = pixelsp->Alpha();
                pixelsp++;
            }
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glTexImage2D(GL_TEXTURE_2D, 0, 4, image->Width(), image->Height(), 0, GL_RGBA, GL_FLOAT, buffer);
            delete [] buffer;
        }

        // Select texture
        glBindTexture(GL_TEXTURE_2D, material->texture_index);
        glEnable(GL_TEXTURE_2D);
    }
    else {
        glDisable(GL_TEXTURE_2D);
    }

    // Enable blending for transparent surfaces
    if (opacity < 1) {
        glDepthMask(false);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
    }
    else {
        glDisable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ZERO);
        glDepthMask(true);
    }
}

/*
void swipe()  //sorry this in a random location, didn't know where to put it
{
    fprintf(stderr, "Swiped!\n");
    fprintf(stderr, "camera eye at  {%f, %f, %f}\n",camera.eye.X(), camera.eye.Y(), camera.eye.Z());
    R3Shape* hshape = new R3Shape();
    hshape->type = R3_MESH_SHAPE;
    R3Mesh hmesh;
    const char* huntermeshloc = "input/orthancclaw.off";
    hmesh.Read(huntermeshloc);
    
    R3Point offsetinit = R3Point(0,-1.5,5);
    R3Point meshat = camera.eye + offsetinit;
    R3Vector moveto = meshat - hmesh.Center();
    hmesh.Translate(moveto.X(),moveto.Y(),moveto.Z());
    fprintf(stderr, "paw centered at  {%f, %f, %f}\n",hmesh.Center().X(), hmesh.Center().Y(), hmesh.Center().Z());
    //hmesh.Scale(1,1,1);
    hshape->mesh = &hmesh;
	Bearpaw newpaw = Bearpaw(camera.eye, offsetinit, R3null_vector, *hshape);
    fprintf(stderr, "newpaw centered at  {%f, %f, %f}\n",newpaw.shape->Center().X(), newpaw.shape->Center().Y(), newpaw.shape->Center().Z());
	bearpaw_list.push_back(newpaw);
} */


void LoadCamera(R3Camera *camera)
{
    // Set projection transformation
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(2*180.0*camera->yfov/M_PI, (GLdouble) GLUTwindow_width /(GLdouble) GLUTwindow_height, 0.01, 10000);

    // Set camera transformation
    R3Vector t = -(camera->towards);
    R3Vector& u = camera->up;
    R3Vector& r = camera->right;
    GLdouble camera_matrix[16] = { r[0], u[0], t[0], 0, r[1], u[1], t[1], 0, r[2], u[2], t[2], 0, 0, 0, 0, 1 };
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMultMatrixd(camera_matrix);
    glTranslated(-(camera->eye[0]), -(camera->eye[1]), -(camera->eye[2]));
}


void LoadMinimapCamera(void)
{
    // Set projection transformation
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-100, 100, -100, 100, 0.01, 1000);

    // Set camera transformation
    R3Vector t = -(R3negy_vector);
    R3Vector& u = camera.towards;
    R3Vector& r = camera.right;
    GLdouble camera_matrix[16] = { r[0], u[0], t[0], 0, r[1], u[1], t[1], 0, r[2], u[2], t[2], 0, 0, 0, 0, 1 };
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMultMatrixd(camera_matrix);
    glTranslated(-(player.getPosition().X()), -(player.getPosition().Y() + 300), -(player.getPosition().Z()));
}



void LoadLights(R3Scene *scene)
{
    GLfloat buffer[4];

    // Load ambient light
    static GLfloat ambient[4];
    ambient[0] = scene->ambient[0];
    ambient[1] = scene->ambient[1];
    ambient[2] = scene->ambient[2];
    ambient[3] = 1;
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

    // Load scene lights
    for (int i = 0; i < (int) scene->lights.size(); i++) {
        R3Light *light = scene->lights[i];
        int index = GL_LIGHT0 + i;

        // Temporarily disable light
        glDisable(index);

        // Load color
        buffer[0] = light->color[0];
        buffer[1] = light->color[1];
        buffer[2] = light->color[2];
        buffer[3] = 1.0;
        glLightfv(index, GL_DIFFUSE, buffer);
        glLightfv(index, GL_SPECULAR, buffer);

        // Load attenuation with distance
        buffer[0] = light->constant_attenuation;
        buffer[1] = light->linear_attenuation;
        buffer[2] = light->quadratic_attenuation;
        glLightf(index, GL_CONSTANT_ATTENUATION, buffer[0]);
        glLightf(index, GL_LINEAR_ATTENUATION, buffer[1]);
        glLightf(index, GL_QUADRATIC_ATTENUATION, buffer[2]);

        // Load spot light behavior
        buffer[0] = 180.0 * light->angle_cutoff / M_PI;
        buffer[1] = light->angle_attenuation;
        glLightf(index, GL_SPOT_CUTOFF, buffer[0]);
        glLightf(index, GL_SPOT_EXPONENT, buffer[1]);

        // Load positions/directions
        if (light->type == R3_DIRECTIONAL_LIGHT) {
            // Load direction
            buffer[0] = -(light->direction.X());
            buffer[1] = -(light->direction.Y());
            buffer[2] = -(light->direction.Z());
            buffer[3] = 0.0;
            glLightfv(index, GL_POSITION, buffer);
        }
        else if (light->type == R3_POINT_LIGHT) {
            // Load position
            buffer[0] = light->position.X();
            buffer[1] = light->position.Y();
            buffer[2] = light->position.Z();
            buffer[3] = 1.0;
            glLightfv(index, GL_POSITION, buffer);
        }
        else if (light->type == R3_SPOT_LIGHT) {
            // Load position
            buffer[0] = light->position.X();
            buffer[1] = light->position.Y();
            buffer[2] = light->position.Z();
            buffer[3] = 1.0;
            glLightfv(index, GL_POSITION, buffer);

            // Load direction
            buffer[0] = light->direction.X();
            buffer[1] = light->direction.Y();
            buffer[2] = light->direction.Z();
            buffer[3] = 1.0;
            glLightfv(index, GL_SPOT_DIRECTION, buffer);
        }
        else if (light->type == R3_AREA_LIGHT) {
            // Load position
            buffer[0] = light->position.X();
            buffer[1] = light->position.Y();
            buffer[2] = light->position.Z();
            buffer[3] = 1.0;
            glLightfv(index, GL_POSITION, buffer);

            // Load direction
            buffer[0] = light->direction.X();
            buffer[1] = light->direction.Y();
            buffer[2] = light->direction.Z();
            buffer[3] = 1.0;
            glLightfv(index, GL_SPOT_DIRECTION, buffer);
        }
        else {
            fprintf(stderr, "Unrecognized light type: %d\n", light->type);
            return;
        }

        // Enable light
        glEnable(index);
    }
}

int insideViewingFrustrum(R3Camera *camera, R3Node *node) {
    // find planes that define viewing frustrum
    // we need a normal and a point to define each plane.
    R3Point p = camera->eye;
    R3Point np = p + camera->towards*camera->neardist;
    R3Point fp = p + camera->towards*camera->fardist;

    //normal to near plane
    R3Vector nn = camera->towards;
    double Dn = -nn.Dot(R3Vector(np.X(), np.Y(), np.Z()));

    //to far plane
    R3Vector nf = -nn;
    double Df = -nf.Dot(R3Vector(fp.X(), fp.Y(), fp.Z()));

    //to right plane
    R3Vector nr = np + (camera->right*camera->xfov*camera->neardist) - p;
    nr.Normalize();
    nr.Cross(camera->up);
    nr = -nr;
    double Dr = -nr.Dot(R3Vector(p.X(), p.Y(), p.Z()));

    //to left plane
    R3Vector nl = np + (-camera->right*camera->xfov*camera->neardist) - p;
    nl.Normalize();
    nl.Cross(camera->up);
    double Dl = -nl.Dot(R3Vector(p.X(), p.Y(), p.Z()));

    //top plane
    R3Vector nu = np + (camera->up*camera->yfov*camera->neardist) - p;
    nu.Normalize();
    nu.Cross(camera->right);
    double Du = -nu.Dot(R3Vector(p.X(), p.Y(), p.Z()));

    //bottom plane
    R3Vector nd = np + (-camera->up*camera->yfov*camera->neardist) - p;
    nd.Normalize();
    nd.Cross(camera->right);
    nd = -nd;
    double Dd = -nd.Dot(R3Vector(p.X(), p.Y(), p.Z()));

    //it is possible that a box with all corners outside the viewing
    //frustrum is still partly within the viewing frustrum.
    //so, check that all points are on the wrong side of the SAME
    //plane.  Some shapes might be drawn whose bounding boxes are
    //completely outside of the viewing frustrum, but whatever.

    R3Box box = node->bbox;

    //check near plane
    if (R3Vector(box.XMin(), box.YMin(), box.ZMin()).Dot(nn) + Dn >= 0);
    else if (R3Vector(box.XMax(), box.YMin(), box.ZMin()).Dot(nn) + Dn >= 0);
    else if (R3Vector(box.XMax(), box.YMax(), box.ZMin()).Dot(nn) + Dn >= 0);
    else if (R3Vector(box.XMax(), box.YMax(), box.ZMax()).Dot(nn) + Dn >= 0);
    else if (R3Vector(box.XMin(), box.YMax(), box.ZMin()).Dot(nn) + Dn >= 0);
    else if (R3Vector(box.XMin(), box.YMax(), box.ZMax()).Dot(nn) + Dn >= 0);
    else if (R3Vector(box.XMin(), box.YMin(), box.ZMax()).Dot(nn) + Dn >= 0);
    else if (R3Vector(box.XMax(), box.YMin(), box.ZMax()).Dot(nn) + Dn >= 0);
    else { return 0; }

    //far plane
    if (R3Vector(box.XMin(), box.YMin(), box.ZMin()).Dot(nf) + Df >= 0);
    else if (R3Vector(box.XMax(), box.YMin(), box.ZMin()).Dot(nf) + Df >= 0);
    else if (R3Vector(box.XMax(), box.YMax(), box.ZMin()).Dot(nf) + Df >= 0);
    else if (R3Vector(box.XMax(), box.YMax(), box.ZMax()).Dot(nf) + Df >= 0);
    else if (R3Vector(box.XMin(), box.YMax(), box.ZMin()).Dot(nf) + Df >= 0);
    else if (R3Vector(box.XMin(), box.YMax(), box.ZMax()).Dot(nf) + Df >= 0);
    else if (R3Vector(box.XMin(), box.YMin(), box.ZMax()).Dot(nf) + Df >= 0);
    else if (R3Vector(box.XMax(), box.YMin(), box.ZMax()).Dot(nf) + Df >= 0);
    else { return 0; }

    //right plane
    if (R3Vector(box.XMin(), box.YMin(), box.ZMin()).Dot(nr) + Dr >= 0);
    else if (R3Vector(box.XMax(), box.YMin(), box.ZMin()).Dot(nr) + Dr >= 0);
    else if (R3Vector(box.XMax(), box.YMax(), box.ZMin()).Dot(nr) + Dr >= 0);
    else if (R3Vector(box.XMax(), box.YMax(), box.ZMax()).Dot(nr) + Dr >= 0);
    else if (R3Vector(box.XMin(), box.YMax(), box.ZMin()).Dot(nr) + Dr >= 0);
    else if (R3Vector(box.XMin(), box.YMax(), box.ZMax()).Dot(nr) + Dr >= 0);
    else if (R3Vector(box.XMin(), box.YMin(), box.ZMax()).Dot(nr) + Dr >= 0);
    else if (R3Vector(box.XMax(), box.YMin(), box.ZMax()).Dot(nr) + Dr >= 0);
    else { return 0; }

    //left plane
    if (R3Vector(box.XMin(), box.YMin(), box.ZMin()).Dot(nl) + Dl >= 0);
    else if (R3Vector(box.XMax(), box.YMin(), box.ZMin()).Dot(nl) + Dl >= 0);
    else if (R3Vector(box.XMax(), box.YMax(), box.ZMin()).Dot(nl) + Dl >= 0);
    else if (R3Vector(box.XMax(), box.YMax(), box.ZMax()).Dot(nl) + Dl >= 0);
    else if (R3Vector(box.XMin(), box.YMax(), box.ZMin()).Dot(nl) + Dl >= 0);
    else if (R3Vector(box.XMin(), box.YMax(), box.ZMax()).Dot(nl) + Dl >= 0);
    else if (R3Vector(box.XMin(), box.YMin(), box.ZMax()).Dot(nl) + Dl >= 0);
    else if (R3Vector(box.XMax(), box.YMin(), box.ZMax()).Dot(nl) + Dl >= 0);
    else { return 0; }

    //up plane
    if (R3Vector(box.XMin(), box.YMin(), box.ZMin()).Dot(nu) + Du >= 0);
    else if (R3Vector(box.XMax(), box.YMin(), box.ZMin()).Dot(nu) + Du >= 0);
    else if (R3Vector(box.XMax(), box.YMax(), box.ZMin()).Dot(nu) + Du >= 0);
    else if (R3Vector(box.XMax(), box.YMax(), box.ZMax()).Dot(nu) + Du >= 0);
    else if (R3Vector(box.XMin(), box.YMax(), box.ZMin()).Dot(nu) + Du >= 0);
    else if (R3Vector(box.XMin(), box.YMax(), box.ZMax()).Dot(nu) + Du >= 0);
    else if (R3Vector(box.XMin(), box.YMin(), box.ZMax()).Dot(nu) + Du >= 0);
    else if (R3Vector(box.XMax(), box.YMin(), box.ZMax()).Dot(nu) + Du >= 0);
    else { return 0; }

    //down plane
    if (R3Vector(box.XMin(), box.YMin(), box.ZMin()).Dot(nd) + Dd >= 0);
    else if (R3Vector(box.XMax(), box.YMin(), box.ZMin()).Dot(nd) + Dd >= 0);
    else if (R3Vector(box.XMax(), box.YMax(), box.ZMin()).Dot(nd) + Dd >= 0);
    else if (R3Vector(box.XMax(), box.YMax(), box.ZMax()).Dot(nd) + Dd >= 0);
    else if (R3Vector(box.XMin(), box.YMax(), box.ZMin()).Dot(nd) + Dd >= 0);
    else if (R3Vector(box.XMin(), box.YMax(), box.ZMax()).Dot(nd) + Dd >= 0);
    else if (R3Vector(box.XMin(), box.YMin(), box.ZMax()).Dot(nd) + Dd >= 0);
    else if (R3Vector(box.XMax(), box.YMin(), box.ZMax()).Dot(nd) + Dd >= 0);
    else { return 0; }

    return 1;

}

void DrawNode(R3Scene *scene, R3Node *node, int minimap)
{
    // Push transformation onto stack
    glPushMatrix();
    LoadMatrix(&node->transformation);

    // Load material
    if (node->material) LoadMaterial(node->material);
    int inside;
    if (minimap) inside = 1;
    else inside = insideViewingFrustrum(&camera, node);
    //int inside = 1;

    // Draw shape
    if (node->shape) {
        //only draw shape if its bounding box is within the viewing frustrum
        if (inside) DrawShape(node->shape);
    }

    // Draw children nodes, but only if current node's bounding box
    // is within the viewing frustrum
    if (inside) {

        for (int i = 0; i < (int) node->children.size(); i++) {
            DrawNode(scene, node->children[i], minimap);
        }
    }

    // Restore previous transformation
    glPopMatrix();

    // Show bounding box
    if (show_bboxes) {
        GLboolean lighting = glIsEnabled(GL_LIGHTING);
        glDisable(GL_LIGHTING);
        node->bbox.Outline();
        if (lighting) glEnable(GL_LIGHTING);
    }

}



void DrawLights(R3Scene *scene)
{
    // Check if should draw lights
    if (!show_lights) return;

    // Setup
    GLboolean lighting = glIsEnabled(GL_LIGHTING);
    glDisable(GL_LIGHTING);

    // Draw all lights
    double radius = scene->bbox.DiagonalRadius();
    for (int i = 0; i < scene->NLights(); i++) {
        R3Light *light = scene->Light(i);
        glColor3d(light->color[0], light->color[1], light->color[2]);
        if (light->type == R3_DIRECTIONAL_LIGHT) {
            // Draw direction vector
            glLineWidth(5);
            glBegin(GL_LINES);
            R3Point centroid = scene->bbox.Centroid();
            R3Vector vector = radius * light->direction;
            glVertex3d(centroid[0] - vector[0], centroid[1] - vector[1], centroid[2] - vector[2]);
            glVertex3d(centroid[0] - 1.25*vector[0], centroid[1] - 1.25*vector[1], centroid[2] - 1.25*vector[2]);
            glEnd();
            glLineWidth(1);
        }
        else if (light->type == R3_POINT_LIGHT) {
            // Draw sphere at point light position
            R3Sphere(light->position, 0.1 * radius).Draw();
        }
        else if (light->type == R3_SPOT_LIGHT) {
            // Draw sphere at point light position and line indicating direction
            R3Sphere(light->position, 0.1 * radius).Draw();

            // Draw direction vector
            glLineWidth(5);
            glBegin(GL_LINES);
            R3Vector vector = radius * light->direction;
            glVertex3d(light->position[0], light->position[1], light->position[2]);
            glVertex3d(light->position[0] + 0.25*vector[0], light->position[1] + 0.25*vector[1], light->position[2] + 0.25*vector[2]);
            glEnd();
            glLineWidth(1);
        }
        else if (light->type == R3_AREA_LIGHT) {
            // Draw circular area
            R3Vector v1, v2;
            double r = light->radius;
            R3Point p = light->position;
            int dim = light->direction.MinDimension();
            if (dim == 0) { v1 = light->direction % R3posx_vector; v1.Normalize(); v2 = light->direction % v1; }
            else if (dim == 1) { v1 = light->direction % R3posy_vector; v1.Normalize(); v2 = light->direction % v1; }
            else { v1 = light->direction % R3posz_vector; v1.Normalize(); v2 = light->direction % v1; }
            glBegin(GL_POLYGON);
            glVertex3d(p[0] +  1.00*r*v1[0] +  0.00*r*v2[0], p[1] +  1.00*r*v1[1] +  0.00*r*v2[1], p[2] +  1.00*r*v1[2] +  0.00*r*v2[2]);
            glVertex3d(p[0] +  0.71*r*v1[0] +  0.71*r*v2[0], p[1] +  0.71*r*v1[1] +  0.71*r*v2[1], p[2] +  0.71*r*v1[2] +  0.71*r*v2[2]);
            glVertex3d(p[0] +  0.00*r*v1[0] +  1.00*r*v2[0], p[1] +  0.00*r*v1[1] +  1.00*r*v2[1], p[2] +  0.00*r*v1[2] +  1.00*r*v2[2]);
            glVertex3d(p[0] + -0.71*r*v1[0] +  0.71*r*v2[0], p[1] + -0.71*r*v1[1] +  0.71*r*v2[1], p[2] + -0.71*r*v1[2] +  0.71*r*v2[2]);
            glVertex3d(p[0] + -1.00*r*v1[0] +  0.00*r*v2[0], p[1] + -1.00*r*v1[1] +  0.00*r*v2[1], p[2] + -1.00*r*v1[2] +  0.00*r*v2[2]);
            glVertex3d(p[0] + -0.71*r*v1[0] + -0.71*r*v2[0], p[1] + -0.71*r*v1[1] + -0.71*r*v2[1], p[2] + -0.71*r*v1[2] + -0.71*r*v2[2]);
            glVertex3d(p[0] +  0.00*r*v1[0] + -1.00*r*v2[0], p[1] +  0.00*r*v1[1] + -1.00*r*v2[1], p[2] +  0.00*r*v1[2] + -1.00*r*v2[2]);
            glVertex3d(p[0] +  0.71*r*v1[0] + -0.71*r*v2[0], p[1] +  0.71*r*v1[1] + -0.71*r*v2[1], p[2] +  0.71*r*v1[2] + -0.71*r*v2[2]);
            glEnd();
        }
        else {
            fprintf(stderr, "Unrecognized light type: %d\n", light->type);
            return;
        }
    }

    // Clean up
    if (lighting) glEnable(GL_LIGHTING);
}



void DrawCamera(R3Scene *scene)
{
    // Check if should draw lights
    if (!show_camera) return;

    // Setup
    GLboolean lighting = glIsEnabled(GL_LIGHTING);
    glDisable(GL_LIGHTING);
    glColor3d(1.0, 1.0, 1.0);
    glLineWidth(5);

    // Draw view frustum
    R3Camera& c = scene->camera;
    double radius = scene->bbox.DiagonalRadius();
    R3Point org = c.eye + c.towards * radius;
    R3Vector dx = c.right * radius * tan(c.xfov);
    R3Vector dy = c.up * radius * tan(c.yfov);
    R3Point ur = org + dx + dy;
    R3Point lr = org + dx - dy;
    R3Point ul = org - dx + dy;
    R3Point ll = org - dx - dy;
    glBegin(GL_LINE_LOOP);
    glVertex3d(ur[0], ur[1], ur[2]);
    glVertex3d(ul[0], ul[1], ul[2]);
    glVertex3d(ll[0], ll[1], ll[2]);
    glVertex3d(lr[0], lr[1], lr[2]);
    glVertex3d(ur[0], ur[1], ur[2]);
    glVertex3d(c.eye[0], c.eye[1], c.eye[2]);
    glVertex3d(lr[0], lr[1], lr[2]);
    glVertex3d(ll[0], ll[1], ll[2]);
    glVertex3d(c.eye[0], c.eye[1], c.eye[2]);
    glVertex3d(ul[0], ul[1], ul[2]);
    glEnd();

    // Clean up
    glLineWidth(1);
    if (lighting) glEnable(GL_LIGHTING);
}



void DrawScene(R3Scene *scene, int minimap)
{
    // Draw nodes recursively
    DrawNode(scene, scene->root, minimap);
}


void DrawParticles(R3Scene *scene, double current_time, double delta_time)
{
    // Generate new particles
    GenerateParticles(scene, current_time , delta_time, player.getPosition());

    // Render particles
    if (show_particles) RenderParticles(scene, current_time, delta_time);


}

void DrawParticleSources(R3Scene *scene, double delta_time)
{
  // Check if should draw particle sources
  if (!show_particle_sources_and_sinks) return;

  // Setup
  GLboolean lighting = glIsEnabled(GL_LIGHTING);
  glEnable(GL_LIGHTING);

  // Define source material
  static R3Material source_material;
  if (source_material.id != 33) {
    // green
    /*source_material.ka.Reset(0.2,0.2,0.2,1);
    source_material.kd.Reset(0,1,0,1);
    source_material.ks.Reset(0,1,0,1);
    source_material.kt.Reset(0,0,0,1);
    source_material.emission.Reset(0,0,0,1);
    source_material.shininess = 1;
    source_material.indexofrefraction = 1;
    source_material.texture = NULL;
    source_material.texture_index = -1;
    source_material.id = 33;*/
    
    // red
    source_material.ka.Reset(0.2,0.2,0.2,1);
    source_material.kd.Reset(1,0,0,1);
    source_material.ks.Reset(1,0,0,1);
    source_material.kt.Reset(0,0,0,1);
    source_material.emission.Reset(0,0,0,1);
    source_material.shininess = 1;
    source_material.indexofrefraction = 1;
    source_material.texture = NULL;
    source_material.texture_index = -1;
    source_material.id = 33;
  }

  // Draw all particle sources
  glEnable(GL_LIGHTING);
  LoadMaterial(&source_material);
  for (int i = 0; i < scene->NParticleSources(); i++) {
    R3ParticleSource *source = scene->ParticleSource(i);
    DrawShape(source->shape);
  }

  // Clean up
  if (!lighting) glDisable(GL_LIGHTING);
}

void DrawParticleSinks(R3Scene *scene)
{
    // Check if should draw particle sinks
    if (!show_particle_sources_and_sinks) return;

    // Setup
    GLboolean lighting = glIsEnabled(GL_LIGHTING);
    glEnable(GL_LIGHTING);

    // Define sink material
    static R3Material sink_material;
    if (sink_material.id != 33) {
        sink_material.ka.Reset(0.2,0.2,0.2,1);
        sink_material.kd.Reset(1,0,0,1);
        sink_material.ks.Reset(1,0,0,1);
        sink_material.kt.Reset(0,0,0,1);
        sink_material.emission.Reset(0,0,0,1);
        sink_material.shininess = 1;
        sink_material.indexofrefraction = 1;
        sink_material.texture = NULL;
        sink_material.texture_index = -1;
        sink_material.id = 33;
    }

    // Draw all particle sinks
    glEnable(GL_LIGHTING);
    LoadMaterial(&sink_material);
    for (int i = 0; i < scene->NParticleSinks(); i++) {
        R3ParticleSink *sink = scene->ParticleSink(i);
        DrawShape(sink->shape);
    }

    // Clean up
    if (!lighting) glDisable(GL_LIGHTING);
}


void DrawPrey(void)
{
    // Setup
    GLboolean lighting = glIsEnabled(GL_LIGHTING);
    glEnable(GL_LIGHTING);

    // Define prey material
    static R3Material prey_material;
    if (prey_material.id != 33) {
        // green
        prey_material.ka.Reset(0.2,0.2,0.2,1);
        prey_material.kd.Reset(0,1,0,1);
        prey_material.ks.Reset(0,1,0,1);
        prey_material.kt.Reset(0,0,0,1);
        prey_material.emission.Reset(0,0,0,1);
        prey_material.shininess = 1;
        prey_material.indexofrefraction = 1;
        prey_material.texture = NULL;
        prey_material.texture_index = -1;
        prey_material.id = 33;
    }

    // Draw all prey
    glEnable(GL_LIGHTING);
    LoadMaterial(&prey_material);
    for (unsigned int i = 0; i < prey_list.size(); i++)
        DrawShape(&prey_list[i].shape);

    // Clean up
    if (!lighting) glDisable(GL_LIGHTING);
}

void DrawHunters(void)
{
    // Setup
    GLboolean lighting = glIsEnabled(GL_LIGHTING);
    glEnable(GL_LIGHTING);

    // Define hunter material
    static R3Material hunter_material;
    if (hunter_material.id != 33) {
        // red
        hunter_material.ka.Reset(0.2,0.2,0.2,1);
        hunter_material.kd.Reset(1,0,0,1);
        hunter_material.ks.Reset(1,0,0,1);
        hunter_material.kt.Reset(0,0,0,1);
        hunter_material.emission.Reset(0,0,0,1);
        hunter_material.shininess = 1;
        hunter_material.indexofrefraction = 1;
        hunter_material.texture = NULL;
        hunter_material.texture_index = -1;
        hunter_material.id = 33;
    }

    // Draw all particle sources
    glEnable(GL_LIGHTING);
    LoadMaterial(&hunter_material);
    for (unsigned int i = 0; i < hunter_list.size(); i++) {
        DrawShape(hunter_list[i].source.shape);
    }

    // Clean up
    if (!lighting) glDisable(GL_LIGHTING);
}

void DrawBearpaws(void)
{
    // Setup
    GLboolean lighting = glIsEnabled(GL_LIGHTING);
    glEnable(GL_LIGHTING);

    // Define hunter material
    static R3Material hunter_material;
    if (hunter_material.id != 33) {
        // red
        hunter_material.ka.Reset(0.2,0.2,0.2,1);
        hunter_material.kd.Reset(0,0,1,1);
        hunter_material.ks.Reset(0,0,1,1);
        hunter_material.kt.Reset(0,0,1,1);
        hunter_material.emission.Reset(0,0,0,1);
        hunter_material.shininess = 1;
        hunter_material.indexofrefraction = 1;
        hunter_material.texture = NULL;
        hunter_material.texture_index = -1;
        hunter_material.id = 33;
    }

    // Draw all paws 
    /*
    glEnable(GL_LIGHTING);
    LoadMaterial(&hunter_material);
    for (unsigned int i = 0; i < bearpaw_list.size(); i++) {
        //bearpaw_list[i].draw();
        DrawShape(&bearpaw_list[i].shape);
        //fprintf(stderr, "paw shape %f \n",bearpaw_list[i].shape.mesh->Center().X());
    } */

    // Clean up
    if (!lighting) glDisable(GL_LIGHTING);
}

void DrawParticleSprings(R3Scene *scene)
{
    // Check if should draw particle springs
    if (!show_particle_springs) return;

    // Setup
    GLboolean lighting = glIsEnabled(GL_LIGHTING);
    glDisable(GL_LIGHTING);

    // Draw all particle sources
    glColor3d(0.5, 0.5, 0.5);
    glBegin(GL_LINES);
    for (unsigned int i = 0; i < scene->particle_springs.size(); i++) {
        R3ParticleSpring *spring = scene->particle_springs[i];
        const R3Point& p0 = spring->particles[0]->position;
        const R3Point& p1 = spring->particles[1]->position;
        glVertex3d(p0[0], p0[1], p0[2]);
        glVertex3d(p1[0], p1[1], p1[2]);
    }
    glEnd();

    // Clean up
    if (lighting) glEnable(GL_LIGHTING);
}


void DrawMinimapPrey(void)
{
    // Setup
    GLboolean lighting = glIsEnabled(GL_LIGHTING);
    glEnable(GL_LIGHTING);

    // Define prey material
    static R3Material prey_material;
    if (prey_material.id != 33) {
        // green
        prey_material.ka.Reset(0.2,0.2,0.2,1);
        prey_material.kd.Reset(0,1,0,1);
        prey_material.ks.Reset(0,1,0,1);
        prey_material.kt.Reset(0,0,0,1);
        prey_material.emission.Reset(0,0,0,1);
        prey_material.shininess = 1;
        prey_material.indexofrefraction = 1;
        prey_material.texture = NULL;
        prey_material.texture_index = -1;
        prey_material.id = 33;
    }

    // Draw all prey
    glEnable(GL_LIGHTING);
    LoadMaterial(&prey_material);
    for (unsigned int i = 0; i < prey_list.size(); i++)
        DrawShape(&prey_list[i].icon);

    // Clean up
    if (!lighting) glDisable(GL_LIGHTING);
}


void DrawMinimapHunters(void)
{
    // Setup
    GLboolean lighting = glIsEnabled(GL_LIGHTING);
    glEnable(GL_LIGHTING);

    // Define hunter material
    static R3Material hunter_material;
    if (hunter_material.id != 33) {
        // red
        hunter_material.ka.Reset(0.2,0.2,0.2,1);
        hunter_material.kd.Reset(1,0,0,1);
        hunter_material.ks.Reset(1,0,0,1);
        hunter_material.kt.Reset(0,0,0,1);
        hunter_material.emission.Reset(0,0,0,1);
        hunter_material.shininess = 1;
        hunter_material.indexofrefraction = 1;
        hunter_material.texture = NULL;
        hunter_material.texture_index = -1;
        hunter_material.id = 33;
    }

    // Draw all particle sources
    glEnable(GL_LIGHTING);
    LoadMaterial(&hunter_material);
    for (unsigned int i = 0; i < hunter_list.size(); i++) {
        DrawShape(&hunter_list[i].icon);
    }

    // Clean up
    if (!lighting) glDisable(GL_LIGHTING);
}



void DrawMinimapSelf(void)
{
    // Setup
    GLboolean lighting = glIsEnabled(GL_LIGHTING);
    glEnable(GL_LIGHTING);

    // Define prey material
    static R3Material prey_material;
    if (prey_material.id != 33) {
        // blue
        prey_material.ka.Reset(0.2,0.2,0.2,1);
        prey_material.kd.Reset(0,0,1,1);
        prey_material.ks.Reset(0,0,1,1);
        prey_material.kt.Reset(0,0,0,1);
        prey_material.emission.Reset(0,0,0,1);
        prey_material.shininess = 1;
        prey_material.indexofrefraction = 1;
        prey_material.texture = NULL;
        prey_material.texture_index = -1;
        prey_material.id = 33;
    }

    // Draw self
    glEnable(GL_LIGHTING);
    LoadMaterial(&prey_material);
    R3Shape self = R3Shape();
    self.type = R3_CIRCLE_SHAPE;
    R3Circle circle = R3Circle(player.getPosition(), 7.5, R3yaxis_vector);
    self.circle = &circle;
    DrawShape(&self);

    // Clean up
    if (!lighting) glDisable(GL_LIGHTING);
}


void DrawMinimap(void)
{
    int XSize = glutGet(GLUT_WINDOW_WIDTH);
    int YSize = glutGet(GLUT_WINDOW_HEIGHT);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho (0, XSize, YSize, 0, 0, 1);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glColor3d(1, 1, 1);

    glBindTexture(GL_TEXTURE_2D, fbo_texture); // Bind our frame buffer texture

    glBegin(GL_QUADS);
    // Top left corner of the screen is (0, 0)
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(XSize - 155, 5);          //Top Left

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(XSize - 5, 5);       //Top Right

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(XSize - 5, 155);     // Bottom Right

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(XSize - 155, 155);       //Bottom Left
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind

    glEnable(GL_DEPTH_TEST);
}


////////////////////////////////////////////////////////////
// GLUT USER INTERFACE CODE
////////////////////////////////////////////////////////////

void GLUTMainLoop(void)
{
    // Run main loop -- never returns
    glutMainLoop();
}



void GLUTDrawText(const R3Point& p, const char *s)
{
    // Draw text string s and position p
    glRasterPos3d(p[0], p[1], p[2]);
    while (*s) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *(s++));
}



void GLUTSaveImage(const char *filename)
{
    // Create image
    R2Image image(GLUTwindow_width, GLUTwindow_height);

    // Read screen into buffer
    GLfloat *pixels = new GLfloat [ 3 * GLUTwindow_width * GLUTwindow_height ];
    glReadPixels(0, 0, GLUTwindow_width, GLUTwindow_height, GL_RGB, GL_FLOAT, pixels);

    // Load pixels from frame buffer
    GLfloat *pixelsp = pixels;
    for (int j = 0; j < GLUTwindow_height; j++) {
        for (int i = 0; i < GLUTwindow_width; i++) {
            double r = (double) *(pixelsp++);
            double g = (double) *(pixelsp++);
            double b = (double) *(pixelsp++);
            R2Pixel pixel(r, g, b, 1);
            image.SetPixel(i, j, pixel);
        }
    }

    // Write image to file
    image.Write(filename);

    // Delete buffer
    delete [] pixels;
}



void GLUTStop(void)
{
    // Destroy window
    glutDestroyWindow(GLUTwindow);

    // Delete scene
    delete scene;

    // Exit
    exit(0);
}



void GLUTIdle(void)
{
    // Set current window
    if ( glutGetWindow() != GLUTwindow )
        glutSetWindow(GLUTwindow);

    // Redraw
    glutPostRedisplay();
}



void GLUTResize(int w, int h)
{
    // Resize window
    glViewport(0, 0, w, h);

    // Resize camera vertical field of view to match aspect ratio of viewport
    camera.yfov = atan(tan(camera.xfov) * (double) h/ (double) w);

    // Remember window size
    GLUTwindow_width = w;
    GLUTwindow_height = h;

    // Redraw
    glutPostRedisplay();
}



void GLUTRedraw(void)
{
    // Initialize OpenGL drawing modes
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);
    glDepthMask(true);

    // Clear window
    R3Rgb background = scene->background;
    glClearColor(background[0], background[1], background[2], background[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    //  Do time stuff
    // Get current time (in seconds) since start of execution
    double current_time = GetTime();
    static double previous_time = 0;

    static double time_lost_taking_videos = 0; // for switching back and forth
    // between recording and not
    // recording smoothly

    // program just started up?
    if (previous_time == 0) previous_time = current_time;

    // time passed since starting
    double delta_time = current_time - previous_time;

    if (save_video) { // in video mode, time that passes only depends on frame rate
        delta_time = VIDEO_FRAME_DELAY;
        // ... but we need to keep track how much time we gained and lost
        //     so that we can arbitrarily switch back and forth ...
        time_lost_taking_videos += (current_time - previous_time) - VIDEO_FRAME_DELAY;
    } else { // real time simulation
        delta_time = current_time - previous_time;
    }

    // normalizing things for rotation?
    camera.up = R3Vector(0,1,0);
    R3Point origin = R3Point(0,0,0);
    R3Plane xzplane = R3Plane(origin, camera.up);
    camera.towards.Project(xzplane);
    camera.right.Project(xzplane);

    // move or turn if user pressed keys
    // TODO - offload this into a separate function?
    R3Point oldPosition = R3Point(player.position);

    // update y-position
    player.updatePosition(delta_time);
    /*
    for (unsigned int i = 0; i < bearpaw_list.size(); i++) {
        bearpaw_list[i].updatePosition(delta_time, camera.eye);
    } */

    if (move_forward) {
        R3Vector forward = R3Vector(camera.towards);
        forward.Normalize();
        forward.SetY(0);
        player.position += forward*delta_time*player.speed*sprint;
        if (player.collides(scene, prey_list, hunter_list))
            player.position -= 2*forward*delta_time*player.speed*sprint;
    }
    if (move_backward) {
        R3Vector backward = R3Vector(-camera.towards);
        backward.Normalize();
        backward.SetY(0);
        player.position += backward*delta_time*player.speed*sprint;
        if (player.collides(scene, prey_list, hunter_list))
            player.position -= 2*backward*delta_time*player.speed*sprint;
    }
    if (move_left) {
        R3Vector left = R3Vector(-camera.right);
        left.Normalize();
        left.SetY(0);
        player.position += left*delta_time*player.speed*sprint;
        if (player.collides(scene, prey_list, hunter_list))
            player.position -= 2*left*delta_time*player.speed*sprint;
    }
    if (move_right) {
        R3Vector right = R3Vector(camera.right);
        right.Normalize();
        right.SetY(0);
        player.position += right*delta_time*player.speed*sprint;
        if (player.collides(scene, prey_list, hunter_list))
            player.position -= 2*right*delta_time*player.speed*sprint;
    }
    if (move_jump) {
        if (player.position.Y() <= (player.height + TOLERANCE)) {
            player.velocity += R3Vector(0,10,0);
        }
        move_jump = 0;
    }
    double turn_rate = .25;
    if (turn_right) {
        R3Vector axis = R3Vector(0, 1, 0);
        camera.towards.Rotate(axis, -PI* turn_rate * delta_time);
        camera.right.Rotate(axis, -PI * turn_rate * delta_time);
    }
    if (turn_left) {
        R3Vector axis = R3Vector(0, 1, 0);
        camera.towards.Rotate(axis, PI * turn_rate * delta_time);
        camera.right.Rotate(axis, PI * turn_rate * delta_time);
    }
	if (swiped) {
        //swipe();
        swiped = 0;
	}

    // make sure that player doesn't leave the map
    if (player.position.X() > BOUND)
        player.position.SetX(BOUND);
    else if (player.position.X() < -BOUND)
        player.position.SetX(-BOUND);
    if (player.position.Z() > BOUND)
        player.position.SetZ(BOUND);
    else if (player.position.Z() < -BOUND)
        player.position.SetZ(-BOUND);

    
    // update the player's bbox
    //fprintf(stderr, "player:  %f, %f, %f\n", player.position.X(), player.position.Y(), player.position.Z());
    //fprintf(stderr, "box min: %f, %f, %f\n", player.bbox.XMin(), player.bbox.YMin(), player.bbox.ZMin());
    //fprintf(stderr, "box max: %f, %f, %f\n", player.bbox.XMax(), player.bbox.YMax(), player.bbox.ZMax());
    player.bbox.Translate(player.position - oldPosition);

    // check for bullet hits
    int numHits = countBulletHits(scene, player.bbox);
    player.health -= (2*numHits);

    /* Drawing minimap */
#ifndef cygwin
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo); // Render to texture

    // Clear the background of our window
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    //Clear the colour buffer (more buffers later on)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Load minimap camera
    LoadMinimapCamera();

    // Load scene lights
    LoadLights(scene);

    // Draw particle sources
    DrawParticleSources(scene, delta_time);

    // Draw particle sinks
    DrawParticleSinks(scene);

    // Draw prey
    DrawMinimapPrey();

    // Draw hunters
    DrawMinimapHunters();

    // Draw self
    DrawMinimapSelf();

    // Draw scene surfaces
    if (show_faces) {
        glEnable(GL_LIGHTING);
        DrawScene(scene, 1);
    }

    // Draw scene edges
    if (show_edges) {
        glDisable(GL_LIGHTING);
        glColor3d(1 - background[0], 1 - background[1], 1 - background[2]);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        DrawScene(scene, 1);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); // Unbind
#endif
    /* End drawing minimap */

    // set the camera to the player's position
    camera.eye = player.getPosition();

    // Load camera
    LoadCamera(&camera);

    // Load scene lights
    LoadLights(scene);

    //DrawBearpaws();
    // Update and draw particles
    UpdateParticles(scene, current_time, delta_time, integration_type, 1);
    DrawParticles(scene, current_time - time_lost_taking_videos, delta_time);

    // Draw particle sources
    DrawParticleSources(scene, delta_time);

    // Draw particle sinks
    DrawParticleSinks(scene);

    // Draw particle springs
    DrawParticleSprings(scene);

    // Update and draw prey
    for (unsigned int i = 0; i < prey_list.size(); i++)
        prey_list[i].updatePosition(delta_time, player.getPosition(), BOUND, scene, prey_list, hunter_list, player.bbox);
    DrawPrey();

    // Update and draw hunters
    for (unsigned int i = 0; i < hunter_list.size(); i++) {
        hunter_list[i].updatePosition(delta_time, player.getPosition(), BOUND, scene, prey_list, hunter_list, player.bbox);
        hunter_list[i].shoot(scene, current_time, delta_time, player.getPosition());
    }
    DrawHunters();
    
    

    // Draw scene surfaces
    if (show_faces) {
        glEnable(GL_LIGHTING);
        DrawScene(scene, 0);
    }

    // Draw scene edges
    if (show_edges) {
        glDisable(GL_LIGHTING);
        glColor3d(1 - background[0], 1 - background[1], 1 - background[2]);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        DrawScene(scene, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    glDisable(GL_LIGHTING);

    // Display on-screen menu
    DrawTextBox();

    // Display minimap
    DrawMinimap();

    // Save image
    if (save_image) {
        char image_name[256];
        static int image_number = 1;
        for (;;) {
            sprintf(image_name, "image%d.jpg", image_number++);
            FILE *fp = fopen(image_name, "r");
            if (!fp) break;
            else fclose(fp);
        }
        GLUTSaveImage(image_name);
        printf("Saved %s\n", image_name);
        save_image = 0;
    }

    // Save video
    if (save_video) {
        char frame_name[512];
        static int next_frame = 0;
        static int num_frames_recorded = 0;
        for (;;) {
            sprintf(frame_name, "%sframe%04d.jpg", video_prefix, next_frame++);
            FILE *fp = fopen(frame_name, "r");
            if (!fp) break;
            else fclose(fp);
        }
        GLUTSaveImage(frame_name);
        if (next_frame % 100 == 1) {
            printf("Saved %s\n", frame_name);
        }
        if (num_frames_to_record == ++num_frames_recorded) {
            save_video = 0;
            printf("Recorded %d frames, stopping as instructed.\n", num_frames_recorded);
            quit = 1;
        }
    }

    /* Sounds */
#ifndef cygwin
    // Listener
    alListener3f(AL_POSITION, camera.eye.X(), camera.eye.Y(), camera.eye.Y());
    ALfloat orientation[] = {camera.towards.X(), camera.towards.Y(), camera.towards.Z(), camera.up.X(), camera.up.Y(), camera.up.Z()};
    alListenerfv(AL_ORIENTATION, orientation);

    // Hunters
    for (unsigned int i = 0; i < hunter_list.size(); i++) {
        alSource3f(source_list[i+1], AL_POSITION, hunter_list[i].getPosition().X(),
                hunter_list[i].getPosition().Y(), hunter_list[i].getPosition().Z());

        ALenum state;
        alGetSourcei(source_list[i+1], AL_SOURCE_STATE, &state);
        if ((hunter_list[i].getPosition() - player.getPosition()).Length() < 100) {
            if (state != AL_PLAYING)
                alSourcePlay(source_list[i+1]);
        }
        else {
            if (state == AL_PLAYING)
                alSourceStop(source_list[i+1]);
        }
    }
#endif
    /* End sounds */

    // Quit here so that can save image before exit
    if (quit) {
        if (output_image_name) GLUTSaveImage(output_image_name);
        GLUTStop();
    }

    previous_time = current_time;
    // Swap buffers
    glutSwapBuffers();
}

void GLUTMotion(int x, int y)
{
    // Invert y coordinate
    y = GLUTwindow_height - y;

    // Compute mouse movement
    int dx = x - GLUTmouse[0];
    int dy = y - GLUTmouse[1];

    // Process mouse motion event
    if ((dx != 0) || (dy != 0)) {
        R3Point scene_center = scene->bbox.Centroid();
        if ((GLUTbutton[0] && (GLUTmodifiers & GLUT_ACTIVE_SHIFT)) || GLUTbutton[1]) {
            // Scale world
            double factor = (double) dx / (double) GLUTwindow_width;
            factor += (double) dy / (double) GLUTwindow_height;
            factor = exp(2.0 * factor);
            factor = (factor - 1.0) / factor;
            R3Vector translation = (scene_center - camera.eye) * factor;
            camera.eye += translation;
            glutPostRedisplay();
        }
        else if (GLUTbutton[0] && (GLUTmodifiers & GLUT_ACTIVE_CTRL)) {
            // Translate world
            double length = R3Distance(scene_center, camera.eye) * tan(camera.yfov);
            double vx = length * (double) dx / (double) GLUTwindow_width;
            double vy = length * (double) dy / (double) GLUTwindow_height;
            R3Vector translation = -((camera.right * vx) + (camera.up * vy));
            camera.eye += translation;
            glutPostRedisplay();
        }
        else if (GLUTbutton[0]) {
            // Rotate world
            double vx = (double) dx / (double) GLUTwindow_width;
            double vy = (double) dy / (double) GLUTwindow_height;
            double theta = 4.0 * (fabs(vx) + fabs(vy));
            R3Vector vector = (camera.right * vx) + (camera.up * vy);
            R3Vector rotation_axis = camera.towards % vector;
            rotation_axis.Normalize();
            camera.eye.Rotate(R3Line(scene_center, rotation_axis), theta);
            camera.towards.Rotate(rotation_axis, theta);
            camera.up.Rotate(rotation_axis, theta);
            camera.right = camera.towards % camera.up;
            camera.up = camera.right % camera.towards;
            camera.towards.Normalize();
            camera.up.Normalize();
            camera.right.Normalize();
            glutPostRedisplay();
        }
    }

    // Remember mouse position
    GLUTmouse[0] = x;
    GLUTmouse[1] = y;
}


void GLUTMouse(int button, int state, int x, int y)
{
    // Invert y coordinate
    y = GLUTwindow_height - y;

    // Process mouse button event
    if (state == GLUT_DOWN) {
        if (button == GLUT_LEFT_BUTTON) {
            if (x <= 150 && x >= 7 && y <= 50 && y >= 7) {
                menu = 1;
            }
	    else if (menu && x <= 150 && x >= 7 && y >=50 && y <= 100) {
	      quit = 1;
	    }
            else {
                menu = 0;
            }
        }
        else if (button == GLUT_MIDDLE_BUTTON) {
        }
        else if (button == GLUT_RIGHT_BUTTON) {
        }
    }

    // Remember button state
    int b = (button == GLUT_LEFT_BUTTON) ? 0 : ((button == GLUT_MIDDLE_BUTTON) ? 1 : 2);
    GLUTbutton[b] = (state == GLUT_DOWN) ? 1 : 0;

    // Remember modifiers
    GLUTmodifiers = glutGetModifiers();

    // Remember mouse position
    GLUTmouse[0] = x;
    GLUTmouse[1] = y;

    // Redraw
    glutPostRedisplay();
}



void GLUTSpecial(int key, int x, int y)
{
    // Invert y coordinate
    y = GLUTwindow_height - y;

    // Process keyboard button event
    switch (key) {
        case GLUT_KEY_F1:
            save_image = 1;
            break;
        case GLUT_KEY_F2:
            save_video = save_video ^ 1;
            break;
    }

    // Remember mouse position
    GLUTmouse[0] = x;
    GLUTmouse[1] = y;

    // Remember modifiers
    GLUTmodifiers = glutGetModifiers();

    // Redraw
    glutPostRedisplay();
}



void GLUTKeyboard(unsigned char key, int x, int y)
{
    // Invert y coordinate
    y = GLUTwindow_height - y;

    // Process keyboard button event
    switch (key) {
        case 'B':
        case 'b':
            show_bboxes = !show_bboxes;
            break;

        case 'C':
        case 'c':
            show_camera = !show_camera;
            break;
            /*
               case 'E':
               case 'e':
               show_edges = !show_edges;
               break;
               */
            /*
               case 'F':
               case 'f':
               show_faces = !show_faces;
               break;
               */
            /*
               case 'L':
               case 'l':
               show_lights = !show_lights;
               break;

               case 'P':
               case 'p':
               show_particles = !show_particles;
               break;

               case 'R':
               case 'r':
               show_particle_springs = !show_particle_springs;
               break;
               */
            /*
               case 'S':
               case 's':
               show_particle_sources_and_sinks = !show_particle_sources_and_sinks;
               break;
               */
        case 'W':
        case 'w':
            move_forward = 1;
            break;

        case 'S':
        case 's':
            move_backward = 1;
            break;

        case 'A':
        case 'a':
            move_left = 1;
            break;

        case 'D':
        case 'd':
            move_right = 1;
            break;

        case ' ':
            move_jump = 1;
            break;

        case 'Q':
        case 'q':
            turn_left = 1;
            break;

        case 'E':
        case 'e':
            turn_right = 1;
            break;

        case 'F':
        case 'f':
            sprint = 2.5;
            break;
			
		case 'R':
		case 'r':
			swiped = 1;
			break;

            /*
               case 'Q':
               case 'q':
               case 27: // ESCAPE
               quit = 1;
               break;
               */

            /*
               case ' ': {
               printf("camera %g %g %g  %g %g %g  %g %g %g  %g  %g %g \n",
               camera.eye[0], camera.eye[1], camera.eye[2],
               camera.towards[0], camera.towards[1], camera.towards[2],
               camera.up[0], camera.up[1], camera.up[2],
               camera.xfov, camera.neardist, camera.fardist);
               break; }*/
    }

    // Remember mouse position
    GLUTmouse[0] = x;
    GLUTmouse[1] = y;

    // Remember modifiers
    GLUTmodifiers = glutGetModifiers();

    // Redraw
    glutPostRedisplay();
}

void GLUTKeyboardUp(unsigned char key, int x, int y)
{
    // Invert y coordinate
    y = GLUTwindow_height - y;

    // Process keyboard button event
    switch (key) {

        case 'W':
        case 'w':
            move_forward = 0;
            break;

        case 'S':
        case 's':
            move_backward = 0;
            break;

        case 'A':
        case 'a':
            move_left = 0;
            break;

        case 'D':
        case 'd':
            move_right = 0;
            break;

        case 'Q':
        case 'q':
            turn_left = 0;
            break;

        case 'E':
        case 'e':
            turn_right = 0;
            break;

        case 'F':
        case 'f':
            sprint = 1;
            break;

        /*
            case ' ':
            move_jump = 1;
            break;

            case 'Q':
            case 'q':
            case 27: // ESCAPE
            quit = 1;
            break;
        */
        /*
           case ' ': {
           printf("camera %g %g %g  %g %g %g  %g %g %g  %g  %g %g \n",
           camera.eye[0], camera.eye[1], camera.eye[2],
           camera.towards[0], camera.towards[1], camera.towards[2],
           camera.up[0], camera.up[1], camera.up[2],
           camera.xfov, camera.neardist, camera.fardist);
           break; }
        */
    }

    // Remember mouse position
    GLUTmouse[0] = x;
    GLUTmouse[1] = y;

    // Remember modifiers
    GLUTmodifiers = glutGetModifiers();

    // Redraw
    glutPostRedisplay();
}



void GLUTCommand(int cmd)
{
    // Execute command
    switch (cmd) {
        case DISPLAY_PARTICLES_TOGGLE_COMMAND: show_particles = !show_particles; break;
        case DISPLAY_PARTICLE_SPRINGS_TOGGLE_COMMAND: show_particle_springs = !show_particle_springs; break;
        case DISPLAY_PARTICLE_SOURCES_AND_SINKS_TOGGLE_COMMAND: show_particle_sources_and_sinks = !show_particle_sources_and_sinks; break;
        case DISPLAY_FACE_TOGGLE_COMMAND: show_faces = !show_faces; break;
        case DISPLAY_EDGE_TOGGLE_COMMAND: show_edges = !show_edges; break;
        case DISPLAY_BBOXES_TOGGLE_COMMAND: show_bboxes = !show_bboxes; break;
        case DISPLAY_LIGHTS_TOGGLE_COMMAND: show_lights = !show_lights; break;
        case DISPLAY_CAMERA_TOGGLE_COMMAND: show_camera = !show_camera; break;
        case SAVE_IMAGE_COMMAND: save_image = 1; break;
        case SAVE_VIDEO_COMMAND: save_video = save_video ^ 1; break;
        case QUIT_COMMAND: quit = 1; break;
    }

    // Mark window for redraw
    glutPostRedisplay();
}



void GLUTCreateMenu(void)
{
    // Display sub-menu
    int display_menu = glutCreateMenu(GLUTCommand);
    glutAddMenuEntry("Particles (P)", DISPLAY_PARTICLES_TOGGLE_COMMAND);
    glutAddMenuEntry("Particle springs (R)", DISPLAY_PARTICLE_SPRINGS_TOGGLE_COMMAND);
    glutAddMenuEntry("Particle sources and sinks (S)", DISPLAY_PARTICLE_SOURCES_AND_SINKS_TOGGLE_COMMAND);
    glutAddMenuEntry("Faces (F)", DISPLAY_FACE_TOGGLE_COMMAND);
    glutAddMenuEntry("Edges (E)", DISPLAY_EDGE_TOGGLE_COMMAND);
    glutAddMenuEntry("Bounding boxes (B)", DISPLAY_BBOXES_TOGGLE_COMMAND);
    glutAddMenuEntry("Lights (L)", DISPLAY_LIGHTS_TOGGLE_COMMAND);
    glutAddMenuEntry("Camera (C)", DISPLAY_CAMERA_TOGGLE_COMMAND);

    // Main menu
    glutCreateMenu(GLUTCommand);
    glutAddSubMenu("Display", display_menu);
    glutAddMenuEntry("Save Image (F1)", SAVE_IMAGE_COMMAND);
    glutAddMenuEntry("Capture Video (F2)", SAVE_VIDEO_COMMAND);
    glutAddMenuEntry("Quit", QUIT_COMMAND);

    // Attach main menu to right mouse button
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}




/***** Framebuffer Initialization Functions *****/
#ifndef cygwin

void initFrameBufferDepthBuffer(void) {
    glGenRenderbuffersEXT(1, &fbo_depth); // Generate one render buffer and store the ID in fbo_depth
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbo_depth); // Bind the fbo_depth render buffer

    // Set the render buffer storage to be a depth component, with a width and height of the window
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, window_width, window_height);

    // Set the render buffer of this buffer to the depth buffer
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbo_depth);

    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0); // Unbind the render buffer
}

void initFrameBufferTexture(void) {
    glGenTextures(1, &fbo_texture); // Generate one texture
    glBindTexture(GL_TEXTURE_2D, fbo_texture); // Bind the texture fbo_texture

    // Create a standard texture with the width and height of our window
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // Setup the basic texture parameters
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

void initFrameBuffer(void) {
    initFrameBufferDepthBuffer(); // Initialize our frame buffer depth buffer

    initFrameBufferTexture(); // Initialize our frame buffer texture

    glGenFramebuffersEXT(1, &fbo); // Generate one frame buffer and store the ID in fbo
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo); // Bind our frame buffer

    // Attach the texture fbo_texture to the color buffer in our frame buffer
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fbo_texture, 0);

    // Attach the depth buffer fbo_depth to our frame buffer
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbo_depth);

    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT); // Check that status of our generated frame buffer

    if (status != GL_FRAMEBUFFER_COMPLETE_EXT) // If the frame buffer does not report back as complete
    {
        exit(0); // Exit the application
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); // Unbind our frame buffer
}

#endif
/***** End Framebuffer Initialization Functions *****/





void GLUTInit(int *argc, char **argv)
{
    // Open window
    glutInit(argc, argv);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(GLUTwindow_width, GLUTwindow_height);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // | GLUT_STENCIL
    GLUTwindow = glutCreateWindow("OpenGL Viewer");

    // Initialize GLUT callback functions

    glutIgnoreKeyRepeat(1);


    glutIdleFunc(GLUTIdle);
    glutReshapeFunc(GLUTResize);
    glutDisplayFunc(GLUTRedraw);
    glutKeyboardFunc(GLUTKeyboard);
    glutKeyboardUpFunc(GLUTKeyboardUp);
    glutSpecialFunc(GLUTSpecial);
    glutMouseFunc(GLUTMouse);
    glutMotionFunc(GLUTMotion);

    // Initialize graphics modes
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

    // Create menus
    GLUTCreateMenu();

    // Initialize glew
#ifndef cygwin
    glewInit();
    initFrameBuffer();
#endif
}




////////////////////////////////////////////////////////////
// SCENE READING
////////////////////////////////////////////////////////////


    R3Scene *
ReadScene(const char *filename)
{
    // Allocate scene
    R3Scene *scene = new R3Scene();
    if (!scene) {
        fprintf(stderr, "Unable to allocate scene\n");
        return NULL;
    }

    // Read file
    if (!scene->Read(filename)) {
        fprintf(stderr, "Unable to read scene from %s\n", filename);
        return NULL;
    }

    // Remember initial camera
    camera = scene->camera;

    // Return scene
    return scene;
}



////////////////////////////////////////////////////////////
// PROGRAM ARGUMENT PARSING
////////////////////////////////////////////////////////////

int ParseArgs(int argc, char **argv)
{
    // Innocent until proven guilty
    int print_usage = 0;

    // Parse arguments
    argc--; argv++;
    while (argc > 0) {
        if ((*argv)[0] == '-') {
            if (!strcmp(*argv, "-help")) { print_usage = 1; }
            else if (!strcmp(*argv, "-exit_immediately")) { quit = 1; }
            else if (!strcmp(*argv, "-output_image")) { argc--; argv++; output_image_name = *argv; }
            else if (!strcmp(*argv, "-video_prefix")) { argc--; argv++; video_prefix = *argv; }
            else if (!strcmp(*argv, "-euler")) integration_type = EULER_INTEGRATION;
            else if (!strcmp(*argv, "-midpoint")) integration_type = MIDPOINT_INTEGRATION;
            else if (!strcmp(*argv, "-rk4")) integration_type = RK4_INTEGRATION;
            else if (!strcmp(*argv, "-adaptive_step_size")) integration_type = ADAPTIVE_STEP_SIZE_INTEGRATION;
            else if (!strcmp(*argv, "-recordandquit")) {
                argc--; argv++; num_frames_to_record = atoi(*argv);
                GLUTwindow_width = 256;
                GLUTwindow_height = 256;
                save_video = 1;
            }
            else { fprintf(stderr, "Invalid program argument: %s", *argv); exit(1); }
            argv++; argc--;
        }
        else {
            if (!input_scene_name) input_scene_name = *argv;
            else { fprintf(stderr, "Invalid program argument: %s", *argv); exit(1); }
            argv++; argc--;
        }
    }

    // Check input_scene_name
    if (!input_scene_name || print_usage) {
        printf("Usage: particleview <input.scn> [-exit_immediately] [-output_image OUTPUT.JPG]  [-video_prefix VIDEO_DIR/PREFIX_] [-euler] [-midpoint] [-rk4] [-adaptive_step_size]  [-recordandquit NUM_FRAMES] [-v]\n");
        return 0;
    }

    // Return OK status
    return 1;
}



////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    // Parse program arguments
    if (!ParseArgs(argc, argv)) exit(1);

    // Initialize GLUT
    GLUTInit(&argc, argv);

    // Read scene
    scene = ReadScene(input_scene_name);
    if (!scene) exit(-1);

    // initialize the player bear
    double mass = 100;
    double speed = 20;
    double height = 5;
    R3Vector init_velocity = R3Vector(0, 0, 0);
    camera.eye.SetY(height);
    player = Bear(mass, speed, height, init_velocity, R3Point(camera.eye));

    // fix the player's bbox
    R3Point cyl_center = R3Point(player.position);
    cyl_center.SetY((height/2.0) + 0.5);
    R3Cylinder player_cyl = R3Cylinder(cyl_center, 1.5, height);
    player.bbox = player_cyl.BBox();

    // initialize prey shapes
    /*R3Shape *shape1 = new R3Shape();
    R3Shape *shape2 = new R3Shape();
	

    shape1->type = R3_SPHERE_SHAPE;
    shape2->type = R3_SPHERE_SHAPE;

    R3Sphere sphere1 = R3Sphere(R3Point(7,4,7), 3);
    R3Sphere sphere2 = R3Sphere(R3Point(-7,4,-7), 3);
    shape1->sphere = &sphere1;
    shape2->sphere = &sphere2;*/
	
	R3Shape* preyshape = new R3Shape();
	preyshape->type = R3_MESH_SHAPE;
	R3Mesh pmesh;
	const char* preymeshloc = "input/badbunny.off";
    pmesh.Read(preymeshloc);
    pmesh.Translate(7,4,7);
    pmesh.Scale(1,1,1);
	//pmesh.Rotate(PI, R3posx_vector);
    preyshape->mesh = &pmesh;
    // initialize some prey
    Prey prey1 = Prey(100, 20, R3Point(7,4,7), R3Vector(0,0,0), *preyshape);
    //Prey prey2 = Prey(100, 15, R3Point(-7,4,-7), R3Vector(0,0,0), *shape2);
    prey1.bbox = pmesh.bbox;
    //prey2.bbox = sphere2.BBox();
    prey_list.push_back(prey1);
    //prey_list.push_back(prey2);

    // initialize hunter particle source
    R3ParticleSource *hsource = new R3ParticleSource();
    R3Shape *hshape = new R3Shape();
    hshape->type = R3_MESH_SHAPE;
    R3Mesh hmesh;
    const char* huntermeshloc = "input/orthancclaw.off";
    hmesh.Read(huntermeshloc);
    hmesh.Translate(10,0,10);
    hmesh.Scale(2,2,2);
    hshape->mesh = &hmesh;
    //hshape->type = R3_SPHERE_SHAPE;
    //R3Sphere hsphere = R3Sphere(R3Point(10,4,10), 3);
    //hshape->sphere = &hsphere;

    hsource->shape = hshape;
    hsource->rate = 0.5;
    hsource->velocity = 40;
    hsource->angle_cutoff = 0.00005;
    hsource->mass = 0.01;
    hsource->fixed = false;
    hsource->drag = 0;
    hsource->elasticity = 0;
    hsource->lifetime = 5;
    hsource->todo = 0;

    R3Material *bullet_material = new R3Material();
    bullet_material->ka = R3Rgb(0, 0, 0, 1);
    bullet_material->kd = R3Rgb(0, 0, 0, 1);
    bullet_material->ks = R3Rgb(0, 0, 0, 1);
    bullet_material->kt = R3Rgb(0.0, 0.0, 0.0, 1);
    bullet_material->emission = R3Rgb(0, 0, 0, 1);
    bullet_material->shininess = 10;
    bullet_material->indexofrefraction = 1;
    bullet_material->texture = NULL;
    bullet_material->id = 0;

    hsource->material = bullet_material;

    // initialize a hunter
    Hunter hunter = Hunter(100, 5, R3Point(10, 4, 10), R3Vector(0,0,0), *hsource);
    hunter.bbox = hmesh.bbox;
    hunter_list.push_back(hunter); // commented out for FIRE TESTING // DEBUG

    /* Sounds */
#ifndef cygwin
    // Note: must run from upper directory

    alutInit(&argc, argv);

    // Device and context
    ALCdevice *device = alcOpenDevice(NULL);
    ALCcontext *context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);

    // Ambient sound
    ALuint source0, buffer0;
    source_list.push_back(source0);
    alGenSources(1, &source0);
    buffer0 = alutCreateBufferFromFile("input/naturesounds.wav");
    alSourcef(source0, AL_PITCH, 1);
    alSourcef(source0, AL_GAIN, 1);
    alSourcei(source0, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(source0, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSourcei(source0, AL_LOOPING, AL_TRUE);
    alSourcei(source0, AL_BUFFER, buffer0);
    alSourcePlay(source0);

    // Hunter sounds
    for (unsigned int i = 0; i < hunter_list.size(); i++) {
        ALuint source1, buffer1;
        alGenSources(1, &source1);
        source_list.push_back(source1);
        buffer1 = alutCreateBufferFromFile("input/mars.wav");
        alSourcef(source1, AL_PITCH, 1);
        alSourcef(source1, AL_GAIN, 10);
        alSource3f(source1, AL_POSITION, hunter_list[i].getPosition().X(),
                hunter_list[i].getPosition().Y(), hunter_list[i].getPosition().Z());
        alSourcei(source1, AL_LOOPING, AL_TRUE);
        alSourcei(source1, AL_BUFFER, buffer1);
    }
#endif
    /* End sounds */

    // Run GLUT interface
    GLUTMainLoop();

    /*
       alDeleteSources(1, &source);
       alDeleteBuffers(1, &buffer);
       alcDestroyContext(context);
       alcCloseDevice(device);
    */

    delete preyshape;
    //delete shape2;
    delete hshape;
    delete hsource;
    delete bullet_material;

    // Return success
    return 0;
}
