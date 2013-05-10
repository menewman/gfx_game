// source file for the Prey class

#include "R3/R3.h"
#include "R3Scene.h"
#include "Prey.h"

Prey::
Prey(void) {}

Prey::
Prey(double mass, double speed, R3Point position, R3Vector velocity, R3Shape shape)
    : mass(mass),
      speed(speed),
      position(position),
      velocity(velocity),
      shape(shape)
{}

void Prey::
setPosition(const R3Point& newPosition)
{
    position = newPosition;
}

void Prey::
setVelocity(const R3Vector& newVelocity)
{
    velocity = newVelocity;
}

void Prey::
updatePosition(double delta_time, R3Point playerPos, double bound)
{
    R3Vector fromPlayer = position - playerPos;
    fromPlayer.SetY(0);
    fromPlayer.Normalize();
    
    position += fromPlayer*delta_time*speed;
    
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
    }
    else if (shape.type == R3_BOX_SHAPE) {
        // translate along the vector between centroid and new position
        R3Vector toNew = position - shape.box->Centroid();
        toNew.SetY(0);
        shape.box->Translate(toNew);
    }
    else if (shape.type == R3_CYLINDER_SHAPE) {
        shape.cylinder->Reposition(position);
    }
    else if (shape.type == R3_CONE_SHAPE) {
        shape.cone->Reposition(position);
    }
    else if (shape.type == R3_MESH_SHAPE) {
        ; // ????? how do you move meshes anyway?
    }
    else if (shape.type == R3_SEGMENT_SHAPE) {
        // for segment: 'position' is arbitrarily the centroid
        R3Vector toNew = position - shape.segment->Centroid();
        toNew.SetY(0);
        shape.segment->Translate(toNew);
    }
    else if (shape.type == R3_CIRCLE_SHAPE) {
        shape.circle->Reposition(position);
    }
}