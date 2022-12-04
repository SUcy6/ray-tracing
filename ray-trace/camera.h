#ifndef CAMERA
#define CAMERA

#include "ray.h"

vec3 random_in_unit_disk() {
    vec3 p;
    do {
        p = 2.0*vec3(drand48(), drand48(), 0.0) - vec3(1,1,0);
    }while(dot(p,p) >= 1.0);
    return p;
}

class camera {
    public:
        vec3 lower_left_corner, horizontal, vertical, origin;
        float lens_radius;
        vec3 u,v,w;
        camera(vec3 lookfrom, vec3 lookat, vec3 up, float vfov, float aspect, float aperture, float focus_dist){
            lens_radius = aperture / 2;
            float theta = vfov * M_PI/180;
            float half_height = tan(theta/2);
            float half_width = aspect * half_height;

            origin = lookfrom;
            w = unit_vector(lookfrom - lookat);
            u = unit_vector(cross(up, w));
            v = cross(w, u);
            lower_left_corner = origin - half_width*u*focus_dist - half_height*v*focus_dist - focus_dist*w;
            horizontal = 2*half_width*u*focus_dist;
            vertical = 2*half_height*v*focus_dist;
        }
        ray get_ray(float s, float t) {
            vec3 rd = lens_radius * random_in_unit_disk();
            vec3 offset = u*rd.x() +v*rd.y();
            ray r(origin+offset, lower_left_corner + s*horizontal + t*vertical - origin-offset);
            return r;
        }

};



#endif