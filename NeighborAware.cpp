/* 
 * NeighborAware.cpp - Encoding or decoding points by exploiting neighbor relations 
 * 
 * Author:      Lianyin Jia
 *              Dept. of Computer Science
 *              Kunming University of Science & Technology
 * Date:        Mon Aug 23 2021
 * Copyright (c)  Kunming University of Science & Technology
 *
 * Acknowledgement:
 * This implementation is based on the work of Lianyin Jia
 * "Efficient Neighbor-aware Hilbert Encoding and Decoding algorithms"
 */

#include <stdio.h>
#define getOneBitByPos(X,bit) ((X>>bit) & 0x01)  
#define getTwoBitByPos(X,i) (X>>(i-1)& 3UL) 
#define ones(T,k) ((((T)2) << (k-1)) - 1)  

char T[32]={0};

//the mappings from L1-coord to its L1-code
char CHM[4][2][2] = { 0,1,3,2,0,3,1,2,2,3,1,0,2,1,3,0 };
//the mappings from L1-coord to its Ln-state
char CSM[4][2][2] = { 1,0,3,0,0,2,1,1,2,1,2,3,3,3,0,2 };

//the mappings from L1-code to its L1-coord
char HCM[4][4] ={0,1,3,2,0,2,3,1,3,2,0,1,3,1,0,2};
//the mappings from L1-code to its Ln-state
char HSM[4][4] ={1,0,0,3,0,1,1,2,3,2,2,1,2,3,3,0};

typedef unsigned long long bitmask_t;
typedef unsigned long halfmask_t;

//get the left most position with bit 1 of a 32-bit number
int msb32_idx(halfmask_t n)
{
	int b = 0;
	if (!n) return -1;  
#define step(x) if (n >= ((halfmask_t)1) << x) b += x, n >>= x
	step(16); step(8); step(4); step(2); step(1);
#undef step
	return b;
}
//get the left most position with bit 1 of a 64-bit number
int msb64_idx(bitmask_t n)
{
	int b = 0;
	if (!n) return -1;  
#define step(x) if (n >= ((bitmask_t)1) << x) b += x, n >>= x
	step(32);	step(16); 	step(8); 	step(4);	step(2); 	step(1);
#undef step
	return b;
}
/*
NA-HE
input:
   prevCode: the encoded value of the previous coordinate
   iterStartPos: iterate start position (count in levels, one bits represent a level,the right most position is 0)  
   gridX¡¢gridY: current coordinate  
   nextX¡¢nextY: next coordinate
   k: total number of levels 
Output: the Hilbert code of the current coordinate
*/
bitmask_t en_neibourAware(bitmask_t &prevCode,int &iterStartPos, halfmask_t gridX, halfmask_t gridY,halfmask_t nextX, halfmask_t nextY, int k)
{
	char nType = 0;
	int startPos = 0,newIterStartPos = iterStartPos;
	unsigned bitX = 0, bitY = 0; 
	bitmask_t resKey=0; //the code of current coordinate
 
	// compute the position of current coordinate that can be used for the next coordinate
	newIterStartPos = nextX == gridX?msb32_idx(nextY ^ gridY):msb32_idx(nextX ^ gridX);
	//get the state of iterative-start-position from state array
	nType = T[k-iterStartPos-1];
	
	for (int i = iterStartPos; i >=0; i--) 
	{ 
		//get the code of current level
		bitX = getOneBitByPos(gridX,i);
		bitY = getOneBitByPos(gridY,i);
		resKey = (resKey << 2) | CHM[nType][bitX][bitY];
		//get the state of the next level
		nType = CSM[nType][bitX][bitY]; 
		//dynamically update T
		if(i>newIterStartPos)
			T[k-i] = nType; 
	} 
	//get the final code of current coordinate
	int movedbits = 2*(iterStartPos+1);
	resKey |=  prevCode>>movedbits<<movedbits;
	
	//prepare for the next coordinate
	iterStartPos = newIterStartPos;
	prevCode = resKey;
 
	return resKey;
}
//decoding 
/*
	NA-HD
	prevLon,prevLat: the decoded coordinate of previous code
	iterStartPos: iterate start position (count in levels, two bits represent a level,the right most position is 0)
	currentLon,currentLat: the coordinate of current code
	currentCode: current code
	nextCode: next code
	k:the total number of levels
*/
void de_neibourAware(halfmask_t &prevLon, halfmask_t &prevLat, int &iterStartPos, halfmask_t &currentLon, halfmask_t &currentLat,bitmask_t currentCode, bitmask_t nextCode, int k)
{
	
	char nType = 0;
	char posKey=0;
	int startPos = 0, newIterStartPos = iterStartPos;
	unsigned bitsZ = 0 ; //two bits
	// compute the position of current code that can be used for the next code
	newIterStartPos = msb64_idx(currentCode ^ nextCode)/2;
	
	//get the state of iterative-start-position from state array
	nType = T[k - iterStartPos - 1];
	 
	for (int i = iterStartPos; i >=0; i--)
	{		
		//get the coordinate of current level
		bitsZ = getTwoBitByPos(currentCode,(2*i+1));
		posKey = HCM[nType][bitsZ];
		currentLat = (currentLat << 1) | (posKey & 0x1);//y
		currentLon = (currentLon << 1) | (posKey >> 1 & 0x1);//x
		//get the state of the next level
		nType = HSM[nType][bitsZ];
		//dynamically update T
		if(i > newIterStartPos)
			T[k - i] = nType;
	}
	//get the final coordinate of current code
 	int movedbits =  (iterStartPos + 1);	
	currentLon = currentLon | (prevLon >> movedbits << movedbits);
	currentLat = currentLat | (prevLat >> movedbits << movedbits);
	
	//prepare for the next code
	iterStartPos = newIterStartPos;
	prevLat = currentLat;
	prevLon = currentLon;
}
 
void main()
{	
	int k = 4;	//total levels
	halfmask_t maxCoordVal= ones(halfmask_t,k);  // the maximum coordinate value 2^k -1
	int iterStartPos = k-1; //encode from the left most level
	bitmask_t prevCode = 0; // the Hilbert code of previous coordinate, start from 0
	bitmask_t curCode; // the Hilbert code of current coordinate
	printf("start encoding!\n");
	// encoding a k-window using S-scan
	for (int i = 0; i <= maxCoordVal; i++)
	{
		if(i%2==0)  
		{
			for(int j=0;j<=maxCoordVal;j++)  // an even row,scan from left to right
			{				
				if(j==maxCoordVal) //reach the right most coordinate
					curCode = en_neibourAware(prevCode,iterStartPos,i,j,i+1,j,k);					
				else 
					curCode = en_neibourAware(prevCode,iterStartPos,i,j,i,j+1,k);
				printf("The Hilbert code of coordinate (%u,%u): %I64u\n", i,j,curCode);
			}
		}
		else  
		{
			for(int j=maxCoordVal;j>=0;j--) // an odd row,scan from right to left
			{				
				if(j==0) //reach the top most coordinate
					curCode = en_neibourAware(prevCode,iterStartPos,i,j,i+1,j,k);
				else
					curCode = en_neibourAware(prevCode,iterStartPos,i,j,i,j-1,k);
					
				printf("The Hilbert code of coordinate (%u,%u): %I64u\n", i,j,curCode);
			}
		}			
	} 
	printf("end encoding!\n");
	
	halfmask_t gridX,gridY;  //the coordinate of current code
	bitmask_t maxCodeVal = ones(bitmask_t,2*k); // the maximum code value 2^(2*k) -1
	iterStartPos = k-1;  //decode from the left most level
	halfmask_t prevLon = 0,prevLat =0; // the coordinate of previous code, start from (0,0)
	printf("start decoding!\n");
	//decoding in value order
	for (bitmask_t i = 0; i <= maxCodeVal; i++)
	{
		gridX = 0;
		gridY = 0;
		de_neibourAware(prevLon,prevLat,iterStartPos,gridX,gridY,i,i+1,k); 	
		printf("The coordinate  of code %I64u: (%u,%u)\n", i,gridX,gridY);
	}
	printf("end decoding!\n");
}