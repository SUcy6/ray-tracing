#include "Hittable.h"

// Sphere
bool Sphere::Hit(const Ray& ray, HitRecord *hit_record) const {

    bool ret = false;
    float A, B, C, det;
    float t1, t2, t = 0.f;
    A = glm::dot(ray.d, ray.d);
    B = 2 * glm::dot(ray.d, ray.o-o_);
    C = glm::dot(ray.o - o_, ray.o - o_) - r_ * r_;
    det = B * B - 4 * A * C;

    if (det >= 0){
        ret = true;
        t1 = (- B + sqrt(det)) / (2 * A);
        t2 = (- B - sqrt(det)) / (2 * A);
        t = glm::min(t1, t2);
    }
    if (ret) {
        hit_record->position = ray.o + t * ray.d;
        hit_record->normal = glm::normalize(hit_record->position - o_);
        hit_record->distance = abs(glm::distance(ray.o, hit_record->position));
        hit_record->in_direction = glm::normalize(ray.d);
        hit_record->reflection = glm::normalize(glm::reflect(hit_record->in_direction, hit_record->normal));
        hit_record->material = material_;
    }
    if (t < 0){ 
        ret = false;
    }
    return ret;


}

// Quadric
bool Quadric::Hit(const Ray& ray, HitRecord *hit_record) const {
    bool ret = false;
    float t = -1;
    glm::vec4 O = {ray.o[0], ray.o[1], ray.o[2], 1};
    glm::vec4 D = {ray.d[0], ray.d[1], ray.d[2], 0};
    float a = glm::dot(D, A_ * D);
    float b = 2 * glm::dot(O, A_ * D);
    float c = glm::dot(O, A_ * O);
    float det = b*b - 4 * a * c;
    if (det > 0){
        ret = true;
        float t0 = (-b + sqrt(det)) / (2 * a);
        float t1 = (-b - sqrt(det)) / (2 * a);
        t = glm::min(t0, t1);
    }
    else if(det == 0){
        ret = true;
        t = -b / (2 * a);
    }
    else{
        ret = false;
    }
    if (ret) {
        glm::vec4 X = O + t * D;
        hit_record->position = ray.o + t * ray.d;
        glm::mat4 A1 = A_ + glm::transpose(A_);
        hit_record->normal = A1 * X;
        hit_record->distance = glm::distance(ray.o, hit_record->position);
        hit_record->in_direction = glm::normalize(ray.d);
        hit_record->reflection = glm::normalize(glm::reflect(hit_record->in_direction, hit_record->normal));
        hit_record->material = material_;
    }
    if (t < 0){ 
        ret = false;
    }
    return ret;
}

// Triangle
bool Triangle::Hit(const Ray& ray, HitRecord *hit_record) const {
    bool ret = false;
    Vec n = glm::cross(b_ - a_, c_ - a_);
    float t = -(glm::dot(n, ray.o) - glm::dot(n, b_)) / glm::dot(n, ray.d);
    if (t >= 0) {ret = true;}
    if (t < 1e-3f) {return false;}
    Vec P = ray.o + t * ray.d;

    Vec pa_pb = glm::cross(a_ - P, b_ - P);
    Vec pb_pc = glm::cross(b_ - P, c_ - P);
    Vec pc_pa = glm::cross(c_ - P, a_ - P);
    if (glm::dot(pa_pb, pb_pc) < 0 || glm::dot(pb_pc, pc_pa) < 0) {
        return false; }

    if (ret)
    {
        if (phong_interpolation_)
        {
            float triangle_area = glm::length(glm::cross(b_ - a_, c_ - a_)) / 2;
            float oab_area = glm::length(glm::cross(a_ - P, b_ - P)) / 2;
            float oca_area = glm::length(glm::cross(c_ - P, a_ - P)) / 2;
            float obc_area = glm::length(glm::cross(b_ - P, c_ - P)) / 2;
            float a1 = oab_area / triangle_area;
            float a2 = oca_area / triangle_area;
            float a3 = obc_area / triangle_area;
            hit_record->normal = glm::normalize(a1 * n_c_ + a2 * n_b_ + a3 * n_a_);
        }
        else
        {
            
            hit_record->normal = glm::normalize(glm::cross(b_ - a_, c_ - a_));
        }
        hit_record->position = P;
        hit_record->distance = abs(glm::distance(ray.o, hit_record->position));
        hit_record->in_direction = glm::normalize(ray.d);
        hit_record->reflection = glm::normalize(glm::reflect(hit_record->in_direction, hit_record->normal));
    }
    return ret;
}

bool CompleteTriangle::Hit(const Ray& ray, HitRecord *hit_record) const {
    bool ret = triangle_.Hit(ray, hit_record);
    if (ret) {
        hit_record->material = material_;
    }
    return ret;
}


// Mesh
Mesh::Mesh(const std::string& file_path,
           const Material& material,
           bool phong_interpolation):
           ply_data_(file_path), material_(material), phong_interpolation_(phong_interpolation) {
    std::vector<std::array<double, 3>> v_pos = ply_data_.getVertexPositions();
    vertices_.resize(v_pos.size());

    for (int i = 0; i < vertices_.size(); i++) {
        vertices_[i] = Point(v_pos[i][0], v_pos[i][1], v_pos[i][2]);
    }

    f_ind_ = ply_data_.getFaceIndices();

    
    for (const auto& face : f_ind_) {
        Vec normal = glm::normalize(glm::cross(vertices_[face[1]] - vertices_[face[0]], vertices_[face[2]] - vertices_[face[0]]));
        face_normals_.emplace_back(normal);
    }

   
    vertex_normals_.resize(vertices_.size(), Vec(0.f, 0.f, 0.f));
    for (int i = 0; i < f_ind_.size(); i++) {
        for (int j = 0; j < 3; j++) {
            vertex_normals_[f_ind_[i][j]] += face_normals_[i];
        }
    }
    for (auto& vertex_normal : vertex_normals_) {
        vertex_normal = glm::normalize(vertex_normal);
    }

    
    for (const auto& face : f_ind_) {
        triangles_.emplace_back(vertices_[face[0]], vertices_[face[1]], vertices_[face[2]],
                                vertex_normals_[face[0]], vertex_normals_[face[1]], vertex_normals_[face[2]],
                                phong_interpolation_);
    }

    Point bbox_min( 1e5f,  1e5f,  1e5f);
    Point bbox_max(-1e5f, -1e5f, -1e5f);
    for (const auto& vertex : vertices_) {
        bbox_min = glm::min(bbox_min, vertex - 1e-3f);
        bbox_max = glm::max(bbox_max, vertex + 1e-3f);
    }

    tree_nodes_.emplace_back(new OctreeNode());
    tree_nodes_.front()->bbox_min = bbox_min;
    tree_nodes_.front()->bbox_max = bbox_max;

    root_ = tree_nodes_.front().get();
    for (int i = 0; i < f_ind_.size(); i++) {
        InsertFace(root_, i);
    }
}

bool Mesh::Hit(const Ray& ray, HitRecord *hit_record) const {
    const bool brute_force = false;
    if (brute_force) {
        // Naive hit algorithm
        float min_dist = 1e5f;
        for (const auto &triangle : triangles_) {
            HitRecord curr_hit_record;
            if (triangle.Hit(ray, &curr_hit_record)) {
                if (curr_hit_record.distance < min_dist) {
                    *hit_record = curr_hit_record;
                    min_dist = curr_hit_record.distance;
                }
            }
        }
        if (min_dist + 1.0 < 1e5f) {
            hit_record->material = material_;
            return true;
        }
        return false;
    } else {
        bool ret = OctreeHit(root_, ray, hit_record);
        if (ret) {
            hit_record->material = material_;
        }
        return ret;
    }
}

bool Mesh::IsFaceInsideBox(const std::vector<size_t>& face, const Point& bbox_min, const Point& bbox_max) const {
    for (size_t idx : face) {
        const auto& pt = vertices_[idx];
        for (int i = 0; i < 3; i++) {
            if (pt[i] < bbox_min[i] + 1e-6f) return false;
            if (pt[i] > bbox_max[i] - 1e-6f) return false;
        }
    }
    return true;
}

bool Mesh::IsRayIntersectBox(const Ray& ray, const Point& bbox_min, const Point& bbox_max) const {
    float t_min = -1e5f;
    float t_max =  1e5f;

    for (int i = 0; i < 3; i++) {
        if (glm::abs(ray.d[i]) < 1e-6f) {
            if (ray.o[i] < bbox_min[i] + 1e-6f || ray.o[i] > bbox_max[i] - 1e-6f) {
                t_min =  1e5f;
                t_max = -1e5f;
            }
        }
        else {
            if (ray.d[i] > 0.f) {
                t_min = glm::max(t_min, (bbox_min[i] - ray.o[i]) / ray.d[i]);
                t_max = glm::min(t_max, (bbox_max[i] - ray.o[i]) / ray.d[i]);
            }
            else {
                t_min = glm::max(t_min, (bbox_max[i] - ray.o[i]) / ray.d[i]);
                t_max = glm::min(t_max, (bbox_min[i] - ray.o[i]) / ray.d[i]);
            }
        }
    }

    return t_min + 1e-6f < t_max;
}

void Mesh::InsertFace(OctreeNode* u, size_t face_idx) {
    const Point& bbox_min = u->bbox_min;
    const Point& bbox_max = u->bbox_max;

    Vec bias = bbox_max - bbox_min;
    Vec half_bias = bias * 0.5f;

    bool inside_childs = false;

    for (size_t a = 0; a < 2; a++) {
        for (size_t b = 0; b < 2; b++) {
            for (size_t c = 0; c < 2; c++) {
                size_t child_idx = ((a << 2) | (b << 1) | c);
                Point curr_bbox_min = bbox_min + half_bias * Vec(float(a), float(b), float(c));
                Point curr_bbox_max = curr_bbox_min + half_bias;
                if (IsFaceInsideBox(f_ind_[face_idx], curr_bbox_min, curr_bbox_max)) {
                    if (u->childs[child_idx] == nullptr) {
                        tree_nodes_.emplace_back(new OctreeNode());
                        OctreeNode* child = tree_nodes_.back().get();
                        u->childs[child_idx] = tree_nodes_.back().get();
                        child->bbox_min = curr_bbox_min;
                        child->bbox_max = curr_bbox_max;
                    }
                    InsertFace(u->childs[child_idx], face_idx);
                    inside_childs = true;
                }
            }
        }
    }

    if (!inside_childs) {
        u->face_index.push_back(face_idx);
    }
}

bool Mesh::OctreeHit(OctreeNode* u, const Ray& ray, HitRecord* hit_record) const {
    if (!IsRayIntersectBox(ray, u->bbox_min, u->bbox_max)) {
        return false;
    }
    float distance = 1e5f;
    for (const auto& face_idx : u->face_index) {
        HitRecord curr_hit_record;
        if (triangles_[face_idx].Hit(ray, &curr_hit_record)) {
            if (curr_hit_record.distance < distance) {
                distance = curr_hit_record.distance;
                *hit_record = curr_hit_record;
            }
        }
    }

    for (const auto& child : u->childs) {
        if (child == nullptr) {
            continue;
        }
        HitRecord curr_hit_record;
        if (OctreeHit(child, ray, &curr_hit_record)) {
            if (curr_hit_record.distance < distance) {
                distance = curr_hit_record.distance;
                *hit_record = curr_hit_record;
            }
        }
    }
    return distance + 1 < 1e5f;
}


// Hittable list
void HittableList::PushHittable(const Hittable& hittable) {
    hittable_list_.push_back(&hittable);
}

bool HittableList::Hit(const Ray& ray, HitRecord *hit_record) const {
    float min_dist = 1e5f;
    for (const auto &hittable : hittable_list_) {
        HitRecord curr_hit_record;
        if (hittable->Hit(ray, &curr_hit_record)) {
            if (curr_hit_record.distance < min_dist) {
                *hit_record = curr_hit_record;
                min_dist = curr_hit_record.distance;
            }
        }
    }
    return min_dist + 1.0 < 1e4f;
}