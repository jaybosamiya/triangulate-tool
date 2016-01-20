#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Mat input_image, input_image_gray;
Mat output_image;

const char *window_name = "Triangulate Tool (c) Jay Bosamiya";
vector<Point> point_set;

int opacity = 75;

void run_filter(int, void*) {
  output_image = input_image.clone();

  Subdiv2D subdiv(Rect(0, 0, input_image.cols, input_image.rows));

  for ( int i = 0 ; i < point_set.size() ; i++ ) {
    subdiv.insert(point_set[i]);
  }

  output_image = input_image.clone();

  vector<Vec6f> triangle_list;
  subdiv.getTriangleList(triangle_list);
  for ( int i = 0 ; i < triangle_list.size() ; i++ ) {
    Vec6f &t = triangle_list[i];
    Point pt[3];
    pt[0] = Point(cvRound(t[0]), cvRound(t[1]));
    pt[1] = Point(cvRound(t[2]), cvRound(t[3]));
    pt[2] = Point(cvRound(t[4]), cvRound(t[5]));

    const Point* pts[1] = {pt};
    int npts[1] = {3};
    Mat mask(input_image.size(), CV_8UC1);
    mask = Scalar::all(0);
    fillPoly(mask, pts, npts, 1, Scalar(255));

    Scalar colour = mean(input_image, mask);
    fillPoly(output_image, pts, npts, 1, colour);
  }

  Mat display_image;
  addWeighted(input_image, (100-opacity)/100.0, output_image, opacity/100.0, 0, display_image);
  imshow(window_name, display_image);
}

void show_help(const char *progname) {
    cerr <<"Usage: " << progname << " input_image output_image" << endl;
}

void on_mouse(int event, int x, int y, int, void*) {
  if( event != EVENT_LBUTTONDOWN )
    return;

  point_set.push_back(Point(x, y));

  run_filter(0, 0);
}

int main( int argc, char** argv ) {
  if( argc < 3 ) {
    show_help(argv[0]);
    return -1;
  }

  input_image = imread(argv[1], CV_LOAD_IMAGE_COLOR);
  if ( ! input_image.data ) {
      show_help(argv[0]);
      cerr <<  "ERROR: Could not open or find the image " << argv[1] << endl;
      return -1;
  }

  cvtColor(input_image, input_image_gray, CV_BGR2GRAY);

  namedWindow(window_name, CV_WINDOW_AUTOSIZE);
  createTrackbar("Opacity of triangles: ", window_name, &opacity, 100, run_filter);
  setMouseCallback(window_name, on_mouse);

  output_image = input_image.clone();
  imshow(window_name, output_image);

  run_filter(0, NULL);

  bool saved_once_atleast = false;
  while (true) {
    char input = waitKey(0);
    if ( input == 's' ) {
      imwrite(argv[2],output_image);
      saved_once_atleast = true;
    } else if ( input == 'q' ) {
      break;
    }
  }

  if ( ! saved_once_atleast ) {
    imwrite(argv[2],output_image);
  }

  return 0;
}
