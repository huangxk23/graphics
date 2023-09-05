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

