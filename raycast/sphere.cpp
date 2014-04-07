#include "sphere.h"
#include <stdlib.h>
#include <math.h>
#include <float.h>

/**********************************************************************
 * This function intersects a ray with a given sphere 'sph'. You should
 * use the parametric representation of a line and do the intersection.
 * The function should return the parameter value for the intersection, 
 * which will be compared with others to determine which intersection
 * is closest. The value -1.0 is returned if there is no intersection
 *
 * If there is an intersection, the point of intersection should be
 * stored in the "hit" variable
 **********************************************************************/
float intersect_sphere(Point o, Vector u, Spheres *sph, Point *hit) {
    // Wikipedia, Line-sphere intersection
    Vector o_c = get_vec(sph->center, o);
    float a = vec_dot(u, u);
    float b = 2 * vec_dot(u, o_c);
    float c = vec_dot(o_c, o_c) - sph->radius * sph->radius;
    float d = b * b - 4 * a * c;
    if (d >= 0) {
        float t0 = (-b - sqrt(d)) / (2 * a);
//        float t1 = (-b + sqrt(d)) / (2 * a);
        if (t0 < 0) {
            return -1.0;
        }
        hit->x = o.x + t0 * u.x;
        hit->y = o.y + t0 * u.y;
        hit->z = o.z + t0 * u.z;
        return t0;
    }
    return -1.0;
}

bool is_in_shadow(Point o, Vector u, Spheres *sph, Spheres *source) {
    while (sph) {
        if (sph->index != source->index) {
            Vector o_c = get_vec(sph->center, o);
            float a = vec_dot(u, u);
            float b = 2 * vec_dot(u, o_c);
            float c = vec_dot(o_c, o_c) - sph->radius * sph->radius;
            float d = b * b - 4 * a * c;
            if (d >= 0) {
                float t0 = (-b - sqrt(d)) / (2 * a);
                float t1 = (-b + sqrt(d)) / (2 * a);
                if (t0 > 0 || t1 > 0) {
                    return true;
                }
            }
        }
        sph = sph->next;
    }
    return false;
}

/*********************************************************************
 * This function returns a pointer to the sphere object that the
 * ray intersects first; NULL if no intersection. You should decide
 * which arguments to use for the function. For exmaple, note that you
 * should return the point of intersection to the calling function.
 **********************************************************************/

Spheres *intersect_scene(Point ray_o, Vector ray_u, Spheres *sphere_list, Point *hit) {
    Spheres *sph = sphere_list;
    Spheres *first_intersect_sph = NULL;
    float first_t = FLT_MAX;
    while (sph) {
        float cur_t = intersect_sphere(ray_o, ray_u, sph, hit);
        if (cur_t != -1.0 && first_t > cur_t) {
            first_t = cur_t;
            first_intersect_sph = sph;
        }
        sph = sph->next;
    }
	return first_intersect_sph;
}

/*****************************************************
 * This function adds a sphere into the sphere list
 *
 * You need not change this.
 *****************************************************/
Spheres *add_sphere(Spheres *slist, Point ctr, float rad, float amb[],
		    float dif[], float spe[], float shine, 
		    float refl, int sindex) {
  Spheres *new_sphere;

  new_sphere = (Spheres *)malloc(sizeof(Spheres));
  new_sphere->index = sindex;
  new_sphere->center = ctr;
  new_sphere->radius = rad;
    (new_sphere->mat_ambient)[0] = amb[0];
    (new_sphere->mat_ambient)[1] = amb[1];
    (new_sphere->mat_ambient)[2] = amb[2];
    (new_sphere->mat_diffuse)[0] = dif[0];
    (new_sphere->mat_diffuse)[1] = dif[1];
    (new_sphere->mat_diffuse)[2] = dif[2];
    (new_sphere->mat_specular)[0] = spe[0];
    (new_sphere->mat_specular)[1] = spe[1];
    (new_sphere->mat_specular)[2] = spe[2];
  new_sphere->mat_shineness = shine;
  new_sphere->reflectance = refl;
  new_sphere->next = NULL;

  if (slist == NULL) { // first object
    slist = new_sphere;
  } else { // insert at the beginning
    new_sphere->next = slist;
    slist = new_sphere;
  }

  return slist;
}

/******************************************
 * computes a sphere normal - done for you
 ******************************************/
Vector sphere_normal(Point q, Spheres *sph) {
  Vector rc;

  rc = get_vec(sph->center, q);
  normalize(&rc);
  return rc;
}
