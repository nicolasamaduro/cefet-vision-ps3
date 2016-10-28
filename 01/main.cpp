#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

vector<vector<Point> > contours;
vector<Vec4i> hierarchy;

int const MAX_VALUE = 255;
enum Menus { menuInicial, menuOpcao1,menuOpcao2};
Menus menuAtual=menuInicial;

void putOptions(Mat img, string textOption1, float origScale) {
    int fontFace = FONT_HERSHEY_COMPLEX_SMALL;
    double fontScale = 0.7;
    int thickness = 1;
    int baseline = 0;
    Size textSize = getTextSize(textOption1, fontFace,
                                fontScale, thickness, &baseline);
    baseline += thickness;
    Point textOrig((img.cols - textSize.width) / 2,
                    (img.rows + img.rows / origScale + textSize.height) / 2);
    putText(img, textOption1, textOrig, fontFace, fontScale,
            Scalar::all(150), thickness, 8);
}

int coutBlackPixels(Mat img){
    int sum=0;
    for (int j =0;j<img.cols;j++)
        for (int i =0;i<img.rows;i++)
            if (img.at<uchar>(i,j)==0){
                sum++;
            }
    return sum;
}

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


void onMouse(int event, int x, int y, int flags, void* param)
{
    if ( event == CV_EVENT_LBUTTONDOWN  && menuAtual==menuOpcao2)
    {
        double a;
        for (int i=0 ;i<contours.size();i++) {
            a = pointPolygonTest(contours[i], Point2f(x, y), true);
            if (a>0) {
                double perimeter = arcLength(contours[i], true);
                double area = contourArea(contours[i], false);
                double diamete =diameter(contours[i]);
                cout<<"Perimeter: "<<perimeter<<endl;
                cout<<"Area: "<<area<<endl;
                cout<<"Diameter: "<<diamete<<"\n"<<endl;
                break;
            }
        }
    }
}

void writeMenu(Mat img){
    putOptions(img,"1. Counting components",2);
    putOptions(img,"2. Geometric features of a selected component",1.7);
}


int main(int argc,char** argv){
    Mat img = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    if(!img.data)
        return -1;

    Mat imgBin,imgBinInv;
    int thresholdValue = 240;
    int thresholdType = 0;
    blur(img, img, Size(3,3));

    threshold( img, imgBin, thresholdValue, MAX_VALUE,thresholdType );
    threshold( imgBin, imgBinInv, 40, MAX_VALUE,1 );

    Mat currentImg=imgBin.clone();

    namedWindow("Binary image", CV_WINDOW_AUTOSIZE );
    Mat imgMenu = currentImg.clone();
    writeMenu(imgMenu);
    imshow("Binary image", imgMenu);

    setMouseCallback( "Binary image", onMouse, 0 );
    char a='a';
    while(a!=27){
        a=waitKey(0);
        switch (menuAtual){
            case menuInicial: {
                imgMenu = currentImg.clone();
                writeMenu(imgMenu);
                imshow("Binary image", imgMenu);
                if (a == '1') {
                    menuAtual = menuOpcao1;
                    Mat imgOpcao1 = currentImg.clone();
                    putOptions(imgOpcao1, "1. White < Black", 2);
                    putOptions(imgOpcao1, "2. Black < White", 1.7);
                    imshow("Binary image", imgOpcao1);
                } else if (a == '2') {
                    menuAtual = menuOpcao2;
                    imshow("Binary image", currentImg);
                }
                break;
            }case menuOpcao1: {
                if (a == '1') {
                    cout << "Black pixels " << coutBlackPixels(imgBinInv) << endl;
                    imshow("Binary image", imgBinInv);
                    currentImg = imgBinInv.clone();
                } else if (a == '2') {
                    cout << "Black pixels " << coutBlackPixels(imgBin) << endl;
                    imshow("Binary image", imgBin);
                    currentImg = imgBin.clone();
                }
                menuAtual = menuInicial;
                break;
            }case menuOpcao2:{
                Mat canny_output;
                Canny( currentImg, canny_output, 100, 200, 3 );
                /// Find contours
                findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

            }
        }
    }
}