#pragma once
#include <string>
#include "Camera.h"
#include "Hittable.h"

struct LightSource {
    LightSource(const Point& position, float intensity) : position(position), intensity(intensity) {}
    Point position;
    float intensity;
};


class Scene {
public:
    Scene(const std::string& work_dir, const std::string& scane_file_name);
    Camera camera_;
    std::vector<LightSource> light_sources_;
    HittableList hittable_collection_;

private:
    std::vector<std::unique_ptr<Hittable>> hittable_pool_;
    std::vector<std::unique_ptr<Material>> material_pool_;
};
