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
	char state;								//Ready�� R /Sent�� S/Acked�� A
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

	int sender[BUFFER_SIZE] = { 0 };												//��ٸ��� ACK�� �ǹ��ϰ� �װ��� �ٷ� ���� ���̰���?
	int receiver[BUFFER_SIZE] = { 0 };												//���� ACK�� �ǹ��ϰ� �װ��� �ڱⰪ�̰���?
	wndw wndw_sender[WNDW_SIZE];													//window size�� 4�� ���ش�.
	wndw wndw_receiver[WNDW_SIZE];						
	int timer = 0;


	for (int i = 0; i < 16; i++) {
		data[i] = rand()%16;
	}
	for (int i = 0; i < 16; i++) {														//sender�� codeword��ȯ
		codeword[i] = data[i] * 8 + crc(data[i],0);	
	}
	for (int i = 0; i < BUFFER_SIZE; i++) {												//sender�� codeword��ȯ
		sender[i] = codeword[i];
	}
	showcrc(data, codeword);															//==============================================CRC �������ִ� �κ��Դϴ�=====================//

	//�ʱ�ȭ���·�
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
		//���͸� ���������� �ð��� �����ٰ� �����Ѵ� ������ �̷��� ���ص� sleep�ָ� �����ϴ�

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
			if (wndw_sender[i].state == 'S' && wndw_sender[i].time < TIMEOUT)		//�켱 6������ ���ڸ� �ø���
				wndw_sender[i].time++;
			else if (wndw_sender[i].state == 'S' && wndw_sender[i].time == TIMEOUT)
				wndw_sender[i].time = 1;
		}

		for (int i = 0; i < WNDW_SIZE; i++) {
			if (wndw_sender[i].time == 7) {
				cout << wndw_sender[i].buf_num << "��° ������ ������ �ֳ���?(Y/N) : ";
				cin >> answer;
				if (answer == 'Y' || answer == 'y') {
					sender[wndw_sender[i].buf_num]++;														//�����ִ� �κ�
					wndw_sender[i].time = 18;																
					//���������� seder�� Ÿ�̸Ӹ� ���߾� �ش�. Time�� Ȯ �ٰ����ش�. �׷��� �ؼ� One way ���� 7�̹Ƿ� �� 9������ 27���� Ÿ�Ӿƿ��� �ɰ��̸� �׸��Ͽ� ������ ��ų���̴�.
					sender[wndw_sender[i].buf_num]--;												//���󺹱����ַ���
				}
				else if (!crc(sender[wndw_sender[i].buf_num] / 8, sender[wndw_sender[i].buf_num] % 8))			//CRCȮ���Ͽ� 0���� ������ ������ ������ �߻���Ų������!!
				{
					for (int j = 0;j<WNDW_SIZE;j++)
						if(wndw_sender[i].buf_num==wndw_receiver[j].buf_num)
							wndw_receiver[j].state = 'S';
				}
			}
			if (wndw_sender[i].time == 14) {															//14��� �ǹ̴� ���ƿԴٴ� �ǹ��̴�
				wndw_sender[i].state = 'A';
			}
		}
	}


	return 0;

}


void wndw_sender_check(wndw wndw_sender[]) {
	
	char answer = '\0';

	for (int i = 0; i < WNDW_SIZE; i++) {				// window�� R�� �Ȱ��� ã�´�.
		if (wndw_sender[i].state == 'R' && (wndw_sender[i].buf_num<BUFFER_SIZE)) {				// ������ ������ ������ �Ѵ�.
			cout << wndw_sender[i].buf_num << "��° �����͸� ���� �غ� �Ǿ�����?(Y/N) : ";
			cin >> answer;
			if (answer == 'Y' || answer == 'y')
				wndw_sender[i].state = 'S';
		}
	}
	for (int j = 0;j<4;j++)
		if (wndw_sender[0].state == 'A'){				// Ack���� ������ �Ǿ��� Ack�� �Ǿ����� Ȯ���غ��� �����̵��� �ʿ䰡 �ִ�.
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
		if (wndw_receiver[0].state == 'S') {				// Ack���� ������ �Ǿ��� Ack�� �Ǿ����� Ȯ���غ��� �����̵��� �ʿ䰡 �ִ�.
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



//�̰� 1 ���ºκ�
	for (int j = 1; j <= 6; j++) {
		cout << "\t";
		for (int i = 0; i < BUFFER_SIZE; i++) {
			for (int k = 0; k < WNDW_SIZE; k++) {
				if (wndw_sender[k].time == j && wndw_sender[k].buf_num == i)
					cout << "��";
				else if((14-wndw_sender[k].time == j)&& wndw_sender[k].buf_num == i)
					cout << "��";
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




int crc(int data, int remainder_int)    //crc �Լ�
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
	remainder_int = 0;						//���߿� �������ַ���

											//for (int i = 0; i<7; i++)					//Driver
											//cout << data_for_div[i];

	for (int i = 0; i < 4; i++) {						//����� �����ִ� Ƚ��
		if (data_for_div[i] == 0)
			continue;
		for (int j = 0; j < 4; j++) {				//XOR����
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
		cout << i << "��°�� ������ : " << data[i] << endl;					//``````
	}

	cout << "���� ���ڽ��ϱ� ? (Y / N) :";
	cin >> answer;

	for (int i = 0; i < 16; i++) {									//sender�� codeword��ȯ
		cout << i << "��°�� �ڵ���� : " << codeword[i] << " / " << i << "��°�� ������ : " << codeword[i] / 8 << " / " << i << "��°�� Reminder : " << codeword[i] % 8 << endl;
	}

	if (answer == 'N' || answer == 'n') {
		for (int i = 0; i < 16; i++) {									//sender�� codeword��ȯ
			cout << i << "��°�� CRC(0�̸� ������ / �ٸ� ���̸� ���������� : " << crc(codeword[i] / 8, codeword[i] % 8) << endl;
		}
	}
	else {
		for (int i = 0; i < 16; i++) {									//sender�� codeword��ȯ
			codeword[i]++;
		}
		for (int i = 0; i < 16; i++) {									//sender�� codeword��ȯ
			cout << i << "��°�� CRC(0�̸� ������ / �ٸ� ���̸� ���������� : " << crc(codeword[i] / 8, codeword[i] % 8) << endl;
		}
	}
}
