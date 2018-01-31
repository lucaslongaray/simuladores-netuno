#include <iostream>
#include <numeric>
#include <string>
#include <QApplication>

// Sonar includes
#include <Sonar.hpp>
#include <Utils.hpp>
#include <widget_collection/sonar_widget/SonarPlot.h>
#include <widget_collection/sonar_widget/SonarWidget.h>
#include <vizkit3d_normal_depth_map/src/NormalDepthMap.hpp>
#include <vizkit3d_normal_depth_map/src/ImageViewerCaptureTool.hpp>
#include <base-types/base/Angle.hpp>

// OpenCV includes
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Openscenegraph includes
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>

// ROS includes
#include "ros/ros.h"
#include "geometry_msgs/PoseStamped.h"
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <tf_conversions/tf_eigen.h>

using namespace gpu_sonar_simulation;
using namespace vizkit3d_normal_depth_map;
using namespace cv;

#define BEAM_WIDTH 130.0
#define BEAM_HEIGHT 30.0
#define BIN_COUNT 1000//502
#define BEAM_COUNT 256//768
#define GAIN 0.88
#define RANGE 50

#define myrand ((float)(random())/(float)(RAND_MAX) )

geometry_msgs::PoseStamped sonar_pose;

void poseCallback(const geometry_msgs::PoseStamped& msg){
    sonar_pose = msg;
}

// create a depth and normal matrixes to test
cv::Mat createRandomImage(int rows, int cols) {
    cv::Mat raw_image = cv::Mat::zeros(rows, cols, CV_32FC3);

    for (int k = 0; k < raw_image.channels() - 1; k++)
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                raw_image.at<Vec3f>(i, j)[k] = myrand;

    return raw_image;
}

void addScene(osg::ref_ptr<osg::Group> root){
    std::string current_path(__FILE__);
    current_path = current_path.substr(0, current_path.find_last_of("/"));
    osg::Node* scene = osgDB::readNodeFile(current_path + "/../uwmodels/yacht/teste.osgb");
    osg::Node* terrain = osgDB::readNodeFile(current_path + "/../uwmodels/yacht/terrain.osgb");
    root->addChild(terrain);
    root->addChild(scene);
}

cv::Mat QImage2Mat(QImage const& src){
     cv::Mat tmp(src.height(),src.width(),CV_8UC3,(uchar*)src.bits(),src.bytesPerLine());
     cv::Mat result;
     cvtColor(tmp, result,CV_BGR2RGB);
     return result;
}

int main(int argc, char* argv[]){
    QApplication a(argc,argv);
    SonarPlot s;
    //SonarWidget s;

    osg::ref_ptr<osg::Image> osg_image;

    /* generate a random shader image */
    uint width = BIN_COUNT*5.12;  // 5.12 pixels are needed for each bin;
    base::Angle beam_width = base::Angle::fromDeg(BEAM_WIDTH);
    base::Angle beam_height = base::Angle::fromDeg(BEAM_HEIGHT);
    uint height = width * tan(beam_height.rad * 0.5) / tan(beam_width.rad * 0.5);

    float range = RANGE;
    NormalDepthMap normal_depth_map(range, beam_width.getDeg(), beam_height.getDeg());
    ImageViewerCaptureTool capture = ImageViewerCaptureTool(beam_height.getRad(), beam_width.getRad(), width, false);
    capture.setBackgroundColor(osg::Vec4d(0.0, 0.0, 0.0, 1.0));

    // add oilrig
    osg::ref_ptr<osg::Group> root = new osg::Group();
    addScene(root);
    normal_depth_map.addNodeChild(root);

    // init ros node and subscribe to pose topic
    ros::init(argc, argv, "gpu_sonar_simulation");
    ros::NodeHandle n;
    ros::Subscriber pose_sub = n.subscribe("/ground_truth_to_tf_rexrov/pose", 10, poseCallback);

    // node to publish sonar topic
    ros::init(argc, argv, "sonar");
    ros::NodeHandle nh;
    image_transport::ImageTransport it(nh);
    image_transport::Publisher pub_normal_depth_map = it.advertise("/sonar_sensor/normal_depth_map", 1);
    image_transport::Publisher pub_sonar = it.advertise("/sonar_sensor/sonar_image", 1);

    int count = 0;
    while(true){
        // set up for camera's pov
        osg::Matrixd cameraMatrix, cameraRotation, cameraTrans;

        cameraRotation.setRotate(osg::Quat(-sonar_pose.pose.orientation.x, -sonar_pose.pose.orientation.y, -sonar_pose.pose.orientation.z, sonar_pose.pose.orientation.w));
        // openscenegraph has a -90 degrees offset heading compared to Gazebo
        cameraRotation = cameraRotation * osg::Matrixd::rotate(osg::DegreesToRadians(90.0), osg::Vec3(0,0,1));

        cameraTrans.makeTranslate(osg::Vec3(-sonar_pose.pose.position.x, -sonar_pose.pose.position.y, -sonar_pose.pose.position.z));

        cameraMatrix = cameraTrans * cameraRotation;
        capture.setViewMatrix(cameraMatrix*osg::Matrix::rotate(-M_PI/2.0, 1,0,0));

        // grab capture
        osg::ref_ptr<osg::Image> osgImage = capture.grabImage(normal_depth_map.getNormalDepthMapNode());
        cv::Mat cv_image;
        convertOSG2CV(osgImage, cv_image);
        
        // receives shader image in opencv format
    	cv::Mat cv_depth;
        osg::ref_ptr<osg::Image> osg_depth = capture.getDepthBuffer();
        gpu_sonar_simulation::convertOSG2CV(osg_depth, cv_depth);

        // replace depth matrix
        std::vector<cv::Mat> channels;
        cv::split(cv_image, channels);
        channels[1] = cv_depth;
        cv::merge(channels, cv_image);

        // image flip in y axis is needed to fix mirrored problem
        cv::flip(cv_image, cv_image, 0);
        //cv::imshow("Normal Depth Map", cv_image);

        // converting cv_image (normal_depth_map) to publish in ros
        cv::Mat image_ndm;
        cv_image.convertTo(image_ndm, CV_8UC3, 255);
        sensor_msgs::ImagePtr msg_ndm = cv_bridge::CvImage(std_msgs::Header(),"bgr8", image_ndm).toImageMsg();

        char k = cv::waitKey(1);

        switch(k) {
            case 27:		//	ESC
                exit(0);
                break;
        }

//        std::cout << "--- camera params ---" << std::endl;
//        std::cout << "position: " << -sonar_pose.pose.position.x << "," << -sonar_pose.pose.position.y << "," << -sonar_pose.pose.position.z << std::endl;
//        std::cout << "orientation: " << sonar_pose.pose.orientation.x << ", " << sonar_pose.pose.orientation.y << ", " << -sonar_pose.pose.orientation.z << ", " << sonar_pose.pose.orientation.w << std::endl;
//        std::cout << "time: " << ros::Time::now().toSec() << std::endl;

        /* initialize Sonar Simulation */
        uint32_t bin_count = BIN_COUNT;
        uint32_t beam_count = BEAM_COUNT;
        Sonar sonar_sim(bin_count, beam_count, beam_width, beam_height);

        /* simulate sonar image */
        std::vector<float> bins;
        sonar_sim.decodeShader(cv_image, bins);

        /* apply additional gain */
        float gain = GAIN;
        sonar_sim.applyAdditionalGain(bins, gain);

        /* encapsulate in rock's sonar structure */
        //float range = 22.0;
        base::samples::Sonar sonar = sonar_sim.simulateSonar(bins, range);

        s.show();
        s.setData(sonar);

        //set sonar image to publish in ros
        cv::Mat image_sonar = QImage2Mat(s.getImg());
        sensor_msgs::ImagePtr msg_sonar = cv_bridge::CvImage(std_msgs::Header(),"bgr8", image_sonar).toImageMsg();

        // saving images
        count++;
        if(count>=11000) break;
        double heading = tf::getYaw(sonar_pose.pose.orientation);
        //std::string image_name = "/home/longaray/sonar_simulator_images/" + std::to_string(ros::Time::now().toSec()) + "_"
          //                                                                + std::to_string(sonar_pose.pose.position.x) + "_"
            //                                                              + std::to_string(sonar_pose.pose.position.y) + "_"
              //                                                            + std::to_string(heading) + ".png";
        //cv::imwrite(image_name, image_sonar);

        //Publish sonar
        pub_sonar.publish(msg_sonar);
        pub_normal_depth_map.publish(msg_ndm);

        a.processEvents();

        ros::spinOnce();
    }
}
