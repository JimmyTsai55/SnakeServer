#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define NUM 1024
int count;   //計算有幾個客戶端連上主機
SOCKET cSocket[NUM];
//和客戶端通訊的多線程函數
void communication(int index);   //index 為 客戶端的下標
int main() {
	count = 0;
	//1. 確認協議版本
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		printf("協議版本不是V2.2!\n");
		//9. 清理協議版本訊息
		WSACleanup();
		return -1;
	}
	printf("協議版本為V2.2!\n");
	//2. 創建SOCKET
	SOCKET sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SOCKET_ERROR == sSocket) {
		printf("創建SOCKET失敗: %d\n", GetLastError());
		return -2;
	}
	printf("創建SOCKET成功\n");
	//3. 確定SERVER的協議地址
	SOCKADDR_IN addr = { 0 };
	addr.sin_family = AF_INET;							//與SOCKET函數第一個參數保持一樣
	//addr.sin_addr.S_un.S_addr = inet_pton("192.168.1.108"); //本地IP地址
	int status = inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_port = htons(9527);					//大小端轉換  在10000左右
	//4. 綁定
	int r = bind(sSocket, (sockaddr*)&addr, sizeof addr);
	if (r == -1) {
		printf("綁定失敗: %d\n", GetLastError());
		//8. 斷開連結
		closesocket(sSocket);
		//9. 清理協議版本訊息
		WSACleanup();
	}
	printf("綁定成功\n");
	//5. 監聽
	r = listen(sSocket, 10);
	if (r == -1) {
		printf("監聽失敗: %d\n", GetLastError());
		//8. 斷開連結
		closesocket(sSocket);
		//9. 清理協議版本訊息
		WSACleanup();
		return -2;
	}
	printf("監聽成功\n");
	//6. 等待客戶端連接
	for (int i = 0; i < NUM; i++) {
		cSocket[i] = accept(sSocket, NULL, NULL);
		if (SOCKET_ERROR == cSocket[i]) {
			printf("Server Crush: %d\n", GetLastError());
			//8. 斷開連結
			closesocket(sSocket);
			//9. 清理協議版本訊息
			WSACleanup();
			return -3;
		}
		printf("有客戶端連接成功: %d!\n", i);
		count++;     //當有一個客戶端成功連接 客戶端數量+1
		//創建一個多線程的客戶端通訊
		CreateThread(NULL, NULL,
			(LPTHREAD_START_ROUTINE)communication,   //呼叫多線程函式並用LP做轉型
			(LPVOID)i,
			NULL, NULL);
	}

	while (1);
	return 0;
}

//和客戶端通訊的多線程函數  
void communication(int index) {
	//7. 通訊
	char buff[256] = { 0 };
	int r;
	char temp[256];
	while (1) {
		r = recv(cSocket[index], buff, 255, NULL);  //從客戶端接收數據
		if (r > 0) {
			buff[r] = 0;   // 添加結束符號
			printf(">>%d: %s\n", index, buff);     // 輸出到Server端上面

			//Server接收到以後，發送給所有連上Server的客戶端
			memset(temp, 0, sizeof(temp));   //清空陣列
			snprintf(temp, sizeof(temp), "%d: %s", index, buff);
			for (int i = 0; i < count; i++) {
				send(cSocket[i], temp, (strlen(temp)), NULL);
			}
		}
	}
}

