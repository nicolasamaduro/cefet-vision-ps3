#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace cv;
using namespace std;

vector<vector<Point> > contours;
vector<Vec4i> hierarchy;

Mat binImg;

int const MAX_VALUE = 255;
int MAX_DIAMETER=100;

float diameter(vector<Point> points){
    float diameter=0;
    for(int i=0; i<points.size();i++){
        for(int j=0; j<points.size();j++){
            if(diameter*diameter<pow(points[i].x-points[j].x,2)+pow(points[i].y-points[j].y,2)){
                diameter=sqrt(pow(points[i].x-points[j].x,2)+pow(points[i].y-points[j].y,2));
            }
        }
    }
    return diameter;
}

float centralMoment(Mat S,Point2d centroid,int a,int b){
    float u=0;
    for(int i =0;i<S.rows;i++){
        for(int j =0;j<S.cols;j++) {
            if(S.at<uchar>(i,j)==0){
                u+=pow(j-centroid.x,a)*pow(i-centroid.y,b);
            }
        }
    }
    return u;
}

Point2d calculateCentroid(Mat S){
    float m00=0,m01=0,m10=0;
    for(int i =0;i<S.rows;i++){
        for(int j =0;j<S.cols;j++) {
            if(S.at<uchar>(i,j)==0){
                m00++;
                m10+=j;
                m01+=i;
            }
        }

    }

    float xs = m10 / m00;
    float ys = m01 / m00;
    Point centroid((int) xs, (int) ys);
    return centroid;
}

float eccentricity(Mat S){
    Point2d centroidS=calculateCentroid(S);
    float u20=centralMoment(S,centroidS,2,0);
    float u02=centralMoment(S,centroidS,0,2);
    float u11=centralMoment(S,centroidS,1,1);
    float e=(pow(u20-u02,2)-4*pow(u11,2))/pow(u20+u02,2);
    return e;
}

void mainAxis(Mat S,Mat img){
    Point2d centroidS=calculateCentroid(S);
    float a=2.0*centralMoment(S,centroidS,1,1)/(centralMoment(S,centroidS,2,0)-centralMoment(S,centroidS,0,2));
    float b=centroidS.y-a*centroidS.x;
    Point pt1(centroidS.x-30,a*(centroidS.x-30)+b);
    Point pt2(centroidS.x+30,a*(centroidS.x+30)+b);

    line(img, pt1, pt2, Scalar(150), 1,8,0);
}



void onMouse(int event, int x, int y, int flags, void* param)
{
    if ( event == CV_EVENT_LBUTTONDOWN  )
    {
        bool flag=true;
        for (int k=0 ;k<contours.size()&&flag;k++) {
            Mat raw_dist(binImg.size(),CV_8UC1,255);

            double diamete =diameter(contours[k]);
            if(diamete>MAX_DIAMETER){
                if(pointPolygonTest( contours[k], Point2f(x,y), true)>=0) {
                    for( int j = 0; j < binImg.rows; j++ )
                    { for( int i = 0; i < binImg.cols; i++ )
                        {
                            if (pointPolygonTest( contours[k], Point2f(i,j), false)>=0)
                               raw_dist.at<uchar>(j, i) = 0;
                        }
                    }
                    flag=false;
                    Point centroid=calculateCentroid(raw_dist);
                    circle(binImg, centroid, 5,Scalar(150),-1, 8, 0);
                    mainAxis(raw_dist,binImg);
                    cout<<"Eccentricity: "<<eccentricity(raw_dist)<<endl;
                }
            }
        }
        imshow("Binary image",binImg);
    }
}

int main(int argc,char** argv){

    Mat img = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    if(!img.data)
        return -1;

    int thresholdValue = 250;
    int thresholdType = 0;
    blur(img, img, Size(3,3));
    threshold( img, binImg, thresholdValue, MAX_VALUE,thresholdType );

    Mat currentImg=binImg.clone();

    namedWindow("Binary image", CV_WINDOW_AUTOSIZE );
    imshow("Binary image", binImg);

    setMouseCallback( "Binary image", onMouse, 0 );
    Mat canny_output;
    Canny( currentImg, canny_output, 100, 200, 3 );
    findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    waitKey(0);
}