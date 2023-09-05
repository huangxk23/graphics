#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>

constexpr double MY_PI = 3.1415926;

//计算camera transformation的变换矩阵
Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}


//计算model transformation的变换矩阵
Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    
    //c++ 的cmath 函数库中的sin cos tan等角度计算函数的输入是弧度
    //使用弧度是因为角度是具有单位的并不是实数，在数学运算中可以直接使用
    //360度 = 2 PI
    //角度转换为弧度
    //rotation_angle * PI / 180.0

    float rotation_radian = rotation_angle * MY_PI / 180.0;

    Eigen :: Matrix4f translate;
    translate << cos(rotation_radian),-sin(rotation_radian),0,0,
                 sin(rotation_radian),cos(rotation_radian),0,0,
                 0,0,0,0,
                 0,0,0,1;

    model = translate * model;

    return model;
}


//计算projection transformation的变换矩阵
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

    Eigen::Matrix4f p2o;
    p2o << zNear,0,0,0,
           0,zNear,0,0,
           0,0,zNear+zFar,-zNear*zFar,
           0,0,1,0;
    
    float theta = eye_fov / 2 * MY_PI / 180.0;
    float top = zNear * tan(theta);
    float bottom = - top;
    float right = top * aspect_ratio;
    float left = -right;
    Eigen::Matrix4f translate;
    
    translate << 1,0,0,-(left + right)/2,
                 0,1,0,-(top + bottom)/2,
                 0,0,1,-(zNear + zFar)/2,
                 0,0,0,1;
    
    Eigen::Matrix4f ortho;
    ortho << 2.0/(right - left),0,0,0,
             0,2.0/(top - bottom),0,0,
             0,0,2.0/(zNear - zFar),0,
             0,0,0,1;
    
    Eigen::Matrix4f transform_coordinate;
    transform_coordinate<<1,0,0,0,
                          0,-1,0,0,
                          0,0,-1,0,
                          0,0,0,1;

    projection = transform_coordinate * ortho * translate * p2o * projection * transform_coordinate;
    
    return projection;
}


int main(int argc, const char** argv)
{
    float angle = 0;

    //默认不保存图片
    bool command_line = false;
    std::string filename = "output.png";

    //命令行接收参数大于等于3的保存图片
    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
