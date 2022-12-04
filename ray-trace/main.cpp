#include <iostream>
#include <SDL2/SDL.h>
// #include "vec3.h"
#include "ray.h"
#include "hitable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

bool running;
const char *title = "SDL2 Template";
int Width = 500, Height = 500;
SDL_Renderer *renderer;
SDL_Window *window;
SDL_Texture *screen;
SDL_Rect screensize;
const Uint8 *keystates;
SDL_Point mouse;
Uint32 mousestate;
SDL_Event event;

SDL_Color bkg;
SDL_Color blue = {0,0,255,255};

// define sphere
// vec3 sphere;
// float sphere_radius;

// functions
void sdl_init(const char* text, int w, int h);
void loop();
SDL_Color createColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void setDrawColor(SDL_Color c);
void drawPoint(int x, int y);
vec3 color(const ray& r, hitable * world, int depth);
float hit_sphere(const vec3 center, float radius, const ray&r);
hitable *random_scene();

int main()
{
    // window heigth and width
    int width = 800; int height = 400;
    // sample per pixel
    int ns = 500;

    // sphere
    // sphere = vec3(0,0,-1);
    // sphere_radius = 0.5;

    // name
    std::cout << "P3\n" << width << " " << height << "\n255\n";

    // sdl window
    sdl_init("Ray Tracer", width, height);
    // start drawing
    loop();

    // positions
    // vec3 lower_left_corner(-2.0, -1.0, -1.0);
    // vec3 horizontal(4.0, 0.0, 0.0);
    // vec3 vertical(0.0, 2.0, 0.0);
    // vec3 origin(0.0, 0.0, 0.0);

    // create world
    // hitable *list[5];
    // list[0]= new sphere(vec3(0,0,-1), 0.5, new lambertian(vec3(0.1, 0.3, 0.5)));
    // list[1]= new sphere(vec3(0,-100.5,-1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
    // list[2]= new sphere(vec3(1,0,-1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.2));
    // list[3]= new sphere(vec3(-1,0,-1), 0.5, new dielectric(1.5));
    // list[4]= new sphere(vec3(-1,0,-1), -0.45, new dielectric(1.5));
    
    // hitable * world = new hitable_list(list, 5);

    // hitable *list[2];
    // float R = cos(M_PI/4);
    // list[0]=new sphere(vec3(-R, 0, -1), R, new lambertian(vec3(0,0,1)));
    // list[1]=new sphere(vec3(R, 0, -1), R, new lambertian(vec3(1,0,0)));
    // hitable * world = new hitable_list(list, 2);
    vec3 lookfrom(13,2,3);
    vec3 lookat(0,0,0);
    // float dist_to_focus = (lookfrom-lookat).length();
    float dist_to_focus = 10.0;
    float aperture = 0.1;

    camera cam(lookfrom, lookat, vec3(0,1,0), 20, float(width)/float(height), aperture, dist_to_focus); // set smaller fov to zoom in

    hitable * world = random_scene();


    // trace loop
    for (int y=height-1; y>=0; y--) {
        for (int x=0; x<width; x++) {
            vec3 col(0,0,0);
            for (int s=0; s<ns; s++) {
                float u = float(x + drand48())/float(width);
                float v = float(y + drand48())/float(height);

                // ray r(origin, lower_left_corner + u*horizontal + v*vertical);
                ray r = cam.get_ray(u, v);
                // vec3 color(float(x) / float(width), float(y) / float(height), 0.2);
                // vec3 col = color(r);
                vec3 p = r.point_at_param(2.0);
                col += color(r, world, 0);

            }

            
            col /= float(ns);
            col = vec3(sqrt(col.r()),sqrt(col.g()),sqrt(col.b()));
            int ir = int(255.99*col[0]);
            int ig = int(255.99*col[1]);
            int ib = int(255.99*col[2]);

            std::cout << ir << " " << ig << " " << ib << "\n";
            setDrawColor(createColor(ir, ig, ib, 255));
            drawPoint(x, height - y); // flip image - the y axis is different from ppm

        }
    }

    
    while(running) {
        loop();
    }

}

void setDrawColor(SDL_Color c) { 
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a); 
}

SDL_Color createColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
 SDL_Color c = {r,g,b,a};
 return c;
}

void drawPoint(int x, int y) { 
    SDL_RenderDrawPoint(renderer, x, y); 
}

void sdl_init(const char* text, int w, int h) {

    // update
    title = text;
    Width = w;
    Height = h;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                       Width, Height, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    running = 1;
    screensize.x=screensize.y=0;
    screensize.w=Width; screensize.h=Height;
    screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, Width, Height);
}

void loop() {
  //end_render
  SDL_SetRenderTarget(renderer, NULL);
  SDL_RenderCopy(renderer, screen, &screensize, &screensize);
  SDL_RenderPresent(renderer);

  // updateKeys
  keystates = SDL_GetKeyboardState(NULL);
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT)
      running = false;
  }
  mousestate = SDL_GetMouseState(&mouse.x, &mouse.y);

  // begin_render
  SDL_SetRenderTarget(renderer, screen);
}

vec3 color(const ray& r, hitable * world, int depth) {
    hit_record rec;
    if(world->hit(r, 0.001, MAXFLOAT, rec)){
        // vec3 target = rec.p + rec.normal + random_in_unit_sphere();
        // return 0.5 * color(ray(rec.p, target-rec.p), world);
        ray scattered;
        vec3 attenuation;
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
            return attenuation * color(scattered, world, depth+1);
        }
        else {
            return vec3(0,0,0);
        }
    }
    else{
        // negative
        vec3 unit_dir = unit_vector(r.direction());
        float t = 0.5*(unit_dir.y() + 1.0);

        return (1.0-t)*vec3(1.0,1.0,1.0) + t * vec3(0.5, 0.7, 1.0); // background
    }

}

hitable *random_scene() {
    int n = 500;
    hitable ** list = new hitable*[n+1];
    list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
    int a = 1;
    for(int i=-11; i<11; i++){
        for(int j=-11; j<11; j++) {
            float choose_mat = drand48();
            vec3 center(i+0.9*drand48(), 0.2, j+0.9*drand48());

            if (choose_mat < 0.4) {
                list[a++] = new sphere(center, 0.2, new lambertian(vec3(drand48()*drand48(), drand48()*drand48(), drand48()*drand48())));
            }
            else if (choose_mat < 0.8) {
                list[a++] = new sphere(center, 0.2, new metal(vec3(0.5*(1+drand48()), 0.5*(1+drand48()), 0.5*(1+drand48())), 0.5*(1+drand48())));
            }
            else {
                list[a++] = new sphere(center, 0.2, new dielectric(1.5));
            }
        }
    }
    list[a++] = new sphere(vec3(0,1,0), 1, new dielectric(1.5));
    list[a++] = new sphere(vec3(-4,1,-4), 1, new lambertian(vec3(0.1,0.2,0.4)));
    list[a++] = new sphere(vec3(4,1,4), 1, new metal(vec3(0.7, 0.6, 0.5), 0));
    list[a++] = new sphere(vec3(5,0.5,1), 0.5, new metal(vec3(0.5, 0.4, 0.7), 0));

    return new hitable_list(list, a);
}

// function to check whether ray hit the circle
// float hit_sphere(const vec3 center, float radius, const ray&r) {
//     vec3 oc = r.origin() - center;
//     float a = dot(r.direction(), r.direction());
//     float b = 2.0 * dot(oc, r.direction());
//     float c = dot(oc, oc) - radius*radius;
//     float discriminant = b*b - 4*a*c;
    
//     // check hitting
//     if (discriminant < 0)
//         return -1.0;
//     else
//         return (-b - sqrt(discriminant))/(2.0*a);
// }

