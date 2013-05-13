// Source file for the R3 Box class 



// Include files 

#include "R3.h"
#include <stdlib.h>
#define TOLERANCE 0.000001


// Public variables 

const R3Box R3zero_box(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
const R3Box R3unit_box(-1.0, -1.0, -1.0, 1.0, 1.0, 1.0);
const R3Box R3null_box( INT_MAX, INT_MAX, INT_MAX, 
		       -INT_MAX, -INT_MAX, -INT_MAX);



// Public functions 

R3Box::
R3Box(void)
{
}



R3Box::
R3Box(const R3Box& box)
  : minpt(box.minpt),
    maxpt(box.maxpt)
{
}



R3Box::
R3Box(const R3Point& minpt, const R3Point& maxpt)
  : minpt(minpt), 
    maxpt(maxpt)
{
}



R3Box::
R3Box(double xmin, double ymin, double zmin,
      double xmax, double ymax, double zmax)
  : minpt(xmin, ymin, zmin),
    maxpt(xmax, ymax, zmax)
{
}



R3Point R3Box::
Corner (int xdir, int ydir, int zdir) const
{
  // Return corner point 
  return R3Point(Coord(xdir, R3_X), Coord(ydir, R3_Y), Coord(zdir, R3_Z));
}



int R3Box::
ShortestAxis (void) const
{
  // Compute length of each axis
  double dx = this->XLength();
  double dy = this->YLength();
  double dz = this->ZLength();

  // Return shortest axis
  if (dx < dy) {
    if (dx < dz) return R3_X;
    else return R3_Z;
  }
  else {
    if (dy < dz) return R3_Y;
    else return R3_Z;
  }    
}



int R3Box::
LongestAxis (void) const
{
  // Compute length of each axis
  double dx = this->XLength();
  double dy = this->YLength();
  double dz = this->ZLength();

  // Return longest axis
  if (dx > dy) {
    if (dx > dz) return R3_X;
    else return R3_Z;
  }
  else {
    if (dy > dz) return R3_Y;
    else return R3_Z;
  }    
}



double R3Box::
ShortestAxisLength (void) const
{
  // Compute length of each axis
  double dx = this->XLength();
  double dy = this->YLength();
  double dz = this->ZLength();

  // Return length of shortest axis
  if (dx < dy) {
    if (dx < dz) return dx;
    else return dz;
  }
  else {
    if (dy < dz) return dy;
    else return dz;
  }    
}



double R3Box::
LongestAxisLength (void) const
{
  // Compute length of each axis
  double dx = this->XLength();
  double dy = this->YLength();
  double dz = this->ZLength();

  // Return longest axis
  if (dx > dy) {
    if (dx > dz) return dx;
    else return dz;
  }
  else {
    if (dy > dz) return dy;
    else return dz;
  }    
}



double R3Box::
DiagonalLength (void) const
{
  // Return length of Box along diagonal
  return R3Distance(minpt, maxpt);
}



R3Point R3Box::
ClosestPoint(const R3Point& point) const
{
  // Return closest point in box
  R3Point closest(point);
  if (closest.X() < XMin()) closest[R3_X] = XMin();
  else if (closest.X() > XMax()) closest[R3_X] = XMax();
  if (closest.Y() < YMin()) closest[R3_Y] = YMin();
  else if (closest.Y() > YMax()) closest[R3_Y] = YMax();
  if (closest.Z() < ZMin()) closest[R3_Z] = ZMin();
  else if (closest.Z() > ZMax()) closest[R3_Z] = ZMax();
  return closest;
}



R3Point R3Box::
Centroid (void) const
{
  // Return center point
  return R3Point(XCenter(), YCenter(), ZCenter());
}



void R3Box::
Empty (void) 
{
  // Copy empty box 
  *this = R3null_box;
}



void R3Box::
Translate(const R3Vector& vector)
{
  // Move box by vector
  minpt += vector;
  maxpt += vector;
}



void R3Box::
Union (const R3Point& point) 
{
  // Expand this to include point
  if (minpt.X() > point.X()) minpt[0] = point.X();
  if (minpt.Y() > point.Y()) minpt[1] = point.Y();
  if (minpt.Z() > point.Z()) minpt[2] = point.Z();
  if (maxpt.X() < point.X()) maxpt[0] = point.X();
  if (maxpt.Y() < point.Y()) maxpt[1] = point.Y();
  if (maxpt.Z() < point.Z()) maxpt[2] = point.Z();
}



void R3Box::
Union (const R3Box& box) 
{
  // Expand this to include box
  if (minpt.X() > box.XMin()) minpt[0] = box.XMin();
  if (minpt.Y() > box.YMin()) minpt[1] = box.YMin();
  if (minpt.Z() > box.ZMin()) minpt[2] = box.ZMin();
  if (maxpt.X() < box.XMax()) maxpt[0] = box.XMax();
  if (maxpt.Y() < box.YMax()) maxpt[1] = box.YMax();
  if (maxpt.Z() < box.ZMax()) maxpt[2] = box.ZMax();
}



void R3Box::
Intersect (const R3Box& box) 
{
  // Intersect with box
  if (minpt.X() < box.XMin()) minpt[0] = box.XMin();
  if (minpt.Y() < box.YMin()) minpt[1] = box.YMin();
  if (minpt.Z() < box.ZMin()) minpt[2] = box.ZMin();
  if (maxpt.X() > box.XMax()) maxpt[0] = box.XMax();
  if (maxpt.Y() > box.YMax()) maxpt[1] = box.YMax();
  if (maxpt.Z() > box.ZMax()) maxpt[2] = box.ZMax();
}



void R3Box::
Transform (const R3Matrix& matrix)
{
  // Do not transform empty box
  if (IsEmpty()) return;

  // Transform box 
  R3Box tmp = R3null_box;
  tmp.Union(matrix * Corner(0,0,0));
  tmp.Union(matrix * Corner(0,0,1));
  tmp.Union(matrix * Corner(0,1,0));
  tmp.Union(matrix * Corner(0,1,1));
  tmp.Union(matrix * Corner(1,0,0));
  tmp.Union(matrix * Corner(1,0,1));
  tmp.Union(matrix * Corner(1,1,0));
  tmp.Union(matrix * Corner(1,1,1));
  *this = tmp;
}



void R3Box::
Reset(const R3Point& min, const R3Point& max)
{
  // Move box by vector
  minpt = min;
  maxpt = max;
}

// utility function
int sign(double x) {
    if (x > 0) return 1;
    if (x < 0) return -1;
    return 0;
}

// returns true iff this box aligns with that box
// ONLY WORKS IF BOTH BOXES ARE AXIS-ALIGNED
bool R3Box::
intersects(const R3Box& box)
{
    // have to overlap in all three planes - x, y, z
    if (XMax() < box.XMin() || XMin() > box.XMax())
        return false; // doesn't overlap in x
    if (YMax() < box.YMin() || YMin() > box.YMax())
        return false; // doesn't overlap in y
    if (ZMax() < box.ZMin() || ZMin() > box.ZMax())
        return false; // doesn't overlap in z
    return true;
}

/*
bool R3Box::
intersects(const R3Box& box)
{
    vector<R3Plane> this_faces;
    vector<R3Plane> that_faces;
    vector<R3Point> this_vertices;
    vector<R3Point> that_vertices;
    
    R3Point xyz = R3Point(box.XMin(), box.YMin(), box.ZMin());
    R3Point Xyz = R3Point(box.XMax(), box.YMin(), box.ZMin());
    R3Point xYz = R3Point(box.XMin(), box.YMax(), box.ZMin());
    R3Point xyZ = R3Point(box.XMin(), box.YMin(), box.ZMax());
    R3Point XYz = R3Point(box.XMax(), box.YMax(), box.ZMin());
    R3Point XyZ = R3Point(box.XMax(), box.YMin(), box.ZMax());
    R3Point xYZ = R3Point(box.XMin(), box.YMax(), box.ZMax());
    R3Point XYZ = R3Point(box.XMax(), box.YMax(), box.ZMax());
    
    that_vertices.push_back(xyz);
    that_vertices.push_back(Xyz);
    that_vertices.push_back(xYz);
    that_vertices.push_back(xyZ);
    that_vertices.push_back(XYz);
    that_vertices.push_back(XyZ);
    that_vertices.push_back(xYZ);
    that_vertices.push_back(XYZ);
    
    that_faces.push_back(R3Plane(xyz, Xyz, xyZ));
    that_faces.push_back(R3Plane(xYz, XYz, xYZ));
    that_faces.push_back(R3Plane(xyz, Xyz, xYz));
    that_faces.push_back(R3Plane(xyZ, XyZ, xYZ));
    that_faces.push_back(R3Plane(xyz, xYz, xyZ));
    that_faces.push_back(R3Plane(Xyz, XYz, XyZ));
    
    xyz = R3Point(XMin(), YMin(), ZMin());
    Xyz = R3Point(XMax(), YMin(), ZMin());
    xYz = R3Point(XMin(), YMax(), ZMin());
    xyZ = R3Point(XMin(), YMin(), ZMax());
    XYz = R3Point(XMax(), YMax(), ZMin());
    XyZ = R3Point(XMax(), YMin(), ZMax());
    xYZ = R3Point(XMin(), YMax(), ZMax());
    XYZ = R3Point(XMax(), YMax(), ZMax());
    
    this_vertices.push_back(xyz);
    this_vertices.push_back(Xyz);
    this_vertices.push_back(xYz);
    this_vertices.push_back(xyZ);
    this_vertices.push_back(XYz);
    this_vertices.push_back(XyZ);
    this_vertices.push_back(xYZ);
    this_vertices.push_back(XYZ);
    
    this_faces.push_back(R3Plane(xyz, Xyz, xyZ));
    this_faces.push_back(R3Plane(xYz, XYz, xYZ));
    this_faces.push_back(R3Plane(xyz, Xyz, xYz));
    this_faces.push_back(R3Plane(xyZ, XyZ, xYZ));
    this_faces.push_back(R3Plane(xyz, xYz, xyZ));
    this_faces.push_back(R3Plane(Xyz, XYz, XyZ));
    
    // iterate over each face of both boxes
    int not_same_count = 0;
    for (unsigned int i = 0; i < this_faces.size(); i++) {
        int dot_sign = 0;
        bool same_side = true;
        for (unsigned int j = 0; j < that_vertices.size(); j++) {
            R3Point planepoint = R3Point(that_vertices[j]);
            planepoint.Project(this_faces[i]);
            R3Vector v = that_vertices[j] - planepoint;
            double dotProduct = v.Dot(this_faces[i].Normal());
            //if (abs(dotProduct) < TOLERANCE)
                //same_side = false;
            if (j == 0)
                dot_sign = sign(dotProduct);
            else {
                if (sign(dotProduct) != dot_sign)
                    same_side = false;
            }
        }
        if (!same_side)
            not_same_count++;
            
        same_side = true;
        for (unsigned int j = 0; j < this_vertices.size(); j++) {
            R3Point planepoint = R3Point(this_vertices[j]);
            planepoint.Project(that_faces[i]);
            R3Vector v = this_vertices[j] - planepoint;
            double dotProduct = v.Dot(that_faces[i].Normal());
            //if (abs(dotProduct) < TOLERANCE)
                //same_side = false;
            if (j == 0)
                dot_sign = sign(dotProduct);
            else {
                if (sign(dotProduct) != dot_sign)
                    same_side = false;
            }
        }
        if (!same_side)
            not_same_count++;
    }
    return (not_same_count >= 2);
}
*/

void R3Box::
Draw(void) const
{
  // Get box corner points 
  R3Point corners[8];
  corners[0] = Corner(0, 0, 0);
  corners[1] = Corner(0, 0, 1);
  corners[2] = Corner(0, 1, 1);
  corners[3] = Corner(0, 1, 0);
  corners[4] = Corner(1, 0, 0);
  corners[5] = Corner(1, 0, 1);
  corners[6] = Corner(1, 1, 1);
  corners[7] = Corner(1, 1, 0);

  // Get normals, texture coordinates, etc.
  static GLdouble normals[6][3] = {
    { -1.0, 0.0, 0.0 },
    { 1.0, 0.0, 0.0 },
    { 0.0, -1.0, 0.0 },
    { 0.0, 1.0, 0.0 },
    { 0.0, 0.0, -1.0 },
    { 0.0, 0.0, 1.0 }
  };
  static GLdouble texcoords[4][2] = {
    { 0.0, 0.0 },
    { 1.0, 0.0 },
    { 1.0, 1.0 },
    { 0.0, 1.0 }
  };
  static int surface_paths[6][4] = {
    { 3, 0, 1, 2 },
    { 4, 7, 6, 5 },
    { 0, 4, 5, 1 },
    { 7, 3, 2, 6 },
    { 3, 7, 4, 0 },
    { 1, 5, 6, 2 }
  };
	
	// normal draw
	glBegin(GL_QUADS);
	for (int i = 0; i < 6; i++) {
		glNormal3d(normals[i][0], normals[i][1], normals[i][2]);
		for (int j = 0; j < 4; j++) {
			const R3Point& p = corners[surface_paths[i][j]];
			glTexCoord2d(texcoords[j][0], texcoords[j][1]);
			glVertex3d(p[0], p[1], p[2]);
		}
	}
	glEnd();

	/*      For tiling textures, doesn't currently work but I saved it
	// Draw box
	glBegin(GL_QUADS);

	double tex_size = 100;
	for (int i = 0; i < 6; i++) {
		R3Point pLL = corners[surface_paths[i][0]];
	R3Point pLR = corners[surface_paths[i][1]];
	R3Point pUL = corners[surface_paths[i][3]];
	double dim1rep = (pLR - pLL).Length();
	fprintf(stderr, "dim1rep: %f\n", dim1rep);
	double dim2rep = (pUL - pLL).Length();
	R3Vector dim1V = pLR - pLL;
	dim1V.Normalize();
	R3Vector dim2V = pUL - pLL;
	dim2V.Normalize();
	
	for (int k = 0; k < (dim1rep / tex_size); k++) {
		for (int j = 0; j < (dim2rep / tex_size); j++) {
			glNormal3d(normals[i][0], normals[i][1], normals[i][2]);
			R3Point pTemp = pLL + i * dim1V + j * dim2V;
			glTexCoord2d(texcoords[0][0], texcoords[0][1]);
			glVertex3d(pTemp[0], pTemp[1], pTemp[2]);
			
			pTemp = pLL + (i + 1) * dim1V + j * dim2V;
			glTexCoord2d(texcoords[1][0], texcoords[1][1]);
			glVertex3d(pTemp[0], pTemp[1], pTemp[2]);
			
			pTemp = pLL + (i + 1) * dim1V + (j + 1) * dim2V;
			glTexCoord2d(texcoords[2][0], texcoords[2][1]);
			glVertex3d(pTemp[0], pTemp[1], pTemp[2]);
			
			pTemp = pLL + i * dim1V + (j+1) * dim2V;
			glTexCoord2d(texcoords[3][0], texcoords[3][1]);
			glVertex3d(pTemp[0], pTemp[1], pTemp[2]);
			
			/*
				const R3Point& p = corners[surface_paths[i][j]];
				glTexCoord2d(texcoords[j][0], texcoords[j][1]);
				glVertex3d(p[0], p[1], p[2]); 
				
			}
			
			
		}
	} 
    
  glEnd();
  */
}



void R3Box::
Outline(void) const
{
  // Draw box in wireframe
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  Draw();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}



void R3Box::
Print(FILE *fp) const
{
  // Print min and max points
  fprintf(fp, "(%g %g %g) (%g %g %g)", minpt[0], minpt[1], minpt[2], maxpt[0], maxpt[1], maxpt[2]);
}




