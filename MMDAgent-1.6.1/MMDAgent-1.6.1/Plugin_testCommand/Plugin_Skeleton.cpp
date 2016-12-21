/* definitions */
#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif /* _WIN32 */

#include <stdio.h>
#include <tchar.h>
#include "include\opencv2\opencv.hpp"
#include "include\opencv2\core\core.hpp"
#include "include\opencv2/highgui/highgui.hpp"
#include "include\opencv2/objdetect/objdetect.hpp"
#include "include\opencv2\imgproc\imgproc.hpp"

#include <iostream>
#include "MMDAgent.h"
#include <fstream>
using namespace std;

// �t���[���̃T�C�Y�ݒ�
#define WIDTH  320
#define HEIGHT 240

static bool enable;

// command & event
#define PLUGINCAMERA_NAME  "camera"
#define PLUGINCAMERA_EVENT_HANDPOINT "CAMERA_EVENT_HANDPOINT"
#define PLUGINCAMERA_EVENT_HANDSHAKE "CAMERA_EVENT_HANDSHAKE"
#define PLUGINCAMERA_HAND_CONTROL_START "CAMERA_HAND_CONTROL_START"
#define PLUGINCAMERA_HAND_CONTROL_STOP  "CAMERA_HAND_CONTROL_STOP"
#define PLUGINCAMERA_INPUT "CAMERA_INPUT"
#define PLUGINCAMERA_FACE "FACE_POSITION"


// ���check�p�e�L�X�g�t�@�C��
static ofstream ofs;

//臒l(threshold)�̐ݒ�
static const int th = 5;

//�ϐ��̏���
static bool output_flag;
static cv::Mat im, im1, im2, im3, hsv_im, hsv2_im, frame;
static cv::Mat d1, d2, diff;
static cv::Mat im_mask, mask;
static cv::Mat out, out2;
static cv::Mat face;

// �J�����L���v�`��
static cv::VideoCapture cap;
static cv::VideoWriter writer_out;
static cv::VideoWriter writer_out2;
static cv::VideoWriter writer_hsv;
static cv::VideoWriter writer_mask;
static cv::VideoWriter writer_in;
cv::VideoWriter Video_set(char *name);

static int video_count;
static bool video_flag;

// �t���[���擾�̉�
static bool cap_enable;

// �t���[���擾�̊Ԋu
#define frame_rate 18
static int frame_time;

//�w�K�ς݌��o��
static cv::CascadeClassifier cascade;

//��̍��W
vector<cv::Rect> faces;


bool Motion_Detection();
void face_detection(cv::Mat img);

// ��̂Ђ�F���Ɏg���_�̍\���̂��`
class point {
public:
	int new_x, new_y;
	int now_x, now_y;
	int num;
};

static point R, L;
static vector<point> R_recode, L_recode;

static point F;

static bool shake_hand_R, shake_hand_L;

#define R_x WIDTH*3/4
#define R_y HEIGHT/2
#define L_x WIDTH/4
#define L_y HEIGHT/2
#define D   10

// ��̂Ђ�F���Ɏg���֐�
int distans(int d) { return d * d; }
int distans_pp(int cx, int cy, int x, int y) { return (cx - x)*(cx - x) + (cy - y)*(cy - y); }

// ��̂Ђ��p�����R���g���[���Ɏg���ϐ�
static bool hand_control;
static bool controler_R, controler_L;
static bool in_left, in_right, in_up, in_down;
static int c_x, c_y;
#define c_range 20

EXPORT void extAppStart(MMDAgent *mmdagent)
{
	enable = true;
	cap_enable = true;

	// �w�K�ς݌��o��̓ǂݍ���
	string cascadeName = ".\\haarcascades\\haarcascade_frontalface_alt.xml";
	if (!cascade.load(cascadeName)) {
	}

	// �t���[���Ԋu�̌v���ϐ��̏�����
	frame_time = 0;

	// check�p�t�@�C���̃I�[�v��
	ofs.open("camera_check.txt");

	//�@�J�����̃I�[�v��
	cv::VideoCapture cap_buff(CV_CAP_DSHOW + 0);
	cap = cap_buff;
	cap_buff.release();

	// �r�f�I�̃I�[�v��
	writer_out = Video_set("video\\video_out");
	writer_out2 = Video_set("video\\video_out2");
	writer_mask = Video_set("video\\video_mask.avi");
	writer_hsv = Video_set("video\\video_hsv");
	writer_in = Video_set("video\\video_in.avi");

	video_count = 30000;
	video_flag = false;

	//�J�����̃E�B���h�E�T�C�Y��ݒ�
	cap.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT);

	// �L���v�`���擾���������܂őҋ@
	cv::Mat buff;
	do { cap >> buff; } while (buff.empty());

	// ���ꂼ��̉摜�̏�����
	frame = cv::Mat(cv::Size(WIDTH, HEIGHT), CV_8UC3);
	out = cv::Mat(cv::Size(WIDTH, HEIGHT), CV_8UC3);
	out2 = cv::Mat(cv::Size(WIDTH, HEIGHT), CV_8UC3);
	hsv_im = cv::Mat(cv::Size(WIDTH, HEIGHT), CV_8UC3);
	hsv2_im = cv::Mat(cv::Size(WIDTH, HEIGHT), CV_8UC3);
	face = cv::Mat(cv::Size(WIDTH, HEIGHT), CV_8UC3);
	im1 = cv::Mat(cv::Size(WIDTH, HEIGHT), CV_HSV2BGR);
	im2 = cv::Mat(cv::Size(WIDTH, HEIGHT), CV_HSV2BGR);
	im3 = cv::Mat(cv::Size(WIDTH, HEIGHT), CV_HSV2BGR);
	output_flag = false;

	//�J��������3�t���[�����o��
	cap >> frame;
	cv::cvtColor(frame, im1, CV_RGB2GRAY);
	cap >> frame;
	cv::cvtColor(frame, im2, CV_RGB2GRAY);
	cap >> frame;
	cv::cvtColor(frame, im3, CV_RGB2GRAY);

	// ��̂Ђ�̈ʒu�̏�����
	R.new_x = R_x; R.new_y = R_y;
	L.new_x = L_x; L.new_y = L_y;

	// ���U���Ă��邩�̔��������t���O��false�ɂ��Ă���
	shake_hand_R = false;
	shake_hand_L = false;

	// ��̂Ђ�̃R���g���[�����I�t�ɂ��Ă����Ɠ����ɃZ���^�[�|�C���g�̍��W��������
	hand_control = false;
	controler_R = false; controler_L = false;
	c_x = -1; c_y = -1;

	// plugin���g���邱�Ƃ�mmdagent�ɒʒm
	mmdagent->sendMessage(MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINCAMERA_NAME);
}

EXPORT void extProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
	if (enable == true) {
		if (MMDAgent_strequal(type, "KEY")) {
			if (MMDAgent_strequal(args, "0"))
				video_flag = true;
			else if ((MMDAgent_strequal(args, "*"))) {
				if (output_flag) {
					output_flag = false;
					cv::destroyAllWindows();
				}
				else {
					output_flag = true;
				}
			}
			else if ((MMDAgent_strequal(args, "/"))) {
				if (hand_control) {
					hand_control = false;
				}
				else {
					hand_control = true;
				}
			}
		}
		else if ((MMDAgent_strequal(type, PLUGINCAMERA_HAND_CONTROL_START))) {
			hand_control = true;
		}
		else if ((MMDAgent_strequal(type, PLUGINCAMERA_HAND_CONTROL_STOP))) {
			hand_control = false;
			controler_R = false; controler_L = false;
			c_x = -1; c_y = -1;
		}
		// value_check
		else if ((MMDAgent_strequal(type, "value_check"))) {
			ofs << "value_check : " << args << endl;
		}
	}
}

EXPORT void extAppEnd(MMDAgent *mmdagent)
{
	cv::destroyAllWindows();
	cap.release();
}

/* execUpdate: run when motion is updated */
EXPORT void extUpdate(MMDAgent *mmdagent, double deltaFrame) {
	if (cap_enable && frame_time++ >= frame_rate) {

		// �t���[���Ԋu�v���ϐ��̃��Z�b�g
		frame_time = 0;

		// Motion_Detection���d�����Ď��s����Ȃ��悤�ɂ��邽�߂Ɏ��s����false�ɂ��Ă���
		cap_enable = false;
		// Motion_Detection���I�������true��return�����
		cap_enable = Motion_Detection();

		if (shake_hand_R)  mmdagent->sendMessage(PLUGINCAMERA_EVENT_HANDSHAKE, "%s", "right_hand");
		if (shake_hand_L)  mmdagent->sendMessage(PLUGINCAMERA_EVENT_HANDSHAKE, "%s", "left_hand");

		if (hand_control) {
			if (in_left) {
				mmdagent->sendMessage(PLUGINCAMERA_INPUT, "left");
			}
			if (in_right) {
				mmdagent->sendMessage(PLUGINCAMERA_INPUT, "right");
			}
			if (in_up) {
				mmdagent->sendMessage(PLUGINCAMERA_INPUT, "up");
			}
			if (in_down) {
				mmdagent->sendMessage(PLUGINCAMERA_INPUT, "down");
			}
			if (shake_hand_R || shake_hand_L) {
				mmdagent->sendMessage(PLUGINCAMERA_INPUT, "shake");
			}
		}

		if (faces.size() == 1) {
			mmdagent->sendMessage(PLUGINCAMERA_FACE, "%d,%d", F.new_x, F.new_y);
		}
	}
}

/* execRender: run when scene is rendered */
EXPORT void extRender(MMDAgent *mmdagent) {
}

bool Motion_Detection() {
	//����1�F�t���[��1��2�̍������߂�
	cv::absdiff(im1, im2, d1);
	//����2�F�t���[��2��3�̍������߂�
	cv::absdiff(im2, im3, d2);
	//����1�ƍ���2�̌��ʂ��r(�_����)���Adiff�ɏo��
	cv::bitwise_and(d1, d2, diff);

	//����diff�̂����A臒lth�𒴂��Ă��镔����1�A����ȊO��0�Ƃ���mask�ɏo��
	cv::threshold(diff, mask, 5, 1, cv::THRESH_BINARY);

	//�}�X�Nmask�̂����A1(True)�̕�����(0)�ɁA0(False)�̕�������(255)�ɂ���im_mask�ɏo��
	cv::threshold(mask, im_mask, 0, 255, cv::THRESH_BINARY);

	//���f�B�A���t�B���^���g�����������ɂ���ăS�}���m�C�Y�������A�A�p�[�`���T�C�Y5
	cv::medianBlur(im_mask, im_mask, 5);


	// ���̌��o���ʂ�hsv��Ԃɂ�锧�F���o�̌��ʂ��A���F���̌��o
	uchar hue, BW;
	out = cv::Scalar(0, 0, 0);
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			hue = hsv2_im.at<cv::Vec3b>(y, x)[0];
			BW = im_mask.at<uchar>(y, x);
			if ((hue != 0) && (BW == 255)) {
				out.data[out.step * y + 3 * x] = 255;
			}
		}
	}

	face = cv::Scalar(0, 0, 0);

	// ��F�������ꏊ�ɋ�`��`��
	for (vector<cv::Rect>::iterator iter = faces.begin(); iter != faces.end(); iter++) {
		rectangle(frame, *iter, cv::Scalar(0, 255, 0), 1);
		rectangle(face, *iter, cv::Scalar(255, 0, 0), 1);
	}

	// ��̒��S�_��������
	if (faces.size() == 1) {
		F.new_x = 0; F.new_y = 0; F.num = 0;
		for (int y = 0; y < HEIGHT; y++) {
			for (int x = 0; x < WIDTH; x++) {
				if (face.data[face.step * y + 3 * x] == 255) {
					F.new_x += x; F.new_y += y; F.num++;
				}
			}
		}
		ofs << "Fb: " << F.new_x << ", " << F.new_y << endl;
		F.new_x /= F.num; F.new_y /= F.num;
		ofs << "Fa: " << F.new_x << ", " << F.new_y << endl;
	}

	// k_means�@(k = 2)�ŉE��A����A��̈ʒu�����o
	for (int loop = 0; loop < 5; loop++) {
		// �l�̏�����
		R.num = 0; L.num = 0;
		R.now_x = R.new_x; R.now_y = R.new_y;
		L.now_x = L.new_x; L.now_y = L.new_y;
		R.new_x = 0;       R.new_y = 0;
		L.new_x = 0;       L.new_y = 0;
		out2 = cv::Scalar(0, 0, 0); // �N���X�^�����O��̉摜
		for (int y = 0; y < HEIGHT; y++) {
			for (int x = 0; x < WIDTH; x++) {
				if (out.data[out.step * y + 3 * x] == 255) {
					if (distans_pp(R.now_x, R.now_y, x, y) < distans_pp(L.now_x, L.now_y, x, y)) {
						R.new_x += x;
						R.new_y += y;
						R.num++;
						out2.data[out.step * y + 3 * x + 0] = 255;
					}
					else {
						L.new_x += x;
						L.new_y += y;
						L.num++;
						out2.data[out.step * y + 3 * x + 1] = 255;
					}
				}
			}
		}

		// ���ꂼ��̒��S���v�Z
		if (R.num == 0) { R.new_x = R.now_x; R.new_y = R.now_y; }
		else { R.new_x /= R.num;  R.new_y /= R.num; }

		if (L.num == 0) { L.new_x = L.now_x; L.new_y = L.now_y; }
		else { L.new_x /= L.num;  L.new_y /= L.num; }
	}

	// �ʒu���߂����Č������ꂽ�Ƃ��ɏC��
	if (R.new_x < L.new_x) {
		point buff;
		buff = R;
		R = L;
		L = buff;
	}

	// ���S�_���N���X�^�����O�摜�Ɠ��͉摜�ɏo��
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			if (distans_pp(R.new_x, R.new_y, x, y) < distans(D)) {
				out2.data[out.step * y + 3 * x + 0] = 100;
				frame.data[frame.step * y + 3 * x + 0] = 255;
			}
			if (distans_pp(L.new_x, L.new_y, x, y) < distans(D)) {
				out2.data[out.step * y + 3 * x + 1] = 100;
				frame.data[frame.step * y + 3 * x + 1] = 255;
			}
		}
	}

	// �_���L�^
	R_recode.push_back(R);
	L_recode.push_back(L);

	// �_��10�܂ŋL�^����
	if (R_recode.size() == 10 + 1) {
		vector<point> buff;
		for (vector<point>::iterator it = R_recode.begin() + 1; it != R_recode.end(); it++) {
			buff.push_back(*it);
		}
		R_recode = buff;
	}
	if (L_recode.size() == 10 + 1) {
		vector<point> buff;
		for (vector<point>::iterator it = L_recode.begin() + 1; it != L_recode.end(); it++) {
			buff.push_back(*it);
		}
		L_recode = buff;
	}

	// ���U���Ă��邩�̔�����s��
	int d_R = 0;
	shake_hand_R = (R_recode.size() == 10) ? true : false;
	for (vector<point>::iterator it = R_recode.begin() + 1; it != R_recode.end(); it++) {
		if (distans_pp(R.new_x, R.new_y, it->new_x, it->new_y) > distans(25) || distans_pp((it - 1)->new_x, (it - 1)->new_y, it->new_x, it->new_y) <= distans(4)) {
			shake_hand_R = false;
		}
	}
	shake_hand_L = (L_recode.size() == 10) ? true : false;
	for (vector<point>::iterator it = L_recode.begin() + 1; it != L_recode.end(); it++) {
		if (distans_pp(L.new_x, L.new_y, it->new_x, it->new_y) > distans(25) || distans_pp((it - 1)->new_x, (it - 1)->new_y, it->new_x, it->new_y) <= distans(4)) {
			shake_hand_L = false;
		}
	}

	// ��̂Ђ�R���g���[���̃Z���^�[�|�C���g�����߂�
	if (hand_control) {
		/*
		if (shake_hand_R && shake_hand_L) { }
		else if (shake_hand_R) {c_x = R.new_x; c_y = R.new_y; controler_R = true;}
		else if (shake_hand_L) {c_x = L.new_x; c_y = L.new_y; controler_L = true;}
		*/
		c_x = WIDTH * 0.8; c_y = HEIGHT * 0.7; controler_R = true;
	}

	// ��̂Ђ�R���g���[���œ���
	if (controler_R) {
		in_right = false; in_left = false; in_up = false; in_down = false; controler_R = false;
		if (c_x - R.new_x > c_range) in_left = true;
		if (R.new_x - c_x > c_range) in_right = true;
		if (c_y - R.new_y > c_range) in_up = true;
		if (R.new_y - c_y > c_range) in_down = true;
	}
	if (controler_L) {
		in_right = false; in_left = false; in_up = false; in_down = false;
		if (c_x - L.new_x > c_range) in_left = true;
		if (L.new_x - c_x > c_range) in_right = true;
		if (c_y - L.new_y > c_range) in_up = true;
		if (L.new_y - c_y > c_range) in_down = true;
	}

	if (hand_control) {
		for (int y = 0; y < HEIGHT; y++) {
			for (int x = 0; x < WIDTH; x++) {
				if (distans_pp(c_x, c_y, x, y) < distans(c_range)) {
					out2.data[out.step * y + 3 * x + 2] = 255;
				}
			}
		}
	}

	for (vector<point>::iterator it = R_recode.begin() + 1; it != R_recode.end(); it++) {
		cv::line(out2, cv::Point(it->new_x, it->new_y), cv::Point((it - 1)->new_x, (it - 1)->new_y), cv::Scalar(255, 100, 100), 2);
	}

	for (vector<point>::iterator it = L_recode.begin() + 1; it != L_recode.end(); it++) {
		cv::line(out2, cv::Point(it->new_x, it->new_y), cv::Point((it - 1)->new_x, (it - 1)->new_y), cv::Scalar(100, 255, 100), 2);
	}

	// �摜�̉�ʏo��
	if (output_flag) {
		cv::imshow("in", frame);
		cv::imshow("out", out);
		cv::imshow("out2", out2);
		cv::imshow("hsv", hsv2_im);
		cv::imshow("im_mask", im_mask);
	}

	// �r�f�I�^��
	if (video_flag) {
		writer_in << frame;
		writer_out << out;
		writer_out2 << out2;
		writer_mask << im_mask;
		writer_hsv << hsv2_im;
		video_count--;
	}
	if (video_count == 0) {
		video_flag = false;
		video_count = 10;
	}

	//�V�����t���[�����J�����������o���A3�̃t���[����S�Ă��炷
	im2.copyTo(im1, im2);
	im3.copyTo(im2, im3);
	cap >> frame;
	cv::flip(frame, frame, 1);


	cv::cvtColor(frame, im3, CV_RGB2GRAY);
	face_detection(im3);

	cv::cvtColor(frame, hsv_im, CV_RGB2HSV);
	cv::medianBlur(hsv_im, hsv_im, 7);

	// hsv��ԂŔ��F���o
	hsv2_im = cv::Scalar(0, 0, 0);
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			hue = hsv_im.at<cv::Vec3b>(y, x)[0];
			if (((hue >= 100) && (hue <= 130))) {
				hsv2_im.at<cv::Vec3b>(y, x) = hsv_im.at<cv::Vec3b>(y, x);
			}
		}
	}
	cv::medianBlur(hsv2_im, hsv2_im, 7);


	return true;
}

void face_detection(cv::Mat img) {
	// �猟�o�����s
	cascade.detectMultiScale(
		img,   // �摜
		faces,       // �o�͂�����`
		1.1,         // �k���p�̃X�P�[��
		2,           // �ŏ��̓��[��
		CV_HAAR_SCALE_IMAGE,  // �t���O
		cv::Size(30, 30) // �ŏ��̋�`
		);
}

cv::VideoWriter Video_set(char *name) {
	cv::VideoWriter writer_buff(name, CV_FOURCC_DEFAULT, 10, cv::Size(WIDTH, HEIGHT), true);
	return writer_buff;
	writer_buff.release();
}