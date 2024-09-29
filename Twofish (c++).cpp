#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include <bitset>
using namespace std;
static uint32_t Me[4] = { 0,0,0,0 }; // Массив Me
static uint32_t Mo[4] = { 0,0,0,0 }; // Массив Mo
static uint32_t V[4] = { 0,0,0,0 }; // Массив V
static uint32_t keys[40]; // 40 подключей
// Таблица замены для функции q0
static const uint8_t t_q0[4][16] = {
{0x8, 0x1, 0x7, 0xD, 0x6, 0xf, 0x3, 0x2, 0x0, 0xb, 0x5, 0x9, 0xe, 0xc, 0xa,
0x4},
{0xe, 0xc, 0xb, 0x8, 0x1, 0x2, 0x3, 0x5, 0xf, 0x4, 0xa, 0x6, 0x7, 0x0, 0x9,
0xd},
{0xb, 0xa, 0x5, 0xe, 0x6, 0xd, 0x9, 0x0, 0xc, 0x8, 0xf, 0x3, 0x2, 0x4, 0x7,
0x1},
{0xd, 0x7, 0xf, 0x4, 0x1, 0x2, 0x6, 0xe, 0x9, 0xb, 0x3, 0x0, 0x8, 0x5, 0xc,
0xa}
};
// Таблица замены для функции q1
static const uint8_t t_q1[4][16] = {
{0x2, 0x8, 0xb, 0xd, 0xf, 0x7, 0x6, 0xe, 0x3, 0x1, 0x9, 0x4, 0x0, 0xa, 0xc,
0x5},
{0x1, 0xe, 0x2, 0xb, 0x4, 0xc, 0x3, 0x7, 0x6, 0xd, 0xa, 0x5, 0xf, 0x9, 0x0,
0x8},
{0x4, 0xc, 0x7, 0x5, 0x1, 0x6, 0x9, 0xa, 0x0, 0xe, 0xd, 0x8, 0x2, 0xb, 0x3,
0xf},
{0xb, 0x9, 0x5, 0x1, 0xc, 0x3, 0xd, 0xe, 0x6, 0x4, 0x7, 0xf, 0x2, 0x0, 0x8,
0xa}
};
// Постоянная ро
static const uint32_t ro = 0x01010101;
// Матрица М1
static const uint8_t M1[4][4] = {
{0x01, 0xEF, 0x5B, 0x5B},
{0x5B, 0xEF, 0xEF, 0x01},
{0xEF, 0x5B, 0x01, 0xEF},
{0xEF, 0x01, 0xEF, 0x5B}
};
// Матрица М2
static const uint8_t M2[4][8] = {
{0x01, 0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E},
{0xA4, 0x56, 0x82, 0xF3, 0x1E, 0xC6, 0x68, 0xE5},
{0x02, 0xA1, 0xFC, 0xC1, 0x47, 0xAE, 0x3D, 0x19},
{0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E, 0x03}
};
// Функция циклического сдвига бит вправо
uint32_t rightcycle(uint32_t x, int k) {
	return (x >> k) | (x << (32 - k));
}
// Функция циклического сдвига бит влево
uint32_t leftcycle(uint32_t x, int k) {
	return (x << k) | (x >> (32 - k));
}
// Функция циклического сдвига вправо 4-битных величин
uint8_t rightcycle41(uint8_t x) {
	return ((x & 0xf) >> 1) | ((x & 0xf) << 3);
}
// Функция умножения в конечном поле GF2^8 с образующим полиномом x^8 + x^6 + x^5 + x ^ 3 + x + 1
uint8_t gf_multiply(uint8_t a, uint8_t b) {
	uint8_t result = 0;
	while (b) {
		if (b & 1) {
			// Если младший бит b равен 1, добавляем a к результату операцией XOR
			result ^= a;
		}
		if (a & 0x80) {
			// Если старший бит a равен 1 двигаем a влево и выполняем редукцию с использованием образующего полинома
			a = (a << 1) ^ 0x8D;
		}
		else {
			a <<= 1;
		}
		b >>= 1;
	}
	return result;
}
// Функция замены q0
uint8_t q0(uint8_t x) {
	uint8_t a0, b0, a1, b1, a2, b2, a3, b3, a4, b4;
	a0 = x / 16;
	b0 = x % 16;
	a1 = a0 ^ b0;
	b1 = a0 ^ rightcycle41(b0) ^ 8 * a0 % 16;
	a2 = t_q0[0][a1];
	b2 = t_q0[1][b1];
	a3 = a2 ^ b2;
	b3 = a2 ^ rightcycle41(b2) ^ 8 * a2 % 16;
	a4 = t_q0[2][a3];
	b4 = t_q0[3][b3];
	return 16 * b4 + a4;
}
// Функция замены q1
uint8_t q1(uint8_t x) {
	uint8_t a0, b0, a1, b1, a2, b2, a3, b3, a4, b4;
	a0 = x / 16;
	b0 = x % 16;
	a1 = a0 ^ b0;
	b1 = a0 ^ rightcycle41(b0) ^ 8 * a0 % 16;
	a2 = t_q1[0][a1];
	b2 = t_q1[1][b1];
	a3 = a2 ^ b2;
	b3 = a2 ^ rightcycle41(b2) ^ 8 * a2 % 16;
	a4 = t_q1[2][a3];
	b4 = t_q1[3][b3];
	return 16 * b4 + a4;
}
// Функция h
uint32_t h(uint32_t x, const uint32_t* arr) {
	// Массив bytes нужен для разбиения входного слова на байты
	// Массив ret нужен для финального суммирования элементов
	uint8_t bytes[4], ret[4];
	// Шаг 1 выполняется если ключ имеет размер 256 бит
	if (arr[3] != 0) {
		// Функции замены
		bytes[0] = q1(x % 256);
		bytes[1] = q0((x >> 8) % 256);
		bytes[2] = q0((x >> 16) % 256);
		bytes[3] = q1((x >> 24) % 256);
		// Объединение байтов
		x = bytes[3];
		for (int i = 3; i >= 0; i--) {
			x <<= 8;
			x += bytes[i];
		}
		x += arr[3];
	}
	// Шаг 2 выполняется если ключ имеет размер более 192 бит
	if (arr[2] != 0) {
		// Функции замены
		bytes[0] = q1(x % 256);
		bytes[1] = q1((x >> 8) % 256);
		bytes[2] = q0((x >> 16) % 256);
		bytes[3] = q0((x >> 24) % 256);
		// Объединение байтов
		x = bytes[3];
		for (int i = 3; i >= 0; i--) {
			x <<= 8;
			x += bytes[i];
		}
		x += arr[2];
	}
	// Шаг 3
	// Функции замены
	bytes[0] = q0(x % 256);
	bytes[1] = q1((x >> 8) % 256);
	bytes[2] = q0((x >> 16) % 256);
	bytes[3] = q1((x >> 24) % 256);
	// Объединение байтов
	x = bytes[3];
	for (int i = 3; i >= 0; i--) {
		x <<= 8;
		x += bytes[i];
	}
	x += arr[1];
	// Шаг 4
	// Функции замены
	bytes[0] = q0(x % 256);
	bytes[1] = q0((x >> 8) % 256);
	bytes[2] = q1((x >> 16) % 256);
	bytes[3] = q1((x >> 24) % 256);
	// Объединение байтов
	x = bytes[3];
	for (int i = 3; i >= 0; i--) {
		x <<= 8;
		x += bytes[i];
	}
	x += arr[0];
	// Шаг 5
	// Функции замены
	bytes[0] = q1(x % 256);
	bytes[1] = q0((x >> 8) % 256);
	bytes[2] = q1((x >> 16) % 256);
	bytes[3] = q0((x >> 24) % 256);
	// Умножение байтов шага 5 на матрицу M1
	for (int i = 0; i < 4; i++) {
		ret[i] = gf_multiply(bytes[0], M1[i][0]);
		for (int j = 1; j < 4; j++) {
			ret[i] += gf_multiply(bytes[j], M1[i][j]);
		}
	}
	// Суммирование байтов
	return ret[0] + ret[1] * 0x100 + ret[2] * 0x10000 + ret[3] * 0x1000000;;
}
void form_arr(const vector<uint32_t>& key) {
	// Массив m нужен для разбиения ключа на байты
	// Массив Vi нужен для умножения части массива m на матрицу M2
	uint8_t m[32], Vi[4] = { 0,0,0,0 };
	uint32_t A, B;
	// создание массива байт m
	for (int i = 0; i < key.size(); i++) {
		for (int j = 0; j < 4; j++) {
			m[i * 4 + j] = (key[i] >> j * 8) % 256;
		}
	}
	// Заполнение массивов Me и Mo
	for (int i = 0; i < key.size() / 2; i++) {
		Me[i] = key[i * 2];
		Mo[i] = key[i * 2 + 1];
	}
	// Заполнение массива V
	for (int i = 0; i < key.size() / 2; i++) {
		Vi[0] = 0;
		Vi[1] = 0;
		Vi[2] = 0;
		Vi[3] = 0;
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 8; k++) {
				Vi[j] += gf_multiply(m[i * 8 + k], M2[j][k]);
			}
		}
		V[i] = Vi[0] + Vi[1] * 0x100 + Vi[2] * 0x10000 + Vi[3] * 0x1000000;
	}
	//генерация 40 подключей
	for (int i = 0; i < 20; i++) {
		A = h(2 * i * ro, Me);
		B = leftcycle(h((2 * i + 1) * ro, Mo), 8);
		keys[2 * i] = (A + B);
		keys[2 * i + 1] = leftcycle(A + 2 * B, 9);
	}
}
void encrypt(const vector<uint32_t>& arr, vector<uint32_t>& exit) {
	//субблоки 128-ого блока исходной информации
	uint32_t A, B, C, D, old_A, old_B;
	//Входное отбеливание
	A = arr[0] ^ keys[0];
	B = arr[1] ^ keys[1];
	C = arr[2] ^ keys[2];
	D = arr[3] ^ keys[3];
	// 16 раундов шифрования
	for (int i = 0; i < 16; i++) {
		old_A = A;
		old_B = B;
		B = leftcycle(B, 8);
		A = h(A, V);
		B = h(B, V);
		A += B;
		B += A;
		A += keys[2 * i + 8];
		B += keys[2 * i + 9];
		C ^= A;
		D = leftcycle(D, 1);
		D ^= B;
		C = rightcycle(C, 1);
		// Замена значений после раунда
		A = C;
		C = old_A;
		B = D;
		D = old_B;
	}
	//Выходное отбеливание
	exit.push_back(C ^ keys[4]);
	exit.push_back(D ^ keys[5]);
	exit.push_back(A ^ keys[6]);
	exit.push_back(B ^ keys[7]);
}
void decrypt(const vector<uint32_t>& arr, vector<uint32_t>& exit) {
	uint32_t A, B, C, D, old_A, old_B;
	// Входное отбеливание
	C = arr[0] ^ keys[4];
	D = arr[1] ^ keys[5];
	old_A = arr[2] ^ keys[6];
	old_B = arr[3] ^ keys[7];
	// 16 раундов дешифрования
	for (int i = 15; i >= 0; i--) {
		// Замена значений перед раундом
		A = old_A;
		B = old_B;
		old_A = C;
		old_B = D;
		C = A;
		D = B;
		B = leftcycle(old_B, 8);
		A = h(old_A, V);
		B = h(B, V);
		A += B;
		B += A;
		A += keys[2 * i + 8];
		B += keys[2 * i + 9];
		C = leftcycle(C, 1);
		D ^= B;
		D = rightcycle(D, 1);
		C ^= A;
	}
	// Выходное отбеливание
	exit.push_back(old_A ^ keys[0]);
	exit.push_back(old_B ^ keys[1]);
	exit.push_back(C ^ keys[2]);
	exit.push_back(D ^ keys[3]);
}
// Перевод строки в вектор из 32-битных чисел
vector<uint32_t> stringToInt(const string& str) {
	vector<uint32_t> arr;
	uint32_t dop = 0;
	for (int i = 0; i < str.length(); i++) {
		if (i % 4 == 0 && i != 0) {
			arr.push_back(dop);
			dop = 0;
		}
		dop >>= 8;
		dop += uint8_t(str[i]) * 0x1000000;
	}
	arr.push_back(dop);
	while (arr.size() < 4 || arr.size() % 2 != 0) {
		arr.push_back(0);
	}
	return arr;
}
// Перевод вектора 32-битных чисел в строку
string intToString(const vector<uint32_t>& kod) {
	string str;
	for (int i = 0; i < kod.size(); i++) {
		for (int j = 0; j < 4; j++) {
			str += char((kod[i] >> j * 8) % 256);
		}
	}
	return str;
}
int main() {
	setlocale(LC_ALL, "Russian");
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	// Переменная sw нужна для выбора действия
	// Переменные key и str нужны для ввода ключа и сообщенмя
	// Вектора text и shift нужны для хранения изначального и зашифрованного текста в виде 32 - битных слов
		// переменная byte4 используется для преобразования зашифрованного сообщения в двоичный код
	int sw;
	string key, str;
	string dop;
	vector<uint32_t> text, shifr;
	bitset <32> byte4;
	while (true) {
		// Обнуление массивов и векторов
		shifr.clear();
		text.clear();
		for (int i = 0; i < 4; i++) {
			Me[i] = 0;
			Mo[i] = 0;
			V[i] = 0;
		}
		cout << "Выберите действие: 1 - зашифровать сообщение 2 - расшифровать сообщение 0 - закончить работу программы ";
		cin >> sw;
		cin.ignore();
		switch (sw) {
		case 1:
			cout << "Введите ключ не более 32 символов:\n";
			getline(cin, key);
			if (key.length() > 32) {
				cout << "Неправильный размер ключа!\n\n";
				continue;
			}
			// Заполнение массивов для шифрования
			form_arr(stringToInt(key));
			cout << "Введите сообщение:\n";
			getline(cin, str);
			// Разбиение сообщения на блоки по 128 бит и их шифровка
			while (str.length() > 16) {
				dop = str.substr(0, 16);
				str.erase(0, 16);
				encrypt(stringToInt(dop), shifr);
			}
			dop = str.substr(0, str.length());
			encrypt(stringToInt(dop), shifr);
			cout << "Зашифрованное сообщение:\n";
			// Перевод зашифрованного сообщения в двоичный код
			for (int i = 0; i < shifr.size(); i++) {
				byte4 = shifr[i];
				cout << byte4;
			}
			cout << "\n\n";
			break;
		case 2:
			cout << "Введите ключ не более 32 символов:\n";
			getline(cin, key);
			if (key.length() > 32) {
				cout << "Неправильный размер ключа!\n\n";
				continue;
			}
			// Заполнение массивов для дешифрования
			form_arr(stringToInt(key));
			cout << "Введите зашифрованное сообщение:\n";
			cin >> str;
			int len;
			// количество 32-битных слов в зашифрованном сообщении
			len = str.size() / 32;
			for (int i = 0; i < len; i++) {
				byte4.reset();
				dop = str.substr(0, 32);
				str.erase(0, 32);
				// Перевод 32-битного слова из строки в uint32_t
				for (int j = 0; j < 32; j++) {
					if (dop[j] == '0') byte4[31 - j] = 0;
					else byte4[31 - j] = 1;
				}
				// Добавление 32-битного слова к 128-битному блоку сообщения
				shifr.push_back(uint32_t(byte4.to_ulong()));
				// Расшифровка 128-битного блока сообщения
				if ((i + 1) % 4 == 0) {
					decrypt(shifr, text);
					shifr.clear();
				}
			}
			// Вывод расшифрованного сообщения
			cout << intToString(text) << "\n";
			break;
		default:
			return 0;
			break;
		}
	}
}