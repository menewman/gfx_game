// source file for the Bear class

#include "R3/R3.h"
#include "Bear.h"

Bear::
Bear(void)
{
}

Bear::
Bear(double mass, double speed, double height, R3Vector velocity)
    : mass(mass),
      speed(speed),
      height(height),
      velocity(velocity)
{
}

void Bear::
setVelocity (const R3Vector& newVelocity) 
{
  velocity = newVelocity;
}
