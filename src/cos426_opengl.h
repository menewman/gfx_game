// Include the appropriate OpenGL and GLUT headers

#if defined(_WIN32) || defined(__CYGWIN__)
# ifndef USE_OPENGL32
#  define USE_OPENGL32
# endif
# ifndef NOMINMAX
#  define NOMINMAX
# endif
# include <windows.h>
# include <GL/glew.h>
# include <GL/glut.h>
# include <AL/al.h>
# include <AL/alc.h>
# include <AL/alut.h>
#elif defined(__APPLE__)
# include <GL/glew.h>
# include <GLUT/glut.h>
#else 
# include <GL/glew.h>
# include <GL/glut.h>
# include <AL/al.h>
# include <AL/alc.h>
# include <AL/alut.h>
#endif

