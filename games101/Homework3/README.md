### 作业三 学习笔记
----

### 渲染管线

光栅化的过程和光线追踪的过程并不相同。光线追踪是遍历每一个像素，并且试图找到可能对该像素的颜色造成影响的物体。光栅化是依次遍历每一个object，并找到每一个当前object可能影响到的像素，并进行着色。找到每一个物体影响的像素的过程就是光栅化的过程，光栅化实际上是按照物体顺序进行渲染的。图形渲染管线实际上就是从world space中的物体出发，到更新屏幕空间中的像素的过程，如图所示。
![img1](img/1.PNG)

输入是world space 中的三维顶点，经过vertex processing,也就是对三维空间中的点经过model transformation,camera transformation,projection transformation以及viewport transformation,输出vertex stream,也就是screen space中的顶点.

Rasterization接收screen space中的triangle作为输入，通过sample以及z-buffering找到屏幕上当前三角形会影响的像素，也就是fragment stream,fragment可以翻译为片元，可以理解为当前triangle可以影响的像素就是片元fragment。

Fragment processing 对给定的fragment stream 进行着色以及texture mapping.

从代码框架上来看对应着draw() -> rasterize_triangel() -> fragment_shader(),代码框架变化不大，还是能够轻松看懂的。

### shading
shading,着色，可以看作是将某种material 应用在物体表面的过程。

#### Blinn-phong Reflectance model 
Blinn-phong Reflectance model将每个shading point的光源划分为三种不同的类型：Specular highlights,diffuse reflection,Ambient lighting 三种不同的光照。
1. diffuse reflection
  需要定义三种不同的属性：
```math
  漫散射系数:k_d
```
```math
  到达shading point的光强：\frac{I}{r^2} 
```
```math
  shading point接收到的光强：max(0,l · n)
```
![img2](img/2.PNG)
```math
L_d = k_d \times \frac{I}{r^2} max(0,l · n)
```
2. Specular highlight
   同样需要定义三种不同的属性：
```math
  高光系数系数:k_d
```
```math
  到达shading point的光强：\frac{I}{r^2} 
```

```math
  camera看到的光强：max(0,n · h)^p
```
![img3](img/3.PNG)

```math
L_s = k_d \times \frac{I}{r^2} max(0,n · h)^p
```

3. Ambient lighting

  Assumption : add constant color to account for disregarded illumination and fill in black shadows.

```math
L_a = k_a \times I_a
```

```cpp
Eigen::Vector3f phong_fragment_shader(const fragment_shader_payload& payload)
{
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = payload.color;
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    Eigen::Vector3f result_color = {0, 0, 0};
    for (auto& light : lights)
    {
        
        Eigen::Vector3f light_direction = (light.position - point).normalized();
        Eigen::Vector3f view_direction = (eye_pos - point).normalized();

        auto r_2 = (light.position - point).squaredNorm();

        auto ambient_l = ka.cwiseProduct(amb_light_intensity);

        auto diffuse_l = kd.cwiseProduct(light.intensity / r_2) * std::max(0.0f,light_direction.dot(normal));

        auto h = (light_direction + view_direction).normalized();
        auto specular_l = ks.cwiseProduct(light.intensity / r_2) * std::pow(std::max(0.0f,h.dot(normal)),p);

        result_color += ambient_l + diffuse_l + specular_l;
        
        
    }

    return result_color * 255.f;
}
```
   


