#include <iostream>
#include <windows.h>
#include <time.h>
#include <iomanip>
using namespace std;

#define TIMEOUT 27
#define WNDW_SIZE 4
#define BUFFER_SIZE 16


struct wndw {
	int buf_num;
	int time;
	char state;								//Ready의 R /Sent의 S/Acked의 A
	int errorFlag = false;
};

int crc(int data, int remainder_int);
void wndw_sender_check(wndw wndw_sender[]);
void wndw_receiver_check(wndw wndw_receiver[]);
void print_display(wndw wndw_sender[], wndw wndw_receiver[]);
void showcrc(int data[], int codeword[]);

int main() {
	srand((unsigned)time(NULL));
	int data[16];
	int codeword[16]; //data+divisor

	int sending_Cnt = 0;
	char answer = '\0';

	int sender[BUFFER_SIZE] = { 0 };												//기다리는 ACK를 의미하고 그것은 바로 뒤의 값이겟지?
	int receiver[BUFFER_SIZE] = { 0 };												//보낼 ACK를 의미하고 그것은 자기값이겟지?
	wndw wndw_sender[WNDW_SIZE];													//window size는 4로 해준다.
	wndw wndw_receiver[WNDW_SIZE];						
	int timer = 0;


	for (int i = 0; i < 16; i++) {
		data[i] = rand()%16;
	}
	for (int i = 0; i < 16; i++) {														//sender측 codeword변환
		codeword[i] = data[i] * 8 + crc(data[i],0);	
	}
	for (int i = 0; i < BUFFER_SIZE; i++) {												//sender측 codeword변환
		sender[i] = codeword[i];
	}
	showcrc(data, codeword);															//==============================================CRC 증명해주는 부분입니다=====================//

	//초기화상태로
	for (int i = 0; i < WNDW_SIZE; i++) {
		wndw_sender[i].buf_num = i;
		wndw_sender[i].state = 'R';
		wndw_sender[i].time = 0;
	}
	for (int i = 0; i < WNDW_SIZE; i++) {
		wndw_receiver[i].buf_num = i;
		wndw_receiver[i].state = 'R';
	}





	while (1){
		//엔터를 누를때마다 시간이 지난다고 가정한다 하지만 이렇게 안해도 sleep주면 가능하다

	/*	if (cin.peek() == '\n')
			cin.ignore();
		else
			cin.ignore();*/

		Sleep(800);
		system("cls");


		print_display(wndw_sender, wndw_receiver);


		wndw_sender_check(wndw_sender);
		wndw_receiver_check(wndw_receiver);
		
		for (int i = 0; i < WNDW_SIZE; i++) {
			if (wndw_sender[i].state == 'S' && wndw_sender[i].time < TIMEOUT)		//우선 6까지는 숫자를 올리고
				wndw_sender[i].time++;
			else if (wndw_sender[i].state == 'S' && wndw_sender[i].time == TIMEOUT)
				wndw_sender[i].time = 1;
		}

		for (int i = 0; i < WNDW_SIZE; i++) {
			if (wndw_sender[i].time == 7) {
				cout << wndw_sender[i].buf_num << "번째 데이터 오류가 있나요?(Y/N) : ";
				cin >> answer;
				if (answer == 'Y' || answer == 'y') {
					sender[wndw_sender[i].buf_num]++;														//보여주는 부분
					wndw_sender[i].time = 18;																
					//에러난것은 seder쪽 타이머를 멈추어 준다. Time을 확 뛰게해준다. 그렇게 해서 One way 값이 7이므로 약 9정도인 27에서 타임아웃을 걸것이며 그리하여 리센딩 시킬것이다.
					sender[wndw_sender[i].buf_num]--;												//원상복귀해주려고
				}
				else if (!crc(sender[wndw_sender[i].buf_num] / 8, sender[wndw_sender[i].buf_num] % 8))			//CRC확인하여 0값이 나오지 않으면 문제를 발생시킨것이지!!
				{
					for (int j = 0;j<WNDW_SIZE;j++)
						if(wndw_sender[i].buf_num==wndw_receiver[j].buf_num)
							wndw_receiver[j].state = 'S';
				}
			}
			if (wndw_sender[i].time == 14) {															//14라는 의미는 돌아왔다는 의미이다
				wndw_sender[i].state = 'A';
			}
		}
	}


	return 0;

}


void wndw_sender_check(wndw wndw_sender[]) {
	
	char answer = '\0';

	for (int i = 0; i < WNDW_SIZE; i++) {				// window가 R가 된것을 찾는다.
		if (wndw_sender[i].state == 'R' && (wndw_sender[i].buf_num<BUFFER_SIZE)) {				// 데이터 보내는 결정을 한다.
			cout << wndw_sender[i].buf_num << "번째 데이터를 보낼 준비가 되었나요?(Y/N) : ";
			cin >> answer;
			if (answer == 'Y' || answer == 'y')
				wndw_sender[i].state = 'S';
		}
	}
	for (int j = 0;j<4;j++)
		if (wndw_sender[0].state == 'A'){				// Ack까지 받으면 맨앞의 Ack가 되었는지 확인해보고 슬라이딩할 필요가 있다.
			for (int i = 0; i < WNDW_SIZE; i++) {
				if (i == WNDW_SIZE - 1) {
					wndw_sender[i].buf_num++;
					wndw_sender[i].state = 'R';
					wndw_sender[i].time = 0;
					break;
				}
				wndw_sender[i] = wndw_sender[i+1];
			}
		}
}

void wndw_receiver_check(wndw wndw_receiver[]) {
	char answer = '\0';
	for (int j = 0; j < 4; j++) {
		if (wndw_receiver[0].state == 'S') {				// Ack까지 받으면 맨앞의 Ack가 되었는지 확인해보고 슬라이딩할 필요가 있다.
			for (int i = 0; i < WNDW_SIZE; i++) {
				if (i == WNDW_SIZE - 1) {
					wndw_receiver[i].buf_num++;
					wndw_receiver[i].state = 'R';
					break;
				}
				wndw_receiver[i] = wndw_receiver[i + 1];
			}
		}
	}
}




void print_display(wndw wndw_sender[], wndw wndw_receiver[]) {

	cout << "wndSnd:\t";
	for (int i = 0; i < BUFFER_SIZE; i++) {
		for (int j = 0; j < WNDW_SIZE; j++)
			if (wndw_sender[j].buf_num == i)
				cout << '[' << wndw_sender[j].state << ']';
		cout << "\t";
	}
	cout << endl;

	cout << "Snd:\t";
	for (int i = 0; i < BUFFER_SIZE; i++)
		cout << i << "\t";
	cout << endl;



//이건 1 찍어내는부분
	for (int j = 1; j <= 6; j++) {
		cout << "\t";
		for (int i = 0; i < BUFFER_SIZE; i++) {
			for (int k = 0; k < WNDW_SIZE; k++) {
				if (wndw_sender[k].time == j && wndw_sender[k].buf_num == i)
					cout << "↓";
				else if((14-wndw_sender[k].time == j)&& wndw_sender[k].buf_num == i)
					cout << "↑";
			}
				cout << "\t";
		}
		cout << endl;
	}




	cout << "Rcv:\t";
	for (int i = 0; i < BUFFER_SIZE; i++)
		cout << i << "\t";
	cout << endl;

	
	cout << "wndRcv:\t";
	for (int i = 0; i < BUFFER_SIZE; i++) {
		for (int j = 0; j < WNDW_SIZE; j++) {
			if (wndw_receiver[j].buf_num == i)
				cout << '[' << wndw_receiver[j].state << ']';
		}
		cout << "\t";
	}
	cout << endl;
}




int crc(int data, int remainder_int)    //crc 함수
{
	int divisor[4] = { 1,0,0,1 };			//Divisor = 9;
	int data_for_div[7] = { 0 };

	for (int i = 0; data > 0; i++) {
		data_for_div[3 - i] = data % 2;
		data = data / 2;
	}
	for (int i = 0; remainder_int > 0; i++) {
		data_for_div[6 - i] = remainder_int % 2;
		remainder_int = remainder_int / 2;
	}
	remainder_int = 0;						//나중에 리턴해주려고

											//for (int i = 0; i<7; i++)					//Driver
											//cout << data_for_div[i];

	for (int i = 0; i < 4; i++) {						//요놈은 나눠주는 횟수
		if (data_for_div[i] == 0)
			continue;
		for (int j = 0; j < 4; j++) {				//XOR연산
			if (data_for_div[i + j] != divisor[j])
				data_for_div[i + j] = 1;
			else
				data_for_div[i + j] = 0;
		}
	}

	for (int i = 0; i < 3; i++)
		remainder_int += data_for_div[4 + i] * (int)pow(2, 2 - i);
	//cout << endl;

	return remainder_int;
}
void showcrc(int data[], int codeword[]) {
	char answer;

	for (int i = 0; i < 16; i++) {
		cout << i << "번째의 데이터 : " << data[i] << endl;					//``````
	}

	cout << "오류 내겠습니까 ? (Y / N) :";
	cin >> answer;

	for (int i = 0; i < 16; i++) {									//sender측 codeword변환
		cout << i << "번째의 코드워드 : " << codeword[i] << " / " << i << "번째의 데이터 : " << codeword[i] / 8 << " / " << i << "번째의 Reminder : " << codeword[i] % 8 << endl;
	}

	if (answer == 'N' || answer == 'n') {
		for (int i = 0; i < 16; i++) {									//sender측 codeword변환
			cout << i << "번째의 CRC(0이면 오케이 / 다른 값이면 오류난거지 : " << crc(codeword[i] / 8, codeword[i] % 8) << endl;
		}
	}
	else {
		for (int i = 0; i < 16; i++) {									//sender측 codeword변환
			codeword[i]++;
		}
		for (int i = 0; i < 16; i++) {									//sender측 codeword변환
			cout << i << "번째의 CRC(0이면 오케이 / 다른 값이면 오류난거지 : " << crc(codeword[i] / 8, codeword[i] % 8) << endl;
		}
	}
}
