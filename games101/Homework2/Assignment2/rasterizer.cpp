// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>
#include <bits/stdc++.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


static bool insideTriangle(float x, float y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    //通过叉积判断是否在三角形内部
    //v0v1*v0p      v1v2*v1p        v2v0 * v2p
    //叉积方向相同：在三角形内部
    //叉积方向不同：在三角形外部
    //叉积的方向：右手定则
    //叉积的大小：|a| * |b| * sin(\theta)

    //在笛卡尔坐标系中:(x1,y1,z1) * (x2,y2,z2) = (y1z2-z1y2,z1x2-x1z2,x1y2-y1x2)
    //方向的判断：方向相同就是两者在同一直线上->使用单位向量表示方向
    //方向相同：单位向量相同

    //浮点数精度问题：判断相等

    //只有x,y吗？？？？
    
    //疑问解答：实际上只需要两个维度即可，z可以视为0，也就是(x,y)
    //这样判断方向也是十分简单的
    //+z为一个方向，-z为另一个方向，不需要使用单位向量

    Vector3f v[3];

    for(int i = 0;i < 3;i ++)
        v[i] = _v[i],v[i].z() = 0;
    
    Vector3f p{(float)x,(float)y,0};

    Vector3f cross1 = (v[1] - v[0]).cross(p - v[0]);
    Vector3f cross2 = (v[2] - v[1]).cross(p - v[1]);
    Vector3f cross3 = (v[0] - v[2]).cross(p - v[2]);

    if((cross1.z() > 0 && cross2.z() > 0 && cross3.z() > 0) || (cross1.z() < 0 && cross2.z() < 0 && cross3.z() < 0)) return true;
    else return false;



}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    //std::cout<<pos_buf.size()<<std::endl;
    //std::cout<<ind_buf.size()<<std::endl;
    //std::cout<<col_buf.size()<<std::endl;
    //sizeof(posbuf):1
    //std::cout<<buf.size()<<std::endl;
    //sizeof(buf):6
    //也就是将两个三角形的vertex放到了一块，那hash_map的意义在哪里？？
    //我觉得不应该给每个三角形的三个顶点一个key吗？？
    //虽然后面是通过ind将两个三角形分开
    //ind = {{0,1,2},{3,4,5}}
    //第一个三角形的三个顶点属性对应buf和col中的{0,1,2}
    //第二个三角形的三个顶点属性对应buf和col中的{3,4,5}
    //先初始化第一个三角形并绘制然后是第二个

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle

    //If so, use the following code to get the interpolated z value.
    //auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
    //float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
    //float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
    //z_interpolated *= w_reciprocal;

    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
    float x0 = std::min<float>({t.v[0].x(),t.v[1].x(),t.v[2].x()});
    float x1 = std::max<float>({t.v[0].x(),t.v[1].x(),t.v[2].x()});
    
    float y0 = std::min<float>({t.v[0].y(),t.v[1].y(),t.v[2].y()});
    float y1 = std::max<float>({t.v[0].y(),t.v[1].y(),t.v[2].y()});

    int x_min = std::floor(x0);
    int x_max = std::ceil(x1);

    int y_min = std::floor(y0);
    int y_max = std::ceil(y1);

    
    
    //for(auto i : t.v)
    //{
    //    std::cout<<i.x()<<" "<<i.y()<<" "<<i.z()<<std::endl;
    //}

    //if(insideTriangle(350,120,t.v)) std::cout<<"Yes"<<std::endl;
    //else std::cout<<"No"<<std::endl;
    
    //问题：程序像素中心的定义是？？(0,0)还是(0.5,0.5)
    //(0,0)
    
    std::vector<std::pair<float,float>> offset{{0.25,0.25},{0.25,0.75},{0.75,0.25},{0.75,0.75}};
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
        
    }

    

}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;
    //std::cout<<frame_buf[ind].x()<<" "<<frame_buf[ind].y()<<" "<<frame_buf[ind].z()<<std::endl;

}

// clang-format on