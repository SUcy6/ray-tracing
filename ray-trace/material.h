#ifndef MATERIAL
#define MATERIAL

#include "ray.h"
#include "hitable.h"

struct hit_record;

class material {
    public:
    virtual bool scatter(const ray &ray_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const = 0; // how to react to the ray

};

class lambertian : public material {
    public:
    vec3 albedo;
    lambertian(const vec3& a): albedo(a) {}
    virtual bool scatter(const ray &ray_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const {
        vec3 target = rec.p + rec.normal +random_in_unit_sphere();
        scattered = ray(rec.p, target-rec.p);
        attenuation = albedo;
        return true;
    }
};

class metal : public material {
    public:
    vec3 albedo;
    int fuzz;
    metal(const vec3& a, float f) : albedo(a){
        if (f<1) {
            fuzz = f;
        }
        else{
            fuzz = 1;
        }
    }
    virtual bool scatter(const ray &ray_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const {
        vec3 reflected = reflect(unit_vector(ray_in.direction()), rec.normal);
        scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
        attenuation = albedo;
        return dot(scattered.direction(), rec.normal) > 0;
    }
};

class dielectric : public material {
    public:
    vec3 albedo;
    int fuzz;
    float ref_idx;
    dielectric(float ri) : ref_idx(ri){ }
    virtual bool scatter(const ray &ray_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const {
        vec3 outward_normal;
        vec3 reflected = reflect(ray_in.direction(), rec.normal);
        float ni_over_nt;
        attenuation = vec3(1.0, 1.0, 1.0);
        vec3 refracted;
        float reflect_prob;
        float cosine;

        if (dot(ray_in.direction(), rec.normal) > 0) {
            outward_normal = -rec.normal;
            ni_over_nt = ref_idx;
            cosine = dot(ray_in.direction(), rec.normal) / ray_in.direction().length();
            cosine = sqrt(1 - ref_idx * ref_idx* (1- cosine*cosine));
        }
        else {
            outward_normal = rec.normal;
            ni_over_nt = 1.0 / ref_idx;
            cosine = -dot(ray_in.direction(), rec.normal) / ray_in.direction().length();
        }

        if (refract(ray_in.direction(), outward_normal, ni_over_nt, refracted)){
            
            reflect_prob = schlick(cosine, ref_idx);
        }
        else {
            reflect_prob = 1.0;
        }

        if(drand48() < reflect_prob) {
            scattered = ray(rec.p, reflected);
        }
        else {
            scattered = ray(rec.p, refracted);
        }

        return true;
    }
};



#endif