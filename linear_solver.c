#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// Конечный автомат, который разбирает строку по коэффициентам (ax=b) за 1 проход
uint8_t parse_coeffs(char *str, double *coeff_a, double *coeff_b)
{
	uint8_t int_part=1; // Идёт ли разбор челой части числа
	uint32_t number_int=0, number_frac=0; // Целая и дробная часть числа
	double coeff=1.0, number=0.0; // Коэффициент (состоящий из множителей) и последнее распознанное число
	int8_t sign=0; // Знак числа +1 -1, 0 - неизвестно
	uint8_t equality_sign_found=0; // Встретился знак равенства
	uint8_t number_found=0; // Начался разбор числа
	int8_t div_found=0; // Встретился ли знак деления
	uint8_t div_x=0; // x в знаменателе
	uint8_t operation=0; // Встретился ли знак математической операции
	uint8_t x_found=0; // Найден знак неизвестного
	uint8_t x_count=0; // Количество найденных знаков X
	
	*coeff_a = 0.0;
	*coeff_b = 0.0;
	
	for (uint8_t i=0; (i < 255); i++) {
		if ((str[i] >= '0') && (str[i] <= '9')) {
			if (int_part) {
				// Разбор целой части числа
				number_int *= 10;
				number_int += str[i] - '0';
			} else {
				// Разбор дробной части числа
				number_frac *= 10;
				number_frac += (str[i] - '0');
			}
			number_found = 1;
			operation = 0;
		} else
		if (str[i] == '.') {
			if (int_part == 0) return 1; // Ошибка: Встретились две точки в одном числе
			int_part = 0;
			operation = 0;
		} else 
		if ((str[i] == 'x') || (str[i] == 'X')) { // Встретился X
			if (x_found) return 6;
			x_found = 1;
			x_count++;
			if (div_found) div_x = 1;
			operation = 0;
		} else
		if ((str[i] == '-') || (str[i] == '+') || (str[i] == '*') || (str[i] == '/') || (str[i] == '=') || (str[i] == '\0')) { // Встретилась арифметическая операция или знак равенства
			if ((!x_found) && (!number_found)) {
				if ((i==0) && (str[i] == '-')) {
					// Минус может стоять на первой позиции
				} else 
				if (str[i-1] == '=') {
					// Минус может стоять после знака равенства
				} else {
					return 7; // Ошибка: Арифметический знак без константы или X
				}
			}
			
			if (number_found) { // Если знак встретился после начала разбора числа
				// Завершаем парсинг текущего числа
				number = number_frac;
				while (number >= 1.0) number /= 10.0;
				number += number_int;
				if (sign == -1) number *= -1;
				
				// Инициазизируем переменные
				number_found = 0;
				number_frac = 0;
				number_int = 0;
				int_part = 1;
				sign = 0;
			} else {
				if (x_found) { // Если числа не было обнаружено и первый сомножитель это X
					number = 1.0; // Коэффициент перед ним равен единице
					if (sign == -1) number *= -1;
					sign = 0;
				} else {
					number = 0; // Если знак "-" встретился после знака "="
				}
			}
			
			// Добавляем сомножитель к текущему коэффициенту
			if (div_found) {
				coeff /= number;
				div_found = 0;
			} else {
				coeff *= number;
			}
			if (str[i] == '/') {
				div_found = 1;
			}
			
			if ((str[i] == '-') || (str[i] == '+') || (str[i] == '=') || (str[i] == '\0')) { // Обнаружилось слагаемое
				// Добавляем текущий коэффициент в нужную переменную
				if (x_found) {
					*coeff_a += coeff * (equality_sign_found ? -1.0 : 1.0);
					x_found = 0;
				} else {
					*coeff_b += coeff * (equality_sign_found ? 1.0 : -1.0);
				}
				div_found = 0;
				coeff = 1.0;
			}
				
			if ((str[i] == '-') || (str[i] == '+')) { // Запоминаем знак для следующего слагаемого
				if (sign != 0) return 2; // Ошибка: Встретились подряд два знака
				if (str[i] == '-') { // Запоминаем знак числа
					sign = -1;
				} else {
					sign = 1;
				}
			}
			
			if (str[i] == '=') {
				if (equality_sign_found) return 3; // Ошибка: Встретились два знака равенства
				equality_sign_found = 1;
			}
			
			if ((str[i] == '*') || (str[i] == '/')) {
				if (operation) return 11;
				operation = 1;
			}
		} else {
			return 4; // Ошибка: встретился неизвестный символ
		}
		
		if (str[i] == '\0') break; // Конец разбора строки
	}
	
	if (x_count == 0) return 8; // Ошибка: Не встретилось переменной X
	if (!equality_sign_found) return 9; // Ошибка: Не встретилось ни одного знака равенства
	if (div_x) { // Поменять местами константы, если X находится в знаменателе
		// a/x=b -> ax=b
		double coeff_t;
		coeff_t = *coeff_a;
		*coeff_a = *coeff_b;
		*coeff_b = coeff_t;
	}
	return 0;
}

int main(void)
{
	printf("Enter equation: ");
	
	char str[255];
	scanf("%s", str);
	
	double a, b; // ax=b
	uint8_t result = parse_coeffs(str, &a, &b);
	if (result == 0) {
		printf("%f * x = %f\n", a, b);
		printf("x = %f\n", b/a);
	} else {
		printf ("Parse Error %d\n", result);
	}
	
	system("pause");
	return 0;
}
