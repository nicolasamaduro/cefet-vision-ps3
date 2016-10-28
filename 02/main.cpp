#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cmath>
#include <vector>

using namespace cv;
using namespace std;

void produceSmoothedResidualToBoxFilter(Mat img,Mat R[],Mat S[],int n){
    blur(img, S[0], Size( 3, 3 ));
    R[0]=img-S[0];

    for (int i=1;i<n;i++){
        blur(S[i-1], S[i], Size( 3, 3 ));
        R[i]=img-S[i];
    }
}

void produceSmoothedResidualToMedianFilter(Mat img,Mat R[],Mat S[],int n){
    medianBlur(img, S[0],3);
    R[0]=img-S[0];

    for (int i=1;i<n;i++){
        medianBlur ( S[i-1],S[i], 3 );
        R[i]=img-S[i];
    }
}

Mat constructCoOccurrenceMatriceA4(Mat img,int Gmax){
    Mat coOccurrence(Gmax,Gmax,CV_32SC1,Scalar(0));
    for(int i = 0;i<img.rows;i++){
        for(int j=0;j<img.cols;j++){
            int x=img.at<uchar>(i,j);
            if (i+1<img.cols){//bottom
                int y=img.at<uchar>(i+1,j);
                coOccurrence.at<int>(y,x)++;
            }
            if (j+1<img.rows){//right
                int y=img.at<uchar>(i,j+1);
                coOccurrence.at<int>(y,x)++;
            }
            if (i>0){//top
                int y=img.at<uchar>(i-1,j);
                coOccurrence.at<int>(y,x)++;
            }
            if (j>0){//left
                int y=img.at<uchar>(i,j-1);
                coOccurrence.at<int>(y,x)++;
            }
        }
    }
    return coOccurrence;
}

int getSumCoOccurrenceMatrice(int rows,int adjacency){
    return rows*(rows-1)*adjacency;
}

float calculateHomogeneity(Mat img,int Gmax){
    Mat C=constructCoOccurrenceMatriceA4(img,Gmax);
    int n =getSumCoOccurrenceMatrice(img.rows,4);
    float hom=0;
    for(int u=0;u<Gmax;u++)
        for(int v=0;v<Gmax;v++) {
            float P = (float) C.at<int>(u, v) / (float) n;
            hom+=P/(1.0+(float)abs(u-v));
        }
    return hom;
}

float calculateUniformity(Mat img,int Gmax){
    Mat C=constructCoOccurrenceMatriceA4(img,Gmax);
    int n =getSumCoOccurrenceMatrice(img.rows,4);
    float h=0;
    float uni=0;
    for(int u=0;u<Gmax;u++)
        for(int v=0;v<Gmax;v++) {
            float P = (float) C.at<int>(u, v) / (float) n;
            uni+=P*P;
        }
    return uni;
}

Mat plotGraph(Mat values, int XRange[2])
{
    int w = 200;
    int h = 100;
    Mat graph(h, w, CV_8UC3, Scalar( 0,0,0) );
    int size = XRange[1]-XRange[0]+1;
    int bin_w = cvRound( (double) w/size );
    for( int i = XRange[0]+1; i < XRange[1]; i++ )
    {

        line( graph, Point( bin_w*(i-1), h - cvRound(values.at<float>(0,i-1)) ) ,
              Point( bin_w*(i), h - cvRound(values.at<float>(0,i)) ),
              Scalar( 255, 255, 0), 2, 8, 0  );

    }
    return graph;
}

int main(int argc,char** argv){
    int Gmax=255;
    Mat img = imread(argv[1],CV_LOAD_IMAGE_GRAYSCALE);
    if(!img.data)
        return -1;
    namedWindow("Image", CV_WINDOW_AUTOSIZE );
    imshow("Image",img);

    int n=31;
    Mat bfR[n],bfS[n];
    produceSmoothedResidualToBoxFilter(img,bfR,bfS,n);

    Mat mfR[n],mfS[n];
    produceSmoothedResidualToMedianFilter(img,mfR,mfS,n);

    Mat homBfR(1,n,CV_32F);
    Mat homBfS(1,n,CV_32F);

    Mat uniBfR(1,n,CV_32F);
    Mat uniBfS(1,n,CV_32F);

    Mat homMfR(1,n,CV_32F);
    Mat homMfS(1,n,CV_32F);

    Mat uniMfR(1,n,CV_32F);
    Mat uniMfS(1,n,CV_32F);

    for (int i=0;i<n;i++){
        homBfR.at<float>(0,i)=100*calculateHomogeneity(bfR[i],Gmax);
        homBfS.at<float>(0,i)=100*calculateHomogeneity(bfS[i],Gmax);

        uniBfR.at<float>(0,i)=100*calculateUniformity(bfR[i],Gmax);
        uniBfS.at<float>(0,i)=100*calculateUniformity(bfS[i],Gmax);

        homMfR.at<float>(0,i)=100*calculateHomogeneity(mfR[i],Gmax);
        homMfS.at<float>(0,i)=100*calculateHomogeneity(mfS[i],Gmax);

        uniMfR.at<float>(0,i)=100*calculateUniformity(mfR[i],Gmax);
        uniMfS.at<float>(0,i)=100*calculateUniformity(mfS[i],Gmax);
    }

    int Xrange[2]={0,n};

    Mat homGraphBfR=plotGraph(homBfR,Xrange);
    Mat homGraphBfS=plotGraph(homBfS,Xrange);

    Mat uniGraphBfR=plotGraph(uniBfR,Xrange);
    Mat uniGraphBfS=plotGraph(uniBfS,Xrange);

    Mat homGraphMfR=plotGraph(homMfR,Xrange);
    Mat homGraphMfS=plotGraph(homMfS,Xrange);

    Mat uniGraphMfR=plotGraph(uniMfR,Xrange);
    Mat uniGraphMfS=plotGraph(uniMfS,Xrange);

    namedWindow("Hom. Box Fil. R", CV_WINDOW_AUTOSIZE );
    imshow("Hom. Box Fil. R",homGraphBfR);

    namedWindow("Hom. Box Fil. S", CV_WINDOW_AUTOSIZE );
    namedWindow("Uni. Box Fil. R", CV_WINDOW_AUTOSIZE );
    namedWindow("Uni. Box Fil. S", CV_WINDOW_AUTOSIZE );
    namedWindow("Hom. Med. Fil. R", CV_WINDOW_AUTOSIZE );
    namedWindow("Hom. Med. Fil. S", CV_WINDOW_AUTOSIZE );
    namedWindow("Uni. Med. Fil. R", CV_WINDOW_AUTOSIZE );
    namedWindow("Uni. Med. Fil. S", CV_WINDOW_AUTOSIZE );

    imshow("Hom. Box Fil. S",homGraphBfS);
    imshow("Uni. Box Fil. R",uniGraphBfR);
    imshow("Uni. Box Fil. S",uniGraphBfS);
    imshow("Hom. Med. Fil. R",homGraphMfR);
    imshow("Hom. Med. Fil. S",homGraphMfS);
    imshow("Uni. Med. Fil. R",uniGraphMfR);
    imshow("Uni. Med. Fil. S",uniGraphMfS);


    imshow("Image",img);

    waitKey(0);
    return 0;
}