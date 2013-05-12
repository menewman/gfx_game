// Include file for the particle system

// Particle system integration types
enum {
  EULER_INTEGRATION,
  MIDPOINT_INTEGRATION,
  ADAPTIVE_STEP_SIZE_INTEGRATION,
  RK4_INTEGRATION
};

// stores information about intersection points
struct R3Intersection {
    bool hit; 
    R3Node *node; 
    R3Point position; 
    R3Vector normal; 
    double t;
    
    R3Intersection() {
        hit = false;
        node = NULL;
        position = R3Point(0,0,0);
        normal = R3Vector(0,0,0);
        t = 0;
    }
};

// Particle system functions
double RandomNumber(void);
R3Vector getNormal(R3Vector vector);
R3Intersection *ComputeIntersection(R3Ray ray, R3Sphere sphere);
R3Intersection *ComputeIntersection(R3Segment segment, R3Sphere sphere);
R3Intersection *ComputeIntersection(R3Ray ray, R3Circle circle);
R3Intersection *ComputeIntersection(R3Segment segment, R3Circle circle);
R3Intersection *ComputeIntersection(R3Segment seg1, R3Segment seg2);
double countBulletHits(R3Scene *scene, R3Box bbox);
void UpdateParticles(R3Scene *scene, double current_time, double delta_time, int integration_type);
void UpdateParticles(R3Scene *scene, double current_time, double delta_time, int integration_type, int substeps);
void GenerateParticles(R3Scene *scene, double current_time, double delta_time, R3Point playerPos);
void RenderParticles(R3Scene *scene, double current_time, double delta_time);


