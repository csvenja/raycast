#include "sphere.h"
#include <stdlib.h>
#include <math.h>
#include <float.h>

extern Chessboard chess_board;
extern RGB_float background_clr;

void set_transparent(Spheres *sph) {
    while (sph) {
        sph->transparent = true;
        sph = sph->next;
    }
}

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
        if (source == NULL || sph->index != source->index) {
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

bool intersect_board(Point o, Vector u, Point *hit) {
    // Wikipedia Line-plane intersection
    Vector p_l = get_vec(chess_board.center, o);
    float p_l_n = vec_dot(u, chess_board.norm);
    float l_n = vec_dot(p_l, chess_board.norm);

    // parallel
    if (p_l_n == 0 && l_n != 0) {
        return false;
    }
    else {
        float t = -l_n / p_l_n;
        if (t > 0) {
            hit->x = o.x + t * u.x;
            hit->y = o.y + t * u.y;
            hit->z = o.z + t * u.z;
            if (hit->x > chess_board.center.x + chess_board.width / 2 ||
                hit->x < chess_board.center.x - chess_board.width / 2 ||
                hit->z > chess_board.center.z + chess_board.length / 2 ||
                hit->z < chess_board.center.z - chess_board.length / 2) {
                return false;
            }
            return true;
        }
    }
    return false;
}

RGB_float chess_board_color(Point p)
{

    if (p.x > 0) {
        p.x += 1;
    }
    if ((int(p.x) + int(p.z)) % 2 == 0) {
        return {0, 0, 0};
    }
    else {
        return {1, 1, 1};
    }
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
    (new_sphere->mat_ambient).r = amb[0];
    (new_sphere->mat_ambient).g = amb[1];
    (new_sphere->mat_ambient).b = amb[2];
    (new_sphere->mat_diffuse).r = dif[0];
    (new_sphere->mat_diffuse).g = dif[1];
    (new_sphere->mat_diffuse).b = dif[2];
    (new_sphere->mat_specular).r = spe[0];
    (new_sphere->mat_specular).g = spe[1];
    (new_sphere->mat_specular).b = spe[2];
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
