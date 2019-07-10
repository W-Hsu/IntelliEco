//
//  main.cpp
//  DrawContour
//
//  Created by william on 2019/7/6.
//  Copyright © 2019 W-Hsu. All rights reserved.
//
/*----------------------------------------------*/
//WARNING : USED GLOBAL VARIABLE NAMES (On line 22) :
//SourceImg, LineImg, _SourceImg, _LineImg.

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream> //our best loved header
#include <vector> //No matter
#include <cmath> //abs()

using namespace cv;
using namespace std;

const int LinePerEdge = 2;

//Not defining global variables

void testImage(const string WindowName, Mat TestingImage, bool DestroyOrNot = true)
{
    //Show an image on window "winname", then destroy the window when pressing a key.
    namedWindow(WindowName); //Create an window
    imshow(WindowName, TestingImage); //Show the image on the window
    waitKey(); //Waiting for pressing a key
    
    if (DestroyOrNot)
        destroyWindow(WindowName); //Destroy the window
}

void FindBiggest(Mat& src, Mat& dst) //dst stores the countour lines
{
    Mat dst_middle;
    
    //Edge detection using Canny:
    Canny( src, dst_middle, 100,150,5);
    
    //Dilate
    //Much time to DE this BUG
    //the contour lines 
    Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
    dilate(dst_middle, dst_middle, element);
    
    //testImage("dst_middle", dst_middle);
    
    //Get the edge of the "white" board:
    vector<vector<Point> > contours; //Claim the container (2 lines)
    vector<Vec4i> hierarchy;
    
    findContours(dst_middle, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);    //Get the edges
    
    //testImage("dst_middle", dst_middle);
    
    //Find the biggest area then output it into dst:
    vector<vector<Point> > polyContours(contours.size());
    int maxArea = 0;
    for (int index = 0; index < contours.size(); index++){
        if (contourArea(contours[index]) > contourArea(contours[maxArea]))
            maxArea = index;
        approxPolyDP(contours[index], polyContours[index], 10, true);
        cout << contourArea(contours[index]) << endl;
    }
    
    cout << endl << contourArea(contours[maxArea]) << endl;
    
    dst = Mat::zeros(src.size(), CV_8UC3);
    drawContours(dst, polyContours, maxArea, Scalar(0,0,255/*rand() & 255, rand() & 255, rand() & 255*/), 2);
    
    //testImage("Test0", dst);
}

void DetectAndDrawLines(Mat& LineImage, Mat& DstImage)
{
    //To Detect lines:
    vector<Vec2f> lines; // will hold the results of the 1st-time detection
    HoughLines(LineImage, lines, 1, CV_PI/180, 150,0 ,0); // runs the actual detection
    //dst1: Source image; lines: container of line's parameter(rho,theta); 1: precision of rho; CV/PI/180: precision of theta(rad).
    
    //To draw lines:
    
    /*
     //Collaberate with "line too close" section, definition job.
     //Create the container for "too close" lines
     float memrho = lines[0][0], memtheta = lines[0][1];
     const double THRESHOLD_RHO = 0, THRESHOLD_THETA = 1 * CV_PI/180;
     */
    int upCount=0, downCount=0, leftCount=0, rightCount=0;
    cout << LineImage.cols << " " << LineImage.rows << "\n" << endl;
    //Go through all the lines
    for( size_t i = 0; i < lines.size(); i++ )
    {
        float rho = lines[i][0], theta = lines[i][1];
        /*
         //See if the line is "too close". If so, omit it. If not, remember it.
         if (abs(memrho - rho)<THRESHOLD_RHO || abs(memtheta - theta)<THRESHOLD_THETA)
         continue;
         else
         {
         memrho = rho;
         memtheta = theta;
         }
         */
        
        //"const int LinePerEdge" lines each edge:
        if (rho < 0)
        {
            if (rightCount >= LinePerEdge)
                continue;
            else
            {
                rightCount++;
                cout << "right" << endl;
            }
            
        }
        else if (rho >= 500)
        {
            if (downCount >= LinePerEdge)
                continue;
            else
            {
                downCount++;
                cout << "down" << endl;
            }
        }
        else
        {
            if (theta >= 0.70)
            {
                if (upCount >= LinePerEdge)
                    continue;
                else
                {
                    upCount++;
                    cout << "up" << endl;
                }
            }
            else
            {
                if (leftCount >= LinePerEdge)
                    continue;
                else
                {
                    leftCount++;
                    cout << "left" << endl;
                }
            }
            
        }
        
        //END MARK 1
        Point pt1, pt2;
        //Calculate the line's k,b(y=kx+b);m,n(x=my+n)
        
        if (theta < 0.01 && theta > -0.01)
        {
            //double k = -1 * cos(theta)/sin(theta), b = rho/sin(theta);
            double n = -1 * rho/sin(theta) / tan(theta);
            pt1.x = n;
            pt2.x = n;
            pt1.y = 20;
            pt2.y = LineImage.rows - 20;
        }
        else
        {
            double k = -1 * cos(theta)/sin(theta), b = rho/sin(theta);
            //double m = -1 * tan(theta), n = -1 * rho/sin(theta) / tan(theta);
            
            for (int x0=0 ; x0<LineImage.cols ; x0++)
            {
                double y0 = k * x0 + b;
                if (y0 < LineImage.rows && y0 > 0)
                {
                    pt1.x = x0 + 40 * sin(theta);
                    if (k>0)
                        pt1.y = y0 + 40 * abs(cos(theta));
                    else
                        pt1.y = y0 - 40 * abs(cos(theta));
                    break;
                }
            }

            for (int x0 = pt1.x + 1 ; x0<LineImage.cols ; x0++)
            {
                double y0 = k * x0 + b;
                pt2.x = x0 - 40 * sin(theta);
                if (k>0)
                    pt2.y = y0 - 40 * abs(cos(theta));
                else
                    pt2.y = y0 + 40 * abs(cos(theta));
                if (y0 > LineImage.rows || y0 < 0)
                    break;
            }
        }
        cout << "rho " << rho << " " << "theta " << theta << endl;
        cout << "pt1 " << pt1.x << " " << pt1.y << endl << "pt2 " << pt2.x << " " << pt2.y << endl;
        //cout << rho << " " << theta << endl; //Find some GUIlv
        line(DstImage, pt1, pt2, 255, 5, LINE_8, 0);
    }
    
    //return i_GiveOut;
}

void GetCountour(Mat& InputImage)
{
    //Define variables
    Mat SourceImg, LineImg;
    Mat& _SourceImg = SourceImg;
    Mat& _LineImg = LineImg;
    
    //Load an image:
    SourceImg = InputImage;
    
    //cvtColor(SourceImg, SourceImg, CV_BGR2GRAY);
    
    FindBiggest(_SourceImg, _LineImg);
    testImage("Test1", _LineImg);
    //IMPORTANT!! Must convert Polypic to GRAYSCALE to (draw lines by using Houghlines)
    //Takes me much time to DE this BUG
    cvtColor(LineImg, LineImg, CV_BGR2GRAY);
    
    return DetectAndDrawLines(_LineImg, InputImage);
    //testImage("Test2", InputImage);
}

