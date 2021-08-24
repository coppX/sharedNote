//仅仅是学习笔记，难免有错误，仅作参考
# 光栅化和光线追踪对比
光栅化和光线追踪的可见性确定可以用双循环来描述
光栅化的可见性描述
```
for (T in triangles)
    for (P in pixels)
        determine if P is inside T
```
光线追踪可见性描述
```
for (P in pixels)
    for (T in triangles)
        determine if ray through P hits T
```
需要速度快使用光栅化，需要更好的效果使用光线追踪(Use raster when faster, else rays to amaze.)  
目前业界主流的实时光线追踪技术是采用的传统光栅化渲染管线+光线追踪的混合渲染管线(Hybrid Rendering Pipeline)的形式。

# 原理
光线追踪示意图
![](./raytracing1.png)
![](./raytracing2.png)
人看见物体是因为物体反射了光线进入人眼中，光线追踪根据光路的可逆性，从人眼中发出光线射向物体，然后计算这个点的颜色，然后再反射到不同的物体上，一直递归下去，直到到达光线深度(递归深度)或者达到结束条件。  
注:shadow ray指交点和光源的连线，因为可以用来判断该点是否处于阴影中

## 代码描述
```
rayTraceImage()
{   //对屏幕的每一个像素进行光线追踪
    for(p in pixels)
        color of p = trace(eye ray through p);
}
trace(ray)
{   //找到与光线相交的最近的点
    pt = find closest intersection
    //对这个点进行着色
    return shade(pt);
}
shade(point)
{
    color = 0;
    for (L in light sources)
    {
        trace(shadow ray to L);
        color += evaluate BRDF;
    }
    color += trace(reflection ray);
    color += trace(refraction ray);
    return color;
}
```
# 光线追踪着色器
- ray generation shader
- closest hit shader
- miss shader
- any hit shader
- interstion shader
- callable shader  
使用参考下方TraceRay控制流部分  
[DXR详情参考](https://docs.microsoft.com/en-us/windows/win32/direct3d12/direct3d-12-raytracing-hlsl-shaders)  

# 优化
## 加速结构
为了加速光线与场景相交运算，光线追踪需要把几何体组织成加速结构(acceleration structure, AS)，这样会减少光线-三角形求交测试(ray-triangle intersection test),这个加速结构在硬件上是一种层次结构，但是暴露给用户的是只有两层结构
![](./raytracing3.png)
[DXR中说明](https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#ray-geometry-interaction-diagram)  
[vulkan中说明](https://nvpro-samples.github.io/vk_raytracing_tutorial_KHR/#accelerationstructure)
- 顶层加速结构(Top-level acceleration structure，简写TLAS)，TLAS是对象实例级别的。每个实例都指向一个BLAS，并且包含一些特定于这个实例的数据信息，例如变换矩阵、InstanceID(24-bit,vulkan)和shader offset index(32-bit，vulkan，用于在shader tables中查找shader使用)。BLAS的数据都是在局部空间的，TLAS维护了从local2world的转换矩阵。可以有多个实例指向一个BLAS,但是他们的transforms不同。
- 底层加速结构(Bottom-level acceleration structure，简写BLAS)，BLAS是集合体级别的，对于传统的三角形Mesh，它使用Bounding Volume Hierarchy(BVH)树型数据结构来管理这些三角形。BLAS一般存储实际的geometry数据，比如vertex/index buffer等，这些数据可以用于计算和光线的交点，对于procedural geometry，还需要存储AABB,并使用intersection shader计算和光线的交点。  
BLAS中的geometry可以被TLAS多次引用，每个引用就是一个实例化的过程。这样虽然某个Mesh在场景中被用到多次，但是它只需要在BLAS中存一次，减少了显存的占用。 
场景更新时，一般更新实例位置更新TLAS就可以了，只有在变更geometry信息时才会更新BLAS的数据，场景中大部分是重复的静态网格资源，所以可以避免频繁的修改BLAS的数据，所以效率是很高的。   
在DXR中，光线可能与任意的geometry相交，并触发对应的material shader，其中geometry存储在BLAS中，而对应的TLAS里面存储了相应的shader offset index，也就是对该geometry进行着色的material shader，对应的shader数据以shader table的方式存储在显存中。如果场景很大，material shader复杂，显存的压力很大。另一方面，光线与场景求交调用material shader的过程，没有明显的特点，很难利用局部性进行优化，所以现在的实时渲染中，基本都不会采用full resolution的光线追踪。
## 连贯性(Coherency)
在程序执行期间利用连贯性是硬件和软件上性能优化最重要的想法之一，在现今的硬件上，在时间和能耗上最昂贵的操作就是内存访问,大多数情况下，性能优化侧重于利用内存的连贯性（缓存）和围绕内存延迟进行调度计算。GPU 本身可被视为一个处理器，它明确约束其运行的程序（数据并行、独立计算线程）的执行模型，以便更好地利用内存的连贯性。
### 场景连贯性(Scene Coherency)
### 光线和着色连贯性(Ray and Shading Coherency)
![](./raytracing4.jpeg)
光线追踪算法在对缓存结构是非常不友好的，为了尽可能提高光线的计算效率，一般的光线追踪实现都需要对光线进行排序，以使其算法对内存访问具有一定的连贯性。为了最大化这种光线处理(例如对场景的遍历和着色计算)的效率，这样的工作显然作为固定管线会更加合适，上图所示，shader产生rays，然后提交给固定管线，固定管线最后给出ray与表面的交点(BLAS中如果采用三角形mesh，固定管线会计算光线与三角形相交，如果是自定义的其他几何体，则需要自己实现求交算法，需要使用intersection shader)，交回给一个表面着色的shader。在这中间发生了光线对整个几何场景加速数据结构的遍历以及求交计算，这个过程完全是基于固定管线的，这个做法其实是为了在遍历和求交计算中尽可能针对GPU进行优化。上图中包含两个调度阶段，其中一个调度发生在遍历场景之前，第二个发生在着色之前，这里面的调度工作就可能包含对光线的排序，例如针对特定数据结构的优化，这样遍历的算法具有更好的并行性和对内存的连贯性，然后遍历完之后再经历一次排序，比如为了让后续的着色计算具有更好的并行性，其可能将碰撞点根据表面来排序，让同一个表面或者材质对应的光线交点放到一起计算，这样效率就会更高。
# TraceRay控制流
![](./raytracing5.png)
- 首先硬件会搜索加速结构里面可能和光线相交的primitive，如果找到了就判断该primitive是否是三角形，如果是三角形就采用固定管线里面的三角形求交算法，如果该primitive不是三角形，则采用自己用intersection shader定义的求交算法来进行求交运算。
- 拿到交点后，判断是不是最近的交点，如果是就判断交点处物体是否不透明(opaque)，判断方法就是通过检测instance flags或者没有any hit shader的时候就默认交点处是不透明的。  
    - 交点处透明:就执行any hit shader， any hit shader通常用来处理透明(transparency)和阴影(shadow),这两种情况下需要计算光线和场景的多个交点。DXR为了避免光线数量爆掉，在any hit shader中强制不允许生成新的光线。  
    - 交点处不透明:执行closest hit shader， closest hit shader在所有的any hit shader完成之后执行，通常用于计算屏幕像素颜色，在closest hit shader中可以产生新的光线。
- 如果光线和场景没有交点，就触发了miss shader。在miss shader中也可以生成新的光线，通常是加长光线的长度或改和lower-LOD的场景相交，其目的是重新尝试获取交点。在miss shader中也可以采样skybox，直接返回skybox的颜色。
# 降噪
![](./raytracing6.jpg)
[图片来源](https://zhuanlan.zhihu.com/p/28288053)
## 采样过滤
- SVGF(Spatiotemporal Variance-Guided Filter)
- A-SVGF(Adaptive Spatiotemporal Variance-Guided Filtering)
## 机器学习
- OIDN(Intel Open Image Denoise)
- Optix(NVIDIA Opitx ray tracing engine)
- DLSS(Deep Learning Super Sampling)


# 参考文献
- [《real-time rendering 4th》chapter26 real-time ray tracing](http://www.realtimerendering.com/raytracing.html)  
- https://zhuanlan.zhihu.com/p/144403005  
- https://zhuanlan.zhihu.com/p/34894883
- https://www.zhihu.com/question/310930978/answer/1606439715  
- https://jerish.blog.csdn.net/article/details/104708361  
- https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html  
- https://nvpro-samples.github.io/vk_raytracing_tutorial_KHR/#accelerationstructure
- https://alain.xyz/blog/ray-tracing-denoising