#include <iostream>
#include <fstream>
#include "ray.h"
#include "hitable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include <random>
using namespace	std;

vec3 color(const ray& r, hitable *world, int depth) {
	hit_record rec;
	if (world->hit(r, 0.001, FLT_MAX, rec)) {
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
			return attenuation*color(scattered, world, depth + 1);
		}
		else{
			return vec3(0, 0, 0);
			//std::cout << "ray end depth: " << depth << "\n";
		}
	}
	else {
		vec3 unit_direction = unit_vector(r.direction());
		float t = 0.5*(unit_direction.y() + 1.0);
		return (1.0 - t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
	}
	
}

hitable *random_scene() {
	int n = 500;
	hitable **list = new hitable*[n + 1];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
	int i = 1;
	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			float choose_mat = drand;
			vec3 center(a + 0.9*drand, 0.2, b+0.9*drand);
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				if (choose_mat < 0.8) {
					list[i++] = new sphere(center, 0.2, new lambertian(vec3(drand*drand, drand*drand, drand*drand)));
				}
				else if (choose_mat < 0.95) {
					list[i++] = new sphere(center, 0.2,
						new metal(vec3(0.5*(1 + drand), 0.5*(1 + drand), 0.5*(1 + drand)), 0.5*drand));
				}
				else {
					list[i++] = new sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}
	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7,0.6,0.5), 0.0));
	std::cout << "Created: " << i << " spheres\n";
	return new hitable_list(list, i);
}

hitable *metal_scene() {
	hitable **list = new hitable*[2];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new metal(vec3(0.5, 0.5, 0.5), 0.0));
	list[1] = new sphere(vec3(0, 0.5, 0), 0.5, new metal(vec3(1.0, 1.0, 0.0), 1.0));
	return new hitable_list(list, 2);
}

hitable *metal_scene_random() {
	int n = 10;
	hitable **list = new hitable*[n+1];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new metal(vec3(0.5, 0.5, 0.5), 0.0));
	int j = 1;
	for (int i = 0; i < n; i++) {
		vec3 pos = vec3(drand, 0.2, drand) * vec3(3.0,1.0,3.0);
		list[j++] = new sphere(pos, 0.2, new metal(vec3(drand*drand, drand*drand, drand*drand), 0.1));
	}
	//list[j++] = new sphere(vec3(0, 0.5, 0), 0.5, new metal(vec3(1.0, 0.0, 0.0), 0.0));
	return new hitable_list(list, j);
}

hitable *lambert_scene() {
	hitable **list = new hitable*[2];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
	list[1] = new sphere(vec3(0, 0.5, 0), 0.5, new lambertian(vec3(1.0, 0.0, 0.0)));
	return new hitable_list(list, 2);
}

hitable *dielectric_scene() {
	hitable **list = new hitable*[2];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
	list[1] = new sphere(vec3(0, 0.5, 0), 0.5, new dielectric(1.5));
	return new hitable_list(list, 2);
}

int main() {
	ofstream imagefile("image.ppm");
	if (imagefile.is_open()) {
		
		/*int width = 160;
		int height = 120;*/
		int width = 800;
		int height = 600;
		int ns = 10;

		imagefile << "P3\n" << width << " " << height << "\n255\n";

		float R = cos(M_PI / 4);
		
		hitable *world = random_scene();
		//hitable *world = metal_scene();
		//hitable *world = lambert_scene();
		//hitable *world = dielectric_scene();
		vec3 lookfrom(13, 2, 3);
		vec3 lookat(0, 0, 0);
		float dist_to_focus = 10.0;
		//float dist_to_focus = (lookfrom - lookat).length();
		float aperture = 0.1;
		camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(width) / float(height), aperture, dist_to_focus);

		for (int j = height-1; j >= 0; j--)
		{
			for (int i = 0; i < width; i++)
			{
				vec3 col(0, 0, 0);
				for (int s = 0; s < ns; s++) {
					float u = float(i + drand) / float(width);
					float v = float(j + drand) / float(height);
					ray r = cam.get_ray(u, v);
					vec3 p = r.point_at_parameter(2.0);
					col += color(r, world, 0);
				}
				col /= float(ns);
				col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
				int ir = int(255.99*col[0]);
				int ig = int(255.99*col[1]);
				int ib = int(255.99*col[2]);

				imagefile << ir << " " << ig << " " << ib << "\n";
			}
			std::cout << (height-(float)j) / (float)height * 100 << "\n";
		}
		imagefile.close();
	}
}