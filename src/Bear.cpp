// source file for the Bear class

#include "R3/R3.h"
#include "Bear.h"
#define TOLERANCE 0.00001

Bear::
Bear(void)
{
}

Bear::
Bear(double mass, double speed, double height, R3Vector velocity, R3Point position)
    : mass(mass),
      speed(speed),
      height(height),
      velocity(velocity),
      position(position),
      health(100),
      stamina(100),
      bbox(R3Box(position.X()-1.5, 0, position.Z()-1.5, position.X()+1.5, height+1, position.Z()+1.5))
{
}

void Bear::
setVelocity(const R3Vector& newVelocity) 
{
    velocity = newVelocity;
}

void Bear::
setPosition(const R3Point& newPosition)
{
    position = newPosition;
}

void Bear::
updatePosition(double delta_time)
{
    if (position.Y() - height < TOLERANCE)
        return;
    R3Vector grav = R3Vector(0,-15,0);
    R3Vector f_grav = mass * grav;
    R3Vector accel = f_grav / mass;
    position.Translate(delta_time * velocity);
    velocity += delta_time * accel;
    if (position.Y() <= height) {
        position.SetY(height);
        velocity.SetY(0);
    }
}
