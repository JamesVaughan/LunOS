#ifndef STRING_H_
#define STRING_H_

//Decimal Encoded Hex decode for uchar
void ucharDEHDecode(unsigned char num, char* string);
//string must be at least 11 characters long [last for 0]
void uintToChar(unsigned int num, char* string);

//string must be at least 11 characters long [last for 0]
void uintToCharX(unsigned int num, char* string);

namespace LunOS
{
	/**
	 * String class
	 *
	 * The kernel's string structure
	 * May or may not be implemented in the future
	 * depending if we are still in favour of
	 * char*.
	 */
	class String
	{
		private:
		unsigned char* data;
		unsigned int length;
		public:
		unsigned char* toCharArray();
		bool toInt(unsigned int* result);
		bool StartsWith(char*);
		bool StartsWith(unsigned char*);
		bool StartsWith(String*);
		unsigned int GetLength();
		static unsigned int GetLength(unsigned char*);
		void operator = (char*);
		void operator = (unsigned char*);
		void operator = (const char*);
		void operator = (const unsigned char*);
		void operator = (String);
		void operator = (String*);
		String* operator + (String*);
		String* operator + (String);

		bool operator == (String);
		bool operator == (String*);
		bool operator == (char*);
		bool operator == (unsigned char*);
		bool operator == (char);
	};
	typedef String string;
}
//also let it be called by the lower case


#endif /*STRING_H_*/
