#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <ctime>
#include <cmath>


using namespace cv;
using namespace std;

int noise;
int quantity;
int lengths;
int densitie;

const int IMG_SIZE=255;

Mat img (IMG_SIZE,IMG_SIZE,CV_8UC1,Scalar(255));

bool drawLine(){
    bool entrou=false;
    img = Mat(IMG_SIZE,IMG_SIZE, CV_8UC1, cvScalar(255));
    srand(time(0)); // use current time as seed for random generator
    for (int i=0;i<img.rows;i++)
        for (int j=0;j<img.cols;j++) {
            int random_variable = rand()%100+1;
            if (random_variable<noise)
                img.at<uchar>(i,j)=0;
        }
    if(quantity!=0&&lengths!=0&&densitie!=0)
        for (int k=0;k<quantity;k++) {
            float d= rand() % (int)sqrt(2*IMG_SIZE*IMG_SIZE);
            float alpha=rand()%90+1;
            alpha*=M_PI/180;

            int t;
            for (int l = IMG_SIZE / 2 - lengths, t = rand() % IMG_SIZE; l < IMG_SIZE / 2 + lengths; l++, t++) {
                float x = t;
                float y=(d-x*cos(alpha))/sin(alpha);
                for (int d1 = ((int) x) - 5; d1 < ((int) x) + 5; d1++)
                    for (int d2 = ((int) y) - 5; d2 < ((int) y) + 5; d2++)
                        if ((rand() % 100 + 1) * 2 < densitie)
                            if (d1 < IMG_SIZE && d2 < IMG_SIZE && d1 > 0 && d1 > 0) {
                                img.at<uchar>((int) d1, (int) d2) = 0;
                                entrou = true;
                            }
            }
        }
    imshow("Image", img);
    return entrou;
}

void detectLines(Mat img){
    Mat colorImg;
    threshold(img,img,100,255,1);
    cvtColor(img, colorImg, CV_GRAY2BGR);

    vector<Vec4i> lines;
    HoughLinesP(img, lines, 1, CV_PI/180, 50, 50, 10 );
    for( size_t i = 0; i < lines.size(); i++ )
    {
        Vec4i l = lines[i];
        line( colorImg, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
    }
    imshow("Detected lines", colorImg);
}


void on_trackbar( int , void*)
{
    if(drawLine());
        detectLines(img);
}

int main(int argc,char** argv){
    namedWindow("Image", CV_WINDOW_AUTOSIZE );
    namedWindow("Detected lines", CV_WINDOW_AUTOSIZE );

    noise=0;
    lengths=0;
    densitie=0;
    quantity=0;

    createTrackbar("Noise: ", "Image", &noise, 100, on_trackbar);
    createTrackbar("Densities: ", "Image", &densitie, 100, on_trackbar);
    createTrackbar("Quantity: ", "Image", &quantity, 15, on_trackbar);
    createTrackbar("Lengths: ", "Image", &lengths, 200, on_trackbar);

    waitKey(0);
    return 0;
}