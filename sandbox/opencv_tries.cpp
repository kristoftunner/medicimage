#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include "core/log.h"

int main()
{
  medicimage::Logger::Init();

  auto selectRect = cv::Rect2f(cv::Point2f{100, 100}, cv::Point2f{300, 300});
  auto boundingRectangle1 = cv::Rect2f(cv::Point2f{200, 200}, cv::Point2f{250, 250});
  auto boundingRectangle2 = cv::Rect2f(cv::Point2f{50, 50}, cv::Point2f{250, 250});
  auto boundingRectangle3 = cv::Rect2f(cv::Point2f{25, 25}, cv::Point2f{75, 75});

  auto createContourFromRect = [](cv::Rect2f rect) -> std::vector<cv::Point2f>
  {
    std::vector<cv::Point2f> contour{rect.tl(), rect.tl() + cv::Point2f{rect.width, 0}, rect.tl() + cv::Point2f(rect.size()), rect.tl() + cv::Point2f{0, rect.height}, rect.tl()};
    return contour;
  };

  auto selectBoxContour = createContourFromRect(selectRect);
  auto boundingBoxContour1 = createContourFromRect(boundingRectangle1);
  auto boundingBoxContour2 = createContourFromRect(boundingRectangle2);
  auto boundingBoxContour3 = createContourFromRect(boundingRectangle3);
  //std::vector<cv::Point2f> selectBoxContour{selectRect.tl(), selectRect.tl() + cv::Point2f{selectRect.width, 0}, selectRect.tl() + cv::Point2f(selectRect.size()), selectRect.tl() + cv::Point2f{0, selectRect.height}, selectRect.tl()};
  //std::vector<cv::Point2f> boundingBoxContour{boundingRectangle1.tl(), boundingRectangle1.tl() + cv::Point2f{boundingRectangle1.width, 0},
  //  boundingRectangle1.tl() + cv::Point2f(boundingRectangle1.size()), boundingRectangle1.tl() + cv::Point2f{0, boundingRectangle1.height}, boundingRectangle1.tl()};

  cv::Mat image(600, 800, CV_8UC3, cv::Scalar(255, 255, 255));
  cv::rectangle(image, selectRect, cv::Scalar{255,0,0}, 3);
  cv::rectangle(image, boundingRectangle1, cv::Scalar{0,255,0}, 3);
  cv::rectangle(image, boundingRectangle2, cv::Scalar{0,255,0}, 3);
  cv::rectangle(image, boundingRectangle3, cv::Scalar{0,255,0}, 3);

  auto selectRectArea1 = selectRect.area();
  auto selectRectArea2 = cv::contourArea(selectBoxContour);
  auto rectInsideArea1 = boundingRectangle1.area();
  auto rectInsideArea2 = cv::contourArea(boundingBoxContour1);
  APP_CORE_INFO("Select rectangle are:{}, contour area:{}", selectRectArea1, selectRectArea2);
  APP_CORE_INFO("Rectangle inside are:{}, contour area:{}", rectInsideArea1, rectInsideArea2);
  
  std::vector<cv::Point2f> intersectionContour1;
  std::vector<cv::Point2f> intersectionContour2;
  std::vector<cv::Point2f> intersectionContour3;

  auto isContourNested = [](std::vector<cv::Point2f> selectContour, std::vector<cv::Point2f> boundingContour) -> bool
  {
    std::vector<cv::Point2f> points;
    if (cv::intersectConvexConvex(selectContour, boundingContour, points, true) > 0.0)
      return cv::intersectConvexConvex(selectContour, boundingContour, points, false) == 0.0;
    else
      return false;
  };

  auto isRect1Inside = isContourNested(selectBoxContour, boundingBoxContour1);
  auto isRect2Inside = isContourNested(selectBoxContour, boundingBoxContour2);
  auto isRect3Inside = isContourNested(selectBoxContour, boundingBoxContour3);
  cv::imshow("frame", image);
  cv::waitKey(0);
}
