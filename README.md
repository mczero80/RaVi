# RaVi
RaVi (The Realtime Raytracing Visualizer)

Long ago, Michael Granz (http://www.winosi.onlinehome.de) wrote a Realtime Raytracer
with impressive Speed. It shared its renderer code with WinOSi (http://www.winosi.onlinehome.de/Downloads_WinOSi.htm).

The RaVi-Demo shows some sample scenes with different shaders and quality settings, ranging from fast approximations
to super high-quality filtered adaptive 4x4 anti-aliasing.



It includes some Demoscenes:

Bouncing Sphere
The first demo animates the famous "mirror-sphere on checkerboard floor" scene in 320x240 pixel resolution at interactive framerates of up to 60 fps.
In adaptive sub-sampling mode, a special interpolation scheme saves many unneccessary ray-casts without decreasing image quality too much.

Moving Light
Here, the lightsource moves around the scene. Phong shading with specular highlights and correct dropshadows are rendered on true quadric surfaces in real-time.
The scene is stored in a special space partitioning structure (BSP Tree), which makes it possible to render very large scenes with many hundreds or thousands of
objects in logarithmic time.

Glass Cube
Another demo shows a spinning glass cube. True reflection and refraction according to fresnels law is calculated in real-time.

Soft Shadow
One of the demo sets shows a cylinder lit by an area light casting a soft shadow with correct penumbra.

Caustics
The current RaVi-Engine can even render caustics in real-time, but this feature is still under development.

Cornell Box
The latest demo shows global illumination with diffuse indirect illumination and color bleeding in real-time!



During the demos, you can click and hold down the left mouse-button inside the window, and move the mouse to rotate the virtual camera around the scene in real-time.

The first commit uses a copy of the last official version 0.38. It supports real-time global illumination and caustics with multithreading for up to 4 threads gaining
a performance increase of 20-50% on Hyperthreading processors and 80-90% on a dualprocessor machine. It has adaptive quality management with mixed sub- and super-sampling
to satisfy the desired frame rate, with best possible image quality. This version uses some (SSE-)assembly optimizations but no SIMD yet.

The setup of caustics and global illumination is a little bit faked in this demo, but future versions could generalize this concept study to allow loading of any 
arbitrary scene file and render it with all optical effects in real-time!
