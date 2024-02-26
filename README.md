# Image Viewer
Is a OpenGL application for viewing and editing images.
The aplication uses OpenGL and Dear ImGui to render images to the screen.

###### Normal viewing

![Normal viewing](Images/img1.png)

###### Rotated+zoomed viewing
You can rotate and view the image with that rotation
**Ctrl + scroll = rotation**
**Scroll = zoom**

![Normal viewing](Images/img3.png)
##### Fullscreen viewing
To return to windowed mode press the **esc** key

![Normal viewing](Images/img4.png)

##### The edit menu
![Normal viewing](Images/img2.png)

#### Editing the image
![Normal viewing](Images/img5.png)

#### Save options
![Normal viewing](Images/img6.png)

### Libraries

Libraries used are : GLFW, Dear ImGui (OpenGL with GLFW), GLAD and stb(stb_image, stb_image_write)

### Building

For the GLFW, Dear ImGui and GLEW i used vcpkg and for stb follow the example on github.

library    | used version | description
------- | ---- | ------------------
**[imgui](https://github.com/ocornut/imgui)** | 1.89.9 | C++ GUI library (opengl3 and glfw bindings)
**[GLFW](https://www.glfw.org/)** | 3.3.8#2 | OpenGL library
**[GLAD](https://github.com/Dav1dde/glad)** | 0.1.36 |Vulkan/GL/GLES/EGL/GLX/WGL Loader-Generator based on the official specifications for multiple languages
**[stb](https://github.com/nothings/stb)** | latest | including stb_image and stb_image_write

Special thanks to  @fairlight1337 for [hsv conversion](https://gist.github.com/fairlight1337/4935ae72bcbcc1ba5c72)