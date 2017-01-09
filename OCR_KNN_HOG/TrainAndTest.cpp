#include <iostream>
#include <opencv.hpp>
#include <opencv2\ml\ml.hpp>

using namespace cv;
using namespace std;

Mat matHandWriting;
Point clickPoint;
bool boolRecognize = true;

void on_Mouse(int event, int x, int y, int flags, void*);

void helpText()
{
	cout << "������ʾ��" << endl;
	cout << "\t�ڴ���д���֣�д��󰴡��ո����ʶ�𣬿���̨���ʶ����" << endl;
	cout << "\t�ٴΰ����ո���������д���ݣ�Ȼ�������һ����д\n" << endl;
	cout << "ʶ������";
}

int main()
{
    FileStorage fsClassifications("classifications.xml", FileStorage::READ);  // ��ȡ classifications.xml �����ļ�

    if (fsClassifications.isOpened() == false) {
        cout << "ERROR, �޷���classifications.xml\n\n";
		system("pause");
        return 0;
    }

	Mat matClassificationInts;
    fsClassifications["classifications"] >> matClassificationInts;  // �� classifications.xml �е� classifications ��ȡ��Mat����
    fsClassifications.release();  // �ر��ļ�

    FileStorage fsTrainingImages("images.xml", FileStorage::READ);  // ��ѵ��ͼƬ�ļ�

    if (fsTrainingImages.isOpened() == false) {
        cout << "ERROR, �޷���images.xml\n\n";
		system("pause");
        return 0;
    }

	// ��ȡѵ��ͼƬ���ݣ���images.xml�У�
    Mat matTrainingImagesAsFlattenedFloats;  // we will read multiple images into this single image variable as though it is a vector
    fsTrainingImages["images"] >> matTrainingImagesAsFlattenedFloats;  // �� images.xml �е� images ��ȡ��Mat����
    fsTrainingImages.release();

	// ѵ��
    Ptr<ml::KNearest> kNearest(ml::KNearest::create());  // ʵ���� KNN ����

	// ���յ���train������ע�⵽������������Mat���ͣ�����Mat��������ʵ�������Ƕ��Ƕ���ͼƬ������
    kNearest->train(matTrainingImagesAsFlattenedFloats, ml::ROW_SAMPLE, matClassificationInts);

    // ����
	matHandWriting.create(Size(160, 160), CV_8UC1);
	matHandWriting = Scalar::all(0);
	imshow("��д����", matHandWriting);
	setMouseCallback("��д����", on_Mouse, 0);

	helpText();
	///////////////////////////////////////////////////////////////////////////////
	while (true)
	{
		int key = waitKey(0);
		if (key == 32 && boolRecognize)
		{
			boolRecognize = false;

			Mat matROICopy = matHandWriting.clone();
			vector<vector<Point>> contours;
			vector<Vec4i> hierarchy;
			findContours(matHandWriting, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
			Rect boundRect = boundingRect(contours[0]);
			Mat numImage = matROICopy(boundRect).clone();
			resize(numImage, numImage, Size(40, 40));

			/* �������Ŀ��ͼ���HOG�����������ݶ�ֱ��ͼ����������֮ǰ�õĻҶ����� */
			HOGDescriptor *hog = new HOGDescriptor(Size(40, 40), Size(16, 16), Size(8, 8), Size(8, 8), 9);
			vector<float> descriptors;
			hog->compute(numImage, descriptors);
			Mat matROIFlattenedFloat(1, (int)(descriptors.size()), CV_32FC1, descriptors.data());

			Mat matCurrentChar(0, 0, CV_32F);  // findNearest�Ľ������������
			
			// ���յ��� findNearest ����
			kNearest->findNearest(matROIFlattenedFloat, 1, matCurrentChar);

			//float fltCurrentChar = (float)matCurrentChar.at<float>(0, 0);
			int intCurrentChar = (int)matCurrentChar.at<float>(0, 0);
			cout << intCurrentChar << "  ";
		}
		else if (key == 32 && !boolRecognize)
		{
			boolRecognize = true;
			matHandWriting = Scalar::all(0);
			imshow("��д����", matHandWriting);
		}
		else if (key == 27)
			break;
	} 

	return 0;
}

void on_Mouse(int event, int x, int y, int flags, void*)
{
	// �����겻�ڴ������򷵻�
	if (x < 0 || x >= matHandWriting.cols || y < 0 || y >= matHandWriting.rows)
		return;

	// ��������������£���ȡ��굱ǰλ�ã������������²����ƶ�ʱ�����ư��ߣ�
	if (event == EVENT_LBUTTONDOWN)
	{
		clickPoint = Point(x, y);
	}
	else if (event == EVENT_MOUSEMOVE && (flags & EVENT_FLAG_LBUTTON))
	{
		Point point(x, y);
		line(matHandWriting, clickPoint, point, Scalar::all(255), 12, 8, 0);
		clickPoint = point;
		imshow("��д����", matHandWriting);
	}
}
