CUDA Path Tracer
================

![sneak peek](img/top_image.png)
*Scene: test_3.txt, Iterations: 30000, Camera lens: 0.5, Camera focal length: 3*

* **Name:** Gizem Dal
  * [LinkedIn](https://www.linkedin.com/in/gizemdal), [personal website](https://www.gizemdal.com/)

## Project Description ##

This is a CUDA-based path tracer capable of rendering globally-illuminated images very quickly with the power of parallelization. Check out the [CUDA Denoiser](https://github.com/gizemdal/Project4-CUDA-Denoiser) project to see how the denoising approach discussed in the [Edge-Avoiding A-Trous Wavelet Transform for fast Global Illumination Filtering](https://jo.dreggn.org/home/2010_atrous.pdf) paper by Dammertz, Sewtz, Hanika, and Lensch is implemented for a CUDA path tracer.

<img src="img/readme_labels.png" alt="Labeled Overview" width=500>

#### Table of Contents  

[Material Overview](#overview-material)  
[Features Overview](#overview-features)
[Denoiser](#denoiser)
[Insights](#insights)   
[Performance Analysis](#performance)   
[Bloopers](#bloopers)  

<a name="overview-material"/> 

## Material Overview ##

Material shading is split into different BSDF evaluation functions based on material type. Supported materials include diffuse, mirror, perfect refractive, fresnel dielectric and glossy materials. Diffuse material scattering is computed by using cosine-weighted samples within a hemisphere. Fresnel dielectric materials are defined in the scene file with an index of refraction, which is used by BSDF evaluation to compute the probability of refracting versus reflecting the scatter ray. Mirror material scattering function reflects the ray along the surface normal while glossy reflection happens within a lobe computed by the specular exponent of the material.

**Speculars**

Fresnel dielectric | Perfect refractive | Perfect specular
:---: | :---: | :---: 
<img src="img/renders/ico_alias_1500_dielec.png" alt="sneak peek" width=300> | <img src="img/renders/ico_glass_1500.png" alt="sneak peek" width=300> | <img src="img/renders/ico_mirror_1500.png" alt="sneak peek" width=300>

**Imperfect Specular Reflection**

Exponent = 0 | Exponent = 5 | Exponent = 12 | Exponent = 50 | Exponent = 500
:---: | :---: | :---: |  :---: | :---:
<img src="img/renders/glossy_0.png" alt="Glossy 0" width=200> | <img src="img/renders/glossy_5.png" alt="Glossy 5" width=200> | <img src="img/renders/glossy_12.png" alt="Glossy 12" width=200> | <img src="img/renders/glossy_50.png" alt="Glossy 50" width=200> | <img src="img/renders/glossy_500.png" alt="Glossy 500" width=200>

Lower specular exponent values give results that are closer to diffuse scattering while larger specular exponent values result in larger highlights and more mirror-like surfaces.

<a name="overview-features"/> 

## Features Overview ##

**GLTF Mesh Loading**
[Icosahedron](https://people.sc.fsu.edu/~jburkardt/data/obj/icosahedron.obj) | [Magnolia](https://people.sc.fsu.edu/~jburkardt/data/obj/magnolia.obj) | [Duck](https://github.com/KhronosGroup/glTF-Sample-Models/tree/master/2.0/Duck)
:---: | :---: | :---:
<img src="img/renders/ico_1500.png" alt="sneak peek" width=300> | <img src="img/renders/magnolia_1000.png" alt="sneak peek" width=300> | <img src="img/renders/duck_1200.png" alt="sneak peek" width=300>

In order to bring the mesh data into C++, I used the [tinygltf](https://github.com/syoyo/tinygltf/) library. I used the VBOs from the imported data to create the mesh triangles and store triangle information per arbitrary mesh.

Bounding volume intersection culling is applied at the ray-geometry intersection test to reduce the number of rays that have to be checked against the entire mesh by first checking rays against a volume that completely bounds the mesh. This feature is implemented as toggleable for performance analysis purposes. Pressing the 'B' key while running the GPU renderer will enable this feature.

In order to smoothen the triangles on round GLTF meshes, the intersection normal is computed from the barycentric interpolation of the 3 normals from the triangle vertices.

**Depth of Field**

Focal Distance = 30, Lens Radius = 2.5 | Focal Distance = 20, Lens Radius = 2.5
:---: | :---:
<img src="img/renders/DOF_focal30.png" alt="sneak peek" width=600> | <img src="img/renders/DOF_focal20.png" alt="sneak peek" width=600>

The scene camera can be set to enable focal distance and lens radius to get a depth of field effect. Geometries located at the focal distance within the lens radius will stay in focus while other geometry around the scene will be distorted.

**Anti-aliasing**

Anti-aliasing enabled | Anti-aliasing disabled
:---: | :---: |
<img src="img/renders/ico_alias_1500_dielec.png" alt="anti-aliasing enabled" width=500> | <img src="img/renders/ico_no_alias_1500_dielec.png" alt="anti-aliasing disabled" width=500>

Using anti-aliasing for subpixel sampling results in smoother geometry edges in the render results. It is important to note that anti-aliasing and first bounce cache do not work together, since the pixel samples will differ per iteration and cached first bounces from the first iteration won't match the generated ray direction in further iterations. In order to provide flexibility, I set first bounce cache usage as a toggleable feature rather than the default, so that anti-aliasing could be enabled if the first bounce cache is not used.

**SDF-Based Implicit Surfaces**

Tanglecube | Bounding Box
:---: | :---:
<img src="img/renders/tanglecube_500.png" alt="tanglecube" width=500> | <img src="img/renders/bound_box_500.png" alt="bounding box" width=500>

Ray-geometry intersection for implicit surfaces is computed by special signed distance functions and ray marching. The sign of the SDF return value determines whether the ray is outside, on or inside the implicit geometry. Ray marching is used for following the ray direction in small increments, passing the current position on the ray to the SDF function and deciding whether an intersection occured or the marching should be terminated if the maximum marching distance is reached. These implicit surfaces can be combined with specular BSDFs such as mirror or fresnel dielectric material.

Mirror Tanglecube | Glass Bounding Box
:---: | :---:
<img src="img/renders/tanglecube_mirror.png" alt="tanglecube" width=500> | <img src="img/renders/box_glass.png" alt="bounding box" width=500>

*Note: For the best render results, the bound box should have the scale value of 1 for all scale components.*

**Procedural Textures**

[FBM](https://thebookofshaders.com/13/) | [Noise](https://thebookofshaders.com/edit.php#11/wood.frag)
:---: | :---:
<img src="img/renders/fb_tex.png" alt="fbm" width=500> | <img src="img/renders/noise_tex.png" alt="noise" width=500>

I closely followed the procedural texture implementations from the link provided on top of the render images. I find the *Book of Shaders* noise texture implementations to be pretty useful for generating aesthetically pleasing procedural textures from fragment data. The two procedural textures currently supported by the renderer include Fractal Brownian Motion texture which benefits from a loop of adding noise to create a fractal looking noise pattern, and Wood Noise with a swirl effect.

*Note: The procedural textures work the best with round geometry such as spheres, round arbitary meshes, tanglecube, etc.*

FBM Tanglecube | Noise Duck
:---: | :---:
<img src="img/renders/tanglecube_tex.png" alt="fbm" width=500> | <img src="img/renders/duck_tex.png" alt="noise" width=500>

**Stratified Sampling**

Stratified (4 samples) | Random (4 samples)
:---: | :---:
<img src="img/renders/STRATIFIED_4.png" alt="stratified 4" width=600> | <img src="img/renders/RANDOM_4.png" alt="random 4" width=600>

Stratified (4 samples close up) | Random (4 samples close up)
:---: | :---:
<img src="img/renders/STRATIFIED_4_close.png" alt="stratified 25" width=600> | <img src="img/renders/RANDOM_4_close.png" alt="random 25" width=600>

Although it isn't very visible at larger sample sizes, using stratified samples compared to random samples results in slightly more converged results at very small iteration steps.

**Hierarchical Spatial Structure - Octree (In progress)**

I started implementing a hierarchical spatial structure named Octree. The purpose of this data structure is to contain the scene geometry within children nodes (at most 8 children nodes per node) by using 3D volume bounding boxes with the goal of eliminating naive geometry iteration in the ray-scene intersection test, thus improve the rendering performance. Due to time constraints, this feature is not completed yet though it is still in the works.

<a name="denoiser"/> 

## Denoiser ##

This project also includes a pathtracing denoiser that uses geometry buffers (G-buffers) to guide a smoothing filter. The technique is based on the [Edge-Avoiding A-Trous Wavelet Transform for fast Global Illumination Filtering](https://jo.dreggn.org/home/2010_atrous.pdf) paper by Dammertz, Sewtz, Hanika, and Lensch. The example renders below show the effect of this denoiser on a scene with 10 iterations and the following denoiser parameters:
- **Filter Size: 10**
- **Blur Size: 64**
- **Col_W: 38.398**
- **Nor_W: 0.610**
- **Pos_W: 3.315**

Original Render | Denoised Render
:---: | :---:
<img src="img/denoiser/10_samples_1.png" alt="reg10" width=600> | <img src="img/denoiser/10_denoised_1.png" alt="den10" width=600>

GBuffers include scene geometry information such as per-pixel normals and per-pixel positions, as well as surface color for preserving detail in mapped or procedural textures. The current implementation stores per-pixel metrics only from the first bounce.

Position | Normal | Base Color
:---: | :---: | :---:
<img src="img/denoiser/buf0.png" alt="pos" width=300> | <img src="img/denoiser/buf1.png" alt="nor" width=300> | <img src="img/denoiser/buf2.png" alt="col" width=300>

### Denoiser Parameters ###

Denoiser parameters such as filter and blur sizes, number of iterations, color/normal/position weights are adjustable from the provided [Imgui](https://github.com/ocornut/imgui) GUI.

**Blur Size**

The examples below use the following parameters:
- **Filter Size: 5**
- **Iterations: 2**
- **Col_W: 11.789**
- **Nor_W: 0.610**
- **Pos_W: 0.407**

Blur=10  | Blur=33 | Blur=80 | Blur=185
:---: | :---: | :---: | :---: 
<img src="img/denoiser/params/blur_10.png" alt="pos" width=300> | <img src="img/denoiser/params/blur_33.png" alt="nor" width=300> | <img src="img/denoiser/params/blur_80.png" alt="col" width=300> | <img src="img/denoiser/params/blur_185.png" alt="col" width=300>

Increasing the blur size allows more denoiser iterations since the blur size determines the largest expansion width the filter can reach. Applying more iterations result in smoother images, especially for very small number of path tracer iterations (which in this example is only 2). However, increasing the blur size has performance implications. You can check the [Performance Analysis](<a name="performance"/> ) section for more detail.

**Color Weight**

The examples below use the following parameters:
- **Filter Size: 5**
- **Iterations: 2**
- **Blur Size: 80**
- **Nor_W: 0.610**
- **Pos_W: 0.407**

Col_W=1.423  | Col_W=4.675 | Col_W=15.244 | Col_W=30.285
:---: | :---: | :---: | :---: 
<img src="img/denoiser/params/col_1.423.png" alt="pos" width=300> | <img src="img/denoiser/params/col_4.675.png" alt="nor" width=300> | <img src="img/denoiser/params/col_15.244.png" alt="col" width=300> | <img src="img/denoiser/params/col_30.285.png" alt="col" width=300>

The results suggest that increasing the color weight results in smoother denoised results. Lower color weights result in denoised renders with more "fireflies" while larger weights give smoother results. However, it is important to mention that the impact of color weight may depend from implementation to implementation. The current adaptation from the A-Trous approach halves the color weight at each denoise blur iteration in order to smoothen smaller smaller illumination variations, as suggested by the paper.

**Light Size**

The examples below use the following parameters:
- **Filter Size: 5**
- **Iterations: 10**
- **Blur Size: 63**
- **Col_W: 28.455**
- **Nor_W: 0.813**
- **Pos_W: 3.862**
- **Light Intensity: 1**

Light Width = 3 | Light Width = 5 | Light Width = 7 | Light Width = 10
:---: | :---: | :---: | :---: 
<img src="img/denoiser/params/3_light.png" alt="pos" width=300> | <img src="img/denoiser/params/5_light.png" alt="nor" width=300> | <img src="img/denoiser/params/7_light.png" alt="col" width=300> | <img src="img/denoiser/params/10_light.png" alt="col" width=300>

While increasing the light size gives us better illuminated renders in these examples, increasing the light intensity may result in less smoother results with a lot of fireflies. The example renders below use the following parameters:
- **Filter Size: 10**
- **Iterations: 10**
- **Blur Size: 64**
- **Col_W: 38.398**
- **Nor_W: 0.610**
- **Pos_W: 3.315**

**Intensity = 1** | **Intensity = 2**
:---: | :---:
<img src="img/denoiser/10_denoised_1.png" alt="den10_1" width=600> | <img src="img/denoiser/10_denoised_2.png" alt="den10_2" width=600>

**Filter Size**

The examples below use the following parameters (The blur sizes are adjusted to allow the same number of blur iterations for different filter sizes):
- **Iterations: 10**
- **Col_W: 16.667**
- **Nor_W: 0.610**
- **Pos_W: 0.813**

Filter = 5, Blur = 80 | Filter = 9, Blur = 80 | Filter = 21, Blur = 81 | Filter = 43, Blur = 103
:---: | :---: | :---: | :---: 
<img src="img/denoiser/params/filter_5.png" alt="pos" width=300> | <img src="img/denoiser/params/filter_9.png" alt="nor" width=300> | <img src="img/denoiser/params/filter_21_81.png" alt="col" width=300> | <img src="img/denoiser/params/filter_43_103.png" alt="col" width=300>

The results suggest that changing the filter size doesn't seem to have a significant impact on denoiser results.

<a name="insights"/> 

## Insights ##

One interesting observation I have is that using material sort results in more stable render results compared to naive approach. The two images below, both rendered with 4950 iterations, are renders from the same camera position. The render on the left is taken by sorting rays by material type while the one on the right is rendered by the naive approach.

Material sort enabled | Material sort disabled
:---: | :---: |
<img src="img/readme.png" alt="anti-aliasing enabled" width=300> | <img src="img/readme_nosort.png" alt="anti-aliasing disabled" width=300>

<a name="performance"/> 

## Performance Analysis ##

The performance is measured on a **Predator G3-571 Intel(R) Core(TM) i7-7700HQ CPU @ 2.80 GHz 2.81 GHz** machine.

I used a GPU timer to conduct my performance analysis on different features and settings in the renderer. This timer is wrapped up as a performance timer class, which uses the CUDAEvent library, in order to measure the time cost conveniently. The timer is started right before the iteration loop and is terminated once a certain number of iterations is reached. The measured time excludes the initial cudaMalloc() and cudaMemset() operations for the path tracer buffers, but still includes the cudaGLMapBuffer operations.

**Stream Compaction**

Using stream compaction to terminate paths which don't hit any geometry in the scene can be very beneficial for open scenes where some portion of the rays will shoot to void from the start. In order to show the impact of stream compaction on render path segments, I have created 3 test scenes with different layouts. All 3 test scenes are rendered with 1500 iterations.

Closed Cornell | Open Cornell I | Open Cornell II
:---: | :---: | :---:
<img src="img/renders/dode_closed_1500.png" alt="Closed Cornell" width=300> | <img src="img/renders/dode_1500.png" alt="Open Cornell I" width=300> | <img src="img/renders/dode_out_1500.png" alt="Open Cornell II" width=300>

For stream compaction analysis, I timed a single iteration of the renderer and recorded the number of remaning paths after each path termination in the 3 test scenes. I used a ray depth of 8, such that once this depth is reached all remaning paths would be terminated.

<img src="img/stream_graph.png" alt="stream compaction graph" width=1200>

I also recorded the total runtime of 1 iteration per test scene.

Scene | Measured runtime (in seconds)
:---: | :---: 
Closed Cornell | 0.260587
Open Cornell I | 0.146422
Open Cornell II | 0.106178

From this data, we can conclude that using stream compaction for path termination is very beneficial performance wise for open scenes where some rays could shoot to void and not hit any geometry within the scene. Although terminating as many paths as possible when needed is a significant performance improvement, it might not be possible to terminate many rays in closed scenes such as Closed Cornell where rays cannot escape.

**Volume Intersection Culling**

I used 4 arbitrary mesh examples to analyze the peformance benefits of enabling volume intersection culling for complex meshes. The table below shows the mesh examples used for this analysis and how many triangles they contain.

Mesh | Number of triangles
:---: | :---: 
Icosahedron | 20
Magnolia | 1372
Duck | 4212
Stanford Bunny | 69630

I measured the total runtime of 6 iterations in 1 test scene per mesh. I used simple open cornell box scenes where no other arbitrary meshes except the subject mesh is included. The runtime measurements yielded the following results.

<img src="img/volume_graph.png" alt="Volume Intersection Culling graph" width=1000>

Using volume intersection culling for simpler arbitrary meshes such as Icosahedron or Dodecahedron doesn't provide a significant performance improvement for a small number of iterations, however as the total number of iterations increases it can provide more significant efficiency. Although we do not observe a significant performance improvement with Icosahedron, we can see that using volume intersection culling improves scene intersection test performance significantly for scenes where much more complex shapes such as Stanford Bunny is present. Just with 6 iterations, using volume intersection culling has saved about 29 seconds.

**Procedural Textures**

Current procedural textures supported by the renderer make many calls to noise helper functions that call many glm math functions. In order to analyze the potential impacts of procedural textures on runtime, I created 3 simple test scenes with a sphere where the diffuse material of the sphere uses FBM, Wood noise and no texture separately and compared the total runtimes of 100 and 500 iterations.

<img src="img/texture_graph.png" alt="Texture graph" width=900>

Although the difference is not very significant due to the small number of iterations, using the FBM texture seems to be slightly less efficient than using Wood Noise or no texture at all. Since FBM functions usually call their helpers the octave amount of times, it is possible that these subsequent function calls could slow down the performance.

<a name="bloopers"/> 

## Bloopers ##

<img src="img/blooper_1.png" alt="b" width=300> <img src="img/blooper_2.png" alt="b" width=300> 

