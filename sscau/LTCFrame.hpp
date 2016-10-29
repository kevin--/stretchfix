/*
SMPTElib - A library of classes for parsing SMPTE LTC
Copyright (C) 2008 Kevin C. Dixon
http://yano.wasteonline.net/software/sscau/
*/
/*
	LTCFrame.hpp
	This class represents one frame of SMPTE LTC data
	
	Kevin C. Dixon
	Yano Signal Processing
	01/19/2008 - incarnation
	02/24/2008 - bug fixes
*/

#include <deque>
using std::deque;
#include "BCDNumber.hpp"

#ifndef _YSP_LTEFRAME_HPP_
#define _YSP_LTEFRAME_HPP_

#define LTC_BCD_SYNC_WORD 4663

class LTCFrame
{
	protected:
		//parser data
		int bitPos;
		deque<short> accum;
		deque<short> curSyncWord;
	
		// -=- time data -=-
		//hours 56/57 + 48/51
		//mins  40/42 + 32/35
		//secs  24/26 + 16/19
		//frame 8/9 + 0/3
		BCDNumber tHours, tMinutes, tSeconds, tFrames;
		
		//43,59 00 = no format 10 = eight bit format 01,11 = unassigned
		short userDataFormat;
		//user data 8 fields, 4/7, 12/15, 20/23, 28/31, 36/39, 40/42, 44/47, 52/55, 60/63
		vector<short> userData;
		
		//10
		bool dropFrameFormat;
		
		//11
		bool colorFrame;
		
		//27
		short biPhaseCorrectionBit;
		
		//64/79 sync word: 0011 1111 1111 1101 = 0x3FFD = 16381
		BCDNumber syncWord;
		
	public:
		LTCFrame() : bitPos(-1), accum(), curSyncWord(16),
					tHours(), tMinutes(), tSeconds(), tFrames(),
					 userDataFormat(0), userData(8),
					dropFrameFormat(false), colorFrame(false), biPhaseCorrectionBit(0),
					syncWord()
		{
			//all inits done in initializer list
		}
		~LTCFrame() { }
		
#pragma mark Frame Meta-Data
		//---- FRAME META-DATA -------------------------
		/*
			Returns true if a full LTC Frame has been parsed
		*/
		bool fullFrameParsed()
		{
			return bitPos == 79;
		}
		
		/*
			Returns true if the "drop frame format" flag (bit 10) is set high
		*/
		bool isDropFrameFormat()
		{
			return dropFrameFormat;
		}
		
		/*
			Returns true if the "color frame" flag (bit 11) is set high
		*/
		bool isColorFrame()
		{
			return colorFrame;
		}
		
		/*
			Returns the value of the Bi-Phase Correcton bit (bit 27) (0 or 1)
		*/
		short getBiPhaseCorrectionBit()
		{
			return biPhaseCorrectionBit;
		}
		
		/*
			Returns true if the sync word is valid
		*/
		bool isValidSyncWord()
		{
			return syncWord.value() == LTC_BCD_SYNC_WORD;
		}
		
		/*
			Returns the value of the sync word as parsed (last 16 bits of frame)
		*/
		int syncWordValue()
		{
			return syncWord.value();
		}
		
#pragma mark Get Time Functions
		//---- GET TIME FUNCTIONS ----------------------
		unsigned int getHours()
		{
			return tHours.value(true);
		}
		
		unsigned int getMinutes()
		{
			return tMinutes.value(true);
		}
		
		unsigned int getSeconds()
		{
			return tSeconds.value(true);
		}
		
		unsigned int getFrames()
		{
			return tFrames.value(true);
		}
		
		
#pragma mark User Data Functions
		//---- USER DATA FUNCTIONS ----------------------
		/*
			Returns the user data format (bits 43 and 59).
			Values 0, 1, 2, or 3
		*/
		short getUserDataFormat()
		{
			return userDataFormat;
		}
		/*
			Returns the value of the user data field
			i	index 1 - 8 of the field to return
		*/
		short getUserData(unsigned int i)
		{
			i--;
			
			if((i < 0) || (i > userData.size() - 1))
				return 0;
				
			return userData[i];
		}
		
		/*
			Returns the value of the user data field
			i	index 1 - 8 of the field to return
		*/
		void setUserData(int i, short value)
		{
			i--;
			
			if((i < 0) || (i > 7))
				return;
				
			userData[i] = value;
		}
		
		
#pragma mark Parsing Functions
		/*
			Clears the data structure and prepares for parsing
		*/
		void clear()
		{
			accum.clear();
			curSyncWord.clear();
			syncWord.clear();
			tHours.clear();
			tMinutes.clear();
			tSeconds.clear();
			tFrames.clear();
			bitPos = -1;
			for(int i = 0; i < 8; i++)
				userData[i] = 0;
		}
		
		/*
			Sucks up bits until a sync word is hit
			Returns false if no Sync word has been found
			Returns true when a sync word is hit
		*/
		bool foundSyncWord(short bit)
		{
			return checkCurSyncWord(bit);
		}
		
		/*
			Calculates the value of the last 16 bits parsed, returns true if the last 16 bits parsed form the Sync Word
		*/
		bool curSyncWordValid()
		{
			if(curSyncWord.size() != 16)
				return false;
				
			BCDNumber test = BCDNumber();
			for(int i = 0; i < 16; i += 4)
				test.append(curSyncWord[i], curSyncWord[i+ 1], curSyncWord[i + 2], curSyncWord[i + 3]);
				
			return (test.value() == LTC_BCD_SYNC_WORD);
		}

		/*
			Adds a bit to the structure
			bit		bit to add
			
			Returns true if parsing up to this point seems valid.
			Returns false if the bit accumulated completes the frame.
			
			Returns false if sync word has been hit, but a full frame has not been received.
			Returns false if a full frame has already been received
		*/
		bool accumulateBit(short bit)
		{
			if(fullFrameParsed())
				return false;
			
			accum.push_back(bit);
			bitPos++;
				
			if(checkCurSyncWord(bit) && !fullFrameParsed())
				return false;
				
			tryParse();
			
			if(fullFrameParsed())
				return false;
				
			return true;
		}
		
	protected:
		/*
			Sees if we've completed a chunk of the LTC frame. If so, the data is parsed and stored accordingly
		*/
		void tryParse()
		{
		
			switch(bitPos)
			{
				case 3:
					tFrames.append(parseAccumBCD());
					break;
				case 7:
					setUserData(1, parseAccumBCD());
					break;
				case 9:
					tFrames.append(parseAccumBCD());
					break;
				case 10:
					dropFrameFormat = accum[0] == 1;
					accum.clear();
					break;
				case 11:
					colorFrame = accum[0] == 1;
					accum.clear();
					break;
				case 15:
					setUserData(2, parseAccumBCD());
					break;
				case 19:
					tSeconds.append(parseAccumBCD());
					break;
				case 23:
					setUserData(3, parseAccumBCD());
					break;
				case 26:
					tSeconds.append(parseAccumBCD());
					break;
				case 27:
					biPhaseCorrectionBit = accum[0];
					accum.clear();
					break;
				case 31:
					setUserData(4, parseAccumBCD());
					break;
				case 35:
					tMinutes.append(parseAccumBCD());
					break;
				case 39:
					setUserData(5, parseAccumBCD());
					break;
				case 42:
					tMinutes.append(parseAccumBCD());
					break;
				case 43:
					userDataFormat = (accum[0] == 0) ? 0 : 2;
					accum.clear();
					break;
				case 47:
					setUserData(6, parseAccumBCD());
					break;
				case 51:
					tHours.append(parseAccumBCD());
					break;
				case 55:
					setUserData(7, parseAccumBCD());
					break;
				case 57:
					tHours.append(parseAccumBCD());
					break;
				case 58:
					accum.clear();
					break;
				case 59:
					userDataFormat += accum[0];
					accum.clear();
					break;
				case 63:
					setUserData(8, parseAccumBCD());
				case 67:
				case 71:
				case 75:
				case 79:
					syncWord.append(parseAccumBCD());
					break;
			}
		}
		
		/*
			returns true if sync word has been hit, regardless if it has been parsed correctly or not
			If not yet hit, then checks the current bit combination
			
			lastBit	the last bit parsed, either by accumulateBit or foundSyncWord
			returns true if the last 16 bits are a valid sync word
		*/
		bool checkCurSyncWord(short lastBit)
		{
			curSyncWord.push_back(lastBit);
			
			if(curSyncWord.size() < 16)
				return false;
			
			while(curSyncWord.size() > 16)
				curSyncWord.pop_front();
				
			return curSyncWordValid();
		}
		
		/*
			Returns the value of the BCD currently in the "accum" shift register
			clear	clear accum after finding value, default true
		*/
		int parseAccumBCD(bool clear = true)
		{
			int total = 0;
			for(int i = accum.size() - 1, worth = 1; i >= 0; i--, worth *= 2)
				total +=  accum[i] * worth;
				
			if(clear)
				accum.clear();
				
			return total;
		}
};

#endif
