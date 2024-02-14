#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h> // Include this for bool type

#define ROWS    480
#define COLUMNS 640
#define PI 3.14159265358979323846

#define sqr(x)  ((x)*(x))

void clear(unsigned char image[][COLUMNS]);
void header(int row, int col, unsigned char head[32]);
void fill_voting_array(unsigned char voting[180][400], int i, int j);
void find_max(unsigned char voting[180][400], int max[3], int index[4][2]);
int main(int argc, char **argv)
{
    int i, j, sgmmax, a;
    int dedx, dedy, sgm;
    int sgm_threshold, hough_threshold, voting[180][400];
    FILE *fp;
    unsigned char image[ROWS][COLUMNS], simage[ROWS][COLUMNS], head[32];
    char filename[50], ifilename[50];
    float theta, rho;
    int index[4][2] = {0}; // Initialize index array
	int localmax[3] = {0, 0, 0};

    clear(simage);
    strcpy(filename, "image.raw");
    memset(voting, 0, sizeof(int) * 180 * 400);
    header(ROWS, COLUMNS, head);
    sgmmax = 0;
    sgm_threshold = 100;

    /* Read in the image */
    if (!(fp = fopen(filename, "rb")))
    {
        fprintf(stderr, "error: couldn't open %s\n", argv[1]);
        exit(1);
    }

    for (i = 0; i < ROWS; i++)
        if (!(COLUMNS == fread(image[i], sizeof(char), COLUMNS, fp)))
        {
            fprintf(stderr, "error: couldn't read %s\n", argv[1]);
            exit(1);
        }
    fclose(fp);

    /* Compute SGM */
    for (i = 1; i < ROWS - 1; i++)
    {
        for (j = 1; j < COLUMNS - 1; j++)
        {
            dedx = abs(image[i - 1][j - 1] + 2 * image[i][j - 1] + image[i + 1][j - 1] -
                       image[i - 1][j + 1] - 2 * image[i][j + 1] - image[i + 1][j + 1]);

            dedy = abs(image[i - 1][j - 1] + 2 * image[i - 1][j] + image[i - 1][j + 1] -
                       image[i + 1][j - 1] - 2 * image[i + 1][j] - image[i + 1][j + 1]);

            sgm = sqr(dedx) + sqr(dedy);
            if (sgm > sgmmax)
                sgmmax = sgm;
        }
    }

    for (i = 1; i < ROWS - 1; i++)
        for (j = 1; j < COLUMNS - 1; j++)
        {
            dedx = abs(image[i - 1][j - 1] + 2 * image[i][j - 1] + image[i + 1][j - 1] -
                       image[i - 1][j + 1] - 2 * image[i][j + 1] - image[i + 1][j + 1]);

            dedy = abs(image[i - 1][j - 1] + 2 * image[i - 1][j] + image[i - 1][j + 1] -
                       image[i + 1][j - 1] - 2 * image[i + 1][j] - image[i + 1][j + 1]);
            simage[i][j] = (float)(sqr(dedx) + sqr(dedy)) / sgmmax * 255;
        }

    /* build up voting array */
    sgm_threshold = 100;
    for (i = 0; i < ROWS; i++)
        for (j = 0; j < COLUMNS; j++)
            if (simage[i][j] == 255)
            	fill_voting_array(voting, j, ROWS-i-1);

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

    /* Save binary image */
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

    /* Save voting image */
    strcpy(filename, "image");
	header(180, 400, head);
    if (!(fp = fopen(strcat(filename, "-voting_array.ras"), "wb")))
    {
        fprintf(stderr, "error: could not open %s\n", filename);
        exit(1);
    }
    fwrite(head, 4, 8, fp);
    for (i = 0; i < 180; i++)
        fwrite(voting[i], sizeof(char), 400, fp);
    fclose(fp);



	// local maxima
	find_max(voting, localmax, index);	

    hough_threshold = localmax[2];
	for(i=0; i < 180; i++)
		for(j=0; j < 400; j++)
			voting[i][j] = (voting[i][j] < hough_threshold) ? 0:255;

	printf("Hough threshold: %d\n", hough_threshold);
    printf("%d %d %d\n%d %d %d\n%d %d %d\n", index[0][0], index[0][1], localmax[0],
                                            index[1][0], index[1][1] , localmax[1],
                                            index[2][0], index[2][1], localmax[2]);
	
	//clear(simage);
	// reconstructing image from voting array
	for(i=0; i < ROWS -1; i++)
		for(j=0; j < COLUMNS-1; j++)
		{
			float radians = (index[2][0] * PI) / 180;
			rho = (index[2][1]-200)*3;
			if((int)((j*sinf(radians)) - ((ROWS-i-1)*cosf(radians)) + rho) == 0)
				simage[i][j] = 255;

			radians = (index[1][0] * PI) / 180;
			rho = (index[1][1] - 200) * 3;
			if((int)((j*sinf(radians)) - ((ROWS-i-1)*cosf(radians)) + rho) == 0)
				simage[i][j] = 255;

			radians = (index[0][0] * PI) / 180;
			rho = (index[0][1] - 200) * 3;
			if((int)((j*sinf(radians)) - ((ROWS-i-1)*cosf(radians)) + rho) == 0)
				simage[i][j] = 255;

		}
	

	// reconstructed image
	strcpy(filename, "image");
	header(ROWS, COLUMNS, head);
    if (!(fp = fopen(strcat(filename, "-reconstructed_image.ras"), "wb")))
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

void clear(unsigned char image[][COLUMNS])
{
    int i, j;
    for (i = 0; i < ROWS; i++)
        for (j = 0; j < COLUMNS; j++)
            image[i][j] = 0;
}

void header(int row, int col, unsigned char head[32])
{
    int *p = (int *)head;
    char *ch;
    int num = row * col;

    *p = 0x956aa659;
    *(p + 3) = 0x08000000;
    *(p + 5) = 0x01000000;
    *(p + 6) = 0x0;
    *(p + 7) = 0xf8000000;

    ch = (char *)&col;
    head[7] = *ch;
    ch++;
    head[6] = *ch;
    ch++;
    head[5] = *ch;
    ch++;
    head[4] = *ch;

    ch = (char *)&row;
    head[11] = *ch;
    ch++;
    head[10] = *ch;
    ch++;
    head[9] = *ch;
    ch++;
    head[8] = *ch;

    ch = (char *)&num;
    head[19] = *ch;
    ch++;
    head[18] = *ch;
    ch++;
    head[17] = *ch;
    ch++;
    head[16] = *ch;
}


void fill_voting_array(unsigned char voting[180][400], int x, int y)
{
	int t_i, r_i, a;
	for(a=0; a < 180; a++)
	{
		float rho = (float)y * cosf(a * PI/180.0) - (float)x * sinf(a * PI/ 180.0);
		t_i = a;
		r_i = (int)((rho / 4.0) + 200); // normalization
		voting[t_i][r_i]++;
	}
}



void find_max(unsigned char voting[180][400], int max[3], int index[4][2])
{
	max[0] = -1;
	max[1] = -1;
	max[2] = -1;
	int i, j, k, idx, idy, distance;
	int temp_max;
	int temp_idx = 0;
	int temp_idy = 0;
	int max_dist = 10;
	bool far;
	for(k=0; k < 3; k++)
	{
		temp_max = -1;
		distance = sqr(180) + sqr(400);
		for(i=0; i < 180; i++)
		{
			for(j=0; j<400; j++)
			{
				if(voting[i][j] > temp_max)
				{
					far = true;
					for(idx=0; idx < k; idx++)
					{
						idy = index[idx][0];
						if(idy < 0) break;
						if (abs(i-idy) < max_dist && abs(j-index[idx][1]) < max_dist && abs(j-index[idx][1]) >= abs(i-idy))
						{
							far=false;
							break;
						}
					}
					if(far)
					{
						temp_max = voting[i][j];
						temp_idx = i;
						temp_idy = j;
					}
				}
			}
		}
		
		max[k] = temp_max;
		index[k][0] = temp_idx;
		index[k][1] = temp_idy;
	}
}
