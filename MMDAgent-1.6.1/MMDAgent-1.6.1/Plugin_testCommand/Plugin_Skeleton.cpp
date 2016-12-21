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

// フレームのサイズ設定
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


// 作業check用テキストファイル
static ofstream ofs;

//閾値(threshold)の設定
static const int th = 5;

//変数の準備
static bool output_flag;
static cv::Mat im, im1, im2, im3, hsv_im, hsv2_im, frame;
static cv::Mat d1, d2, diff;
static cv::Mat im_mask, mask;
static cv::Mat out, out2;
static cv::Mat face;

// カメラキャプチャ
static cv::VideoCapture cap;
static cv::VideoWriter writer_out;
static cv::VideoWriter writer_out2;
static cv::VideoWriter writer_hsv;
static cv::VideoWriter writer_mask;
static cv::VideoWriter writer_in;
cv::VideoWriter Video_set(char *name);

static int video_count;
static bool video_flag;

// フレーム取得の可否
static bool cap_enable;

// フレーム取得の間隔
#define frame_rate 18
static int frame_time;

//学習済み検出器
static cv::CascadeClassifier cascade;

//顔の座標
vector<cv::Rect> faces;


bool Motion_Detection();
void face_detection(cv::Mat img);

// 手のひら認識に使う点の構造体を定義
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

// 手のひら認識に使う関数
int distans(int d) { return d * d; }
int distans_pp(int cx, int cy, int x, int y) { return (cx - x)*(cx - x) + (cy - y)*(cy - y); }

// 手のひらを用いたコントロールに使う変数
static bool hand_control;
static bool controler_R, controler_L;
static bool in_left, in_right, in_up, in_down;
static int c_x, c_y;
#define c_range 20

EXPORT void extAppStart(MMDAgent *mmdagent)
{
	enable = true;
	cap_enable = true;

	// 学習済み検出器の読み込み
	string cascadeName = ".\\haarcascades\\haarcascade_frontalface_alt.xml";
	if (!cascade.load(cascadeName)) {
	}

	// フレーム間隔の計測変数の初期化
	frame_time = 0;

	// check用ファイルのオープン
	ofs.open("camera_check.txt");

	//　カメラのオープン
	cv::VideoCapture cap_buff(CV_CAP_DSHOW + 0);
	cap = cap_buff;
	cap_buff.release();

	// ビデオのオープン
	writer_out = Video_set("video\\video_out");
	writer_out2 = Video_set("video\\video_out2");
	writer_mask = Video_set("video\\video_mask.avi");
	writer_hsv = Video_set("video\\video_hsv");
	writer_in = Video_set("video\\video_in.avi");

	video_count = 30000;
	video_flag = false;

	//カメラのウィンドウサイズを設定
	cap.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT);

	// キャプチャ取得準備完了まで待機
	cv::Mat buff;
	do { cap >> buff; } while (buff.empty());

	// それぞれの画像の初期化
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

	//カメラから3フレーム取り出す
	cap >> frame;
	cv::cvtColor(frame, im1, CV_RGB2GRAY);
	cap >> frame;
	cv::cvtColor(frame, im2, CV_RGB2GRAY);
	cap >> frame;
	cv::cvtColor(frame, im3, CV_RGB2GRAY);

	// 手のひらの位置の初期化
	R.new_x = R_x; R.new_y = R_y;
	L.new_x = L_x; L.new_y = L_y;

	// 手を振っているかの判定をするフラグをfalseにしておく
	shake_hand_R = false;
	shake_hand_L = false;

	// 手のひらのコントロールをオフにしておくと同時にセンターポイントの座標を初期化
	hand_control = false;
	controler_R = false; controler_L = false;
	c_x = -1; c_y = -1;

	// pluginが使えることをmmdagentに通知
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

		// フレーム間隔計測変数のリセット
		frame_time = 0;

		// Motion_Detectionが重複して実行されないようにするために実行中はfalseにしておく
		cap_enable = false;
		// Motion_Detectionが終了するとtrueがreturnされる
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
	//差分1：フレーム1と2の差を求める
	cv::absdiff(im1, im2, d1);
	//差分2：フレーム2と3の差を求める
	cv::absdiff(im2, im3, d2);
	//差分1と差分2の結果を比較(論理積)し、diffに出力
	cv::bitwise_and(d1, d2, diff);

	//差分diffのうち、閾値thを超えている部分を1、それ以外を0としてmaskに出力
	cv::threshold(diff, mask, 5, 1, cv::THRESH_BINARY);

	//マスクmaskのうち、1(True)の部分を白(0)に、0(False)の部分を黒(255)にしてim_maskに出力
	cv::threshold(mask, im_mask, 0, 255, cv::THRESH_BINARY);

	//メディアンフィルタを使った平滑化によってゴマ塩ノイズを除去、アパーチャサイズ5
	cv::medianBlur(im_mask, im_mask, 5);


	// 動体検出結果とhsv空間による肌色検出の結果より、肌色動体検出
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

	// 顔認識した場所に矩形を描く
	for (vector<cv::Rect>::iterator iter = faces.begin(); iter != faces.end(); iter++) {
		rectangle(frame, *iter, cv::Scalar(0, 255, 0), 1);
		rectangle(face, *iter, cv::Scalar(255, 0, 0), 1);
	}

	// 顔の中心点を見つける
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

	// k_means法(k = 2)で右手、左手、顔の位置を検出
	for (int loop = 0; loop < 5; loop++) {
		// 値の初期化
		R.num = 0; L.num = 0;
		R.now_x = R.new_x; R.now_y = R.new_y;
		L.now_x = L.new_x; L.now_y = L.new_y;
		R.new_x = 0;       R.new_y = 0;
		L.new_x = 0;       L.new_y = 0;
		out2 = cv::Scalar(0, 0, 0); // クラスタリング後の画像
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

		// それぞれの中心を計算
		if (R.num == 0) { R.new_x = R.now_x; R.new_y = R.now_y; }
		else { R.new_x /= R.num;  R.new_y /= R.num; }

		if (L.num == 0) { L.new_x = L.now_x; L.new_y = L.now_y; }
		else { L.new_x /= L.num;  L.new_y /= L.num; }
	}

	// 位置が近すぎて交換されたときに修正
	if (R.new_x < L.new_x) {
		point buff;
		buff = R;
		R = L;
		L = buff;
	}

	// 中心点をクラスタリング画像と入力画像に出力
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

	// 点を記録
	R_recode.push_back(R);
	L_recode.push_back(L);

	// 点は10個まで記録する
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

	// 手を振っているかの判定を行う
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

	// 手のひらコントロールのセンターポイントを決める
	if (hand_control) {
		/*
		if (shake_hand_R && shake_hand_L) { }
		else if (shake_hand_R) {c_x = R.new_x; c_y = R.new_y; controler_R = true;}
		else if (shake_hand_L) {c_x = L.new_x; c_y = L.new_y; controler_L = true;}
		*/
		c_x = WIDTH * 0.8; c_y = HEIGHT * 0.7; controler_R = true;
	}

	// 手のひらコントロールで入力
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

	// 画像の画面出力
	if (output_flag) {
		cv::imshow("in", frame);
		cv::imshow("out", out);
		cv::imshow("out2", out2);
		cv::imshow("hsv", hsv2_im);
		cv::imshow("im_mask", im_mask);
	}

	// ビデオ録画
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

	//新しいフレームをカメラから一つ取り出し、3つのフレームを全てずらす
	im2.copyTo(im1, im2);
	im3.copyTo(im2, im3);
	cap >> frame;
	cv::flip(frame, frame, 1);


	cv::cvtColor(frame, im3, CV_RGB2GRAY);
	face_detection(im3);

	cv::cvtColor(frame, hsv_im, CV_RGB2HSV);
	cv::medianBlur(hsv_im, hsv_im, 7);

	// hsv空間で肌色検出
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
	// 顔検出を実行
	cascade.detectMultiScale(
		img,   // 画像
		faces,       // 出力される矩形
		1.1,         // 縮小用のスケール
		2,           // 最小の投票数
		CV_HAAR_SCALE_IMAGE,  // フラグ
		cv::Size(30, 30) // 最小の矩形
		);
}

cv::VideoWriter Video_set(char *name) {
	cv::VideoWriter writer_buff(name, CV_FOURCC_DEFAULT, 10, cv::Size(WIDTH, HEIGHT), true);
	return writer_buff;
	writer_buff.release();
}