# What is it? #
A simple but helpful class for handling render-target and multiple render-target (MRT) through the use of FBOs in OpenGL.

# Sample usage #
Checkout the SVN repository and just add the RenderTarget.h and RenderTarget.cpp files in your project, the code should be usable directly. You may need to tweak the include files that I have given, as they were for my own project, and also include your GL headers.

### Creating a 64bit GBuffer MRT with albedo, normal and depth texture components ###
```
int vp[4];
glGetIntegerv(GL_VIEWPORT, vp);

m_GBuffer = new RenderTarget(vp[2], vp[3]);
m_GBuffer->BeginAdd();
m_GBuffer->AddComponent("albedo", GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_COLOR_ATTACHMENT0);
m_GBuffer->AddComponent("normal", GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_COLOR_ATTACHMENT1);
m_GBuffer->AddComponent("depth", GL_LUMINANCE32F_ARB, GL_RED, GL_FLOAT, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_COLOR_ATTACHMENT1);
m_GBuffer->EndAdd();
```

By default, the FBO will have a RenderBuffer associated with it. You can change this behaviour by passing `false` in the BeginAdd method.

### Getting the RenderTarget index ###
A common scenario is to use the MRT indices with glBindFragDataLocation to allow a shader to write to the MRT textures.
```
GLuint indexAlbedo = m_GBuffer->GetAttachmentIndex("albedo");
GLuint indexNormal = m_GBuffer->GetAttachmentIndex("normal");
GLuint indexDepth = m_GBuffer->GetAttachmentIndex("depth");
```

Notice that if you are setting the MRT output in the shader using the GLSL layout, you do not need to bind the indices manually. Example GLSL code that does this for the above MRT:
```
(fragment shader)
#version 420 core

#define ALBEDO 0
#define NORMAL 1
#define DEPTH 2

layout(location = ALBEDO, index = 0) out vec4 Albedo;
layout(location = NORMAL, index = 1) out vec4 Normal;
layout(location = DEPTH, index = 2) out float Depth;

void main()
{
    Albedo = vec4(1,0,0,1);
    Normal = vec4(0,0,1,1);
    Depth = 0.5f;
}
```
### Binding and resolving ###
It is very easy to bind and resolve the RenderTarget components:
```
// bind them so that a shader can write to their textures
m_GBuffer->Set();
...
...
// resolve and get the textures!
RenderTarget::Resolve();
GLuint albedoTexture = m_GBuffer->GetTexture("albedo");
GLuint normalTexture = m_GBuffer->GetTexture("normal");
GLuint depthTexture = m_GBuffer->GetTexture("depth");

```

# Bugs and requests #
Please feel free to submit any issues you encountered, or improvement requests!