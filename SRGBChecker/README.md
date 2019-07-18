sRGB Checker
=============================

## Testing environment

|        |                                  |
| ------ | -------------------------------- |
| OS     | Windows10                        |
| GPU    | NVIDIA GeForce GTX 760 (v430.86) |
| OpenGL | 4.5.0 NVIDIA 430.86              |
| GLFW   | 3.3.0                            |
| GLEW   | 2.1.0                            |



## Default framebuffer

| framebuffer    | object_type            | R    | G    | B    | A    | D    | S    | component_type         | color_encoding |
| -------------- | ---------------------- | ---- | ---- | ---- | ---- | ---- | ---- | ---------------------- | -------------- |
| GL_FRONT_LEFT  | GL_FRAMEBUFFER_DEFAULT | 8    | 8    | 8    | 8    | 0    | 0    | GL_UNSIGNED_NORMALIZED | GL_LINEAR      |
| GL_FRONT_RIGHT | GL_FRAMEBUFFER_DEFAULT | 0    | 0    | 0    | 0    | 0    | 0    | GL_UNSIGNED_NORMALIZED | GL_LINEAR      |
| GL_BACK_LEFT   | GL_FRAMEBUFFER_DEFAULT | 8    | 8    | 8    | 8    | 0    | 0    | GL_UNSIGNED_NORMALIZED | GL_LINEAR      |
| GL_BACK_RIGHT  | GL_FRAMEBUFFER_DEFAULT | 0    | 0    | 0    | 0    | 0    | 0    | GL_UNSIGNED_NORMALIZED | GL_LINEAR      |



| GL_FRAMEBUFFER_SRGB | 'Linear to Linear' shader |  'Linear to sRGB' shader  |
| :-----------------: | :-----------------------: | :-----------------------: |
|      disabled       | ![Preview0](preview0.png) | ![Preview2](preview2.png) |
|       enabled       | ![Preview1](preview1.png) | ![Preview3](preview3.png) |



This sample is a WORK IN PROGRESS and actually not meant as a sample.

On the TODO list:
*

Run the sample for more information.

