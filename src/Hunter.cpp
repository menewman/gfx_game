// source file for the Hunter class

#include "R3/R3.h"
#include "R3Scene.h"
#include "particle.h"
#include "Prey.h"
#include "Hunter.h"
#define DETECT_BOUND 150
#define MIN_DIST 30
#define MIN_DIST_TOLERANCE 0.5
#define PI 3.14159265359

Hunter::
Hunter(void) {}

Hunter::
Hunter(double mass, double speed, R3Point position, R3Vector velocity, R3ParticleSource source)
    : mass(mass),
      speed(speed),
      position(position),
      velocity(velocity),
      source(source),
      bbox(R3null_box)
{
    icon.type = R3_CIRCLE_SHAPE;
    icon.circle = new R3Circle(position, 7.5, R3yaxis_vector);
}

void Hunter::
setVelocity(const R3Vector& newVelocity)
{
    velocity = newVelocity;
}

void Hunter::
updatePosition(double delta_time, R3Point playerPos, double bound, R3Scene *scene, vector<Prey>& prey_list, vector<Hunter>& hunter_list, R3Box bearBBox)
{       
    R3Vector toPlayer = playerPos - position;
    toPlayer.SetY(0);
    R3Point oldPosition = R3Point(position);
    
    // not moving if within tolerance of min_dist prevents the hunter
    // from spazzing out moving back and forth across the line each update
    if ((toPlayer.Length() > DETECT_BOUND) || (fabs(toPlayer.Length() - MIN_DIST) < MIN_DIST_TOLERANCE))
        return;
        
    double moveToward = (toPlayer.Length() - MIN_DIST)/fabs(toPlayer.Length() - MIN_DIST);
    toPlayer.Normalize();
    position += toPlayer*delta_time*speed*moveToward;
        
    // keep from leaving map
    if (position.X() > bound)
      position.SetX(bound);
    else if (position.X() < -bound)
      position.SetX(-bound);
    if (position.Z() > bound)
      position.SetZ(bound);
    else if (position.Z() < -bound)
      position.SetZ(-bound);
      
    // update the relevant shape parameters
    if (source.shape->type == R3_SPHERE_SHAPE) {
        source.shape->sphere->Reposition(position);
        bbox = source.shape->sphere->BBox();
    }
    else if (source.shape->type == R3_BOX_SHAPE) {
        // translate along the vector between centroid and new position
        R3Vector toNew = position - source.shape->box->Centroid();
        toNew.SetY(0);
        source.shape->box->Translate(toNew);
        bbox = R3Box(*source.shape->box);
    }
    else if (source.shape->type == R3_CYLINDER_SHAPE) {
        source.shape->cylinder->Reposition(position);
        bbox = source.shape->cylinder->BBox();
    }
    else if (source.shape->type == R3_CONE_SHAPE) {
        source.shape->cone->Reposition(position);
        bbox = source.shape->cone->BBox();
    }
    else if (source.shape->type == R3_MESH_SHAPE) {
        ; // ????? how do you move meshes anyway?
    }
    else if (source.shape->type == R3_SEGMENT_SHAPE) {
        // for segment: 'position' is arbitrarily the centroid
        R3Vector toNew = position - source.shape->segment->Centroid();
        toNew.SetY(0);
        source.shape->segment->Translate(toNew);
        bbox = source.shape->segment->BBox();
    }
    else if (source.shape->type == R3_CIRCLE_SHAPE) {
        source.shape->circle->Reposition(position);
        bbox = source.shape->circle->BBox();
    }
    
    if (collides(scene, prey_list, hunter_list, bearBBox)) {
        position = oldPosition - toPlayer*delta_time*speed*moveToward;
        // update the relevant shape parameters
        if (source.shape->type == R3_SPHERE_SHAPE) {
            source.shape->sphere->Reposition(position);
            bbox = source.shape->sphere->BBox();
        }
        else if (source.shape->type == R3_BOX_SHAPE) {
            // translate along the vector between centroid and new position
            R3Vector toNew = position - source.shape->box->Centroid();
            toNew.SetY(0);
            source.shape->box->Translate(toNew);
            bbox = R3Box(*source.shape->box);
        }
        else if (source.shape->type == R3_CYLINDER_SHAPE) {
            source.shape->cylinder->Reposition(position);
            bbox = source.shape->cylinder->BBox();
        }
        else if (source.shape->type == R3_CONE_SHAPE) {
            source.shape->cone->Reposition(position);
            bbox = source.shape->cone->BBox();
        }
        else if (source.shape->type == R3_MESH_SHAPE) {
            ; // ????? how do you move meshes anyway?
        }
        else if (source.shape->type == R3_SEGMENT_SHAPE) {
            // for segment: 'position' is arbitrarily the centroid
            R3Vector toNew = position - source.shape->segment->Centroid();
            toNew.SetY(0);
            source.shape->segment->Translate(toNew);
            bbox = source.shape->segment->BBox();
        }
        else if (source.shape->type == R3_CIRCLE_SHAPE) {
            source.shape->circle->Reposition(position);
            bbox = source.shape->circle->BBox();
        }
    }
    
    icon.circle->Reposition(position);
}

// fire particles at the player
void Hunter::
shoot(R3Scene *scene, double current_time, double delta_time, R3Point playerPos)
{
    R3Vector dist = playerPos - position;
    dist.SetY(0);
    if (dist.Length() > DETECT_BOUND*1.5) {
        source.todo = 0;
        return;
    }

    // 'todo' is list of bullets to be generated
    source.todo += source.rate * delta_time;
    int intpart = (int)(floor(source.todo));
    source.todo -= intpart;
     
    for (int i = 0; i < intpart; i++) {
        //fprintf(stderr, "firing in direction (%f, %f, %f)\n", dist.X(), dist.Y(), dist.Z());
        //fprintf(stderr, "playerPos: (%f, %f, %f)\n", playerPos.X(), playerPos.Y(), playerPos.Z());
        R3Point point; // point on surface
        R3Vector N;  // surface normal
        if (source.shape->type == R3_SPHERE_SHAPE) {
            // select a point on surface of sphere
            R3Sphere *sphere = source.shape->sphere;
            
            R3Vector toPlayer = playerPos - sphere->Center();
            toPlayer.Normalize();
            point = sphere->Center() + sphere->Radius()*toPlayer;
             
            N = point - sphere->Center();
            N.Normalize();
        }
        else if (source.shape->type == R3_CIRCLE_SHAPE) {
            R3Circle *circle = source.shape->circle;
            N = R3Vector(circle->Normal());
             
            // select a random point within circle
            R3Vector A = getNormal(N);
             
            // multiply A by a double between (0, r)
            double magn = ((double)rand() / RAND_MAX);
            magn *= circle->Radius();
            A *= magn;
             
            point = circle->Center() + A;
        }
        else if (source.shape->type == R3_SEGMENT_SHAPE) {
            R3Segment *segment = source.shape->segment;
             
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
             t2 *= sin(source.angle_cutoff);
         } while ((t2 == sin(source.angle_cutoff)) && (sin(source.angle_cutoff) != 0));
            
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
         particle->velocity = source.velocity * V;
         particle->mass = source.mass;
         particle->fixed = source.fixed;
         particle->drag = source.drag;
         particle->elasticity = source.elasticity;
         particle->lifetime = source.lifetime;
         particle->birthday = current_time;
         particle->material = source.material; 
         particle->bullet = true;
         
         // for now, no y-component to velocity
         particle->velocity.SetY(0);  

         // Add particle to scene
         scene->particles.push_back(particle);
     }
}

// recursive method, returns true if hunter collides with
//     node or any of its children
bool Hunter::
collides(R3Scene *scene, R3Node *node)
{
    if (bbox.intersects(node->bbox)) {
        return true;
    }
    for (unsigned int i = 0; i < node->children.size(); i++) {
        if (collides(scene, node->children[i])) {
            return true;
        }
    }
    return false;
}

// returns true iff hunter collides with a node in the scene
bool Hunter::
collides(R3Scene *scene, vector<Prey>& prey_list, vector<Hunter>& hunter_list, R3Box bearBBox)
{
    // recurse through the scene's nodes
    R3Node *root = scene->Root();
    for (unsigned int i = 0; i < root->children.size(); i++) {
        if (collides(scene, root->children[i])) {
            return true;
        }
    }
    for (unsigned int i = 0; i < prey_list.size(); i++) {
        // check prey bounding box
        if (bbox.intersects(prey_list[i].bbox))
            return true;
    }
    for (unsigned int i = 0; i < hunter_list.size(); i++) {
        // shouldn't check intersection with itself
        if (hunter_list[i].position == position)
            continue;
        // check hunter bounding box
        if (bbox.intersects(hunter_list[i].bbox))
            return true;
    }
    if (bbox.intersects(bearBBox))
        return true;
    return false;
}
