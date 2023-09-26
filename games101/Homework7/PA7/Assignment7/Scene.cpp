//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

Vector3f Scene::shade(const Intersection & point,const Vector3f & wo) const
{
    const double eps = 1e-4;
    //get the parameters of the shading point
    //std::cout<<"shade"<<std::endl;
    Vector3f p = point.coords;
    Vector3f N = point.normal;

    if(point.m -> hasEmission()) return point.m -> getEmission();

    Intersection intersection1;
    float pdf_area;
    sampleLight(intersection1,pdf_area);

    Vector3f ws = normalize(intersection1.coords - p);
    Ray ray1(p,ws);
    Intersection middle = Scene::intersect(ray1);
    
    bool blocked;
    if((middle.coords - intersection1.coords).norm() < eps) blocked = false;
    else blocked = true; 

    Vector3f l_dir;
    
    if(!blocked)
    {
        Vector3f NN = intersection1.normal;
        Vector3f xx = (p - intersection1.coords) * (p - intersection1.coords);
        float r2 = xx.x + xx.y + xx.z;
        float cos1 = std::max<float>(0.0f,dotProduct(ws,N));
        float cos2 = std::max<float>(0.0f,dotProduct(-ws,NN));
        l_dir = intersection1.emit * point.m -> eval(ws,wo,N) * cos1 * cos2 / pdf_area / r2; 
    }

    Vector3f l_indir;
    //std::srand(static_cast<unsigned int>(std::time(nullptr)));
    //int randomInt = std::rand();
    //float rr = static_cast<float>(randomInt) / RAND_MAX;
    float rr = get_random_float();
    //std::cout<<rr<<std::endl;
    if(rr > this -> RussianRoulette) return l_dir;

    Vector3f wi;
    wi = point.m -> sample(wo,N);

    Ray ray2(p,wi);
    Intersection intersection2 = Scene :: intersect(ray2);
    if(intersection2.happened && !intersection2.m -> hasEmission())
    {
        float cos3 = std::max<float>(dotProduct(wi,N),0.0f);
        Vector3f next_wo = -wi;
        l_indir = shade(intersection2,next_wo) * point.m -> eval(wi,wo,N) * cos3 / point.m -> pdf(wi,wo,N) / this -> RussianRoulette;   
    }

    return l_dir + l_indir; 

}

//radiance的定义是vector3f吗？？？？
//radiance 和像素颜色之间如何映射？？？
//gamma矫正?
//emmm细节都被隐藏起来了
// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here

    //get the hit point of the ray and the scene
    //in the triangel :: getIntersection()
    
    Intersection intersection = Scene :: intersect(ray);

    Vector3f radiance = this -> backgroundColor;

    if(intersection.happened)
    {
        Vector3f wo = -ray.direction;
        radiance = shade(intersection,wo);
    }
    return radiance; 
}