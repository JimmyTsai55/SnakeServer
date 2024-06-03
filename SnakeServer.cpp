#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define NUM 1024
int count;   //�p�⦳�X�ӫȤ�ݳs�W�D��
SOCKET cSocket[NUM];
//�M�Ȥ�ݳq�T���h�u�{���
void communication(int index);   //index �� �Ȥ�ݪ��U��
int main() {
	count = 0;
	//1. �T�{��ĳ����
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		printf("��ĳ�������OV2.2!\n");
		//9. �M�z��ĳ�����T��
		WSACleanup();
		return -1;
	}
	printf("��ĳ������V2.2!\n");
	//2. �Ы�SOCKET
	SOCKET sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SOCKET_ERROR == sSocket) {
		printf("�Ы�SOCKET����: %d\n", GetLastError());
		return -2;
	}
	printf("�Ы�SOCKET���\\n");
	//3. �T�wSERVER����ĳ�a�}
	SOCKADDR_IN addr = { 0 };
	addr.sin_family = AF_INET;							//�PSOCKET��ƲĤ@�ӰѼƫO���@��
	//addr.sin_addr.S_un.S_addr = inet_pton("192.168.1.108"); //���aIP�a�}
	int status = inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_port = htons(9527);					//�j�p���ഫ  �b10000���k
	//4. �j�w
	int r = bind(sSocket, (sockaddr*)&addr, sizeof addr);
	if (r == -1) {
		printf("�j�w����: %d\n", GetLastError());
		//8. �_�}�s��
		closesocket(sSocket);
		//9. �M�z��ĳ�����T��
		WSACleanup();
	}
	printf("�j�w���\\n");
	//5. ��ť
	r = listen(sSocket, 10);
	if (r == -1) {
		printf("��ť����: %d\n", GetLastError());
		//8. �_�}�s��
		closesocket(sSocket);
		//9. �M�z��ĳ�����T��
		WSACleanup();
		return -2;
	}
	printf("��ť���\\n");
	//6. ���ݫȤ�ݳs��
	for (int i = 0; i < NUM; i++) {
		cSocket[i] = accept(sSocket, NULL, NULL);
		if (SOCKET_ERROR == cSocket[i]) {
			printf("Server Crush: %d\n", GetLastError());
			//8. �_�}�s��
			closesocket(sSocket);
			//9. �M�z��ĳ�����T��
			WSACleanup();
			return -3;
		}
		printf("���Ȥ�ݳs�����\: %d!\n", i);
		count++;     //���@�ӫȤ�ݦ��\�s�� �Ȥ�ݼƶq+1
		//�Ыؤ@�Ӧh�u�{���Ȥ�ݳq�T
		CreateThread(NULL, NULL,
			(LPTHREAD_START_ROUTINE)communication,   //�I�s�h�u�{�禡�å�LP���૬
			(LPVOID)i,
			NULL, NULL);
	}

	while (1);
	return 0;
}

//�M�Ȥ�ݳq�T���h�u�{���  
void communication(int index) {
	//7. �q�T
	char buff[256] = { 0 };
	int r;
	char temp[256];
	while (1) {
		r = recv(cSocket[index], buff, 255, NULL);  //�q�Ȥ�ݱ����ƾ�
		if (r > 0) {
			buff[r] = 0;   // �K�[�����Ÿ�
			printf(">>%d: %s\n", index, buff);     // ��X��Server�ݤW��

			//Server������H��A�o�e���Ҧ��s�WServer���Ȥ��
			memset(temp, 0, sizeof(temp));   //�M�Ű}�C
			snprintf(temp, sizeof(temp), "%d: %s", index, buff);
			for (int i = 0; i < count; i++) {
				send(cSocket[i], temp, (strlen(temp)), NULL);
			}
		}
	}
}

