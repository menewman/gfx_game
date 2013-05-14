// source file for the Bearpaw class

#include "R3/R3.h"
#include "R3Scene.h"
#include "Hunter.h"
#include "Bearpaw.h"

Bearpaw::
Bearpaw(void) {}

Bearpaw::
Bearpaw(R3Point position, R3Point playeroffset, R3Vector velocity, R3Shape shape) //double speed, R3Point playeroffset, R3Vector velocity, R3Shape shape)
    : //mass(mass),
      //speed(speed),
      position(position),
      playeroffset(playeroffset),
      velocity(velocity),
      shape(shape)
      //bbox(R3null_box)
{
}

void Bearpaw::
setVelocity(const R3Vector& newVelocity)
{
    velocity = newVelocity;
}

void Bearpaw::
updatePosition(double delta_time, R3Point playerPos) { //, double bound, R3Scene *scene, vector<Bearpaw>& Bearpaw_list, vector<Hunter>& hunter_list, R3Box bearBBox)
{
    fprintf(stderr, "center at {%f, %f, %f}\n",shape.mesh->Center().X(), shape.mesh->Center().Y(), shape.mesh->Center().Z());
    position = playerPos + playeroffset;
    
    R3Vector toNew = position - shape.mesh->Center();
    fprintf(stderr, "moving by {%f, %f, %f}\n",toNew.X(), toNew.Y(), toNew.Z());
	shape.mesh->Translate(toNew.X(), toNew.Y(), toNew.Z());
	bbox = shape.mesh->bbox;
    
    /*
    R3Vector 
    fromPlayer.SetY(0);

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
    

    if (shape.type == R3_MESH_SHAPE) {    // only going to use a mesh for the bear paw
        R3Vector toNew = position - shape.mesh->Center();
		shape.mesh->Translate(toNew.X(), toNew.Y(), toNew.Z());
		bbox = shape.mesh->bbox;
    }
    */
    
    
} }

// recursive method, returns true if Bearpaw collides with
//     node or any of its children

bool Bearpaw::
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
/*
// returns true iff Bearpaw collides with a node in the scene
bool Bearpaw::
collides(R3Scene *scene, vector<Bearpaw>& Bearpaw_list, vector<Hunter>& hunter_list, R3Box bearBBox)
{
    // recurse through the scene's nodes
    R3Node *root = scene->Root();
    for (unsigned int i = 0; i < root->children.size(); i++) {
        if (collides(scene, root->children[i])) {
            return true;
        }
    }
    for (unsigned int i = 0; i < Bearpaw_list.size(); i++) {
        // shouldn't check intersection with itself
        if (Bearpaw_list[i].position == position)
            continue;
        // check Bearpaw bounding box
        if (bbox.intersects(Bearpaw_list[i].bbox))
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
*/

void Bearpaw::
draw()
{
    
    //shape.mesh->Draw();

    /*R3Sphere hsphere = R3Sphere(position, .5);
    hsphere.Draw();*/
    /*
    R3Mesh hmesh;
    const char* huntermeshloc = "input/orthancclaw.off";
    hmesh.Read(huntermeshloc);
    hmesh.Scale(.2,.2,.2);
    hmesh.Translate(position.X(),position.Y(),position.Z());
    hmesh.Draw(); */
    
}
