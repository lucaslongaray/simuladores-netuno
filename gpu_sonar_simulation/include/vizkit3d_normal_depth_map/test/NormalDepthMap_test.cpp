/*
 * NormalDepthMap_test.cpp
 *
 *  Created on: Mar 27, 2015
 *      Author: tiagotrocoli
 */

#include <iostream>
#include <src/ImageViewerCaptureTool.hpp>
#include <src/NormalDepthMap.hpp>

#include <osg/Geode>
#include <osg/Group>
#include <osg/ShapeDrawable>
#include <osgDB/ReadFile>

#include <opencv2/contrib/contrib.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define BOOST_TEST_MODULE "NormalDepthMap_test"
#include <boost/test/unit_test.hpp>

using namespace normal_depth_map;

BOOST_AUTO_TEST_SUITE(test_NormalDepthMap)

void plotSonarTest(cv::Mat3f image, double maxRange, double maxAngleX, cv::Mat1f cv_depth) {

  cv::Mat3b imagePlotMap = cv::Mat3b::zeros(1000, 1000);
  cv::Mat1b imagePlot = cv::Mat1b::zeros(1000, 1000);
  cv::Point2f centerImage(image.cols / 2, image.rows / 2);
  cv::Point2f centerPlot(imagePlot.cols / 2, 0);
  double factor = 1000/maxRange;
  double pointSize = factor / 3;
  cv::Point2f halfSize(pointSize / 2, pointSize / 2);

  double slope = 2 * maxAngleX * (1.0 / (image.cols - 1));
  double constant = - maxAngleX;

  for (int j = 0; j < image.rows; ++j)
    for (int i = 0; i < image.cols; ++i) {
      // double distance = image[j][i][1] * maxRange;
      double distance = cv_depth[j][i] * maxRange;
      double alpha = slope * i + constant;

      cv::Point2f tempPoint(distance * sin(alpha), distance * cos(alpha));
      tempPoint = tempPoint * factor;
      tempPoint += centerPlot;
      // cv::circle(imagePlot, tempPoint, pointSize, cv::Scalar(255 *
      //  image[j][i][0]), -1);
      // cv::rectangle(imagePlot, tempPoint + halfSize, tempPoint
      //  - halfSize, cv::Scalar(255 * image[j][i][0]), -1);
      imagePlot[(uint) tempPoint.y][(uint) tempPoint.x] = 255 * image[j][i][0];
    }

  cv::Mat plotProcess;
  cv::applyColorMap(imagePlot, plotProcess, cv::COLORMAP_HOT);
  cv::applyColorMap(imagePlot, imagePlotMap, cv::COLORMAP_HOT);

  cv::line(imagePlotMap, centerPlot,
    cv::Point2f(maxRange * sin(maxAngleX) * factor,
                maxRange * cos(maxAngleX) * factor) +  centerPlot,
    cv::Scalar(255), 1, CV_AA);

  cv::line(imagePlotMap, centerPlot,
    cv::Point2f(maxRange * sin(-maxAngleX) * factor,
                maxRange * cos(maxAngleX) * factor) + centerPlot,
    cv::Scalar(255), 1, CV_AA);

  cv::imshow("image Plot", imagePlotMap);
  cv::imshow("image IN", image);
  cv::waitKey();
}

// draw the scene with a small ball in the center with a big cube, cylinder and
// cone in back
void makeSimpleScene1(osg::ref_ptr<osg::Group> root) {

  osg::Geode *sphere = new osg::Geode();
  sphere->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(), 1)));
  root->addChild(sphere);

  osg::Geode *cylinder = new osg::Geode();
  cylinder->addDrawable(
    new osg::ShapeDrawable(new osg::Cylinder(osg::Vec3(30, 0, 10), 10, 10)));
  root->addChild(cylinder);

  osg::Geode *cone = new osg::Geode();
  cylinder->addDrawable(
    new osg::ShapeDrawable(new osg::Cone(osg::Vec3(0, 30, 0), 10, 10)));
  root->addChild(cone);

  osg::Geode *box = new osg::Geode();
  cylinder->addDrawable(
    new osg::ShapeDrawable(new osg::Box(osg::Vec3(0, -30, -10), 10)));
  root->addChild(box);
}

void viewPointsFromScene1(std::vector<osg::Vec3d> *eyes,
                          std::vector<osg::Vec3d> *centers,
                          std::vector<osg::Vec3d> *ups) {

  // view1 - near from the ball with the cylinder in back
  eyes->push_back(osg::Vec3d(-8.77105, -4.20531, -3.24954));
  centers->push_back(osg::Vec3d(-7.84659, -4.02528, -2.91345));
  ups->push_back(osg::Vec3d(-0.123867, -0.691871, 0.711317));

  // view2 - near from the ball with the cube in back
  eyes->push_back(osg::Vec3d(3.38523, 10.093, 1.12854));
  centers->push_back(osg::Vec3d(3.22816, 9.12808, 0.918259));
  ups->push_back(osg::Vec3d(-0.177264, -0.181915, 0.967204));

  // view3 - near the cone in up side
  eyes->push_back(osg::Vec3d(-10.6743, 38.3461, 26.2601));
  centers->push_back(osg::Vec3d(-10.3734, 38.086, 25.3426));
  ups->push_back(osg::Vec3d(0.370619, -0.854575, 0.36379));

  // view4 - Faced the cube plane
  eyes->push_back(osg::Vec3d(0.0176255, -56.5841, -10.0666));
  centers->push_back(osg::Vec3d(0.0176255, -55.5841, -10.0666));
  ups->push_back(osg::Vec3d(0, 0, 1));
}

// reference points, and map values for each view in viewPointsFromScene1
void referencePointsFromScene1(
        std::vector<std::vector<cv::Point> > *setPoints,
        std::vector<std::vector<cv::Point3i> > *setValues,
        std::vector<std::vector<int> > *gt_depth_buffer) {

  std::vector<cv::Point> points;
  // image points in view1
  points.push_back(cv::Point(74, 417));
  points.push_back(cv::Point(60, 320));
  points.push_back(cv::Point(267, 130));
  points.push_back(cv::Point(366, 226));
  points.push_back(cv::Point(361, 240));
  points.push_back(cv::Point(424, 314));
  setPoints->push_back(points);
  points.clear();

  // image points in view2
  points.push_back(cv::Point(80, 80));
  points.push_back(cv::Point(130, 475));
  points.push_back(cv::Point(390, 128));
  points.push_back(cv::Point(391, 210));
  points.push_back(cv::Point(280, 187));
  setPoints->push_back(points);
  points.clear();

  // image points in view3
  points.push_back(cv::Point(142, 77));
  points.push_back(cv::Point(254, 309));
  points.push_back(cv::Point(434, 65));
  points.push_back(cv::Point(123, 26));
  points.push_back(cv::Point(200, 100));
  setPoints->push_back(points);
  points.clear();

  // image points in view3
  points.push_back(cv::Point(75, 64));
  points.push_back(cv::Point(250, 251));
  points.push_back(cv::Point(410, 459));
  points.push_back(cv::Point(15, 485));
  points.push_back(cv::Point(461, 36));
  setPoints->push_back(points);

  std::vector<cv::Point3i> values;

  // pixel value from each point in image from view1
  values.push_back(cv::Point3i(992, 184, 0));
  values.push_back(cv::Point3i(270, 200, 0));
  values.push_back(cv::Point3i(905, 639, 0));
  values.push_back(cv::Point3i(952, 603, 0));
  values.push_back(cv::Point3i(266, 615, 0));
  values.push_back(cv::Point3i(168, 980, 0));
  setValues->push_back(values);
  values.clear();

  // pixel value from each point in image from view2
  values.push_back(cv::Point3i(0, 0, 0));
  values.push_back(cv::Point3i(909, 768, 0));
  values.push_back(cv::Point3i(1000, 192, 0));
  values.push_back(cv::Point3i(431, 203, 0));
  values.push_back(cv::Point3i(149, 819, 0));
  setValues->push_back(values);
  values.clear();

  // pixel value from each point in image from view3
  values.push_back(cv::Point3i(898, 462, 0));
  values.push_back(cv::Point3i(823, 505, 0));
  values.push_back(cv::Point3i(200, 662, 0));
  values.push_back(cv::Point3i(58, 682, 0));
  values.push_back(cv::Point3i(686, 474, 0));
  setValues->push_back(values);
  values.clear();

  // pixel value from each point in image from view4
  values.push_back(cv::Point3i(964, 447, 0));
  values.push_back(cv::Point3i(1000, 431, 0));
  values.push_back(cv::Point3i(960, 447, 0));
  values.push_back(cv::Point3i(0, 0, 0));
  values.push_back(cv::Point3i(952, 454, 0));
  setValues->push_back(values);
  values.clear();


  std::vector<int> temp_depth;
  // pixel value from each point in depth buffer from view1
  temp_depth.push_back(185);
  temp_depth.push_back(198);
  temp_depth.push_back(637);
  temp_depth.push_back(604);
  temp_depth.push_back(616);
  temp_depth.push_back(981);
  gt_depth_buffer->push_back(temp_depth);
  temp_depth.clear();

  // pixel value from each point in depth buffer from view2
  temp_depth.push_back(0);
  temp_depth.push_back(769);
  temp_depth.push_back(194);
  temp_depth.push_back(204);
  temp_depth.push_back(818);
  gt_depth_buffer->push_back(temp_depth);
  temp_depth.clear();

  // pixel value from each point in depth buffer from view3
  temp_depth.push_back(463);
  temp_depth.push_back(504);
  temp_depth.push_back(662);
  temp_depth.push_back(683);
  temp_depth.push_back(475);
  gt_depth_buffer->push_back(temp_depth);
  temp_depth.clear();

  // pixel value from each point in depth buffer from view4
  temp_depth.push_back(447);
  temp_depth.push_back(431);
  temp_depth.push_back(448);
  temp_depth.push_back(0);
  temp_depth.push_back(453);
  gt_depth_buffer->push_back(temp_depth);
  temp_depth.clear();
}

BOOST_AUTO_TEST_CASE(applyShaderNormalDepthMap_TestCase) {

std::vector<osg::Vec3d> eyes, centers, ups;
std::vector<std::vector<cv::Point> > setPoints;
std::vector<std::vector<cv::Point3i> > setValues;
std::vector<std::vector<int> > gt_depth_buffer;
referencePointsFromScene1(&setPoints, &setValues, &gt_depth_buffer);

float maxRange = 50;
float maxAngleX = M_PI * 1.0 / 6; // 30 degrees
float maxAngleY = M_PI * 1.0 / 6; // 30 degrees

uint height = 500;
NormalDepthMap normalDepthMap(maxRange, maxAngleX * 0.5, maxAngleY * 0.5);
ImageViewerCaptureTool capture(maxAngleY, maxAngleX, height);
capture.setBackgroundColor(osg::Vec4d(0, 0, 0, 0));

osg::ref_ptr<osg::Group> root = new osg::Group();
viewPointsFromScene1(&eyes, &centers, &ups);
makeSimpleScene1(root);
normalDepthMap.addNodeChild(root);

uint precision = 1000;

  for (uint i = 0; i < eyes.size(); ++i) {
    capture.setCameraPosition(eyes[i], centers[i], ups[i]);

    normalDepthMap.setDrawNormal(true);
    normalDepthMap.setDrawDepth(true);
    osg::ref_ptr<osg::Image> osgImage =
      capture.grabImage(normalDepthMap.getNormalDepthMapNode());
    cv::Mat3f cvImage(osgImage->t(), osgImage->s());
    cvImage.data = osgImage->data();
    cvImage = cvImage.clone();
    cv::cvtColor(cvImage, cvImage, cv::COLOR_RGB2BGR, CV_32FC3);
    cv::flip(cvImage, cvImage, 0);

    //get only normal map
    normalDepthMap.setDrawDepth(false);
    osg::ref_ptr<osg::Image> osgImageNormalMap =
      capture.grabImage(normalDepthMap.getNormalDepthMapNode());
    cv::Mat3f cvImageNormalMap(osgImage->t(), osgImage->s());
    cvImageNormalMap.data = osgImageNormalMap->data();
    cvImageNormalMap = cvImageNormalMap.clone();
    cv::cvtColor(cvImageNormalMap, cvImageNormalMap, cv::COLOR_RGB2BGR,
                 CV_32FC3);
    cv::flip(cvImageNormalMap, cvImageNormalMap, 0);

    //get only half range depth map;
    normalDepthMap.setDrawDepth(true);
    normalDepthMap.setDrawNormal(false);
    osg::ref_ptr<osg::Image> osgImageDepthMap =
      capture.grabImage(normalDepthMap.getNormalDepthMapNode());
    cv::Mat3f cvImageDepthMap(osgImage->t(), osgImage->s());
    cvImageDepthMap.data = osgImageDepthMap->data();
    cv::cvtColor(cvImageDepthMap, cvImageDepthMap, cv::COLOR_RGB2BGR, CV_32FC3);
    cv::flip(cvImageDepthMap, cvImageDepthMap, 0);

    // get linear depth with high resolution
    osg::ref_ptr<osg::Image> osg_depth =  capture.getDepthBuffer();
    cv::Mat1f cv_depth(osg_depth->t(), osg_depth->s());
    cv_depth.data = osg_depth->data();
    cv_depth = cv_depth.clone();
    cv::flip(cv_depth, cv_depth, 0);
    cv_depth = cv_depth.mul( cv::Mat1f(cv_depth < 1)/255);
    cv::imshow("DEPTH BUFFER", cv_depth);


    //start check process
    plotSonarTest(cvImage, maxRange, maxAngleX * 0.5, cv_depth);

    for (uint j = 0; j < setPoints[i].size(); ++j) {
      cv::Point p = setPoints[i][j];

      cv::Point3i imgValue(cvImage[p.y][p.x][0] * precision,
                           cvImage[p.y][p.x][1] * precision,
                           cvImage[p.y][p.x][2] * precision);

      cv::Point3i imgValueNormalMap(cvImageNormalMap[p.y][p.x][0] * precision,
                                    cvImageNormalMap[p.y][p.x][1] * precision,
                                    cvImageNormalMap[p.y][p.x][2] * precision);

      cv::Point3i imgValueDepthMap(cvImageDepthMap[p.y][p.x][0] * precision,
                                   cvImageDepthMap[p.y][p.x][1] * precision,
                                   cvImageDepthMap[p.y][p.x][2] * precision);


      BOOST_CHECK_EQUAL(imgValue, setValues[i][j]);
      BOOST_CHECK_EQUAL(imgValueNormalMap,
                        cv::Point3i(setValues[i][j].x, 0, 0));
      BOOST_CHECK_EQUAL(imgValueDepthMap,
                        cv::Point3i(0, setValues[i][j].y, setValues[i][j].z));
      BOOST_CHECK_EQUAL( (int) (cv_depth[p.y][p.x] * precision),
                         gt_depth_buffer[i][j]);

    }
  }
}

void checkDepthValueRadialVariation(cv::Mat3f image, uint id) {

  static const int groudTruth0[] = {
    4941, 4941, 4901, 4901, 4901, 4901, 4901, 4901, 4862, 4862, 4862,
    4862, 4862, 4862, 4862, 4901, 4901, 4901, 4901, 4901, 4901, 4941,
    4941 };

  static const int groudTruth1[] = {
    9725, 9019, 8313, 7647, 7019, 6431, 5921, 5490, 5137, 4941, 4862,
    4941, 5176, 5490, 5921, 6470, 7058, 7686, 8352, 9058};

  static const int groudTruth2[] = {
    0, 0, 0, 0, 0, 0, 8745, 7333, 6078, 5215, 4862, 5215, 6078, 7333,
    8745, 0, 0, 0, 0, 0, 0};

  std::vector<std::vector<int> > groundTruthVector(3);
  groundTruthVector[0] = std::vector<int>(
    groudTruth0, groudTruth0 + sizeof(groudTruth0) / sizeof(int));
  groundTruthVector[1] = std::vector<int>(
    groudTruth1, groudTruth1 + sizeof(groudTruth1) / sizeof(int));
  groundTruthVector[2] = std::vector<int>(
    groudTruth2, groudTruth2 + sizeof(groudTruth2) / sizeof(int));

  cv::Point centerPoint(image.size().width / 2, image.size().height / 2);
  std::vector<int> histSizes;
  for (int i = 0; i < image.size().width; i = i + image.size().width * 0.05)
    histSizes.push_back((uint)(image[centerPoint.y][i][1] * 10000));

  BOOST_CHECK_EQUAL_COLLECTIONS(histSizes.begin(),
                                histSizes.end(),
                                groundTruthVector[id].begin(),
                                groundTruthVector[id].end());
}

BOOST_AUTO_TEST_CASE(depthValueRadialVariation_testCase) {

  osg::ref_ptr<osg::Geode> scene = new osg::Geode();
  osg::ref_ptr<osg::Shape> box;
  uint numberSphere = 100;
  double multi = 2;
  double boxSize = 5;
  double distance = 100;
  double maxRange = 200;

  for (uint i = 0; i < numberSphere; ++i) {
    box = new osg::Box(osg::Vec3(i * multi, 0, -distance), boxSize);
    scene->addDrawable(new osg::ShapeDrawable(box));
    box = new osg::Box(osg::Vec3(i * -multi, 0, -distance), boxSize);
    scene->addDrawable(new osg::ShapeDrawable(box));
    box = new osg::Box(osg::Vec3(0, i * -multi, -distance), boxSize);
    scene->addDrawable(new osg::ShapeDrawable(box));
    box = new osg::Box(osg::Vec3(0, i * multi, -distance), boxSize);
    scene->addDrawable(new osg::ShapeDrawable(box));
  }

  NormalDepthMap normalDepthMap(maxRange, M_PI / 6, M_PI / 6);
  normalDepthMap.setDrawNormal(false);
  normalDepthMap.addNodeChild(scene);

  uint sizeVector = 3;
  double fovys[] = {150, 120, 20};
  double fovxs[] = {20, 120, 150};
  uint heightSize[] = {500, 500, 500};

  for (uint j = 0; j < sizeVector; ++j) {
    ImageViewerCaptureTool capture(fovys[j] * M_PI / 180.0,
                                   fovxs[j] * M_PI / 180.0, heightSize[j]);
    capture.setBackgroundColor(osg::Vec4d(0, 0, 0, 0));
    osg::ref_ptr<osg::Image> osgImage =
      capture.grabImage(normalDepthMap.getNormalDepthMapNode());

    cv::Mat3f cvImage(osgImage->t(), osgImage->s());
    cvImage.data = osgImage->data();
    cvImage = cvImage.clone();
    cv::cvtColor(cvImage, cvImage, cv::COLOR_RGB2BGR, CV_32FC3);
    cv::flip(cvImage, cvImage, 0);
    checkDepthValueRadialVariation(cvImage, j);
  }
}

BOOST_AUTO_TEST_SUITE_END();
