#ifndef CSI4133_H
#define CSI4133_H
#include <list>
#include <queue>

#include "cv.h"
#include "highgui.h"

class ImgArr{
public:
	int fps;
	int frmWidth;
	int frmHeight;
	int frmDepth;
	int nFrames;
	int fourCC;
	IplImage ** frames;

	ImgArr(ImgArr * _arr){
		fps = _arr->fps;
		frmWidth = _arr->frmWidth;
		frmHeight = _arr->frmHeight;
		frmDepth = _arr->frmDepth;
		fourCC = _arr->fourCC;

		nFrames = 0;
		frames = 0;
	}

	ImgArr(CvCapture * _cap){
		nFrames = (int)cvGetCaptureProperty(_cap, CV_CAP_PROP_FRAME_COUNT);
		fps = (int)cvGetCaptureProperty(_cap, CV_CAP_PROP_FPS);
		frmWidth = (int)cvGetCaptureProperty(_cap, CV_CAP_PROP_FRAME_WIDTH);
		frmHeight = (int)cvGetCaptureProperty(_cap, CV_CAP_PROP_FRAME_HEIGHT);
		frmDepth = -1;
		fourCC = (int)cvGetCaptureProperty(_cap, CV_CAP_PROP_FOURCC);

		frames = new IplImage *[nFrames];

		for (int i = 0; i < nFrames;i++){
			IplImage * tmpImg = cvQueryFrame(_cap);
			frmDepth = tmpImg->depth;
			frames[i] = cvCloneImage(tmpImg);
		}
	}
	void clearFrames(){
		if (frames){
			for (int i = 0 ; i < nFrames; i++)
				cvReleaseImage(&frames[i]);
			delete [] frames;
		}
		frames = 0;
		nFrames = 0;
	}
	~ImgArr(){
		clearFrames();
	}
	void addFrame(IplImage * _img){
		IplImage ** tmpFrames = new IplImage * [nFrames+1];

		for (int i = 0; i < nFrames; i++){
			tmpFrames[i] = frames[i];
		}
		tmpFrames[nFrames] = cvCloneImage(_img);

		delete [] frames;

		frames = tmpFrames;
		nFrames++;
	}
};


class CSI4133{
private:
public:
	ImgArr * vidOriginal;
	ImgArr * vidProcessed;

	CString path;

	int thresh;
	int hue;
	int sat;
	int val;
	int one;
	int two;

	int tolerance;
	int hsValues[6][3];


	CSI4133();
	~CSI4133();

	bool loadVideoPath(CString * _path);
	bool saveVideoPath(CString * _path);
	bool loadVideo(CString * _str);
	void saveVideo(ImgArr * _arr);
	void saveProcessedVideo();
	void showVideo(ImgArr * _arr, CString _title);
	void showImage(CString _name, IplImage * _img);
	static void trackbarChange(int _val1, void * _val2);
	void processTest();
	void updateSlider();

	void processVideo();

	CString getPath(){return path;}
	
};

#endif