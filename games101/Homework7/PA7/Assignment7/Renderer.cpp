//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include "Scene.hpp"
#include "Renderer.hpp"


inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00001;

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void compute_pixel_radiance(Vector3f & frame_val,std::mutex & my_lock,const int & thread_count,const int & spp,const Ray & ray,const Scene& scene)
{
    for(int i = 0;i < spp / thread_count;i ++)
    {
        Vector3f ans = scene.castRay(ray,0);
        my_lock.lock();
        frame_val = frame_val + ans / spp;
        my_lock.unlock();
    }
}
void Renderer::Render(const Scene& scene)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);
    int m = 0;

    //change the spp value to change sample ammount
    //ray_generation need to calculate the radiance of the ray(eyepos,dir) as the value of the pixel
    //the implmentation here does not consider the samplation of the pixel
    //all of the ray is in the same direction
    //if the ray hit an object in the scene at position p
    //we can get its radiance by shade(p,-dir)

    //why the direction of the ray is normalize(Vector3f(-x,y,1))
    //not consider the eyepos
    //the origin is eyepos and the direction is normalize(Vector3f(-x,y,1))
    //why the direction is not pixel_coordinate - eyepos
    //leave this question and go on 
    //I got the answer of this question : the center of image plane is not at the point (0,0,-1) as homework5
    //the center of the image plane is at (eye_pos.x,eye_pos.y, eye_pos.z - 1) 
    
    int spp = 256;
    int thread_count = 8;
    std::mutex my_lock;
    std::cout << "SPP: " << spp << "\n";
    for (uint32_t j = 0; j < scene.height; ++j) {
        for (uint32_t i = 0; i < scene.width; ++i) {
            // generate primary ray direction
            std::vector<std::thread> threads;
            float x = (2 * (i + 0.5) / (float)scene.width - 1) *
                      imageAspectRatio * scale;
            float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;

            Vector3f dir = normalize(Vector3f(-x, y, 1));
            Ray ray(eye_pos,dir);
            for(int k = 0;k < thread_count;k ++)
                threads.emplace_back(std::thread(compute_pixel_radiance,std::ref(framebuffer[m]),std::ref(my_lock),std::ref(thread_count),std::ref(spp),std::ref(ray),std::ref(scene)));
            for(std::thread & t : threads)
                t.join();
            
            m++;
        }
        UpdateProgress(j / (float)scene.height);
    }
    UpdateProgress(1.f);

    // save framebuffer to file
    FILE* fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);    
}
