#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
typedef struct {
	unsigned char R,G,B;
} pixel;

typedef struct nume_sabloane{

    char *nume_Sablon;

}nume_sabloane;

 void XORSHIFT32(unsigned int **aleatoare)
 {   unsigned int W,H,R0,r;
     int i;
     char nume_fisier_img[]="peppers.bmp";
     FILE *fin=fopen(nume_fisier_img,"rb");
     if(fin==NULL)
     {
         printf("Nu am gasit fisierul in format .bmp.");
         exit(-1);
     }
     fseek(fin, 18,SEEK_SET);
     fread(&W,sizeof(unsigned int),1,fin);
     fread(&H,sizeof(unsigned int),1,fin);

     *aleatoare=(unsigned int *)malloc((2*H*W)*sizeof(unsigned int));

     char fisier_cheie_secreta[]="secret_key.txt";
     FILE *key=fopen(fisier_cheie_secreta,"r");
     if(key==NULL)
     {
         printf("Nu am gasit fisierul cu cheia secreta.");
         exit(-1);
     }
     fscanf(key,"%u",&R0);
     (*aleatoare)[0]=R0;
    for(i=1;i<=2*W*H-1;i++)
    {   r=(*aleatoare)[i-1];
        r=r^r<<13;
        r=r^r>>17;
        r=r^r<<5;
        (*aleatoare)[i]=r;

    }
    fclose(fin);
    fclose(key);
 }
 void liniarizare(unsigned char **header, pixel **lin, char*nume_fisier_img)
 {   unsigned int h,H,W,padding;
     FILE *fin=fopen(nume_fisier_img,"rb");
     if(fin==NULL)
     {
         printf("Nu am gasit fisierul in format .bmp.");
         exit(-1);
     }

     (*header)=(unsigned char *)malloc(54*sizeof(unsigned char));
     fseek(fin,0,SEEK_SET);
     for(h=0;h<54;h++)
        fread(&(*header)[h],1,1,fin);

     fseek(fin,18,SEEK_SET);
     fread(&W,sizeof(unsigned int),1,fin);
     fread(&H,sizeof(unsigned int),1,fin);

     if(W % 4 != 0)
        padding = 4 - (3 * W) % 4;
     else
        padding = 0;

     *lin=(pixel*)malloc(H*W*(sizeof(pixel)));
     fseek(fin,54,SEEK_SET);
     for(int i=0;i<H;i++)
        {for(int j=0;j<W;j++)
            {
            fread(&(*lin)[W*(H-i-1)+j].B,1,1,fin);
            fread(&(*lin)[W*(H-i-1)+j].G,1,1,fin);
            fread(&(*lin)[W*(H-i-1)+j].R,1,1,fin);
            }
        fseek(fin,padding,SEEK_CUR);
        }
    fclose(fin);
 }
 void fisier_liniarizat(char *nume_fisier_img ,char* nume_fisier_img_liniarizata)
 {
    pixel *lin;
    unsigned char *header;
    unsigned int H,W,padding;
    FILE *fin=fopen(nume_fisier_img,"rb");
     if(fin==NULL)
     {
         printf("Nu am gasit fisierul in format .bmp pe care dorim sa-l liniarizam.");
         exit(-1);
     }
    fseek(fin,18,SEEK_SET);
    fread(&W,sizeof(unsigned int),1,fin);
    fread(&H,sizeof(unsigned int),1,fin);
    if(W % 4 != 0)
        padding = 4 - (3 * W) % 4;
     else
        padding = 0;
    FILE *fout=fopen(nume_fisier_img_liniarizata,"wb");

    liniarizare(&header,&lin,nume_fisier_img);
    for(int i=0;i<54;i++)
        fwrite(&header[i],1,1,fout);
    for(int i=0;i<H;i++)
        {   for(int j=0;j<W;j++)
            {
            fwrite(&lin[W*(H-i-1)+j].B,1,1,fout);
            fflush(fout);
            fwrite(&lin[W*(H-i-1)+j].G,1,1,fout);
            fflush(fout);
            fwrite(&lin[W*(H-i-1)+j].R,1,1,fout);
            fflush(fout);
            }
        fseek(fin,padding,SEEK_CUR);
        }
    fclose(fin);
    fclose(fout);
 }
void criptare(char *nume_fisier_img,char *nume_img_criptata,char *fisier_cheie_secreta,pixel **criptata)
{
    FILE *fin,*fout,*key;
    fin=fopen(nume_fisier_img,"rb");
    fout=fopen(nume_img_criptata,"wb");
    key=fopen(fisier_cheie_secreta,"r");
    if(fin==NULL)
    {
        printf("Nu am gasit fisierul cu imaginea initiala pentru criptare.");
        exit(-1);
    }
    if(key==NULL)
    {
        printf("Nu am gasit fisierul cu cheia secreta.");
        exit(-1);
    }
    pixel *lin,*intermediara;
    unsigned char *header;
    unsigned int H,W,k,r,SV,R0;
    unsigned int *aleatoare,*permutare;

    XORSHIFT32(&aleatoare);

    liniarizare(&header,&lin,nume_fisier_img);

    fseek(fin,18,SEEK_SET);
    fread(&W,sizeof(unsigned int),1,fin);
    fread(&H,sizeof(unsigned int),1,fin);

    permutare=(unsigned int*)malloc(H*W*sizeof(unsigned int));
    intermediara=(pixel*)malloc(H*W*(sizeof(pixel)));
    *criptata=(pixel*)malloc(H*W*sizeof(pixel));

    int i,aux;
    for(k=0;k<H*W;k++)
        permutare[k]=k;
    for(k=H*W-1;k>=1;k--)             ///PERMUTARE ALEATOARE
    {
        r=aleatoare[(W*H-k)]%(k+1);
        aux=permutare[r];
        permutare[r]=permutare[k];
        permutare[k]=aux;
    }
    for(i=0;i<H*W;i++)                   ///IMAGINE INTERMEDIARA
    {
        intermediara[permutare[i]].B=lin[i].B;
        intermediara[permutare[i]].G=lin[i].G;
        intermediara[permutare[i]].R=lin[i].R;
    }
    fscanf(key,"%u %u",&R0,&SV);

    int j=H*W;
    (*criptata)[0].B =(intermediara[0].B)^(SV&0xFF)^(aleatoare[j]&0xFF);
    (*criptata)[0].G =(intermediara[0].G)^((SV>>8)&0xFF)^((aleatoare[j]>>8)& 0xFF);
    (*criptata)[0].R =(intermediara[0].R)^((SV>>16)&0xFF)^((aleatoare[j]>>16) & 0xFF);
    j++;
    for(i=1;i<H*W;i++)                              ///IMAGINE XOR-ATA
    {
        (*criptata)[i].B=(((*criptata)[i-1].B)^(intermediara[i].B)^(aleatoare[j]& 0xFF));
        (*criptata)[i].G=(((*criptata)[i-1].G)^(intermediara[i].G)^(aleatoare[j]>>8 & 0xFF));
        (*criptata)[i].R=(((*criptata)[i-1].R)^(intermediara[i].R)^(aleatoare[j]>>16  & 0xFF));
        j++;
    }
    for(int i=0;i<54;i++)
        fwrite(&header[i],1,1,fout);

    for(int i=0;i<H;i++)                            ///scrie in fisierul de iesire imaginea criptata
        for(int j=0;j<W;j++)
        {
            fwrite(&(*criptata)[W*(H-i-1)+j].B,1,1,fout);
            fflush(fout);
            fwrite(&(*criptata)[W*(H-i-1)+j].G,1,1,fout);
            fflush(fout);
            fwrite(&(*criptata)[W*(H-i-1)+j].R,1,1,fout);
            fflush(fout);
        }
    fclose(key);
    fclose(fin);
    fclose(fout);
}
void decriptare(char *nume_fisier_criptata,char *nume_img_decriptata,char *fisier_cheie_secreta,pixel **decriptata)
{
    char nume_fisier_img[]="peppers.bmp",nume_img_criptata[]="img.criptata.bmp",cheie_secreta[]="secret_key.txt";
    FILE *fin,*fout,*key;
    fin=fopen(nume_fisier_criptata,"rb");
    fout=fopen(nume_img_decriptata,"wb");
    key=fopen(fisier_cheie_secreta,"r");
    if(fin==NULL)
    {
        printf("Nu am gasit fisierul cu imaginea criptata.");
        exit(-1);
    }
    if(key==NULL)
    {
        printf("Nu am gasit fisierul cu cheia secreta.");
        exit(-1);
    }
    pixel *lin,*intermediara,*criptata;
    unsigned char *header;
    unsigned int H,W,k,r,SV,R0;
    unsigned int *aleatoare,*permutare,*inversa;

    XORSHIFT32(&aleatoare);
    liniarizare(&header,&lin,nume_fisier_img);

    fseek(fin,18,SEEK_SET);
    fread(&W,sizeof(unsigned int),1,fin);
    fread(&H,sizeof(unsigned int),1,fin);

    permutare=(unsigned int*)malloc(H*W*sizeof(unsigned int));
    inversa=(unsigned int*)malloc(H*W*sizeof(unsigned int));
    intermediara=(pixel*)malloc(H*W*(sizeof(pixel)));
    criptata=(pixel*)malloc(H*W*sizeof(pixel));
    *decriptata=(pixel*)malloc(H*W*sizeof(pixel));

    criptare(nume_fisier_img,nume_img_criptata, cheie_secreta,&criptata);

    int i,aux;
    for(k=0;k<H*W;k++)
        permutare[k]=k;
    for(k=H*W-1;k>=1;k--)             ///PERMUTARE ALEATOARE
    {
        r=aleatoare[(W*H-k)]%(k+1);
        aux=permutare[r];
        permutare[r]=permutare[k];
        permutare[k]=aux;
    }
    for(i=0;i<H*W;i++)
        inversa[permutare[i]]=i;
    fscanf(key,"%u %u",&R0,&SV);

    int j=H*W;
    intermediara[0].B =(criptata[0].B)^(SV&0xFF)^(aleatoare[j] & 0xFF);
    intermediara[0].G =(criptata[0].G)^(SV>>8&0xFF)^((aleatoare[j]>>8)& 0xFF);
    intermediara[0].R =(criptata[0].R)^(SV>>16&0xFF)^((aleatoare[j]>>16) & 0xFF);
    j++;
    for(i=1;i<H*W;i++)                              ///IMAGINE XOR-ATA
    {
        intermediara[i].B=(criptata[i-1].B)^(criptata[i].B)^(aleatoare[j]& 0xFF);
        intermediara[i].G=(criptata[i-1].G)^(criptata[i].G)^((aleatoare[j]>>8) & 0xFF);
        intermediara[i].R=(criptata[i-1].R)^(criptata[i].R)^((aleatoare[j]>>16 ) & 0xFF);
        j++;
    }

    for(i=0;i<H*W;i++)
    {
        (*decriptata)[inversa[i]].B=intermediara[i].B;
        (*decriptata)[inversa[i]].G=intermediara[i].G;
        (*decriptata)[inversa[i]].R=intermediara[i].R;
    }
    for(int i=0;i<54;i++)
        fwrite(&header[i],1,1,fout);

    for(int i=0;i<H;i++)
        for(int j=0;j<W;j++)
        {
            fwrite(&(*decriptata)[W*(H-i-1)+j].B,1,1,fout);
            fflush(fout);
            fwrite(&(*decriptata)[W*(H-i-1)+j].G,1,1,fout);
            fflush(fout);
            fwrite(&(*decriptata)[W*(H-i-1)+j].R,1,1,fout);
            fflush(fout);
        }

    free(criptata);
    free(decriptata);
    free(inversa);
    free(intermediara);
    free(aleatoare);
    free(permutare);
    fclose(fin);
    fclose(fout);
}
void chi_squared_test(char *nume_fisier_img)
{   FILE *fin;
    fin=fopen(nume_fisier_img,"rb");
    if(fin==NULL)
    {
        printf("Nu am gasit fisierul pentru care calculam valorile testului chi-patrat. ");
        exit(-1);
    }
    int *f_R,*f_G,*f_B;
    ///vectori de frecventa pentru fiecare culoare
    f_R=(int*)calloc(256,sizeof(unsigned int));
    f_G=(int*)calloc(256,sizeof(unsigned int));
    f_B=(int*)calloc(256,sizeof(unsigned int));
    double f,chi_B=0,chi_G=0,chi_R=0;

    unsigned char *header;
    pixel *lin;
    unsigned int H,W;
    fseek(fin,18,SEEK_SET);
    fread(&W,sizeof(unsigned int),1,fin);
    fread(&H,sizeof(unsigned int),1,fin);

    liniarizare(&header,&lin,nume_fisier_img);
    f=(W*H)/256;   ///frecventa estimata teoretic a oricarei valori i

    ///incrementam vectorii de frecvanta
    for(int i=0;i<H*W;i++)
    {
        f_B[lin[i].B]++;
        f_G[lin[i].G]++;
        f_R[lin[i].R]++;
    }
    ///calculeaza valorile testului chi-patrat pentru fiecare canal de culoare
    for(int i=0;i<256;i++)
    {
        chi_B=chi_B+(((f_B[i]-f)*(f_B[i]-f))/f);
        chi_G=chi_G+(((f_G[i]-f)*(f_G[i]-f))/f);
        chi_R=chi_R+(((f_R[i]-f)*(f_R[i]-f))/f);
    }
    printf("Pentru R:%0.2lf \nPentru G:%0.2lf \nPentru B:%0.2lf",chi_R,chi_G,chi_B);
    free(lin);
    free(header);
    fclose(fin);
}

void grayscale_image(char* nume_fisier_sursa,char* nume_fisier_destinatie)
{
   FILE *fin, *fout;
   unsigned int W, H;
   unsigned char pRGB[3], aux;


   fin = fopen(nume_fisier_sursa, "rb");
   if(fin == NULL)
   	{
   		printf("Nu am gasit imaginea sursa din care citesc");
   		return;
   	}

   fout = fopen(nume_fisier_destinatie, "wb+");

   fseek(fin, 18, SEEK_SET);
   fread(&W, sizeof(unsigned int), 1, fin);
   fread(&H, sizeof(unsigned int), 1, fin);

	fseek(fin,0,SEEK_SET);
	unsigned char c;
	while(fread(&c,1,1,fin)==1)
	{
		fwrite(&c,1,1,fout);
	}
	fclose(fin);
	int padding;
    if(W % 4 != 0)
        padding = 4 - (3 * W) % 4;
    else
        padding = 0;

	fseek(fout, 54, SEEK_SET);
	int i,j;
	for(i = 0; i < H; i++)
	{
		for(j = 0; j < W; j++)
		{
			fread(pRGB, 3, 1, fout);
			aux = 0.299*pRGB[2] + 0.587*pRGB[1] + 0.114*pRGB[0];
			pRGB[0] = pRGB[1] = pRGB[2] = aux;
        	fseek(fout, -3, SEEK_CUR);
        	fwrite(pRGB, 3, 1, fout);
        	fflush(fout);
		}
		fseek(fout,padding,SEEK_CUR);
	}
	fclose(fout);
}

double medie_intensitati(pixel**matrice,int i,int j)
{
    int i1,j1,h=15,w=11;
    double medie=0;
    for(i1=i;i1<h+i;i1++)
        {for(j1=j;j1<w+j;j1++)
            {
            medie=medie+matrice[i1][j1].B;
            }
        }
    medie=medie/(h*w);
    return medie;
}
double deviatie(pixel**matrice,int i,int j)
{
    int i1,j1,h=15,w=11;
    int k=i,l=j;
    double medie=medie_intensitati(matrice,k,l);
    double deviatie=0;
    for(i1=i;i1<h+i;i1++)
        for(j1=j;j1<w+j;j1++)
            deviatie=deviatie+(matrice[i1][j1].B-medie)*(matrice[i1][j1].B-medie);

    deviatie=deviatie/(h*w-1);
    deviatie=sqrt(deviatie);
    return deviatie;
}

pixel alege_culoare(int cifra){ ///alege culoare pt contur

    pixel culoare;

    switch(cifra){

        case 0: {

                culoare.B = 0;
                culoare.G = 0;
                culoare.R = 255;

                break;
        }

         case 1: {

                culoare.B = 0;
                culoare.G = 255;
                culoare.R = 255;

                break;
        }

         case 2: {

                culoare.B = 0;
                culoare.G = 255;
                culoare.R = 0;

                break;
        }

         case 3: {

                culoare.B = 255;
                culoare.G = 255;
                culoare.R = 0;

                break;
        }

         case 4: {

                culoare.B = 255;
                culoare.G = 0;
                culoare.R = 255;

                break;
        }

         case 5: {

                culoare.B = 255;
                culoare.G = 0;
                culoare.R = 0;

                break;
        }

         case 6: {

                culoare.B = 192;
                culoare.G = 192;
                culoare.R = 192;

                break;
        }

         case 7: {

                culoare.B = 0;
                culoare.G = 140;
                culoare.R = 255;

                break;
        }

         case 8: {

                culoare.B = 128;
                culoare.G = 0;
                culoare.R = 128;

                break;
        }

         case 9: {

                culoare.B = 128;
                culoare.G = 0;
                culoare.R = 0;

                break;
        }

        default: {

                printf("\n Culoare inexistenta\n ");
                break;

        }

    }

    return culoare;

}

void desenez_contur(pixel ***img_matrice,int i1,int j1,unsigned char culoareB, unsigned char culoareG,unsigned char culoareR)
{

    for ( int y = j1; y <= j1 + 11; y++ ){ ///linia de sus
        (*img_matrice)[i1][y].B = culoareB;
        (*img_matrice)[i1][y].G = culoareG;
        (*img_matrice)[i1][y].R = culoareR;
    }

    for ( int x = i1+1; x <= i1 + 15; x++ ){///prima coloana si ultima coloana

        (*img_matrice)[x][j1].B = culoareB;
        (*img_matrice)[x][j1].G = culoareG;
        (*img_matrice)[x][j1].R = culoareR;

        (*img_matrice)[x][j1 + 11].B = culoareB;
        (*img_matrice)[x][j1 + 11].G = culoareG;
        (*img_matrice)[x][j1 + 11].R = culoareR;

    }


    for ( int y = j1; y <= j1 + 11; y++ ){/// linia de jos
        (*img_matrice)[i1+15][y].B = culoareB;
        (*img_matrice)[i1+15][y].G = culoareG;
        (*img_matrice)[i1+15][y].R = culoareR;
    }

}
typedef struct
{
    int indice_i,indice_j,nrSablon,stergere;
    double corr;
    pixel culoare;

}detectie;
int cmp(const void *corr1,const void *corr2)
{
    detectie a=*(detectie*)corr1;
    detectie b=*(detectie*)corr2;
    if(b.corr>a.corr)
        return 1;
    return -1;
}
double suprapun(int indice_i_f1,int indice_j_f1,int indice_i_f2,int indice_j_f2)
{
    if (indice_i_f1 + 15 <= indice_i_f2 ||indice_j_f1 + 11 <=indice_j_f2)
        return 0;
    if (indice_i_f2 + 15 <= indice_i_f1 ||indice_j_f2 + 11 <=indice_j_f1)
        return 0;
    int arie_f1,arie_f2,intersectie,reuniune;
    arie_f1=arie_f2=165;
    if(indice_i_f1 < indice_i_f2 && indice_j_f1 < indice_j_f2 )///cazul 1 prezzentat in PDF
        intersectie=(indice_i_f1+15-indice_i_f2)*(indice_j_f1+11-indice_j_f2);

    if(indice_i_f1 < indice_i_f2 && indice_j_f1 > indice_j_f2 )///cazul 2 prezentat in PDF
        intersectie=(indice_i_f1+15-indice_i_f2)*(indice_j_f2+11-indice_j_f1);

    reuniune=arie_f1+arie_f2-intersectie;
    double suprapunere=(double)intersectie/(double)reuniune;
    return suprapunere;
}

void template_matching(char *nume_imagine ,double ps,detectie **vector_detectii,int *cnt)
{
    unsigned int H,W,h,w,padding1,padding2;
    FILE *img,*sablon,*img2,*sablon2;
    double corr;
    char nume_imagine2[]="test_grayscale.bmp";
    char nume_sablon2[]="sablon_grayscale.bmp";
    img=fopen(nume_imagine,"rb");

    pixel **sablon_matrice,**img_auxiliara;

    if(img==NULL)
    {
        printf("Nu am gasit imaginea color pentru template matching.");
        exit(-1);
    }


    fseek(img,18,SEEK_SET);
    fread(&W,sizeof(unsigned int),1,img);
    fread(&H,sizeof(unsigned int),1,img);

     if(W % 4 != 0)
        padding1 = 4 - (3 * W) % 4;
     else
        padding1 = 0;

     img2=fopen(nume_imagine2,"wb+");
     grayscale_image(nume_imagine,nume_imagine2);

     img_auxiliara=(pixel**)malloc(H*(sizeof(pixel*)));
     for(int i=0;i<H;i++)
        img_auxiliara[i] = calloc(W, sizeof(pixel));

     fseek(img2,54,SEEK_SET);

     for(int i=0;i<H;i++)                                   ///trecem imaginea grayscale in forma matriceala
        {for(int j=0;j<W;j++)
            {
            fread(&img_auxiliara[H-i-1][j].B,1,1,img2);
            fread(&img_auxiliara[H-i-1][j].G,1,1,img2);
            fread(&img_auxiliara[H-i-1][j].R,1,1,img2);
            }
        fseek(img2,padding1,SEEK_CUR);
        }

    /// incarcam si prelucram sabloanele

    nume_sabloane *nume = NULL;
    nume = malloc(10 * sizeof(nume_sabloane));

    for ( int i = 0; i < 10; i++)
        nume[i].nume_Sablon = malloc(20 * sizeof(char));
    for(int j=0;j<10;j++)
    {
        printf("\nNumele sablonului cu cifra %d:",j);
        fgets( nume[j].nume_Sablon,20,stdin);
        nume[j].nume_Sablon[strlen(nume[j].nume_Sablon)-1]='\0';
    }

    (*vector_detectii)=(detectie*)malloc(sizeof(detectie));
    (*cnt)=0;

    for ( int nr_sablon = 0; nr_sablon < 10; nr_sablon++){ ///se prelucreaza fiecare sablon

            printf("Template matching pentru sablon: %d\n",nr_sablon);

                sablon=fopen(nume[nr_sablon].nume_Sablon,"rb");

                if(sablon==NULL)
                {
                    printf("Nu am gasit sablonul color pentru template matching.");
                    exit(-1);
                }

                sablon2 = fopen(nume_sablon2,"wb+");

                grayscale_image(nume[nr_sablon].nume_Sablon,nume_sablon2);

                fseek(sablon2,18,SEEK_SET);
                fread(&w,sizeof(unsigned int),1,sablon2);
                fread(&h,sizeof(unsigned int),1,sablon2);
                 if(w % 4 != 0)
                    padding2 = 4 - (3 * w) % 4;
                 else
                    padding2 = 0;

                 sablon_matrice=(pixel**)malloc(h*(sizeof(pixel*)));
                 for(int i=0;i<h;i++)
                    sablon_matrice[i] = calloc(w, sizeof(pixel));

                 fseek(sablon2,54,SEEK_SET);
                 for(int i=0;i<h;i++)
                    {for(int j=0;j<w;j++)
                        {
                        fread(&sablon_matrice[h-i-1][j].B,1,1,sablon2);      ///trecem sablonul in forma matriceala
                        fread(&sablon_matrice[h-i-1][j].G,1,1,sablon2);
                        fread(&sablon_matrice[h-i-1][j].R,1,1,sablon2);
                        }
                    fseek(sablon2,padding2,SEEK_CUR);
                    }

               pixel culoare;

               double medie_intensitati_sablon = medie_intensitati(sablon_matrice,0,0);
               double deviatie_sablon = deviatie(sablon_matrice,0,0);

               for(int i1=0;i1<H-h;i1++)
                    {for(int j1=0;j1<W-w;j1++)
                        {   corr=0;
                            double rezultat = 0;

                            double medie_intensitati_fereastra = medie_intensitati(img_auxiliara,i1,j1);
                            double deviatie_fereastra = deviatie(img_auxiliara,i1,j1);

                            for(int i=0;i<h;i++){
                                for(int j=0;j<w;j++)
                                {
                                    rezultat= ((double)img_auxiliara[i+i1][j+j1].B - medie_intensitati_fereastra )*((double)sablon_matrice[i][j].B-medie_intensitati_sablon);
                                    rezultat= rezultat / ( deviatie_sablon * deviatie_fereastra );

                                    corr = corr + rezultat;

                                }

                            }

                            corr=(double)corr/((double)w*(double)h);

                            if(corr >= ps){

                                  culoare = alege_culoare(nr_sablon);
                                  (*vector_detectii)[(*cnt)].indice_i=i1;
                                  (*vector_detectii)[(*cnt)].indice_j=j1;
                                  (*vector_detectii)[(*cnt)].corr=corr;
                                  (*vector_detectii)[(*cnt)].culoare=culoare;
                                  (*vector_detectii)[(*cnt)].nrSablon=nr_sablon;
                                  (*vector_detectii)[(*cnt)].stergere=0;

                                  (*cnt)++;

                                  (*vector_detectii)=realloc((*vector_detectii), ((*cnt)+1)*sizeof(detectie));
                            }

                        }

                    }
                    for(int i=0;i<h;i++)
                        free(sablon_matrice[i]);

                    free(sablon_matrice);
    }


    for(int i=0;i<H;i++)
        free(img_auxiliara[i]);

    free(img_auxiliara);

    fclose(img);
    fclose(img2);
    fclose(sablon);


}
void elimin_nonmaxime(char *nume_imagine,detectie **vector_detectii,int *cnt)
{
    FILE *fin,*fout;
    fin=fopen(nume_imagine,"rb");
    if(fin==NULL)
    {
        printf("Eroare la deschiderea fisierului test.bmp");
        exit(-1);
    }
    unsigned char *header;
    unsigned int H,W,padding1;
    pixel**img_matrice;

    header=(unsigned char *)malloc(54*sizeof(unsigned char));

    fseek(fin,0,SEEK_SET);
    for(int q=0;q<54;q++)
        fread(&header[q],1,1,fin);

    fseek(fin,18,SEEK_SET);
    fread(&W,sizeof(unsigned int),1,fin);
    fread(&H,sizeof(unsigned int),1,fin);

     if(W % 4 != 0)
        padding1 = 4 - (3 * W) % 4;
     else
        padding1 = 0;


     img_matrice=(pixel**)malloc(H*(sizeof(pixel*)));
     for(int i=0;i<H;i++)
        img_matrice[i] = calloc(W, sizeof(pixel));

     fseek(fin,54,SEEK_SET);
     for(int i=0;i<H;i++)
        {for(int j=0;j<W;j++)
            {
            fread(&img_matrice[H-i-1][j].B,1,1,fin);
            fread(&img_matrice[H-i-1][j].G,1,1,fin);
            fread(&img_matrice[H-i-1][j].R,1,1,fin);
            }
        fseek(fin,padding1,SEEK_CUR);
        }

    qsort((*vector_detectii),(*cnt),sizeof(detectie),cmp);

    for(int a=0;a<(*cnt)-1;a++)
        for(int b=a+1;b<(*cnt);b++)
            {
                if(suprapun((*vector_detectii)[a].indice_i,(*vector_detectii)[a].indice_j,(*vector_detectii)[b].indice_i,(*vector_detectii)[b].indice_j)>0.2)

                    {if((*vector_detectii)[a].corr >(*vector_detectii)[b].corr)

                        (*vector_detectii)[b].stergere=1;

                    else

                        (*vector_detectii)[a].stergere=1;
                    }

            }
    for(int k=0;k<(*cnt);k++)
        if((*vector_detectii)[k].stergere==0)

    desenez_contur(&img_matrice,(*vector_detectii)[k].indice_i,(*vector_detectii)[k].indice_j,(*vector_detectii)[k].culoare.B,(*vector_detectii)[k].culoare.G,(*vector_detectii)[k].culoare.R);

    ///scrierea imaginii finale

   fout=fopen("img_desenata_fin.bmp","wb");

     if ( fout == NULL ){
        printf("Eroare\n");
        exit(-1);
    }

   for(int q=0;q<54;q++)
        fwrite(&header[q],1,1,fout);

    for(int i=0;i<H;i++)
        {for(int j=0;j<W;j++)
            {
            fwrite(&(img_matrice)[H-i-1][j].B,1,1,fout);
            fwrite(&(img_matrice)[H-i-1][j].G,1,1,fout);
            fwrite(&(img_matrice)[H-i-1][j].R,1,1,fout);
            }
        fseek(fout,padding1,SEEK_CUR);
        }
    for(int i=0;i<H;i++)
        free(img_matrice[i]);

    free(img_matrice);

    free(header);
    free(vector_detectii);

    fclose(fin);
    fclose(fout);

}

int main()
{
    int cerinta;
    printf("Tastati 1 pentru criptare.\n");
    printf("Tastati 2 pentru decriptare.\n");
    printf("Tastati 3 pentru afisarea valorilor testului chi-patrat.\n");
    printf("Tastati 4 pentru template matching si eliminare non_maxime.\n");
    for(int i=1;i<20;i++)
        {
            printf("\nCerinta: ");
            scanf("%d", &cerinta);
            switch(cerinta)
            {
            case 1:
                {
                    char nume_fisier_img[101];
                    char nume_img_criptata[101];
                    char cheie_secreta[101];
                    fgets(nume_fisier_img,101,stdin);
                    printf("Numele fisierului cu imaginea pe care vrem sa o criptam:");
                    fgets(nume_fisier_img,101,stdin);
                    nume_fisier_img[strlen(nume_fisier_img)-1]='\0';

                    printf("\nNumele fisierului care va contine imaginea criptata: ");
                    fgets(nume_img_criptata, 101, stdin);
                    nume_img_criptata[strlen(nume_img_criptata) - 1] = '\0';

                    printf("\nNumele fisierului care contine cheia secreta:");
                    fgets(cheie_secreta, 101, stdin);
                    cheie_secreta[strlen(cheie_secreta) - 1] = '\0';

                    char nume_fisier_img_liniarizata[]="imagine.liniarizata.bmp";
                    pixel *criptata;

                    fisier_liniarizat(nume_fisier_img,nume_fisier_img_liniarizata);
                    criptare(nume_fisier_img,nume_img_criptata, cheie_secreta,&criptata);
                    printf("Am criptat imaginea %s.",nume_fisier_img );
                    printf("\n____________________________________________________");
                    break;
                }
            case 2:
                {
                    char nume_img_criptata[101], nume_img_decriptata[101],cheie_secreta[101];
                    fgets(nume_img_criptata, 101, stdin);
                    printf("Numele fisierului cu imaginea pe care vrem sa o decriptam:");
                    fgets(nume_img_criptata, 101, stdin);
                    nume_img_criptata[strlen(nume_img_criptata) - 1] = '\0';

                    printf("\nNumele fisierului care va contine imaginea decriptata:");
                    fgets(nume_img_decriptata, 101, stdin);
                    nume_img_decriptata[strlen(nume_img_decriptata) - 1] = '\0';

                    printf("\nNumele fisierului care contine cheia secreta:");
                    fgets(cheie_secreta, 101, stdin);
                    cheie_secreta[strlen(cheie_secreta) - 1] = '\0';

                    pixel *decriptata;
                    decriptare(nume_img_criptata,nume_img_decriptata, cheie_secreta,&decriptata);
                    printf("Am decriptat imaginea %s",nume_img_criptata );
                    printf("\n____________________________________________________");
                    break;
                }
            case 3:
                {
                    char nume_fisier_img[101],nume_img_criptata[101];
                    fgets(nume_fisier_img,101,stdin);

                    printf("Numele fisierului cu imaginea initiala pentru care calculam valorile testului chi-patrat:");
                    fgets(nume_fisier_img,101,stdin);
                    nume_fisier_img[strlen(nume_fisier_img)-1]='\0';

                    printf("\nValorile testului chi-patrat pentru imaginea initiala:\n");
                    chi_squared_test(nume_fisier_img);

                    printf("\n\nNumele fisierului care contine imaginea criptata pentru care calculam valorile testului chi-patrat: ");
                    fgets(nume_img_criptata, 101, stdin);
                    nume_img_criptata[strlen(nume_img_criptata) - 1] = '\0';

                    printf("\nValorile testului chi-patrat pentru imaginea criptata:\n");
                    chi_squared_test(nume_img_criptata);
                    printf("\n____________________________________________________");
                    break;
                }
            case 4:
                {
                    char nume_imagine[101];
                    fgets(nume_imagine, 101, stdin);
                    printf("Numele fisierului cu imaginea color pentru care facem operatia template matching:");
                    fgets(nume_imagine, 101, stdin);
                    nume_imagine[strlen(nume_imagine) - 1] = '\0';
                    double ps = 0.50;
                    int cnt;
                    detectie *vector_detectii;
                    template_matching(nume_imagine,ps,&vector_detectii, &cnt);
                    printf("Am creat vectorul de detectii.\n");
                    elimin_nonmaxime(nume_imagine,&vector_detectii, &cnt);
                    printf("Am eliminat non-maximele si am desenat in imaginea <img_desenata_fin> detectiile ramase.");
                    printf("\n____________________________________________________");
                    break;

                }

            default:
                {
                    printf("Cerinta gresita.");
                    break;
                }
            }
        }

    return 0;
}
