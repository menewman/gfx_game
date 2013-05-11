// Source file for the particle system



// Include files

#include "R2/R2.h"
#include "R3/R3.h"
#include "R3Scene.h"
#include "particle.h"
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
using namespace std;
#ifdef _WIN32
#   include <windows.h>
#else
#   include <sys/time.h>
#endif

#define PI 3.14159265359
#define GRAV_C 6.67384e-11
#define SUBSTEPS 4


////////////////////////////////////////////////////////////
// Random Number Generator
////////////////////////////////////////////////////////////

double RandomNumber(void)
{
#ifdef _WIN32
  // Seed random number generator
  static int first = 1;
  if (first) {
    srand(GetTickCount());
    first = 0;
  }

  // Return random number
  int r1 = rand();
  double r2 = ((double) rand()) / ((double) RAND_MAX);
  return (r1 + r2) / ((double) RAND_MAX);
#else 
  // Seed random number generator
  static int first = 1;
  if (first) {
    struct timeval timevalue;
    gettimeofday(&timevalue, 0);
    srand48(timevalue.tv_usec);
    first = 0;
  }

  // Return random number
  return drand48();
#endif
}

////////////////////////////////////////////////////////////
// Normal vector methods
////////////////////////////////////////////////////////////

// returns a random vector perpendicular to input vector
R3Vector getNormal(R3Vector vector) {
    // find a vector orthogonal to input vector
    double tiny = 0.0001;
    R3Vector G = R3Vector(0,0,1);
    R3Vector Gcross = R3Vector(G);
    Gcross.Cross(vector);
    if (Gcross.Length() < tiny) {
     G = R3Vector(0,1,0);
     Gcross = R3Vector(G);
     Gcross.Cross(vector);
    }
    if (Gcross.Length() < tiny) {
     G = R3Vector(1,0,0);
    }
    Gcross = R3Vector(vector);
    Gcross.Cross(G);
    Gcross.Normalize();
    R3Vector A = R3Vector(Gcross);
    A.Normalize(); // Vector A is a unit vector in orthogonal plane

    // rotate A around segment by a random amount in (0, 2pi]
    double rot_angle = ((double)rand() / RAND_MAX);
    do {
     rot_angle *= (2*PI);
    } while (rot_angle == 2*PI);
    A.Rotate(vector, rot_angle);

    return R3Vector(A);
}

////////////////////////////////////////////////////////////
// Intersection Methods
////////////////////////////////////////////////////////////

// returns true if q is in [p,r]
bool within(double p, double q, double r) {
    return (((p <= q) && (q <= r)) || ((r <= q) && (q <= p)));
}

// returns true if point c lies within line segment
// relies on the assumption that c and the segment are collinear
bool within_segment(R3Segment seg, R3Point c) {
    if (seg.Start().X() != seg.End().X()) {
        return within(seg.Start().X(), c.X(), seg.End().X());
    }
    else {
        return within(seg.Start().Y(), c.Y(), seg.End().Y());
    }
}

// returns intersection of R3Ray and R3Sphere
R3Intersection *ComputeIntersection(R3Ray ray, R3Sphere sphere) {
    // follow the geometry from the lecture slides
    R3Intersection *interX = new R3Intersection();
    
    R3Vector V = ray.Vector();
    R3Vector L = sphere.Center() - ray.Start();
    double t_ca = L.Dot(V);
    if (t_ca < 0)
        return interX;
    
    double d_squared = L.Dot(L) - (t_ca*t_ca);
    double r_squared = sphere.Radius()*sphere.Radius();
    if (d_squared > r_squared)
        return interX;
    
    double t_hc = sqrt(r_squared - d_squared);
    double t = t_ca - t_hc;
    
    // calculate the position of intersection and the normal vector
    R3Point pos = ray.Start() + t*ray.Vector();
    R3Vector norm = (pos - sphere.Center()) / (pos-sphere.Center()).Length();
    
    // set the intersection struct's fields
    interX->hit = true;
    interX->position = pos;
    interX->normal = norm;
    interX->t = t;
    
    return interX;
}

// returns intersection between segment and sphere
R3Intersection *ComputeIntersection(R3Segment segment, R3Sphere sphere) {
    // create a ray from P1 to P2
    R3Ray ray = R3Ray(segment.Start(), segment.End());
    
    R3Intersection *rayX = ComputeIntersection(ray, sphere);
    if (!rayX->hit)
        return rayX;
    
    if (within_segment(segment, rayX->position))
        return rayX;
        
    rayX->hit = false;
    rayX->node = NULL; 
    rayX->position = R3Point(0,0,0);
    rayX->normal = R3Vector(0,0,0);
    rayX->t = 0;
    return rayX;
}

// returns intersection between ray and circle
R3Intersection *ComputeIntersection(R3Ray ray, R3Circle circle) {
    R3Intersection *interX = new R3Intersection();
    
    // project the ray onto the plane of the circle
    R3Vector N = circle.Normal();
    
    // probably should check for no intersection?
    double denom = N.Dot(ray.Vector());
    if (denom == 0)
        return interX;
    double t = -N.Dot(ray.Start() - circle.Center())/denom;
    //double t = norm.Dot(f->plane.Point() - ray.Start()) / denom;
    if (t < 0)
        return interX;
    R3Point onPlane = ray.Start() + t*ray.Vector();
    
    // does the intersection of the plane lie within the circle?
    double dToCenter = (circle.Center() - onPlane).Length();
    if (dToCenter <= circle.Radius()) {
        interX->hit = true;
        interX->position = onPlane;
        interX->normal = R3Vector(circle.Normal());
        interX->t = t;
    }
    
    return interX;
}

// returns intersection between segment and circle
R3Intersection *ComputeIntersection(R3Segment segment, R3Circle circle) {
    // create a ray from P1 to P2
    R3Ray ray = R3Ray(segment.Start(), segment.End());
    
    // find intersection between ray and circle
    R3Intersection *rayX = ComputeIntersection(ray, circle);
    
    // if no intersection, return no intersection
    if (!rayX->hit)
        return rayX;
    
    /*
    double dist = (rayX->position - segment.Start()).Length();
    if ((dist > 0) && (dist <= segment.Length()))
        return rayX;
    */
    if (within_segment(segment, rayX->position))
        return rayX;
    
    rayX->hit = false;
    rayX->node = NULL; 
    rayX->position = R3Point(0,0,0);
    rayX->normal = R3Vector(0,0,0);
    rayX->t = 0;
    return rayX;
}

// returns intersection between segment and segment
R3Intersection *ComputeIntersection(R3Segment seg1, R3Segment seg2) {
    R3Intersection *interX = new R3Intersection();
    
    R3Vector u = seg1.Vector();
    R3Vector v = seg2.Vector();
    R3Vector w = seg1.Start() - seg2.Start();
    
    // check for parallel segments
    double dotP = u.Dot(v);
    double tolerance = 0.000001;
    if ((dotP - 1.0) < tolerance)
        return interX;
        
    R3Vector uvCross = R3Vector(u);
    uvCross.Cross(v);
    R3Vector uwCross = R3Vector(u);
    uwCross.Cross(w);
    R3Vector vwCross = R3Vector(v);
    vwCross.Cross(w);
    double nA = vwCross.Dot(uvCross);
    double nB = uwCross.Dot(uvCross);
    double d = uvCross.Dot(uvCross);
    //A0 = A1 + (nA/d)*u;
    R3Point A0 = seg1.Start() + ((nA / d) * u);
    //B0 = B1 + (nB/d)*v;
    R3Point B0 = seg2.Start() + ((nB / d) * v);
    
    // closest points don't seem to coincide
    if ((A0 - B0).Length() < tolerance)
        return interX;
        
    // there is an intersection, but it isn't within segments
    if (!within_segment(seg1, A0) || !within_segment(seg2, B0))
        return interX;
    
    interX->hit = true;
    interX->position = A0;
    interX->normal = getNormal(v);
    interX->t = (A0 - seg1.Start()).Length();
    
    return interX;
}

////////////////////////////////////////////////////////////
// Generating Particles
////////////////////////////////////////////////////////////

void GenerateParticles(R3Scene *scene, double current_time, double delta_time, R3Point playerPos)
{
  // Generate new particles for every source
  // loop over sources
  for (int k = 0; k < scene->NParticleSources(); k++) {
      R3ParticleSource *source = scene->ParticleSource(k);
     
     // add to todo
     source->todo += source->rate * delta_time;
     int intpart = (int)(floor(source->todo));
     source->todo -= intpart;
     
     for (int i = 0; i < intpart; i++) {
         R3Point point; // point on surface
         R3Vector N;  // surface normal
         if (source->shape->type == R3_SPHERE_SHAPE) {
             // select a random point on surface of sphere
             R3Sphere *sphere = source->shape->sphere;
             
             // z = random[-r, r]
             double z = ((double)rand() / RAND_MAX);
             z = (-1.0*sphere->Radius()) + z * (2*sphere->Radius());
             
             // phi = random[0, 2*PI)
             double phi = ((double)rand() / RAND_MAX);
             do {
                 phi *= 2*PI;
             } while (phi == 2*PI);
             
             // calculate new point
             double d = sqrt(sphere->Radius()*sphere->Radius() - z*z);
             double px = sphere->Center().X() + d*cos(phi);
             double py = sphere->Center().Y() + d*sin(phi);
             double pz = sphere->Center().Z() + z;
             point = R3Point(px, py, pz);
             
             N = point - sphere->Center();
             N.Normalize();
         }
         else if (source->shape->type == R3_CIRCLE_SHAPE) {
             R3Circle *circle = source->shape->circle;
             N = R3Vector(circle->Normal());
             
             // select a random point within circle
             R3Vector A = getNormal(N);
             
             // multiply A by a double between (0, r)
             double magn = ((double)rand() / RAND_MAX);
             magn *= circle->Radius();
             A *= magn;
             
             point = circle->Center() + A;
         }
         else if (source->shape->type == R3_SEGMENT_SHAPE) {
             R3Segment *segment = source->shape->segment;
             
             // find the segment's normal and also a random point along the segment
             
             // find a random point in the segment
             double lengthAlong = ((double)rand() / RAND_MAX);
             lengthAlong *= segment->Length();
             R3Vector dirV = segment->End() - segment->Start();
             dirV.Normalize();
             point = segment->Start() + (lengthAlong * dirV);
             
             // find a normal vector to the segment
             R3Vector A = getNormal(dirV);
             
             N = R3Vector(A);
         }
         else
            break; // this source shape not yet implemented
            
         // select a random direction, V
             
         // A = any vector on tangent plane
         R3Vector A = getNormal(N);
             
         // t1 = random[0, 2*PI)
         double t1 = ((double)rand() / RAND_MAX);
         do {
             t1 *= (2*PI);
         } while (t1 == 2*PI);
             
         // t2 = random[0, sin(angle_cutoff))
         double t2 = ((double)rand() / RAND_MAX);
         do {
             t2 *= sin(source->angle_cutoff);
         } while ((t2 == sin(source->angle_cutoff)) && (sin(source->angle_cutoff) != 0));
            
         // V = rotate A around N by t1
         A.Rotate(N, t1);
         R3Vector V = R3Vector(A);
         // Rotate V around VxN by acos(t2)
         R3Vector Vcross = R3Vector(V);
         Vcross.Cross(N);
         V.Rotate(Vcross, acos(t2));
             
         // Create particle
         R3Particle *particle = new R3Particle();
         particle->position = point;
         particle->velocity = source->velocity * V;
         particle->mass = source->mass;
         particle->fixed = source->fixed;
         particle->drag = source->drag;
         particle->elasticity = source->elasticity;
         particle->lifetime = source->lifetime;
         particle->birthday = current_time;
         particle->material = source->material;   

         // Add particle to scene
         scene->particles.push_back(particle);
     }
  }
}



////////////////////////////////////////////////////////////
// Updating Particles
////////////////////////////////////////////////////////////

// adaptive step size -- split each step into "substeps" number of smaller steps
void UpdateParticles(R3Scene *scene, double current_time, double delta_time, int integration_type, int substeps) {
    double new_delta = delta_time / SUBSTEPS;
    for (int i = 0; i < SUBSTEPS; i++) {
        double new_curr_time = current_time + (i*new_delta);
        UpdateParticles(scene, new_curr_time, new_delta, integration_type);
    }
}

void UpdateParticles(R3Scene *scene, double current_time, double delta_time, int integration_type)
{
  // Update position for every particle
  
  // vectors to be deleted
  std::vector<int> delIndices;
  
  // calculate attraction, spring forces beforehand
  std::vector<R3Vector> attractions;
  std::vector<R3Vector> hooke;
  for (int k = 0; k < scene->NParticles(); k++) {
      R3Particle *p = scene->Particle(k);
      R3Vector sumOfForces = R3Vector(0, 0, 0);
      for (int j = 0; j < scene->NParticles(); j++) {
          if (j == k)
              continue;
          R3Particle *p2 = scene->Particle(j);
          R3Vector dist = p2->position - p->position;
          double forceMagn = GRAV_C * (p->mass * p2->mass) / (dist.Length() * dist.Length());
          dist.Normalize();
          R3Vector thisForce = forceMagn * dist;
          sumOfForces += thisForce;
      }
      attractions.push_back(sumOfForces);
      
      R3Vector sumOfSprings = R3Vector(0, 0, 0);
      for (unsigned int j = 0; j < p->springs.size(); j++) {
          R3ParticleSpring *spring = p->springs[j];
          R3Particle *q;
          if (spring->particles[0] == p)
              q = spring->particles[1];
          else
              q = spring->particles[0];
          
          R3Vector D = q->position - p->position;
          double d = D.Length();
          D.Normalize();
          
          R3Vector thisForce = (spring->ks*(d-spring->rest_length)
                                 +spring->kd*(q->velocity-p->velocity).Dot(D))*D;
          
          // add to sum of springs
          sumOfSprings += thisForce;
      }
      hooke.push_back(sumOfSprings);
  }
  
  // loop through the particles
  for (int k = 0; k < scene->NParticles(); k++) {
      R3Particle *p = scene->Particle(k);
      bool deleted = false;
      
      // delete the particle if lifetime has expired
      if ((current_time > (p->birthday + p->lifetime)) && (p->lifetime > 0))
          deleted = true;
      
      if (p->fixed)
          continue;
      
      // calculate force due to gravity
      R3Vector gravity = p->mass * scene->gravity;
      
      // calculate force due to drag
      R3Vector drag = (-1.0) * p->drag * p->velocity;
      
      // calculate force due to particle sink
      R3Vector sumOfSinkForces = R3Vector(0,0,0);
      for (int j = 0; j < scene->NParticleSinks(); j++) {
          R3ParticleSink *sink = scene->ParticleSink(j);
          if (sink->shape->type == R3_SPHERE_SHAPE) {
              R3Sphere *sphere = sink->shape->sphere;
              
              R3Vector direction = sphere->Center() - p->position;
              double d = direction.Length() - sphere->Radius();
              if (d <= 0)
                  deleted = true;
              direction.Normalize();
              
              double forceMagn = sink->intensity / (sink->constant_attenuation
                  + sink->linear_attenuation*d + sink->quadratic_attenuation*d*d);
              
              R3Vector thisForce = direction * forceMagn;
              sumOfSinkForces += thisForce;
          }
          else if (sink->shape->type == R3_CIRCLE_SHAPE) {
              R3Circle *circle = sink->shape->circle;
              
              // find the nearest point of the circle?
              // project the particle point onto the plane
              R3Plane plane = circle->Plane();
              R3Point onPlane = R3Point(p->position);
              onPlane.Project(plane);
              
              R3Vector fromCenter = onPlane - circle->Center();
              R3Point nearestPoint;
              if (fromCenter.Length() < circle->Radius()) {
                  // point is in circle!
                  nearestPoint = R3Point(onPlane);
              }
              else {
                  // point is at center + fromCenter.Normalize() * r
                  fromCenter.Normalize();
                  nearestPoint = circle->Center() + (fromCenter*circle->Radius());
              }
              
              R3Vector direction = nearestPoint - p->position;
              double d = direction.Length();
              direction.Normalize();
              
              double forceMagn = sink->intensity / (sink->constant_attenuation
                  + sink->linear_attenuation*d + sink->quadratic_attenuation*d*d);
              R3Vector thisForce = direction * forceMagn;
              sumOfSinkForces += thisForce;
          }
          else if (sink->shape->type == R3_SEGMENT_SHAPE) {
              R3Segment *segment = sink->shape->segment;
              
              // find the nearest point on the segment 
              R3Point nearestPoint = R3Point(p->position);
              nearestPoint.Project(segment->Line());
              if (!within_segment(*segment, nearestPoint)) {
                  if ((nearestPoint-segment->Start()).Length() < (nearestPoint-segment->End()).Length())
                      nearestPoint = R3Point(segment->Start());
                  else
                      nearestPoint = R3Point(segment->End());
              }
              
              R3Vector direction = nearestPoint - p->position;
              double d = direction.Length();
              direction.Normalize();
              
              double forceMagn = sink->intensity / (sink->constant_attenuation
                  + sink->linear_attenuation*d + sink->quadratic_attenuation*d*d);
              R3Vector thisForce = direction * forceMagn;
              sumOfSinkForces += thisForce;
          }
      }
      
      // Newton's second law
      R3Vector acceleration = gravity + drag + attractions[k] + hooke[k] + sumOfSinkForces;
      acceleration /= p->mass;
      
      // save old position for future intersection calculations
      R3Point oldPos = R3Point(p->position);
      
      // find new velocity and new position; update fields
      p->position.Translate(delta_time * p->velocity);
      p->velocity = p->velocity + delta_time * acceleration;
      
      // save new position for intersection calculations
      R3Point newPos = R3Point(p->position);
      R3Segment seg = R3Segment(oldPos, newPos);
      
      // is new position within the radius of a sink?
      for (int j = 0; j < scene->NParticleSinks(); j++) {
          R3ParticleSink *sink = scene->ParticleSink(j);
          if (sink->shape->type == R3_SPHERE_SHAPE) {
              R3Sphere *sphere = sink->shape->sphere;
              R3Intersection *interX = ComputeIntersection(seg, *sphere);
              if (interX->hit)
                  deleted = true;
              delete interX;
          }
          else if (sink->shape->type == R3_CIRCLE_SHAPE) {
              R3Circle *circle = sink->shape->circle;
              R3Intersection *interX = ComputeIntersection(seg, *circle);
              if (interX->hit)
                  deleted = true;
              delete interX;
          }
          else if (sink->shape->type == R3_SEGMENT_SHAPE) {
              R3Segment *segment = sink->shape->segment;
              R3Intersection *interX = ComputeIntersection(seg, *segment);
              if (interX->hit)
                  deleted = true;
              delete interX;
          }
      }
      
      if (deleted)
          delIndices.push_back(k);
  }
  
  // delete any vectors tagged for deletion
  std::reverse(delIndices.begin(), delIndices.end());
  for (unsigned int k = 0; k < delIndices.size(); k++) {
      scene->particles.erase(scene->particles.begin() + delIndices[k]);
  }
}



////////////////////////////////////////////////////////////
// Rendering Particles
////////////////////////////////////////////////////////////

void RenderParticles(R3Scene *scene, double current_time, double delta_time)
{
  // Draw every particle

  // REPLACE CODE HERE
  glDisable(GL_LIGHTING);
  glPointSize(5);
  glBegin(GL_POINTS);
  for (int i = 0; i < scene->NParticles(); i++) {
    R3Particle *particle = scene->Particle(i);
    glColor3d(particle->material->kd[0], particle->material->kd[1], particle->material->kd[2]);
    const R3Point& position = particle->position;
    glVertex3d(position[0], position[1], position[2]);
  }   
  glEnd();
}



