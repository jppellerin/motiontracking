#include "stdafx.h"
#include "CSI4133.h"
#include <queue>

CSI4133::CSI4133(){
	vidOriginal = 0;
	vidProcessed = 0;
	thresh = 1;
	hue = 1;
	sat = 1;
	val = 1;
	one = 1;
	two = 1;

	//could be global
	tolerance = 10;
	
	//0 = green
	hsValues[0][0] = 70;
	hsValues[0][1] = 120;
	hsValues[0][2] = 115;
	//1 = yellow
	hsValues[1][0] = 95;
	hsValues[1][1] = 160;
	hsValues[1][2] = 165;
	//2 = red
	hsValues[2][0] = 120;
	hsValues[2][1] = 210;
	hsValues[2][2] = 120;
	//3 = bleu
	hsValues[3][0] = 20;
	hsValues[3][1] = 140;
	hsValues[3][2] = 50;
	//4 = black
	hsValues[4][0] = 20;
	hsValues[4][1] = 120;
	hsValues[4][2] = 20;
	//5 = circle
	hsValues[5][0] = 120;
	hsValues[5][1] = 180;
	hsValues[5][2] = 110;
}

CSI4133::~CSI4133(){
	cvDestroyAllWindows();
	if (vidOriginal)
		delete vidOriginal;
	if (vidProcessed)
		delete vidProcessed;
}

bool CSI4133::loadVideoPath(CString * _path){
	CFileDialog dlg(TRUE, _T("*.avi"), NULL, OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,_T("video files (*.avi; *.mpeg) |*.avi;*.mpeg|All Files (*.*)|*.*||"),NULL);
	dlg.m_ofn.lpstrTitle= _T("Open Video");

	if (dlg.DoModal() == IDOK) {
		*_path = dlg.GetPathName();
		return true;
	} else {
		return false;
	}
}

bool CSI4133::saveVideoPath(CString * _path){
	CFileDialog dlg(TRUE, _T("*.avi"), NULL, OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,_T("video files (*.avi; *.mpeg) |*.avi;*.mpeg|All Files (*.*)|*.*||"),NULL);
	dlg.m_ofn.lpstrTitle= _T("save Video");

	if (dlg.DoModal() == IDOK) {
		*_path = dlg.GetPathName();
		return true;
	} else {
		return false;
	}
}

bool CSI4133::loadVideo(CString * _str){
	if (loadVideoPath(_str)){
		if (vidOriginal)
			delete vidOriginal;
		if (vidProcessed)
			delete vidProcessed;
		
		CvCapture *tmpCap = cvCaptureFromAVI(*_str);

		vidOriginal = new ImgArr(tmpCap);
		vidProcessed = new ImgArr(vidOriginal);

		cvReleaseCapture(&tmpCap);

		return true;
	} else {
		return false;
	}
}

void CSI4133::showVideo(ImgArr * _arr, CString _title){
	int key = 0;
	if (_arr){
		for (int i = 0; i < _arr->nFrames && key != 'q'; i++){
			cvShowImage(_title, _arr->frames[i]);
			key = cvWaitKey(1000/_arr->fps);
		}
	}else{
		MessageBox(NULL, "Video not yet loaded","Error", MB_OK);
	}
}

void CSI4133::showImage(CString _name, IplImage * _img){
	if (_img)
		cvShowImage(_name, _img);
	else
		MessageBox(NULL, "Image cannot be shown: not yet loaded","Error", MB_OK);
}

void CSI4133::trackbarChange(int _val1, void * _val2){
	((CSI4133*)_val2)->processTest();
}

void CSI4133::processTest(){
	if (vidOriginal){
		cvNamedWindow("Test Window",1);			
		cvCreateTrackbar2("Frame", "Test Window", &thresh, vidOriginal->nFrames-1, trackbarChange, this);
		cvCreateTrackbar2("Hue", "Test Window", &hue, 255, trackbarChange, this);
		cvCreateTrackbar2("Sat", "Test Window", &sat, 255, trackbarChange, this);
		cvCreateTrackbar2("Val", "Test Window", &val, 255, trackbarChange, this);
		cvCreateTrackbar2("One", "Test Window", &one, 255, trackbarChange, this);
		cvCreateTrackbar2("Two", "Test Window", &two, 255, trackbarChange, this);
		updateSlider();
	} else {
		MessageBox(NULL, "Original video not yet loaded", "Error", MB_OK);
	}
}

void CSI4133::updateSlider(){
	//Test out anything you'd like here.
	
	CvPoint pointArray [2][10];
			// GREEN (0) : 10 center points
			// PINKY (1) : 10 center points

		//Structure declaration
		CvSize dim = cvSize(vidOriginal->frmWidth, vidOriginal->frmHeight);
		IplImage * imgOriginal = cvCreateImage(dim,vidOriginal->frmDepth,3);
		IplImage * imgOutput = cvCreateImage(dim,vidOriginal->frmDepth,3);
		IplImage * hsvImage = cvCreateImage(dim,vidOriginal->frmDepth,3);
		IplImage * mask = cvCreateImage(dim, vidOriginal->frmDepth, 1);
		IplImage * greyscale = cvCreateImage(dim, vidOriginal->frmDepth, 1);
		IplImage * empty = cvCreateImage(dim, vidOriginal->frmDepth, 1);

		CvFont font;
		cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX, 0.4, 0.4, 0, 1);
		CvPoint topLeft;
		CvPoint bottomRight;
		CvPoint centerPoint;
		int end;
		int sommeX;
		int sommeY;
		int nbPixels;
	
	cvCopy(vidOriginal->frames[thresh], imgOriginal);
	cvCopy(imgOriginal, imgOutput);
	cvCvtColor(imgOriginal, hsvImage, CV_RGB2HSV);
			
	for(int colour=0; colour<2; colour++) {  // 0 = green, 1 = yellow
		//PROCESS FINGURES ------------------------------------------
		cvInRangeS(hsvImage, cvScalar(hue - tolerance -1, sat - tolerance, val - tolerance), 
			cvScalar(hue + tolerance -1, sat + tolerance, val+tolerance), mask);

		//cvErode(mask,mask,0,1);
		//cvDilate(mask,mask,0,2);
		IplConvKernel * kernel = cvCreateStructuringElementEx(5, 5, 2, 2, CV_SHAPE_ELLIPSE);
		cvDilate(mask, mask, kernel, 1);
		cvErode(mask, mask, kernel, 1);
		
		cvShowImage("mask", mask);

		sommeX = 0;
		sommeY = 0;
		nbPixels = 0;
		for(int x = 0; x < mask->width; x++) {
			for(int y = 0; y < mask->height; y++) {
				if(((uchar *)(mask->imageData + y*mask->widthStep))[x] == 255) {
					sommeX += x;
					sommeY += y;
					nbPixels++;
				}
			}
		}

		if(nbPixels > 15) {
			centerPoint = cvPoint((int)(sommeX / nbPixels), (int)(sommeY / nbPixels));
			topLeft = cvPoint((int)centerPoint.x-20, (int)centerPoint.y-20);
			bottomRight = cvPoint((int)centerPoint.x+20, (int)centerPoint.y+20);
				
			cvRectangle(imgOutput, topLeft, bottomRight, cvScalar(0,0,255), 2, 8, 0);
			
			//Add tracking information
			//Add text
			if(colour == 0)
				cvPutText(imgOutput, "Green tape", cvPoint(topLeft.x, topLeft.y-2), &font, cvScalar(255,0,0));
			else
				cvPutText(imgOutput, "Yellow tape", cvPoint(topLeft.x, topLeft.y-2), &font, cvScalar(255,0,0));
		} else { //if non existant -- do not display on screen
			centerPoint = cvPoint(-5, -5);
		}
		//Add center point + previous ones (for tracking information)
		pointArray[colour][thresh % 10] = centerPoint;

		if(thresh<10)
			end = thresh;
		else
			end = 10;
		for(int l=0; l<end; l++) {
			cvCircle(imgOutput, pointArray[colour][l], 2, cvScalar(255,0,0), 1);
		}
	}

	// ================================================================================================

	cvCopy(empty, greyscale);
	//cvCvtColor(imgOriginal, greyscale, CV_BGR2GRAY);

	cvInRangeS(hsvImage, cvScalar(hue - tolerance -1, sat - tolerance, 0), 
			cvScalar(hue + tolerance -1, sat + tolerance, 255), mask);

	IplConvKernel * kernel = cvCreateStructuringElementEx(5, 5, 2, 2, CV_SHAPE_ELLIPSE);
	cvDilate(mask, mask, kernel, 1);
	cvErode(mask, mask, kernel, 1);

	for(int x = 0; x < mask->width; x++) {
		for(int y = 0; y < mask->height; y++) { 
 
			// If its a tracked pixel, count it to the center of gravity's calcul
			if(((uchar *)(mask->imageData + y*mask->widthStep))[x] == 255) {
				cvSet2D(greyscale, y, x, cvScalar(255,255,255));
			}
		}
	}
		
	//Gaussian smoothing first so that we eliminate the possiblities of false circles.
	cvSmooth(greyscale, greyscale, CV_GAUSSIAN, 9, 9); 

	//Storage for cvHoughCircles function.
	CvMemStorage* storage = cvCreateMemStorage(0);

	//cvHoughCircles finds the circles using a modified Hough Transform.
	CvSeq* circles = cvHoughCircles(greyscale, storage, CV_HOUGH_GRADIENT, 2, greyscale->height/15, one, two, 20, 30);
		
	//Display where the circles are in the image
	if(circles->total > 2) {
		cvSaveImage("newSaveX.jpg", imgOriginal);
	}
	for (int z = 0; z < circles->total; z++) 
	{
		float* p = (float*)cvGetSeqElem( circles, z );
		//Draw the edge around the circle
		cvPutText(imgOutput, "Red Circle", cvPoint((cvRound(p[0]) - cvRound(p[2])), (cvRound(p[1]) - cvRound(p[2]))), &font, cvScalar(255,0,0));
		cvCircle( imgOutput, cvPoint(cvRound(p[0]),cvRound(p[1])), 25, CV_RGB(255,0,0), 3, 8, 0 );
	}
			
	// ================================================================================================

	//Add frame back to output video
	showImage("Test Window", imgOutput);
	//showImage("Original Image with Points", imgOriginal);

	cvReleaseImage(&imgOriginal);
	cvReleaseImage(&hsvImage);
	cvReleaseImage(&mask);
	//cvReleaseImage(&rgbImage);
	cvReleaseImage(&greyscale);
	cvReleaseImage(&empty);

}

void CSI4133::processVideo(){
	
	if (vidOriginal && vidProcessed){
		vidProcessed->clearFrames();
		// Input: class variable 'vidOriginal'
		// Output: class variable 'vidProcessed'
		//<Student's Code>

		CvPoint pointArray [6][10];
			// GREEN	(0) : 10 center points
			// YELLOW	(1) : 10 center points
			// RED		(2) : 10 center points
			// BLEU		(3) : 10 center points
			// BLACK	(4) : 10 center points
			// CIRCLE	(5)	: 10 center points

		//Structure declaration
		CvSize dim = cvSize(vidOriginal->frmWidth, vidOriginal->frmHeight);
		IplImage * imgOriginal = cvCreateImage(dim,vidOriginal->frmDepth,3);
		IplImage * imgOutput = cvCreateImage(dim,vidOriginal->frmDepth,3);
		IplImage * hsvImage = cvCreateImage(dim,vidOriginal->frmDepth,3);
		IplImage * mask = cvCreateImage(dim, vidOriginal->frmDepth, 1);
		IplImage * greyscale = cvCreateImage(dim, vidOriginal->frmDepth, 1);
		IplImage * empty = cvCreateImage(dim, vidOriginal->frmDepth, 1);

		CvFont font;
		cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX, 0.4, 0.4, 0, 1);
		CvScalar printColour;
		CvPoint topLeft;
		CvPoint bottomRight;
		CvPoint centerPoint;
		CvPoint circleCenter = cvPoint(-5,-5);
		int end;
		int sommeX;
		int sommeY;
		int nbPixels;

		for(int k=0; k<vidOriginal->nFrames-1; k++) {

			cvCopy(vidOriginal->frames[k], imgOriginal);
			cvCopy(imgOriginal, imgOutput);
			cvCvtColor(imgOriginal, hsvImage, CV_RGB2HSV);

			// ================================================================================================
			// Circle Processing

			cvCopy(empty, greyscale);
			//cvCvtColor(imgOriginal, greyscale, CV_BGR2GRAY);

			cvInRangeS(hsvImage, cvScalar(hsValues[5][0] - tolerance -1, hsValues[5][1] - tolerance, 0), 
			cvScalar(hsValues[5][0] + tolerance -1, hsValues[5][1] + tolerance, 255), mask);

			IplConvKernel * kernel = cvCreateStructuringElementEx(5, 5, 2, 2, CV_SHAPE_ELLIPSE);
			cvDilate(mask, mask, kernel, 1);
			cvErode(mask, mask, kernel, 1);

			for(int x = 0; x < mask->width; x++) {
				for(int y = 0; y < mask->height; y++) { 
 
					// If its a tracked pixel, count it to the center of gravity's calcul
					if(((uchar *)(mask->imageData + y*mask->widthStep))[x] == 255) {
						cvSet2D(greyscale, y, x, cvScalar(255,255,255));
					}
				}
			}
		
			//Gaussian smoothing first so that we eliminate the possiblities of false circles.
			cvSmooth(greyscale, greyscale, CV_GAUSSIAN, 9, 9); 

			//Storage for cvHoughCircles function.
			CvMemStorage* storage = cvCreateMemStorage(0);

			//cvHoughCircles finds the circles using a modified Hough Transform.
			CvSeq* circles = cvHoughCircles(greyscale, storage, CV_HOUGH_GRADIENT, 2, greyscale->height/15, 50, 40, 20, 30);
		
			//Draw the circle in the image				
			for (int z = 0; z < circles->total; z++) 
			{
				float* p = (float*)cvGetSeqElem( circles, z );
				//Draw the edge around the circle
				cvPutText(imgOutput, "RED Circle", cvPoint((cvRound(p[0]) - cvRound(p[2])), (cvRound(p[1]) - cvRound(p[2]))), &font, cvScalar(0,0,255));
				circleCenter = cvPoint(cvRound(p[0]),cvRound(p[1]));
				cvCircle( imgOutput, circleCenter, 25, CV_RGB(255,0,0), 3, 8, 0 );
			}
			
			// ================================================================================================
			//PROCESS FINGURES ------------------------------------------

			for(int colour=0; colour<5; colour++) {  // 0 = green, 1 = yellow, 2 = red, 3 = blue, 4 = black

				cvInRangeS(hsvImage, cvScalar(hsValues[colour][0] - tolerance -1, hsValues[colour][1] - tolerance, hsValues[colour][2] - tolerance), 
					cvScalar(hsValues[colour][0] + tolerance -1, hsValues[colour][1] + tolerance, hsValues[colour][2] + tolerance), mask);

				IplConvKernel * kernel = cvCreateStructuringElementEx(5, 5, 2, 2, CV_SHAPE_ELLIPSE);
				cvDilate(mask, mask, kernel, 1);
				cvErode(mask, mask, kernel, 1);
				cvDilate(mask, mask, kernel, 1);
				
				sommeX = 0;
				sommeY = 0;
				nbPixels = 0;
				for(int x = 0; x < mask->width; x++) {
					for(int y = 0; y < mask->height; y++) {
						if(((uchar *)(mask->imageData + y*mask->widthStep))[x] == 255) {
							//Do not count the pixel if it is part of the circle -- if a red pixel
							if( colour != 2 || (colour == 2 && ( abs(x - circleCenter.x) > 30 || abs(y - circleCenter.y) > 30)) ) {
								sommeX += x;
								sommeY += y;
								nbPixels++;
							}
						}
					}
				}

				if(nbPixels > 5) {
					centerPoint = cvPoint((int)(sommeX / nbPixels), (int)(sommeY / nbPixels));
					topLeft = cvPoint((int)centerPoint.x-20, (int)centerPoint.y-20);
					bottomRight = cvPoint((int)centerPoint.x+20, (int)centerPoint.y+20);
			
					//Add tracking information
					//Add text
					printColour = cvScalar(0);
					if(colour == 0) {
						printColour = cvScalar(0,255,0);
						cvPutText(imgOutput, "GREEN tape", cvPoint(topLeft.x, topLeft.y-2), &font, printColour);
					} else if(colour == 1) {
						printColour = cvScalar(0,255,255);
						cvPutText(imgOutput, "YELLOW tape", cvPoint(topLeft.x, topLeft.y-2), &font, printColour);
					} else if(colour == 2) {
						printColour = cvScalar(0,0,255);
						cvPutText(imgOutput, "RED tape", cvPoint(topLeft.x, topLeft.y-2), &font, printColour);
					} else if(colour == 3) {
						printColour = cvScalar(255,0,0);
						cvPutText(imgOutput, "BLUE tape", cvPoint(topLeft.x, topLeft.y-2), &font, printColour);
					} else if(colour == 4) {
						printColour = cvScalar(0,0,0);
						cvPutText(imgOutput, "BLACK tape", cvPoint(topLeft.x, topLeft.y-2), &font, printColour);
					}

					cvRectangle(imgOutput, topLeft, bottomRight, printColour, 2, 8, 0);

				} else { //if non existant -- do not display on screen
					centerPoint = cvPoint(-5, -5);
				}
				//Add center point + previous ones (for tracking information)
				pointArray[colour][k % 10] = centerPoint;

				if(k<10)
					end = k;
				else
					end = 10;
				for(int l=0; l<end; l++) {
					cvCircle(imgOutput, pointArray[colour][l], 2, printColour, 1);
				}
			}

			// ================================================================================================
			//Add frame back to output video
			vidProcessed->addFrame(imgOutput);
			//showImage("Original Image with Points", imgOriginal);
		
		} //END LOOP
		
		//Release image spaces created
		cvReleaseImage(&imgOriginal);
		cvReleaseImage(&hsvImage);
		cvReleaseImage(&mask);
		//cvReleaseImage(&rgbImage);
		cvReleaseImage(&greyscale);
		cvReleaseImage(&empty);
		//cvReleaseImage(&temp_image);

		//</Student's Code>
		showVideo(vidProcessed, "Processed Video");
	} else {
		MessageBox(NULL, "Video not yet loaded","Error", MB_OK);
	}	
}

void CSI4133::saveVideo(ImgArr * _arr){
	CString path;
	CvVideoWriter * writer;
	if (vidOriginal && vidProcessed){
		if (saveVideoPath(&path)){
			CvSize dim = cvSize(_arr->frmWidth, _arr->frmHeight);
			int tempDepth = _arr->frmDepth;
			if(writer = cvCreateVideoWriter(path, -1, vidOriginal->fps, dim)){
				for (int i = 0 ; i < _arr->nFrames;i++)
					cvWriteFrame(writer, _arr->frames[i]);
				cvReleaseVideoWriter(&writer);
			}
		}
	} else{
		MessageBox(NULL, "Video not yet loaded","Error", MB_OK);
	}
}

void CSI4133::saveProcessedVideo(){
	saveVideo(vidProcessed);
}