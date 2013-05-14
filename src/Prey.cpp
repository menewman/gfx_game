// source file for the Prey class

#include "R3/R3.h"
#include "R3Scene.h"
#include "Hunter.h"
#include "Prey.h"
#define SCARE_BOUND 100

Prey::
Prey(void) {}

Prey::
Prey(double mass, double speed, R3Point position, R3Vector velocity, R3Shape shape)
    : mass(mass),
      speed(speed),
      position(position),
      velocity(velocity),
      shape(shape),
      bbox(R3null_box)
{
    icon.type = R3_CIRCLE_SHAPE;
    icon.circle = new R3Circle(position, 7.5, R3yaxis_vector);
}

void Prey::
setVelocity(const R3Vector& newVelocity)
{
    velocity = newVelocity;
}

void Prey::
updatePosition(double delta_time, R3Point playerPos, double bound, R3Scene *scene, vector<Prey>& prey_list, vector<Hunter>& hunter_list, R3Box bearBBox)
{
    R3Vector fromPlayer = position - playerPos;
    fromPlayer.SetY(0);
    
    if (fromPlayer.Length() > SCARE_BOUND)
        return;
        
    fromPlayer.Normalize();
    
    position += fromPlayer*delta_time*speed;
    
    // update the y-position
    R3Vector grav = R3Vector(0,-15,0);
    R3Vector f_grav = mass * grav;
    R3Vector accel = f_grav / mass;
    position.Translate(delta_time * velocity);
    velocity += delta_time * accel;
    if (position.Y() <= 4.0) { // currently a magic number
        position.SetY(4.0);
        velocity += R3Vector(0,10,0);
    }
    
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
    if (shape.type == R3_SPHERE_SHAPE) {
        shape.sphere->Reposition(position);
        bbox = shape.sphere->BBox();
    }
    else if (shape.type == R3_BOX_SHAPE) {
        // translate along the vector between centroid and new position
        R3Vector toNew = position - shape.box->Centroid();
        toNew.SetY(0);
        shape.box->Translate(toNew);
        bbox = R3Box(*shape.box);
    }
    else if (shape.type == R3_CYLINDER_SHAPE) {
        shape.cylinder->Reposition(position);
        bbox = shape.cylinder->BBox();
    }
    else if (shape.type == R3_CONE_SHAPE) {
        shape.cone->Reposition(position);
        bbox = shape.cone->BBox();
    }
    else if (shape.type == R3_MESH_SHAPE) {
        R3Vector toNew = position - shape.mesh->Center();
		shape.mesh->Translate(toNew.X(), toNew.Y(), toNew.Z());
		bbox = shape.mesh->bbox;
    }
    else if (shape.type == R3_SEGMENT_SHAPE) {
        // for segment: 'position' is arbitrarily the centroid
        R3Vector toNew = position - shape.segment->Centroid();
        toNew.SetY(0);
        shape.segment->Translate(toNew);
        bbox = shape.segment->BBox();
    }
    else if (shape.type == R3_CIRCLE_SHAPE) {
        shape.circle->Reposition(position);
        bbox = shape.circle->BBox();
    }
    
    
    if (collides(scene, prey_list, hunter_list, bearBBox)) {
        position -= 1.5*fromPlayer*delta_time*speed;
        // update the relevant shape parameters
        if (shape.type == R3_SPHERE_SHAPE) {
            shape.sphere->Reposition(position);
            bbox = shape.sphere->BBox();
        }
        else if (shape.type == R3_BOX_SHAPE) {
            // translate along the vector between centroid and new position
            R3Vector toNew = position - shape.box->Centroid();
            toNew.SetY(0);
            shape.box->Translate(toNew);
            bbox = R3Box(*shape.box);
        }
        else if (shape.type == R3_CYLINDER_SHAPE) {
            shape.cylinder->Reposition(position);
            bbox = shape.cylinder->BBox();
        }
        else if (shape.type == R3_CONE_SHAPE) {
            shape.cone->Reposition(position);
            bbox = shape.cone->BBox();
        }
        else if (shape.type == R3_MESH_SHAPE) {
            ; // ????? how do you move meshes anyway?
        }
        else if (shape.type == R3_SEGMENT_SHAPE) {
            // for segment: 'position' is arbitrarily the centroid
            R3Vector toNew = position - shape.segment->Centroid();
            toNew.SetY(0);
            shape.segment->Translate(toNew);
            bbox = shape.segment->BBox();
        }
        else if (shape.type == R3_CIRCLE_SHAPE) {
            shape.circle->Reposition(position);
            bbox = shape.circle->BBox();
        }
    }
    
    icon.circle->Reposition(position);
}

// recursive method, returns true if prey collides with
//     node or any of its children
bool Prey::
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

// returns true iff prey collides with a node in the scene
bool Prey::
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
        // shouldn't check intersection with itself
        if (prey_list[i].position == position)
            continue;
        // check prey bounding box
        if (bbox.intersects(prey_list[i].bbox))
            return true;
    }
    for (unsigned int i = 0; i < hunter_list.size(); i++) {
        // check hunter bounding box
        if (bbox.intersects(hunter_list[i].bbox))
            return true;
    }
    if (bbox.intersects(bearBBox))
        return true;
    return false;
}
