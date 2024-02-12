#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define ROWS	480
#define COLUMNS	640
#define PI 3.14159265358979323846

#define sqr(x)	((x)*(x))

void clear( unsigned char image[][COLUMNS] );
void header( int row, int col, unsigned char head[32] );

int main( int argc, char** argv )
{
	int				i,j, sgmmax;
	// localmax: number in the three bucket of voting array corrsponding to three local maxima
	// index[3][2]: used for store rho and theta
	int				dedx, dedy, sgm, localmax[3] = {0, 0, 0}, index[3][2] = { 0, 0, 0, 0, 0, 0 };
	// voting; voting array
	int				sgm_threshold, hough_threshold, voting[180][400];
	FILE*			fp;
	unsigned char	image[ROWS][COLUMNS], simage[ROWS][COLUMNS], head[32];
	char			filename[50], ifilename[50];
	float           theta, rho;
	
	clear(simage);
	strcpy ( filename, "image.raw");
	memset ( voting, 0, sizeof(int) * 180 * 400 );  //180 * 400 is the suggested size of voting array
	header(ROWS, COLUMNS, head);
	sgmmax = 0;
	sgm_threshold = 100;

	/* Read in the image */
	if (!( fp = fopen(filename, "rb" ) ))
	{
		fprintf( stderr, "error: couldn't open %s\n", argv[1]);
		exit(1);
	}

	for ( i = 0 ; i < ROWS ; i++ )
		if (!( COLUMNS == fread( image[i], sizeof(char), COLUMNS, fp ) ))
		{
			fprintf( stderr, "error: couldn't read %s\n", argv[1] );
			exit(1);
		}
	fclose(fp);

    /* Compute SGM */
	 /*some suggestions: 1) calculate max sgm value;
	 			 2) calculate normalized sgm value: sgm_norm = sgm_each_pixel / sgm_max * 255 
				  */
	for(i=1; i < ROWS-1; i++)
	{
		for(j=1; j < COLUMNS-1; j++)
		{
			dedx = abs(image[i - 1][j - 1] + 2 * image[i][j - 1] + image[i + 1][j - 1] -
                             image[i - 1][j + 1] - 2 * image[i][j + 1] - image[i + 1][j + 1]);

            dedy = abs(image[i - 1][j - 1] + 2 * image[i - 1][j] + image[i - 1][j + 1] -
                             image[i + 1][j - 1] - 2 * image[i + 1][j] - image[i + 1][j + 1]);

            sgm = sqr(dedx) + sqr(dedy);
			if(sgm > sgmmax)
				sgmmax = sgm;
		}
	}

	for(i=1; i < ROWS-1; i++) // normalizing the sgm values 
		for(j=1; j < COLUMNS-1; j++)
		{
			dedx = abs(image[i - 1][j - 1] + 2 * image[i][j - 1] + image[i + 1][j - 1] -
                             image[i - 1][j + 1] - 2 * image[i][j + 1] - image[i + 1][j + 1]);

            dedy = abs(image[i - 1][j - 1] + 2 * image[i - 1][j] + image[i - 1][j + 1] -
                             image[i + 1][j - 1] - 2 * image[i + 1][j] - image[i + 1][j + 1]);
			simage[i][j] = (float)(sqr(dedx) + sqr(dedy)) / sgmmax * 255;// Normalize SGM value
		}




	/* build up voting array */
	    /* some suggestions:
		    1) if sgm_norm is greater than the sgm_threshold, increase the vote;
			2) find out the three largest values/clusters in the voting array
		*/
	/* Save SGM to an image */
	strcpy(filename, "image");
	if (!(fp = fopen(strcat(filename, "-sgm.ras"), "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", filename);
		exit(1);
	}
	fwrite(head, 4, 8, fp);
	for (i = 0; i < ROWS; i++)
		fwrite(simage[i], sizeof(char), COLUMNS, fp);
	fclose(fp);

	sgm_threshold = 100;
	for(i=0; i < ROWS; i++) // computing binary images 
		for(j=0; j < COLUMNS; j++)
			simage[i][j] = (simage[i][j] > sgm_threshold) ? 255 : 0;

	strcpy(filename, "image");
	if (!(fp = fopen(strcat(filename, "-binary.ras"), "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", filename);
		exit(1);
	}
	fwrite(head, 4, 8, fp);
	for (i = 0; i < ROWS; i++)
		fwrite(simage[i], sizeof(char), COLUMNS, fp);
	fclose(fp);


	// initializing voting array && hough array
	int hough_array[180][400];
	for(i=0; i < 180; i++)
		for(j=0; j < 400; j++)
			voting[i][j] = 0;
			hough_array[i][j] = 0;

	for(theta=0; theta < 180; theta++)
		for(i=0; i < ROWS; i++)
			for(j=0; j < COLUMNS; j++)
				if(simage[i][j] == 255)
				{
					rho = i * cos(theta * PI / 180) - j * sin(theta * PI / 180);
					voting[(int)theta][(int)rho + 200] += 1;
				}

	for(i=0; i < 180; i++)
		for(j=0; j < 400; j++)
			if(voting[i][j] > 170 && voting[i][j] < 400)
				hough_array[i][j] = voting[i][j];

	for(i=0; i < 180; i++)
		for(j=0; j<400; j++)
			if(hough_array[i][j] > localmax[0])
				localmax[0] = hough_array[i][j];


	for(i=0; i < 180; i++)
		for(j=0; j<400; j++)

			if(hough_array[i][j] > localmax[1])
			{
				if(hough_array[i][j] == localmax[0])
					continue;
				else
					localmax[1] = hough_array[i][j];
			}

		for(i=0; i < 180; i++)
			for(j=0; j<400; j++)
				if(hough_array[i][j] > localmax[1])
				{
					if(hough_array[i][j] == localmax[0] || hough_array[i][j] == localmax[1])
						continue;
					else
						localmax[2] = hough_array[i][j];
				}



		for (i = 0; i < 180; i++) 
		{
			for (j = 0; j < 400; j++) {
				if (voting[i][j] == localmax[0]) {
					index[0][0] = i;
					index[0][1] = j - 200;
				}
				if (voting[i][j] == localmax[1]) {
					index[1][0] = i;
					index[1][1] = j-200;
				}
				if (voting[i][j] == localmax[2]) {
					index[2][0] = i;
					index[2][1] = j - 200;
				}
			}
	}

	/* Save original voting array to an image */
	/*
	strcpy(filename, "image");
	header(180, 400, head);
	if (!(fp = fopen(strcat(filename, "-voting_array.ras"), "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", filename);
		exit(1);
	}
	fwrite(head, 4, 8, fp);

	for (i = 0; i < 'depends on size of your voting array'; i++)
		fwrite(simage[i], sizeof(char), 'depends on size of your voting array', fp);
	fclose(fp);
	*/
	/* Threshold the voting array */
	int x = 0;
	for(i=0; i < ROWS; i++)
		for(j=0; j < COLUMNS; j++)
			for(x=0; x < 3; x++)
				if (abs(j * sin(index[x][0] * PI / 180) - i * cos(index[x][0] * PI / 180) + index[x][1]) == 0)
					simage[i][j] = 255;
	/* Write the thresholded voting array to a new image */
	strcpy(filename, "image");
	header(ROWS, COLUMNS, head);
	if (!(fp = fopen(strcat(filename, "-voting_array.ras"), "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", filename);
		exit(1);
	}
	fwrite(head, 4, 8, fp);

	for (i = 0; i < ROWS; i++)
		fwrite(simage[i], sizeof(char), COLUMNS, fp);
	fclose(fp);

	printf("Finished!");

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