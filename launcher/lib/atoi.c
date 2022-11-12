#include <ctype.h>
#include "string.h"

char tbuf[32];
char bchars[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

void itoa(unsigned i, unsigned base, char* buf) {
	int pos = 0;
	int opos = 0;
	int top = 0;

	//진수가 16을 넘거나 제공된 값이 0이면 문자 '0'을 buf에 담는다.
	if (i == 0 || base > 16) {
		buf[0] = '0';
		buf[1] = '\0';
		return;
	}

	//진수에 맞게 문자를 얻어낸다.
	//17이 입력되고 이를 16진수로 변환하면 11이다.
	//아래 루프가 그 기능을 담당한다.
	while (i != 0) {
		tbuf[pos] = bchars[i % base];
		pos++;
		i /= base;
	}
	top = pos--;
	for (opos = 0; opos < top; pos--, opos++) {
		buf[opos] = tbuf[pos];
	}
	buf[opos] = 0;
}


void itoa_s(unsigned int i, unsigned base, char* buf) {
	if (base > 16) return;

	itoa(i, base, buf);
}

int normalize(double* val) {
	int exponent = 0;
	double value = *val;

	while (value >= 1.0) {
		value /= 10.0;
		++exponent;
	}

	while (value < 0.1) {
		value *= 10.0;
		--exponent;
	}
	*val = value;
	return exponent;
}

void ftoa_fixed(char* buffer, double value) {
	/* carry out a fixed conversion of a double value to a string, with a precision of 5 decimal digits.
	* Values with absolute values less than 0.000001 are rounded to 0.0
	* Note: this blindly assumes that the buffer will be large enough to hold the largest possible result.
	* The largest value we expect is an IEEE 754 double precision real, with maximum magnitude of approximately
	* e+308. The C standard requires an implementation to allow a single conversion to produce up to 512
	* characters, so that's what we really expect as the buffer size.
	*/

	int exponent = 0;
	int places = 0;
	static const int width = 4;

	if (value == 0.0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return;
	}

	if (value < 0.0) {
		*buffer++ = '-';
		value = -value;
	}

	exponent = normalize(&value);

	while (exponent > 0) {
		int digit = value * 10;
		*buffer++ = digit + '0';
		value = value * 10 - digit;
		++places;
		--exponent;
	}

	if (places == 0)
		*buffer++ = '0';

	*buffer++ = '.';

	while (exponent < 0 && places < width) {
		*buffer++ = '0';
		--exponent;
		++places;
	}

	while (places < width) {
		int digit = value * 10.0;
		*buffer++ = digit + '0';
		value = value * 10.0 - digit;
		++places;
	}
	*buffer = '\0';
}

char* _i64toa(long long value, char* str, int radix)
{
	unsigned long long val;
	int negative;
	char buffer[65];
	char* pos;
	int digit;

	if (value < 0 && radix == 10) {
		negative = 1;
		val = -value;

	}
	else {
		negative = 0;
		val = value;

	} /* if */
	pos = &buffer[64];
	*pos = '\0';
	do {
		digit = val % radix;
		val = val / radix;
		if (digit < 10) {
			*--pos = '0' + digit;

		}
		else {
			*--pos = 'a' + digit - 10;

		} /* if */

	} while (val != 0L);
	if (negative) {
		*--pos = '-';

	} /* if */
	memcpy(str, pos, &buffer[64] - pos + 1);
	return str;
}

