#pragma once

#include "Camera.hpp"
#include "Ray.hpp"
#include "SkyBox.hpp"
#include "Triangle.hpp"

#include "tinyobjloader/tiny_obj_loader.h"

#include <algorithm>
#include <vector>

// Node for constructing BVH.
struct BVHNode : Node<2> {
  public:
    BBox bbox;
    flt  area;

    BVHNode *fa;

    Triangle const *tri;

    std::array<BVHNode *, 2> children;

  public:
    BVHNode() : area{0}, tri{nullptr}, children{nullptr} {}

    void build(std::vector<Triangle> tris, BVHNode *fa = nullptr) {
        if (fa) {
            this->tdep = fa->tdep + 1;
        }
        /* Naive */
        if (tris.size() == 1) {
            this->tri = new Triangle(tris[0]);
            this->bbox |= tris[0].boundingbox();
            // this->area += t->area();
            this->area   = tris[0].doublearea();
            this->isleaf = true;
            return;
        }
        if (tris.size() == 2) {
            this->children[0] = new BVHNode();
            this->children[1] = new BVHNode();
            this->children[0]->build(std::vector<Triangle>{tris[0]}, this);
            this->children[1]->build(std::vector<Triangle>{tris[1]}, this);
            this->bbox = this->children[0]->bbox | this->children[1]->bbox;
            this->area = this->children[0]->area + this->children[1]->area;
            return;
        }
        BBox centroid_box;
        for (Triangle const &t : tris) {
            centroid_box |= t.boundingbox().centroid();
        }
        // Split along the direction that has the maximum extent.
        auto begining = tris.begin();
        auto ending   = tris.end();
        auto middling = begining + tris.size() * 0.5;
        switch (centroid_box.max_dir()) {
        case 0:
            std::nth_element(begining, middling, ending,
                             [](Triangle const &t1, Triangle const &t2) {
                                 return t1.boundingbox().centroid().x <
                                        t2.boundingbox().centroid().x;
                             });
            break;
        case 1:
            std::nth_element(begining, middling, ending,
                             [](Triangle const &t1, Triangle const &t2) {
                                 return t1.boundingbox().centroid().y <
                                        t2.boundingbox().centroid().y;
                             });
            break;
        case 2:
            std::nth_element(begining, middling, ending,
                             [](Triangle const &t1, Triangle const &t2) {
                                 return t1.boundingbox().centroid().z <
                                        t2.boundingbox().centroid().z;
                             });
            break;
        default:
            errorm("?\n");
        }
        this->children[0] = new BVHNode();
        this->children[1] = new BVHNode();
        this->children[0]->build(std::vector<Triangle>(begining, middling),
                                 this);
        this->children[1]->build(std::vector<Triangle>(middling, ending),
                                 this);
        this->bbox = this->children[0]->bbox | this->children[1]->bbox;
        this->area = this->children[0]->area + this->children[1]->area;
    }

    Intersection intersect(Ray const &ray) const {
        Intersection ret;
        // If ray does not intersect with bbox of node:
        if (!ray.intersect(this->bbox)) {
            return ret;
        }
        // If this node is a leaf node:
        if (this->isleaf) {
            return ray.intersect(this->tri);
        }
        // if this node is an intermediate node:
        Intersection l = this->children[0]->intersect(ray);
        Intersection r = this->children[1]->intersect(ray);

        ret = l;
        if (r.occurred && r.distance < ret.distance) {
            ret = r;
        }

        return ret;
    }
};

class Scene {
  private:
    std::vector<Triangle> orig_tris;
    std::vector<Triangle> tris;

    std::vector<Triangle> lights;
    flt                   area_of_lights;

    SkyBox sky;
    bool   has_sky;

    BVHNode *root;

  private:
    Intersection intersect(Ray const &ray) const;

  public:
    Scene();
    Scene(tinyobj::ObjReader const &loader);

    void load_skybox(std::string const &imgfile);

    void to_camera_space(Camera const &cam);

    void build_BVH();

    // Sample on light source and determine if it is occluded by other objects
    // along the way to position dst.
    // Returns an `Intersection` object, which is the intersetion of a `Ray`
    // from `dst` to sampled light source point with the whole scene.
    // If the ray is occluded along the path, the `occurred` variable of the
    // returned `Intersection` object will be set to `false`, otherwise the
    // ray is not occluded.
    Intersection sample_light(Intersection const &isect) const;

    vec3 shoot(Ray const &ray, flt const &rr, int const &bounce = 0) const;

    std::vector<Triangle> const &triangles() const;
    std::vector<Triangle> const &emissives() const;
    SkyBox const &               skybox() const;
    bool const &                 has_skybox() const;
};

// Author: Blurgy <gy@blurgy.xyz>
// Date:   Jan 31 2021, 21:11 [CST]
