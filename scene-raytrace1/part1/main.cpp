#include <iostream>
#include <vector>
#include "Common.h"
#include "Scene.h"
#include "Camera.h"
#include "Material.h"
#include "Hittable.h"
#include "Utils/lodepng.h"
#include <cmath>
#include <glm/glm.hpp>

using namespace std;

const int kMaxTraceDepth = 5;

Color TraceRay(const Ray& ray,
               const std::vector<LightSource>& light_sources,
               const Hittable& scene,
               int trace_depth);

Color Shade(const std::vector<LightSource>& light_sources, const Hittable& hittable_collection, const HitRecord& hit_record, int trace_depth) {

    Color color(0.f, 0.f, 0.f);
    color = hit_record.material.ambient * hit_record.material.k_a;
    float shininess = 10.f;

    // Defuse and spectacular terms due to each light source
    for (auto i = light_sources.begin(); i != light_sources.end(); ++i) {
        
        Vec shadow_vec = glm::normalize(i->position - hit_record.position);
        Ray shadow_ray;
        shadow_ray.o = hit_record.position;
        shadow_ray.d = shadow_vec;
        
        if (glm::dot(hit_record.normal, shadow_vec) >= 0) {
            HitRecord hit = hit_record;
            bool block = hittable_collection.Hit(shadow_ray, &hit);
           
            if (not block) {
                Color color_d, color_s;
                color_d = glm::max(0.f, glm::dot(hit_record.normal, shadow_vec)) * hit_record.material.k_d * hit_record.material.diffuse;
                Vec R = -glm::normalize(glm::reflect(shadow_vec, hit_record.normal)); // direction of the reflection of shadow ray
                color_s = hit_record.material.k_s * hit_record.material.specular * glm::pow(glm::max(0.f, glm::dot(R, -hit_record.in_direction)), hit_record.material.sh);
                color += i->intensity * (color_d + color_s);
            }
        }
    }

   
    if (trace_depth < kMaxTraceDepth){
       
        if (hit_record.material.k_s > 0) {
            Ray reflected_ray;
            reflected_ray.o = hit_record.position;
            reflected_ray.d = hit_record.reflection;
            Color r_color = TraceRay(reflected_ray, light_sources, hittable_collection, trace_depth + 1);
            color += r_color * hit_record.material.k_s;
        }
    }

    if (color.x > 1.0 || color.y > 1.0 || color.z > 1.0){
        color = glm::clamp(color, 0.f, 1.f);
    }
    return color;

}
// Trace one ray and calculate the color at the closest intersection point
Color TraceRay(const Ray& ray,const std::vector<LightSource>& light_sources,const Hittable& hittable_collection,int trace_depth) {
    HitRecord record;
    Color color(0.0f, 0.0f, 0.0f);
   
    if (hittable_collection.Hit(ray, & record))
    {
        
        color = Shade(light_sources, hittable_collection,record, trace_depth);
        return color;
    }
    else{
        return color;
    }
}

int main() {
    const std::string work_dir("/Users/suchangyue/Documents/GitHub/finalproject-changyue/part1/");

    // Construct scene
    //Scene scene(work_dir, "scene/spheres.toml");
    //Scene scene(work_dir, "scene/sphere.toml");
    //Scene scene(work_dir, "scene/quadric.toml");

    //Scene scene(work_dir, "scene/triangle.toml");
    //Scene scene(work_dir, "scene/teapot.toml");
    Scene scene(work_dir, "scene/monkey.toml");

    const Camera& camera = scene.camera_;
    int width = camera.width_;
    int height = camera.height_;

    std::vector<unsigned char> image(width * height * 4, 0);

    float progress = 0.f;

    // Traverse all pixels
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            Color color(0.f, 0.f, 0.f);
            int count = 0;
            for (float bias_x = 0.25f; bias_x < 1.f; bias_x += .5f) {
                for (float bias_y = 0.25f; bias_y < 1.f; bias_y += .5f) {
                    Ray ray = camera.RayAt(float(x) + bias_x, float(y) + bias_y);
                    color += TraceRay(ray, scene.light_sources_, scene.hittable_collection_, 1);
                    count++;
                }
            }
            color /= float(count);
            int idx = 4 * ((height - y - 1) * width + x);
            for (int i = 0; i < 3; i++) {
                image[idx + i] = (uint8_t) (glm::min(color[i], 1.f - 1e-5f) * 256.f);
            }
            image[idx + 3] = 255;

            float curr_progress = float(x * height + y) / float(height * width);
            if (curr_progress > progress + 0.05f) {
                progress += 0.05f;
                std::cout << "Progress: " << progress << std::endl;
            }
        }
    }

    // Save result as png file
    std::vector<unsigned char> png;
    unsigned error = lodepng::encode(png, image, width, height);
    lodepng::save_file(png, work_dir + "output_mon.png");
}
