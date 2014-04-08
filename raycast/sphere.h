/**********************************************************************
 * Some stuff to handle spheres
 **********************************************************************/
#include "vector.h"

typedef struct sphere {
  int index;               // identifies a sphere; must be greater than 0

  Point center;
  float radius;

    RGB_float mat_ambient;    // material property used in Phong model
    RGB_float mat_diffuse;
    RGB_float mat_specular;
    float mat_shineness;

  float reflectance;       // this number [0,1] determines how much 
                           // reflected light contributes to the color
                           // of a pixel
  struct sphere *next;
} Spheres;   // a list of spheres

typedef struct chessboard {
    Point center;
    Vector norm;
    int width;
    int length;
    float reflectance;
    RGB_float ambient;
} Chessboard;

// intersect ray with sphere
Spheres *intersect_scene(Point, Vector, Spheres *, Point *);
bool is_in_shadow(Point o, Vector u, Spheres *sph, Spheres *source);
// return the unit normal at a point on sphere
Vector sphere_normal(Point, Spheres *);
// add a sphere to the sphere list
Spheres *add_sphere(Spheres *, Point, float, float [], float [], float [], float, float, int);
bool intersect_board(Point o, Vector u, Point *hit);
RGB_float chess_board_color(Point p);
