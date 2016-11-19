#include <String.h>
#include <kern/system.hpp>

using namespace LunOS;

//output the number into the string assumed 3 character array, at least
void ucharDEHDecode(unsigned char num, char* string){
	string[0] = ((0xf0 & num) >> 4) + '0';
	string[1] = (0x0f & num) + '0';
	string[2] = 0;
}


//string must be at least 11 characters long [last for 0]
void uintToChar(unsigned int num, char* string)
{
	int i = 9,j;
	string[10] = 0;
	do
	{
		string[i] = (char)((char)(num % 10) + '0');
		num /= 10;
		i--;
	}while(num && i > 0);
	//make i point to the last char it wrote to
	i++;
	for(j = 0; i <= 11; i++, j++)
	{
		string[j] = string[i];
	}
}


//string must be at least 11 characters long [last for 0]
void uintToCharX(unsigned int num, char* string)
{
	int i = 9,j;
	char c;
	string[10] = 0;
	do
	{
		c = (char)(num % 16);
		if(c <= 9){
			string[i] = c + '0'; 
		}
		else{
			string[i] = (c - 10) + 'A';
		}
		num /= 16;
		i--;
	}while(num && i > 0);
	//make i point to the last char it wrote to
	i++;
	for(j = 0; i <= 11; i++, j++)
	{
		string[j] = string[i];
	}
}

bool String::toInt(unsigned int* result){
	unsigned int i = 0;
	bool hex = false;
	if(this->length == 0) return false;
	if(this->length > 2 && this->data[1] == 'x') {
		i = 2;
		hex = true;
	}
	*result = 0;
	if(hex){
		for(; i < this->length; i++)
		{
			if((this->data[i] >= '0') & (this->data[i] <= '9')){
				*result = (*result * 16) + (this->data[i] - '0');
				
			}
			else if((this->data[i] >= 'a') & (this->data[i] <= 'f')){
				*result = (*result * 16) + (this->data[i] - 'a') + 10;
				
			}
			else
			{
				return false;
			}
		}
	}
	else{
		for(; i < this->length; i++)
		{
			if((this->data[i] < '0') | (this->data[i] > '9')) return false;
		
			*result = (*result * 10) + (this->data[i] - '0');
		}
	}
	return true;
}

unsigned char* String::toCharArray()
{
	return this->data;
}

bool String::operator ==(char* c){
	unsigned int i;
	for(i = 0; i < this->length; i++){
		if(this->data[i] != c[i]) return false;
	}
	if(c[i] != 0) return false;
	return true;
}

bool String::operator ==(unsigned char* c){
	unsigned int i;
	for(i = 0; i < this->length; i++){
		if(this->data[i] != c[i]) return false;
	}
	if(c[i] != 0) return false;
	return true;
}

bool String::operator ==(String s){
	unsigned int i;
	unsigned char* c = s.toCharArray();
	for(i = 0; i < this->length; i++){
		if(this->data[i] != c[i]) return false;
	}
	if(c[i] != 0) return false;
	return true;
}

bool String::operator ==(String* s){
	unsigned int i;
	unsigned char* c = s->toCharArray();
	for(i = 0; i < this->length; i++){
		if(this->data[i] != c[i]) return false;
	}
	if(c[i] != 0) return false;
	return true;	
}

bool String::StartsWith(String* o){
	int i = 0;
	while(this->data[i] && o->data[i] && (this->data[i] == o->data[i]))
	{
		i++;
	}
	return o->data[i] == 0;
}

bool String::StartsWith(unsigned char* o){
	int i = 0;
	while(this->data[i] && o[i] && (this->data[i] == o[i]))
	{
		i++;
	}
	return o[i] == 0;
}

bool String::StartsWith(char* o){
	int i = 0;
	for(;this->data[i] & o[i] & (this->data[i] == o[i]);i++);
	return o[i] == 0;
}

unsigned int String::GetLength()
{
	return this->length;
}

unsigned int String::GetLength(unsigned char* string)
{
	unsigned int i = 0;
		while(string[i]){
			i++;
		}
	return i;
}

void String::operator =(char* c){
	unsigned int i = 0;
	while(c[i]){
		i++;
	}
	this->data = (unsigned char*)c;
	this->length = i;
}

void String::operator =(unsigned char* c){
	unsigned int i = 0;
	while(c[i]){
		i++;
	}
	this->data = c;
	this->length = i;
}

void String::operator =(const unsigned char* c){
	unsigned int i = 0;
	while(c[i]){
		i++;
	}
	unsigned char* data = new unsigned char[i];
	memcpy(data,c,i + 1);
	this->data = data;
	this->length = i;
}

void String::operator =(const char* c){
	unsigned int i = 0;
	while(c[i]){
		i++;
	}
	unsigned char* data = new unsigned char[i];
	memcpy(data,c,i + 1);
	this->data = data;
	this->length = i;
}



void String::operator =(String s){
	this->data = s.data;
	this->length = s.length;
}

void String::operator =(String* s){
	this->data = s->data;
	this->length = s->length;	
}
