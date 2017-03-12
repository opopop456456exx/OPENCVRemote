//--------------------------------------������˵����-------------------------------------------
//		����˵����NCURoboTeamר��ң�أ�OPENCV�棩
//		����������NCURoboTeam Remote
//		������������IDE�汾��Visual Studio 2015
//		������������OpenCV�汾��	2.4.9
//		2017��02�� Created by @����
//		2017��03�� Revised by @����
//------------------------------------------------ ------------------------------------------------

//---------------------------------��ͷ�ļ��������ռ�������֡�-----------------------------
//		����������������ʹ�õ�ͷ�ļ��������ռ�
//-------------------------------------------------------------------------------------------------
#include <opencv2/opencv.hpp>
//#include "DFCOM.h"
using namespace cv;
using namespace std;
//-----------------------------------���궨�岿�֡�--------------------------------------------
//  ����������һЩ������ 
//------------------------------------------------------------------------------------------------ 
#define PICDIV_WINDOW_NAME "��ͼ�����ڡ�"        //Ϊ���ڱ��ⶨ��ĺ� 
#define REMOTE_WINDOW_NAME "���ֱ����ڡ�"        //Ϊ���ڱ��ⶨ��ĺ� 

#define KEY0    1                         //��������
#define KEY1    2                         //ƫ��A��
#define KEY2    3                         //ƫ��B��
#define KEY3    4                         //ȷ��ƫ���
#define KEY4    5                         //ģʽ�л���
#define KEY1C   10                        //ƫ��A�������������
#define KEY2C   11                        //ƫ��B�������������
/////////////////////////////////////////////////////////////////////////////
#define KEY0B    21                         //

//-----------------------------------��ȫ�ֺ����������֡�------------------------------------
//		������ȫ�ֺ���������
//------------------------------------------------------------------------------------------------
void on_MouseHandle(int event, int x, int y, int flags, void* param);
void DrawRectangle( cv::Mat& img, cv::Rect box );
void ShowHelpText( );
void DrawTransCir(Mat img, int x, int y, int width, int height, Scalar& color, double alpha);
void DrawDivLine(Mat &image, Rect Point1, Rect Piont2, Scalar color, int offset);
void DrawMainCircle(Mat& roi, int textx, int texty, Scalar color);
int  KeyJudge(int x, int y);
void imgReset(Mat &img);
void imgResetBackGroung(Mat &img);
void KeyProgressLButtomDown(int x, int y);
void KeyProgressMouseMove(int x, int y);

//-----------------------------------���ṹ���������֡�-----------------------------------
//		�������Զ���ṹ�������
//-----------------------------------------------------------------------------------------------

//-----------------------------------��ȫ�ֱ����������֡�-----------------------------------
//		������ȫ�ֱ���������
//-----------------------------------------------------------------------------------------------

Rect g_rectangleK0;                   //�����ǵ�λ��
Rect g_rectangleK1;
Rect g_rectangleK2;
Rect g_rectangleK3;
Rect g_rectangleK4;

bool g_bStatusClean = false;                                  //�Ƿ�������ò���
RNG g_rng(12345);

bool g_bKey1Press=false;                                      //Key1�Ƿ���
bool g_bKey2Press = false;                                    //Key2�Ƿ���
bool g_bDivConfirm = false;                                   //Key3�Ƿ���
char g_bModuleIndex =  0;                                     //Key4�Ƿ���,ģ������,0ΪPICDIV��1ΪREMOTE

Rect g_lastrectangle = Rect(-200, -200, 0, 0);                //�ֲ�ˢ���õ�ȫ��Rect,��¼��һ��λ��
Rect g_lastrectangle1 = Rect(-200, -200, 0, 0);               //�ֲ�ˢ���õ�ȫ��Rect,��¼��һ��λ��
Rect g_bDivValue=Rect(0, 0, 0, 0);
Mat g_ChangeSave;                                             //�ֲ�ˢ���õ�ȫ��Mat
Mat g_ChangeSave1;                                            //�ֲ�ˢ���õ�ȫ��Mat

Mat tempImage;                                                //һ��������ͼƬ�洢�����¡�ˢ���ã�
///////////////////////////////////////////////////////////////////////////////////////////////////////////
Rect g_rectangleK0B;


//--------------------------------------------------------------------------------------


//-----------------------------------��main( )������--------------------------------------------
//		����������̨Ӧ�ó������ں��������ǵĳ�������￪ʼִ��
//------------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	//��1���ı�console������ɫ
	system("color 9F");

	//��2����ʾ��ӭ�Ͱ�������
	ShowHelpText();
	//��3����������
	//if (COM_Connect("COM3", 115200))
	//{
	//	//����ʧ��
	//	getchar();
	//	return -1;
	//}

	//��4��׼������
	//ͼ�����ڲ���
	Mat dfpic;
	imgReset(dfpic);
	//�ֱ����ڲ���
	Mat remotepic(600,960,CV_8UC3);
	remotepic = Scalar::all(0);     //ȫ����

	//��5�������������ص�����
	//����ģʽ��Ϊ�ɱ�ģʽ��Ϊ�ʺ�ƽ����ʾ
	namedWindow(PICDIV_WINDOW_NAME, 0);
	//moveWindow(PICDIV_WINDOW_NAME, 200, 50);
	setMouseCallback(PICDIV_WINDOW_NAME, on_MouseHandle, (void*)&dfpic);
	//////////////////////////////////////
	//namedWindow(REMOTE_WINDOW_NAME, WINDOW_AUTOSIZE);
	//setMouseCallback(REMOTE_WINDOW_NAME, on_MouseHandle, (void*)&remotepic);
	//////////////////////////////////////

	//��6��������ѭ��
	while (1)
	{
		
		switch (g_bModuleIndex)
		{
		case 0:
		{
			if (g_bStatusClean)imgReset(dfpic);
			imshow(PICDIV_WINDOW_NAME, dfpic);
			if (waitKey(10) == 27) break;//����ESC���������˳�
		}
		break;

		case 1:
		{
			if (g_bStatusClean)imgReset(remotepic);
			imshow(PICDIV_WINDOW_NAME, remotepic);
			if (waitKey(10) == 27) break;//����ESC���������˳�
		}
		break;


		}
	}
	return 0;
}

//--------------------------------------------------------------------------------------
//-----------------------------------��DrawTransCir��-----------------------------------
//		��������һ��ROI�����ڻ�һ����͸���ȵ�Բ��
//      ������
//           int x, int y, int width, int height  �ֱ���ROI���������x,y�ͳ���
//           Scalar& color                        Բ��BRR��ɫ
//           double alpha                         ͸����
//-----------------------------------------------------------------------------------------------
void DrawTransCir(Mat img, int x, int y, int width, int height,  Scalar& color, double alpha)
{
	Mat  rec=img(Rect(x,y, width, height));
	Mat  rec1 = rec.clone();
	circle(rec1, Point( width / 2,  height / 2), 20, color, -1,8);
	addWeighted(rec, alpha, rec1, 1 - alpha, 0.0, rec);
}

//-----------------------------------��DrawDivLine��-----------------------------------
//		������������ROI��������ƫ��offset����λ����ĵ�����ߣ����ڴ����м��ӡ�������ƫ��
//      ������
//           Mat &image                           Mat��ͼ��
//           Rect Point1, Rect Piont2             ����ROI���Rectλ��
//           Scalar color                         �ߺ����ֵ���ɫ
//           int offset                           ƫ��ֵ
//-----------------------------------------------------------------------------------------------

void DrawDivLine(Mat &image,Rect Point1, Rect Piont2, Scalar color,int offset)
{
	char send_message[20];
	line(image, Point(Point1.x + offset, Point1.y + offset), Point(Piont2.x + offset, Piont2.y + offset), color, 1, CV_AA, 0);
	g_bDivValue = Rect(Point1.x - Piont2.x, Point1.y - Piont2.y, 0, 0);
	_snprintf(send_message, 13, "(%d,%d)\0", g_bDivValue.x, g_bDivValue.y);
	putText(image, send_message, cvPoint((Point1.x + Piont2.x ) / 2+ offset, (Point1.y + Piont2.y ) / 2+ offset), FONT_HERSHEY_COMPLEX_SMALL, 1, color, 1, 8);

}

//-----------------------------------��DrawMainCircle��-----------------------------------
//		�������������ͼ�����ֵ��Բ�������ﻭ����
//      ������
//           Mat &roi                             Mat��ͼ��
//           int textx,int texty                  ��ӡ���������֣�����۲�
//           Scalar color                         �ߺ����ֵ���ɫ
//-----------------------------------------------------------------------------------------------
void DrawMainCircle(Mat& roi,int textx,int texty, Scalar color)
{
	char send_message[20];
	_snprintf(send_message, 10, "%d,%d\0", textx, texty);
	DrawTransCir(roi, 0, 0, 50, 50, color, 0.7);
	line(roi, Point(0, 25), Point(50, 25), color, 1, CV_AA, 0);
	circle(roi, Point(25, 25), 10, color, 1, 8);
	line(roi, Point(25, 0), Point(25, 50), color, 1, CV_AA, 0);
	putText(roi, send_message, cvPoint(50, 25), FONT_HERSHEY_COMPLEX_SMALL, 1, color, 1, 8);
}
//-----------------------------------��KeyJudge��-----------------------------------
//		�������ж����������������Ƿ����ض����ⰴ���ϣ�������ֱ𷵻���Ӧ��ֵ�����򷵻�0��
//            �����ʣ���Ϊ����������
//      ������
//           int x, int y                         �ֱ������ص������е��������x,y��
//-----------------------------------------------------------------------------------------------
int KeyJudge(int x, int y)
{
	if (g_bModuleIndex == 0)
	{
		if (x > g_rectangleK0.x&&x<(g_rectangleK0.x + g_rectangleK0.width) && y>g_rectangleK0.y&&y < (g_rectangleK0.y + g_rectangleK0.height))
		{
			int p1 = (g_rectangleK0.x + g_rectangleK0.width / 2);
			int p2 = (g_rectangleK0.y + g_rectangleK0.height / 2);
			if ((x - p1)*(x - p1) + (y - p2)*(y - p2) <= 400)
			{
				return KEY0;
			}
		}

		else if (x > g_rectangleK1.x&&x<(g_rectangleK1.x + g_rectangleK1.width) && y>g_rectangleK1.y&&y < (g_rectangleK1.y + g_rectangleK1.height))
		{
			int p1 = (g_rectangleK1.x + g_rectangleK1.width / 2);
			int p2 = (g_rectangleK1.y + g_rectangleK1.height / 2);
			if ((x - p1)*(x - p1) + (y - p2)*(y - p2) <= 400)
			{
				return KEY1;
			}
		}
		else if (x > g_rectangleK2.x&&x<(g_rectangleK2.x + g_rectangleK2.width) && y>g_rectangleK2.y&&y < (g_rectangleK2.y + g_rectangleK2.height))
		{
			int p1 = (g_rectangleK2.x + g_rectangleK2.width / 2);
			int p2 = (g_rectangleK2.y + g_rectangleK2.height / 2);
			if ((x - p1)*(x - p1) + (y - p2)*(y - p2) <= 400)
			{
				return KEY2;
			}
		}

		else if (x > g_rectangleK3.x&&x<(g_rectangleK3.x + g_rectangleK3.width) && y>g_rectangleK3.y&&y < (g_rectangleK3.y + g_rectangleK3.height))
		{
			int p1 = (g_rectangleK3.x + g_rectangleK3.width / 2);
			int p2 = (g_rectangleK3.y + g_rectangleK3.height / 2);
			if ((x - p1)*(x - p1) + (y - p2)*(y - p2) <= 400)
			{
				return KEY3;
			}
		}
		else if (x > g_rectangleK4.x&&x<(g_rectangleK4.x + g_rectangleK4.width) && y>g_rectangleK4.y&&y < (g_rectangleK4.y + g_rectangleK4.height))
		{
			int p1 = (g_rectangleK4.x + g_rectangleK4.width / 2);
			int p2 = (g_rectangleK4.y + g_rectangleK4.height / 2);
			if ((x - p1)*(x - p1) + (y - p2)*(y - p2) <= 400)
			{
				return KEY4;
			}
		}
		else if (x > (g_lastrectangle1.x + 100) && x<(g_lastrectangle1.x + 150) && y>(g_lastrectangle1.y + 100) && y < (g_lastrectangle1.y + 150))
		{
			int p1 = (g_lastrectangle1.x + 125);
			int p2 = (g_lastrectangle1.y + 125);
			if ((x - p1)*(x - p1) + (y - p2)*(y - p2) <= 400)
			{
				return KEY2C;
			}
		}

		else if (x > (g_lastrectangle.x + 100) && x<(g_lastrectangle.x + 150) && y>(g_lastrectangle.y + 100) && y < (g_lastrectangle.y + 150))
		{
			int p1 = (g_lastrectangle.x + 125);
			int p2 = (g_lastrectangle.y + 125);
			if ((x - p1)*(x - p1) + (y - p2)*(y - p2) <= 400)
			{
				return KEY1C;
			}
		}
		return 0;
	}

	if (g_bModuleIndex == 1)
	{
		if (x > g_rectangleK0B.x&&x<(g_rectangleK0B.x + g_rectangleK0B.width) && y>g_rectangleK0B.y&&y < (g_rectangleK0B.y + g_rectangleK0B.height))
		{
			int p1 = (g_rectangleK0B.x + g_rectangleK0B.width / 2);
			int p2 = (g_rectangleK0B.y + g_rectangleK0B.height / 2);
			if ((x - p1)*(x - p1) + (y - p2)*(y - p2) <= 400)
			{
				return KEY0B;
			}
		}

	}
}


void KeyInit(Mat &img)
{
	if (g_bModuleIndex == 0)
	{
		g_rectangleK0 = Rect(50, img.rows - 75, 50, 50);                   //�����ǵ�λ��
		g_rectangleK1 = Rect(img.cols / 2 - 75, img.rows - 75, 50, 50);
		g_rectangleK2 = Rect(img.cols / 2 + 75, img.rows - 75, 50, 50);
		g_rectangleK3 = Rect(img.cols - 100, img.rows - 75, 50, 50);
		g_rectangleK4 = Rect(img.cols - 100, 50, 50, 50);

		DrawTransCir(img, g_rectangleK0.x, g_rectangleK0.y, g_rectangleK0.width, g_rectangleK0.height, Scalar(0, 255, 0), 0.5);
		DrawTransCir(img, g_rectangleK1.x, g_rectangleK1.y, g_rectangleK1.width, g_rectangleK1.height, Scalar(0, 0, 255), 0.5);
		DrawTransCir(img, g_rectangleK2.x, g_rectangleK2.y, g_rectangleK2.width, g_rectangleK2.height, Scalar(255, 0, 0), 0.5);
		DrawTransCir(img, g_rectangleK3.x, g_rectangleK3.y, g_rectangleK3.width, g_rectangleK3.height, Scalar(0, 0, 255), 0.5);
		DrawTransCir(img, g_rectangleK4.x, g_rectangleK4.y, g_rectangleK4.width, g_rectangleK4.height, Scalar(0, 0, 255), 0.5);
	}
	if (g_bModuleIndex == 1)
	{
		g_rectangleK0B = Rect(img.cols - 100, 50, 50, 50);
		DrawTransCir(img, g_rectangleK0B.x, g_rectangleK0B.y, g_rectangleK0B.width, g_rectangleK0B.height, Scalar(0, 255, 0), 0.5);

	}
}
//-----------------------------------��imgReset��-----------------------------------
//		������ģʽ���ã���ͼƬ���¼��أ����в�����ʼ��һ��
//            �����ʣ���Ϊ����������
//      ������
//           Mat &img                               Mat��ͼ��
//-----------------------------------------------------------------------------------------------
void  imgReset(Mat &img)
{
	if (g_bModuleIndex == 0)
	{
		g_lastrectangle = Rect(-200, -200, 0, 0);//��¼��ʼ��
		g_lastrectangle1 = Rect(-200, -200, 0, 0);//��¼��ʼ��
		Rect g_bDivValue = Rect(0, 0, 0, 0);
		img = imread("df.jpg");

		KeyInit(img);

		img.copyTo(tempImage);
		g_bKey1Press = false;
		g_bKey2Press = false;
		g_bStatusClean = false;
		g_bDivConfirm = false;
	}
	if (g_bModuleIndex == 1)
	{
		KeyInit(img);
		g_bStatusClean = false;
	}
}
//-----------------------------------��imgResetBackGroung��-----------------------------------
//		������ͼƬ���ã���������ͼƬ���¼��أ����������³�ʼ��
//            �����ʣ���Ϊ����������
//      ������
//           Mat &img                               Mat��ͼ��
//-----------------------------------------------------------------------------------------------
void  imgResetBackGroung(Mat &img)
{
	tempImage.copyTo(img);
	KeyInit(img);
}
//-----------------------------------��KeyProgressLButtomDown��-----------------------------------
//		������ͨ������KeyJudge�������صļ�ֵ�ж����ĸ���������ִ����Ӧ�Ĳ���������Ƿ�������
//            �����������������ɨ��ģ�
//            �����ʣ���Ϊ����������
//      ������
//           int x, int y                         �ֱ������ص������е��������x,y��
//-----------------------------------------------------------------------------------------------
void KeyProgressLButtomDown(int x, int y)
{
	if (g_bModuleIndex == 0)
	{
		switch (KeyJudge(x, y))
		{
		case KEY0: g_bStatusClean = true;  break;
		case KEY1: g_bKey1Press = true;    break;
		case KEY2: g_bKey2Press = true;    break;
		case KEY3: g_bDivConfirm = true;   break;
		case KEY4: g_bModuleIndex = 1; g_bStatusClean = true; break;      //ת��ģʽ
		case KEY2C: g_bKey2Press = true;   break;                         //������˳��
		case KEY1C: g_bKey1Press = true;   break;

		default:     break;
		}
	}
	if (g_bModuleIndex == 1)
	{
		switch (KeyJudge(x, y))
		{
		case KEY0B: g_bModuleIndex = 0; g_bStatusClean = true;  break;     //ת��ģʽ
		default:     break;
		}
	}


}

//-----------------------------------��KeyProgressMouseMove��-----------------------------------
//		������ͨ������KeyJudge�������صļ�ֵ�ж����ĸ���������ִ����Ӧ�Ĳ���������Ƿ�������
//            ������������ƶ���ɨ��ģ�
//            �����ʣ���Ϊ����������
//      ������
//           int x, int y                         �ֱ������ص������е��������x,y��
//-----------------------------------------------------------------------------------------------
void KeyProgressMouseMove(int x, int y)
{
	switch (KeyJudge(x, y))
	{
	case KEY0: g_bStatusClean = true;  break;
	default:     break;
	}
}

//--------------------------------��on_MouseHandle( )������-----------------------------
//		���������ص����������ݲ�ͬ������¼����в�ͬ�Ĳ������൱ӷ�ף���֪�����ά��
//-----------------------------------------------------------------------------------------------
void on_MouseHandle(int event, int x, int y, int flags, void* param)
{

	Mat& image = *(cv::Mat*) param;
	switch( event)
	{
		//����ƶ���Ϣ//ƽ���¹�������
	case EVENT_MOUSEMOVE:                         
		{
			KeyProgressMouseMove(x, y);
		}
		break;

		//���������Ϣ
	case EVENT_LBUTTONDOWN: 
		{
			//�������
			KeyProgressLButtomDown(x, y);
			
			//��������
			if (g_bDivConfirm == true)
			{
				if ((g_lastrectangle.x != -200) && (g_lastrectangle1.x != -200))
				{
					imgResetBackGroung(image);
					Mat  rec = image(g_lastrectangle);
					DrawMainCircle(rec, g_lastrectangle.x + 25, g_lastrectangle.y + 25, Scalar(0, 0, 255));

					Mat  rec1 = image(g_lastrectangle1);
					DrawMainCircle(rec1, g_lastrectangle1.x + 25, g_lastrectangle1.y + 25, Scalar(255, 0, 0));
					DrawDivLine(image, g_lastrectangle, g_lastrectangle1, Scalar(0, 255, 0), 25);
				}
				//g_bDivConfirmһ�����£��������ò������
				break;                                                         
			}
			 if (g_bKey1Press==true)
			{
				if (g_lastrectangle.x != -200)
				{
					if (g_lastrectangle1.x != -200)
					{
						//�ؼ����ڵ���Ŀ�ʼ������ͼ���½���һ��Saveͼ��
						Mat temp = tempImage(g_lastrectangle);              
						g_ChangeSave = temp.clone();
					}
				}
				else
				{
					Mat rec = image(Rect(x - 125, y - 125, 150, 150));
					g_lastrectangle = Rect(x - 125, y - 125, 150, 150);
					
					g_ChangeSave = rec.clone();
				}
			}

			 if (g_bKey2Press == true)
			{
				if (g_lastrectangle1.x != -200)
				{
					if (g_lastrectangle.x != -200)
					{
						//�ؼ����ڵ���Ŀ�ʼ������ͼ���½���һ��Saveͼ��
						Mat temp = tempImage(g_lastrectangle1);           
						g_ChangeSave1 = temp.clone();

					}
				}
				else
				{
					Mat rec1 = image(Rect(x - 125, y - 125, 150, 150));
					g_lastrectangle1 = Rect(x - 125, y - 125, 150, 150);
					
					g_ChangeSave1 = rec1.clone();
				}
			}

			////////////////////////////

		}
		break;

		//���̧����Ϣ
	case EVENT_LBUTTONUP:                       
	{

		if (g_bKey1Press = true)
		{
			g_bKey1Press = false;
		}
		if (g_bKey2Press = true)
		{
			g_bKey2Press = false;
		}
		
	}

	break;

	}
	
	switch (flags)
	{
		//�����ҷ
	case CV_EVENT_FLAG_LBUTTON:
	{
		if (x + 25 > image.cols)x = image.cols - 25;
		if (y + 25 > image.rows)y = image.rows - 25;
		if (x < 0)x = 0;
		if (y < 0)y = 0;
		int px = x - 125;
		int py = y - 125;
		if (px <= 0)px = 0;
		if (py<= 0)py = 0;

		if (g_bDivConfirm != true)
		{
			if (g_bKey1Press == true) {

				//һֱ�����ݲ���Ծ���Ǹ���
				if (g_lastrectangle1.x != -200)                          
				{
					Mat temp = tempImage(g_lastrectangle1);
					Mat  rec1 = image(g_lastrectangle1);
					temp.copyTo(rec1);
					DrawMainCircle(rec1, g_lastrectangle1.x + 25, g_lastrectangle1.y + 25, Scalar(255, 0, 0));
					DrawTransCir(rec1, 100, 100, 50, 50, Scalar(255, 0, 0), 0.5);
				}
				Mat  rec = image(g_lastrectangle);
				g_ChangeSave.copyTo(rec);
				rec = image(Rect(px, py, 150, 150));
				rec.copyTo(g_ChangeSave);
				DrawMainCircle(rec, px + 25, py + 25, Scalar(0, 0, 255));
				DrawTransCir(rec, 100, 100, 50, 50, Scalar(0, 0, 255), 0.5);
				g_lastrectangle = Rect(px, py, 150, 150);
			}

			if (g_bKey2Press == true) {

				//һֱ�����ݲ���Ծ���Ǹ���
				if (g_lastrectangle.x != -200)
				{
					Mat temp = tempImage(g_lastrectangle);
					Mat  rec = image(g_lastrectangle);
					temp.copyTo(rec);
					DrawMainCircle(rec, g_lastrectangle.x + 25, g_lastrectangle.y + 25, Scalar(0, 0, 255));
					DrawTransCir(rec, 100, 100, 50, 50, Scalar(0, 0, 255), 0.5);
				}
				Mat  rec1 = image(g_lastrectangle1);
				g_ChangeSave1.copyTo(rec1);

				rec1 = image(Rect(px, py, 150, 150));

				rec1.copyTo(g_ChangeSave1);
				DrawMainCircle(rec1, px + 25, py + 25, Scalar(255, 0, 0));
				DrawTransCir(rec1, 100, 100, 50, 50, Scalar(255, 0, 0), 0.5);
				g_lastrectangle1 = Rect(px, py, 150, 150);
			}
		}
	}
	break;

	}
}

//-----------------------------------��DrawRectangle( )������------------------------------
//		�������Զ���ľ��λ��ƺ���
//-----------------------------------------------------------------------------------------------
void DrawRectangle( cv::Mat& img, cv::Rect box )
{
	cv::rectangle(img,box.tl(),box.br(),cv::Scalar(g_rng.uniform(0, 255), g_rng.uniform(0,255), g_rng.uniform(0,255)));//�����ɫ
}

//-----------------------------------��ShowHelpText( )������-----------------------------
//          ���������һЩ������Ϣ
//----------------------------------------------------------------------------------------------
void ShowHelpText()
{
	//�����ӭ��Ϣ��OpenCV�汾
	printf("\n\n\t\t\t\t NCURoboteam Remote\n");
	printf("\n\n\t\t\t\t      20170311\n");
	printf("\n\n\t\t\t\t��ǰ�汾Ϊ��NCURTR V1.0"  );
	printf("\n\n  ----------------------------------------------------------------------------\n");
	//���һЩ������Ϣ
	printf("\n\n\n\t��ӭʹ��\n"); 
	printf("\n\n\t���ڴ����е�����������϶��Ի�ȡ�ο�����\n");

}
