### 作业2学习

主要实现了如何光栅化一个三角形：伪代码思路如下

```
获取三角形的bounding box;
for pixel in bouding box:
  if(pixel in triangle):
    if(depth of the pixel < depth_buf):
      update(depth_buf)
      update(frame_buf)
```

判断点是否在三角形内部-> 内积。

--------

实验结果如图：
![img2](./img/output2.png)

可以得到正确的实验结果，但是三角形的边缘有比较明显的锯齿。

实现MSAA(Antialiasing By Supersampling)抗锯齿：如果偷懒采用pixel的深度作为采样点的深度，抗锯齿效果还是OK的，就是会有三角形和三角形交叠的地方会有黑边存在。
```cpp
for(int i = x_min;i <= x_max;i++)
    {
        for(int j = y_min;j <= y_max;j ++)
        {
            
            auto[alpha, beta, gamma] = computeBarycentric2D(i, j, t.v);
            float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
            float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();        
            z_interpolated *= w_reciprocal;
            int idx = get_index(i,j);
            bool visible = (z_interpolated < depth_buf[idx]);
            bool update = false;
            Vector3f color(0.0f,0.0f,0.0f);

            for(int k = 0;k < 4;k ++)
            {
                float x_coord = i + offset[k].first;
                float y_coord = j + offset[k].second;
                if(insideTriangle(x_coord,y_coord,t.v) && visible)
                {
                    depth_buf[idx] = z_interpolated;
                    update = true;
                    color += t.getColor() / 4.0f;        
                }
            }

            if(update) set_pixel({(float)i,(float)j,0},color);   
        }
```
![img3](./img/output3.png)

给每一个采样点维护一个深度值和颜色之后并在计算所有采样点的数值之后再计算pixel的平均值：
![img4](./img/output4.png)

