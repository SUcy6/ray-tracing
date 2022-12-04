#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "Utils/happly.h"
#include "Common.h"
#include "Ray.h"
#include "Material.h"


// The data structure to store information of light hit/intersection
struct HitRecord {
    Vec normal;
    Vec in_direction;
    Vec reflection;
    Point position;
    float distance;
    Material material;
};

// Base class of all hittable objects.
class Hittable {
public:
    Hittable() = default;
    virtual ~Hittable() = default;
    virtual bool Hit(const Ray& ray, HitRecord* hit_record) const = 0;
};

// Sphere
class Sphere : public Hittable {
public:

    Sphere(const Point& o, float r, Material material) : o_(o), r_(r), material_(material) {}
    ~Sphere() override = default;

    virtual bool Hit(const Ray& ray, HitRecord* hit_record) const override;

private:
    Point o_;
    float r_;
    Material material_;
};


class Quadric : public Hittable {
public:

    Quadric(const glm::mat4x4& A, Material material) : A_(A), material_(material) {}

    virtual bool Hit(const Ray& ray, HitRecord* hit_record) const override;

private:
    glm::mat4x4 A_;
    Material material_;
};

// Triangle
// This class doesn't own material, for computational efficiency consideration
class Triangle : public Hittable {
public:

    Triangle(const Point& a,
             const Point& b,
             const Point& c,
             const Vec& n_a,
             const Vec& n_b,
             const Vec& n_c,
             bool phong_interpolation=true) :
        a_(a), b_(b), c_(c), n_a_(n_a), n_b_(n_b), n_c_(n_c), phong_interpolation_(phong_interpolation) {
    }
    ~Triangle() override = default;
    virtual bool Hit(const Ray& ray, HitRecord* hit_record) const override;

private:
    Point a_, b_, c_;
    Vec n_a_, n_b_, n_c_;
    bool phong_interpolation_ = true;
};

// CompleteTriangle = triangle + material
class CompleteTriangle : Hittable {
public:
    CompleteTriangle(const Point& a,
                     const Point& b,
                     const Point& c,
                     const Vec& n_a,
                     const Vec& n_b,
                     const Vec& n_c,
                     const Material& material) : triangle_(a, b, c, n_a, n_b, n_c), material_(material) {

    }
    ~CompleteTriangle() override = default;
    virtual bool Hit(const Ray& ray, HitRecord* hit_record) const override;

private:
    Triangle triangle_;
    Material material_;
};


// Data structure of octree
struct OctreeNode {
    OctreeNode* childs[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    Point bbox_min, bbox_max;
    std::vector<size_t> face_index;
};

// Triangle Mesh
class Mesh : public Hittable {
public:
    // file_path: the path of ply file. (https://en.wikipedia.org/wiki/PLY_(file_format))
    // material: data structure storing the shading parameters
    Mesh(const std::string& file_path, const Material& material, bool phong_interpolation);
    ~Mesh() override = default;

    virtual bool Hit(const Ray& ray, HitRecord* hit_record) const override;

private:
    happly::PLYData ply_data_;
    std::vector<Triangle> triangles_;
    std::vector<Point> vertices_;
    std::vector<Vec> face_normals_;
    std::vector<Vec> vertex_normals_;
    std::vector<std::vector<size_t>> f_ind_;
    Material material_;
    bool phong_interpolation_;
    OctreeNode* root_ = nullptr;
    std::vector<std::unique_ptr<OctreeNode>> tree_nodes_;

    bool OctreeHit(OctreeNode* u, const Ray& ray, HitRecord* hit_record) const;
    bool IsFaceInsideBox(const std::vector<size_t>& face, const Point& bbox_min, const Point& bbox_max) const;
    bool IsRayIntersectBox(const Ray& ray, const Point& bbox_min, const Point& bbox_max) const;
    void InsertFace(OctreeNode* u, size_t face_idx);
};

// Collection of hittable objects
class HittableList : public Hittable {
public:
    HittableList() = default;
    ~HittableList() override = default;
    explicit HittableList(const std::vector<const Hittable*>& hittable_list) : hittable_list_(hittable_list) {}

    virtual bool Hit(const Ray& ray, HitRecord* hit_record) const override;
    void PushHittable(const Hittable& hittable);

private:
    std::vector<const Hittable*> hittable_list_;
};