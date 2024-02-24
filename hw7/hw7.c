#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define ROWS	240
#define COLUMNS	240
#define PI 3.14159265358979323846
#define sqr(x)  ((x)*(x))

void clear( unsigned char image[][COLUMNS] );
void header( int row, int col, unsigned char head[32] );

int main( int argc, char** argv )
{
	int				i, j, k;
	FILE*			fp;
	unsigned char	image[ROWS][COLUMNS], head[32];
	char			filename[50], ch;
	float           V[] = { 0, 0, 1 }, H[3], N[3], E[ROWS][COLUMNS];
	float			r, m, a, s[3], alpha, x, y, costheta, len, z, p, q;

	header ( ROWS, COLUMNS, head );
	
	for ( k = 0; k < 9; k ++ )
	{
		clear(image);
		switch(k)
		{
			case 0:
				strcpy(filename, "sphere-1.ras");
				s[0] = 0;
				s[1] = 0;
				s[2] = 1;
				r = 50;
				a = 0.5;
				m = 1;
				break;
			case 1:
				strcpy(filename, "sphere-2.ras");
				s[0] = 1 / sqrt(3);
				s[1] = 1 / sqrt(3);
				s[2] = 1 / sqrt(3);
				r = 50;
				a = 0.5;
				m = 1;
				break;
			case 2:
				strcpy(filename, "sphere-3.ras");
				s[0] = 1;
				s[1] = 0;
				s[2] = 0;
				r = 50;
				a = 0.5;
				m = 1;
				break;
			case 3:
				strcpy(filename, "sphere-4.ras");
				s[0] = 0;
				s[1] = 0;
				s[2] = 1;
				r = 10;
				a = 0.5;
				m = 1;
				break;
			case 4:
				strcpy(filename, "sphere-5.ras");
				s[0] = 0;
				s[1] = 0;
				s[2] = 1;
				r = 100;
				a = 0.5;
				m = 1;
				break;
			case 5:
				strcpy(filename, "sphere-6.ras");
				s[0] = 0;
				s[1] = 0;
				s[2] = 1;
				r = 50;
				a = 0.1;
				m = 1;
				break;
			case 6:
				strcpy(filename, "sphere-7.ras");
				s[0] = 0;
				s[1] = 0;
				s[2] = 1;
				r = 50;
				a = 1;
				m = 1;
				break;
			case 7:
				strcpy(filename, "sphere-8.ras");
				s[0] = 0;
				s[1] = 0;
				s[2] = 1;
				r = 50;
				a = 0.5;
				m = 0.1;
				break;
			case 8:
				strcpy(filename, "sphere-9.ras");
				s[0] = 0;
				s[1] = 0;
				s[2] = 1;
				r = 50;
				a = 0.5;
				m = 10000;
				break;
			default:
				break;
		}
		
		// circle drawing
		x = COLUMNS / 2;
		y = ROWS /2;
		float vmag = 0;
		for(i=0; i<ROWS;i++)
		{
			for(j=0; j<COLUMNS; j++)
			{
				if((sqr(r) - (sqr(j-x) + sqr(i-y)) > 0))
				{
					z = sqrt(sqr(r) - (sqr(j-x) + sqr(i-y)));
					p = -(j-x) / z;
					q = (i-y) / z;
					N[0] = -p / sqrt(sqr(p) + sqr(q) + 1);
					N[1] = -q / sqrt(sqr(p) + sqr(q) + 1);
					N[2] = 1 / sqrt(sqr(p) + sqr(q) + 1);
					float lambert = (N[0] * s[0]) + (N[1] * s[1]) + (N[2] * s[2]);
					if(lambert >= 0)
					{
						vmag = sqrt(sqr(V[0] + s[0]) + sqr(V[1] + s[1]) + sqr(V[2] + s[2]));
						H[0] = (V[0] + s[0]) / vmag;
						H[1] = (V[1] + s[1]) / vmag;
						H[2] = (V[2] + s[2]) / vmag;
						alpha = acos((N[0] * H[0]) + (N[1] * H[1]) + (N[2] * H[2]));
						float irradiance = a * lambert + (1-a) * (exp(-(alpha/m) * (alpha/m)));
						image[i][j] = irradiance * 255;
					}
				}
			}
		}
		
		// IMPORTANT: to let codes compile on Gradescope Autograder, please don't change code below.(line60 - line 137) 
		// FYI, line 60 - line 73 are inside for loop "for (k = 0; k < 9; k ++)"
		// when uploading your code file, you could delete all the comments above for better reading.
		if (!( fp = fopen( filename, "wb" ) ))
		{
		  fprintf( stderr, "error: could not open %s\n", filename);
		  exit( 1 );
		}
		fwrite( head, 4, 8, fp );
		for ( i = 0 ; i < ROWS ; i++ )
		  fwrite( image[i], sizeof(char), COLUMNS, fp );
		fclose( fp );
	}

	printf("Finished!\n");
	//gets ( &ch );
	return 0;
}

void clear( unsigned char image[][COLUMNS] )
{
	int	i,j;
	for ( i = 0 ; i < ROWS ; i++ )
		for ( j = 0 ; j < COLUMNS ; j++ ) image[i][j] = 0;
}

void header( int row, int col, unsigned char head[32] )
{
	int *p = (int *)head;
	char *ch;
	int num = row * col;

	/* Choose little-endian or big-endian header depending on the machine. Don't modify this */
	/* Little-endian for PC */
	
	*p = 0x956aa659;
	*(p + 3) = 0x08000000;
	*(p + 5) = 0x01000000;
	*(p + 6) = 0x0;
	*(p + 7) = 0xf8000000;

	ch = (char*)&col;
	head[7] = *ch;
	ch ++; 
	head[6] = *ch;
	ch ++;
	head[5] = *ch;
	ch ++;
	head[4] = *ch;

	ch = (char*)&row;
	head[11] = *ch;
	ch ++; 
	head[10] = *ch;
	ch ++;
	head[9] = *ch;
	ch ++;
	head[8] = *ch;
	
	ch = (char*)&num;
	head[19] = *ch;
	ch ++; 
	head[18] = *ch;
	ch ++;
	head[17] = *ch;
	ch ++;
	head[16] = *ch;
	

	/* Big-endian for unix */
	/*
	*p = 0x59a66a95;
	*(p + 1) = col;
	*(p + 2) = row;
	*(p + 3) = 0x8;
	*(p + 4) = num;
	*(p + 5) = 0x1;
	*(p + 6) = 0x0;
	*(p + 7) = 0xf8;
*/
}