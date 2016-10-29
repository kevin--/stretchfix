/*
SMPTElib - A library of classes for parsing SMPTE LTC
Copyright (C) 2008 Kevin C. Dixon
http://yano.wasteonline.net/software/sscau/
*/
/*
	BCDNumber.hpp
	A class used to represent a number made up of one or more 4-bit binary coded decimals
	
	Note 1: can hold any number of digits, but can only reasonably return the decimal value of a 9 digit number
	Note 2: 
	
	Kevin C. Dixon
	Yano Signal Processing
	01/19/2008
*/

#include <vector>
using std::vector;
#include <math.h>

#ifndef _YSP_BCDNUMBER_HPP_
#define _YSP_BCDNUMBER_HPP_

class BCDNumber {
	protected:
		vector<int> digits;
	public:
		BCDNumber() : digits() { }
		
		BCDNumber(int firstDigit) : digits() {
			append(firstDigit);
		}
		~BCDNumber() { }
		
		/*
			"four bit value" - Returns the decimal value of a four bit binary number
			msb - most significant bit
			smsb - second most significant bit
			slsb - second least significant bit
			lsb - least significant bit
		*/
		static inline int fbv(short msb, short smsb, short slsb, short lsb) {
			return 8 * msb + 4 * smsb + 2 * slsb + lsb;
		}
		
		/*
			Returns the value of the binary coded decimal number
			backwards		set to true to evaluate number backwards, with units place at left most
						Default is FALSE to evaluate with units place right most
		*/
		unsigned long value(bool backwards = false) {
			unsigned long total = 0;
			
			int exponent = (backwards) ? digits.size() - 1 : 0;
			
			for(int i = digits.size() - 1; i >= 0; i--) {
				total += digits[i] * pow(10, exponent);
				
				if(backwards)
					exponent--;
				else
					exponent++;
			}
			return total;
		}
		
		/*
			Appends a digit onto the BCD Number
			d - digit as integer to append
		*/
		void append(int d) {
			digits.push_back(d);
		}
		
		/*
			Append a digit onto the BCD Number
			msb		most significant bit
			smsb		second-most significant bit
			slsb		second-least significant bit
			lsb		least significant bit
		*/
		void append(short msb, short smsb, short slsb, short lsb) {
			append(fbv(msb, smsb, slsb, lsb));
		}
		
		/*
			Removes all digits from the number
		*/
		void clear() {
			digits.clear();
		}
		
		/*
			Returns an iterator across the the digits that make up the number
		*/
		vector<int>::iterator begin() {
			return digits.begin();
		}
		
};

#endif
