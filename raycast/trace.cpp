#include <stdio.h>
#include "include/Angel.h"
//#include <GL/glut.h>
#include <math.h>
#include "global.h"
#include "sphere.h"

//
// Global variables
//
extern int win_width;
extern int win_height;

extern GLfloat frame[WIN_HEIGHT][WIN_WIDTH][3];  

extern float image_width;
extern float image_height;

extern Point eye_pos;
extern float image_plane;
extern RGB_float background_clr;
extern RGB_float null_clr;

extern Spheres *scene;

// light 1 position and color
extern Point light1;
extern float light1_ambient[3];
extern float light1_diffuse[3];
extern float light1_specular[3];

// global ambient term
extern RGB_float global_ambient;

// light decay parameters
extern float decay_a;
extern float decay_b;
extern float decay_c;

extern int shadow_on;
extern int step_max;

/////////////////////////////////////////////////////////////////////

RGB_float phong(Point q, Vector v, Vector surf_norm, Spheres *sph) {
	RGB_float color = {0, 0, 0};
    RGB_float diffuse = {0, 0, 0};
    RGB_float specular = {0, 0, 0};

    // global ambient
    color = clr_add(color, clr_scale(global_ambient, sph->reflectance));

    // light ambient
    color.r += light1_ambient[0] * sph->mat_ambient[0];
    color.g += light1_ambient[1] * sph->mat_ambient[1];
    color.b += light1_ambient[2] * sph->mat_ambient[2];

    // light diffuse
    Vector l = get_vec(q, light1);
    float d = vec_len(l);
    normalize(&l);
    float diffuse_coeff = vec_dot(surf_norm, l);
    diffuse.r = light1_diffuse[0] * sph->mat_diffuse[0];
    diffuse.g = light1_diffuse[1] * sph->mat_diffuse[1];
    diffuse.b = light1_diffuse[2] * sph->mat_diffuse[2];
    diffuse = clr_scale(diffuse, diffuse_coeff);

    // light specular
    Vector r = vec_minus(vec_scale(surf_norm, 2 * vec_dot(surf_norm, l)), l);
    normalize(&r);
    float specular_coeff = pow(vec_dot(r, v), sph->mat_shineness);
    specular.r = light1_specular[0] * sph->mat_specular[0];
    specular.g = light1_specular[1] * sph->mat_specular[1];
    specular.b = light1_specular[2] * sph->mat_specular[2];
    specular = clr_scale(specular, specular_coeff);

    // attenuation
    float attenuation_coeff = 1 / (decay_a + decay_b * d + decay_c * d * d);
//    specularIllumination(sph, &color, r, v, attenuation_coeff);

//    color = clr_add(color, clr_scale(clr_add(diffuse, specular), attenuation_coeff));
    color = clr_add(color, clr_scale(diffuse, attenuation_coeff));
//    color = clr_add(color, clr_scale(specular, attenuation_coeff));
	return color;
}

//bool in_shadow() {
//
//}

/************************************************************************
 * This is the recursive ray tracer - you need to implement this!
 * You should decide what arguments to use.
 ************************************************************************/
RGB_float recursive_ray_trace(Point ray_o, Vector ray_u, int step) {
	RGB_float color = background_clr;
    Spheres *first_intersect_sph = NULL;
    Point hit;
    first_intersect_sph = intersect_scene(ray_o, ray_u, scene, &hit);
    if (first_intersect_sph) {
        Vector surf_norm = sphere_normal(hit, first_intersect_sph);
//        Vector shadow_ray = get_vec(hit, ray_o);
//        if (!in_shadow()) {
            color = phong(ray_o, ray_u, surf_norm, first_intersect_sph);
//        }
    }
	return color;
}

/*********************************************************************
 * This function traverses all the pixels and cast rays. It calls the
 * recursive ray tracer and assign return color to frame
 *
 * You should not need to change it except for the call to the recursive
 * ray tracer. Feel free to change other parts of the function however,
 * if you must.
 *********************************************************************/
void ray_trace() {
  int i, j;
  float x_grid_size = image_width / float(win_width);
  float y_grid_size = image_height / float(win_height);
  float x_start = -0.5 * image_width;
  float y_start = -0.5 * image_height;
  RGB_float ret_color;
  Point cur_pixel_pos;
  Vector ray;

  // ray is cast through center of pixel
  cur_pixel_pos.x = x_start + 0.5 * x_grid_size;
  cur_pixel_pos.y = y_start + 0.5 * y_grid_size;
  cur_pixel_pos.z = image_plane;

  for (i=0; i<win_height; i++) {
      for (j=0; j<win_width; j++) {
          ray = get_vec(eye_pos, cur_pixel_pos);
          ret_color = recursive_ray_trace(cur_pixel_pos, ray, 1);

      // Parallel rays can be cast instead using below
      //
//       ray.x = ray.y = 0;
//       ray.z = -1.0;
//       ret_color = recursive_ray_trace(cur_pixel_pos, ray, 1);

          frame[i][j][0] = GLfloat(ret_color.r);
          frame[i][j][1] = GLfloat(ret_color.g);
          frame[i][j][2] = GLfloat(ret_color.b);

          cur_pixel_pos.x += x_grid_size;
      }
      cur_pixel_pos.y += y_grid_size;
      cur_pixel_pos.x = x_start;
  }
}