#include <stdio.h>
#include "include/Angel.h"
//#include <GL/glut.h>
#include <math.h>
#include "global.h"
#include "sphere.h"
#include <cstdlib>
#include <ctime>

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
extern Chessboard chess_board;

// light 1 position and color
extern Point light1;
extern RGB_float light1_ambient;
extern RGB_float light1_diffuse;
extern RGB_float light1_specular;

// global ambient term
extern RGB_float global_ambient;

// light decay parameters
extern float decay_a;
extern float decay_b;
extern float decay_c;

extern int shadow_on;
extern int reflection_on;
extern int refraction_on;
extern int chess_board_on;
extern int diffuse_on;
extern int super_sampling_on;
extern int step_max;

/////////////////////////////////////////////////////////////////////

RGB_float phong(Point q, Vector v, Vector surf_norm, Spheres *sph, bool in_shadow) {
	RGB_float color = {0, 0, 0};
    RGB_float ambient = {0, 0, 0};
    RGB_float diffuse = {0, 0, 0};
    RGB_float specular = {0, 0, 0};

    // global ambient
    ambient = clr_add(ambient, clr_scale(global_ambient, 2 * sph->reflectance));

    // light ambient
    ambient = clr_add(ambient, clr_multi(light1_ambient, sph->mat_ambient));

    if (in_shadow) {
        return ambient;
    }

    // light diffuse
    Vector l = get_vec(q, light1);
    float d = vec_len(l);
    normalize(&l);
    float diffuse_coeff = vec_dot(surf_norm, l);
    diffuse = clr_multi(light1_diffuse, sph->mat_diffuse);
    diffuse = clr_scale(diffuse, diffuse_coeff);

    // light specular
    float cos = vec_dot(surf_norm, l);
    cos = cos < 0 ? 0.0 : cos;
    Vector r = vec_minus(vec_scale(surf_norm, 2 * cos), l);
    normalize(&r);
    float specular_coeff = pow(vec_dot(r, v), sph->mat_shineness);
    specular = clr_multi(light1_specular, sph->mat_specular);
    specular = clr_scale(specular, specular_coeff);

    // attenuation
    float attenuation_coeff = 1.0 / (decay_a + decay_b * d + decay_c * pow(d, 2));
    color = clr_add(ambient, clr_scale(clr_add(diffuse, specular), attenuation_coeff));
    return color;
}

/************************************************************************
 * This is the recursive ray tracer - you need to implement this!
 * You should decide what arguments to use.
 ************************************************************************/
RGB_float recursive_ray_trace(Point ray_o, Vector ray_u, int step) {
	RGB_float color = background_clr;

    Spheres *first_intersect_sph = NULL;
    Point hit;
    first_intersect_sph = intersect_scene(ray_o, ray_u, scene, &hit);
    bool in_shadow = false;
    if (chess_board_on) {
        Point board_hit;
        if (intersect_board(ray_o, ray_u, &board_hit)) {
            color = chess_board_color(board_hit);
            Vector board_shadow_ray = get_vec(board_hit, light1);
            if (is_in_shadow(board_hit, board_shadow_ray, scene, NULL)) {
                color = clr_scale(color, 0.5);
            }
            if (reflection_on && step < step_max) {
                Vector board_reflect = ray_u;
                board_reflect.y = -board_reflect.y;
                RGB_float reflect_color = recursive_ray_trace(board_hit, board_reflect, step + 1);
//                if (reflect_color.r != background_clr.r &&
//                    reflect_color.g != background_clr.g &&
//                    reflect_color.b != background_clr.b) {
                    color = clr_add(clr_scale(color, 0.7), clr_scale(reflect_color, 0.3));
//                }
            }
        }
    }
    if (first_intersect_sph) {
        Vector surf_norm = sphere_normal(hit, first_intersect_sph);
        normalize(&surf_norm);
        Vector l = get_vec(hit, ray_o);
        normalize(&l);
        Vector v = get_vec(hit, eye_pos);
        normalize(&v);
        Vector shadow_ray = get_vec(hit, light1);
        if (shadow_on) {
            in_shadow = is_in_shadow(hit, shadow_ray, scene, first_intersect_sph);
        }
        color = phong(ray_o, v, surf_norm, first_intersect_sph, in_shadow);
        if (reflection_on && step < step_max) {
            Vector r = vec_minus(vec_scale(surf_norm, 2 * vec_dot(surf_norm, l)), l);
            normalize(&r);
            RGB_float reflected_color = recursive_ray_trace(hit, r, step + 1);
            reflected_color = clr_scale(reflected_color, first_intersect_sph->reflectance);
            color = clr_add(color, reflected_color);

            if (diffuse_on) {
                srand(static_cast <unsigned> (time(0)));
                for (int i = 0; i < RANDOM_RAYS; ++i) {
                    Vector random_ray = {
                        static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
                        static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
                        static_cast <float> (rand()) / static_cast <float> (RAND_MAX)};
                    normalize(&random_ray);
                    RGB_float diffuse_color = recursive_ray_trace(hit, random_ray, step + 1);
                    diffuse_color = clr_scale(diffuse_color, first_intersect_sph->reflectance);
                    color = clr_add(color, clr_scale(diffuse_color, 0.1));
                }
                color = clr_scale(color, 1.0 / 1.5);
            }
        }
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

    if (chess_board_on) {
        chess_board.center = {0, -3, -5};
        chess_board.norm = {0, -10, 3};
        chess_board.width = 8;
        chess_board.length = 8;
        chess_board.reflectance = 0.3;
        chess_board.ambient = {0.2, 0.2, 0.2};
    }

  // ray is cast through center of pixel
  cur_pixel_pos.x = x_start + 0.5 * x_grid_size;
  cur_pixel_pos.y = y_start + 0.5 * y_grid_size;
  cur_pixel_pos.z = image_plane;

  for (i=0; i<win_height; i++) {
      for (j=0; j<win_width; j++) {
          ray = get_vec(eye_pos, cur_pixel_pos);
          ret_color = recursive_ray_trace(cur_pixel_pos, ray, 0);
          if (super_sampling_on) {
              Point cur_pos;
              Vector cur_ray;
              Point sample_pos[4] = {
                  {cur_pixel_pos.x - x_grid_size / 4, cur_pixel_pos.y - y_grid_size / 4, image_plane},
                  {cur_pixel_pos.x + x_grid_size / 4, cur_pixel_pos.y - y_grid_size / 4, image_plane},
                  {cur_pixel_pos.x + x_grid_size / 4, cur_pixel_pos.y + y_grid_size / 4, image_plane},
                  {cur_pixel_pos.x - x_grid_size / 4, cur_pixel_pos.y + y_grid_size / 4, image_plane}};
              for (int k = 0; k < 4; ++k) {
                  cur_pos = sample_pos[k];
                  cur_ray = get_vec(eye_pos, cur_pos);
                  ret_color = clr_add(ret_color, recursive_ray_trace(cur_pos, cur_ray, 0));
              }
              ret_color = clr_scale(ret_color, 0.2);
          }

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
