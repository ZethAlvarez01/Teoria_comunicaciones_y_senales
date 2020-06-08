#include<stdio.h>  
#include<stdlib.h>
#include<math.h>

#ifndef MPI
#define M_PI 3.14159265358979323846
#endif

void lectura_muestras(FILE *entrada,double *arreglo_muestras_double,char *arreglo_muestras_hex,int num_muestras,int num_bits);
void regresar_arreglo_double(FILE* salida,double *arreglo_muestras_double,int num_muestras,int num_bytes_por_muestra);
int lectura_cabecera(FILE *entrada,unsigned char *cabecera,int *metadata_cabecera,int flg);
void editar_cabecera(unsigned char *cabecera,int pos, unsigned long int nuevo_valor);
void copiar_cabecera(unsigned char *cabecera,unsigned char *copia);
int  FFT(double *xr,double *xi,int N,int inverse);
void swap(double *x1,double *x2,int i,int j);
void regresar_arreglo(double *arreglo_muestras_double,int num_muestras,int bits_muestra,double *muestras_F);
void buscar_picos(double *muestras,double *picos, int num_muestras);

int main(int argc, char* argv[]){

    unsigned char cabecera[44];
    int metadata_cabecera[7]={0,0,0,0,0,0,0}; 
    
    int imprimir=0;  

     if(argc<2){

        return 0;
    }else if (argc>2){

        return 0;
    }

    FILE *entrada=fopen(argv[1],"rb");

    if (entrada==NULL){
        return 0;
    }


    lectura_cabecera(entrada,cabecera,metadata_cabecera,imprimir);

   

    int num_muestras=metadata_cabecera[5];
    int num_muestras_hex=num_muestras*(metadata_cabecera[4]/8);

    double *arreglo_muestras_double=malloc(num_muestras * sizeof(double));
    char *arreglo_muestras_hex=malloc(num_muestras_hex * sizeof(char));
   
    lectura_muestras(entrada,arreglo_muestras_double,arreglo_muestras_hex,num_muestras,metadata_cabecera[4]);

    fclose(entrada);

    int pot=0;
    while(1){
        if(num_muestras/2>pow(2,pot)) pot++;
        else    break;
    }

    int num_m_pot_2=pow(2,pot);

    double *arreglo_muestras_real=malloc(num_m_pot_2 * sizeof(double));
    double *arreglo_muestras_imag=malloc(num_m_pot_2 * sizeof(double));

    double *muestras_F=malloc(num_muestras * sizeof(double));


    for (int i = 0; i < num_m_pot_2; i++){
       arreglo_muestras_real[i] = 0.0;
       arreglo_muestras_imag[i] = 0.0;
   }

   for (int i = 0; i < num_muestras; i++){
       arreglo_muestras_real[i] = arreglo_muestras_double[i];
   }

    FFT(arreglo_muestras_real,arreglo_muestras_imag,num_m_pot_2,1);

    regresar_arreglo(arreglo_muestras_real,num_muestras,metadata_cabecera[4],muestras_F);

    
    double picos[2]={0.0,0.0};

    buscar_picos(muestras_F,picos,num_muestras);


    if((picos[0]>0.89 && picos[0]<0.90) && (picos[1]>0.98 && picos[1]<0.99))
    printf("0");
    if((picos[0]>0.39 && picos[0]<0.40) && (picos[1]>0.98 && picos[1]<0.99))
    printf("1");
    if((picos[0]>0.90 && picos[0]<0.91) && (picos[1]>0.91 && picos[1]<0.92))
    printf("2");
    if((picos[0]>0.82 && picos[0]<0.83) && (picos[1]>0.88 && picos[1]<0.89))
    printf("3");
    if((picos[0]>0.90 && picos[0]<0.91) && (picos[1]>0.94 && picos[1]<0.95))
    printf("4");
    if((picos[0]>0.96 && picos[0]<0.97) && (picos[1]>0.99 && picos[1]<1))
    printf("5");
    if((picos[0]>0.83 && picos[0]<0.84) && (picos[1]>0.92 && picos[1]<0.93))
    printf("6");
    if((picos[0]>0.79 && picos[0]<0.80) && (picos[1]>0.99 && picos[1]<1))
    printf("7");
    if((picos[0]>0.92 && picos[0]<0.93) && (picos[1]>0.96 && picos[1]<0.97))
    printf("8");
    if((picos[0]>0.87 && picos[0]<0.88) && (picos[1]>0.88 && picos[1]<0.89))
    printf("9");

    if((picos[0]>0.83 && picos[0]<0.84) && (picos[1]>0.93 && picos[1]<0.94))
    printf("A");
    if((picos[0]>0.92 && picos[0]<0.93) && (picos[1]>0.93 && picos[1]<0.94))
    printf("B");
    if((picos[0]>0.92 && picos[0]<0.93) && (picos[1]>0.97 && picos[1]<0.98))
    printf("C");
    if((picos[0]>0.72 && picos[0]<0.73) && (picos[1]>0.89 && picos[1]<0.90))
    printf("D");

    if((picos[0]>0.86 && picos[0]<0.87) && (picos[1]>0.99 && picos[1]<1))
    printf("#");
    if((picos[0]>0.65 && picos[0]<0.66) && (picos[1]>0.94 && picos[1]<95))
    printf("*");
    

    return 0;
}


int lectura_cabecera(FILE *entrada,unsigned char *cabecera,int *metadata_cabecera,int flg){
    
    int posicion_archivo=0;

    while(posicion_archivo<44){
        cabecera[posicion_archivo] = fgetc(entrada);
        posicion_archivo++;
    }


    int ind=7;
    long unsigned int tamano_archivo=0x0000;
    while(ind>=4){
        tamano_archivo<<=8;
        tamano_archivo+=cabecera[ind--];
    }
    

    ind=22;
    char b1=cabecera[ind];
    short canales=cabecera[++ind];
    canales<<=8;
    canales+=b1;
    metadata_cabecera[0]=canales;
    
    ind=27;
    long unsigned int frecuencia_muestreo=0x0000;
    while(ind>=24){
        frecuencia_muestreo<<=8;
        frecuencia_muestreo+=cabecera[ind--];
    }
    metadata_cabecera[1]=frecuencia_muestreo;

    ind=31;
    long unsigned int byte_rate=0x0000;
    while(ind>=28){
        byte_rate<<=8;
        byte_rate+=cabecera[ind--];
    }
    metadata_cabecera[2]=byte_rate;

    ind=32;
    b1=cabecera[ind];
    short block_align=cabecera[++ind];
    block_align<<=8;
    block_align+=b1;
    metadata_cabecera[3]=block_align;

    ind=34;
    b1=cabecera[ind];
    short tamano_muestras=cabecera[++ind];
    tamano_muestras<<=8;
    tamano_muestras+=b1;
    metadata_cabecera[4]=tamano_muestras;

    int bytes_x_muestra=tamano_muestras/8;

    ind=43;
    unsigned long int num_muestras=0x0000;
    
    while(ind>=40){
        num_muestras<<=8;
        num_muestras+=cabecera[ind--];
    }
    metadata_cabecera[5]=num_muestras/bytes_x_muestra;

    metadata_cabecera[6]=tamano_archivo;

    if(flg==1){
 
    }

    return 0;
}


void lectura_muestras(FILE *entrada,double *arreglo_muestras_double,char *arreglo_muestras_hex,int num_muestras,int tam_muestras){

    int ind=0;
    int m=0;
    int bytes=tam_muestras/8;
    int leer_n_muestras=num_muestras*bytes;

    while(ind<leer_n_muestras){
        switch (bytes)
        {
        case 1: {
            int muestra = 0x0;
            muestra = fgetc(entrada);
            arreglo_muestras_hex[m] = muestra;
            arreglo_muestras_double[m] = (muestra-128.0)/128.0;            
            ind++;
            m++;
        }
            
            break;
        case 2: {
            short muestra = 0x0;
            unsigned char muestra0 = fgetc(entrada); 
            unsigned char muestra1 = fgetc(entrada);
            arreglo_muestras_hex[ind] = muestra0;
            ind++;
            arreglo_muestras_hex[ind] = muestra1;
            ind++;
            muestra = muestra0|muestra1<<8;
            arreglo_muestras_double[m] = muestra/32768.0;
            m++;
        }
            break;
        }
    }

   
}

void regresar_arreglo_double(FILE* salida,double *arreglo_muestras_double,int num_muestras,int bits_muestra){
   
    int bytes=bits_muestra/8;

    unsigned char regresar[2]={0x00,0x00};

    char regreso[1]={0x00};

    switch (bytes)
    {
    case 1:
        for(int i=0;i<num_muestras;i++){
            regreso[0]=(arreglo_muestras_double[i]*128.0)+128.0;
            fwrite(regreso,1,bytes,salida);
        }
        break;
    default:
        for (int i=0;i<num_muestras;i++){
                unsigned long int aux=arreglo_muestras_double[i]*32768.0;
                
                regresar[0]=(unsigned char) (aux);
                regresar[1]=(unsigned char) (aux>>8);

                fwrite(regresar,1,2,salida);
            }   
        break;
    }    
}


void regresar_arreglo(double *arreglo_muestras_double,int num_muestras,int bits_muestra,double *muestras_F){
   
    int bytes=bits_muestra/8;

    unsigned char regresar[2]={0x00,0x00};

    char regreso[1]={0x00};

    switch (bytes)
    {
    case 1:
        for(int i=0;i<num_muestras;i++){
            regreso[0]=(arreglo_muestras_double[i]*128.0)+128.0;

            int muestra = 0x0;
            double muestra_d=0;
            muestra = regreso[0];
            muestra_d = (muestra-128.0)/128.0;            


            muestras_F[i]=muestra_d;

        }
        break;
    default:
        for (int i=0;i<num_muestras;i++){
                unsigned long int aux=arreglo_muestras_double[i]*32768.0;
                
                regresar[0]=(unsigned char) (aux);
                regresar[1]=(unsigned char) (aux>>8);


                short muestra = 0x0;
                double muestra_d=0;
                unsigned char muestra0 = regresar[0]; 
                unsigned char muestra1 = regresar[1];

                muestra = muestra0|muestra1<<8;
                muestra_d = muestra/32768.0;

                muestras_F[i]=muestra_d;

            }   
        break;
    }    
}



void editar_cabecera(unsigned char *cabecera,int pos, unsigned long int nuevo_valor){
    
   unsigned long int aux=nuevo_valor;
   unsigned char regresar[4]={0x00,0x00,0x00,0x00};
   switch (pos){

        case 0:
        case 3:
        case 4:
                for(int j=0;j<2;j++){
                    regresar[j]=(unsigned char) (aux>>(8*j)); 
                }
                switch (pos){
                case 0:
                    cabecera[22]=regresar[0];
                    cabecera[23]=regresar[1];
                    break;
                case 3:
                    cabecera[32]=regresar[0];
                    cabecera[33]=regresar[1];
                    break;
                case 4:
                    cabecera[34]=regresar[0];
                    cabecera[35]=regresar[1];
                    break;
                default:
                    break;
                }
            break;

        case 1:
        case 2: 
        case 5:
        case 6:
                for(int j=0;j<4;j++){
                    regresar[j]=(unsigned char) (aux>>(8*j)); 
                }
                switch (pos){
                case 1:
                    for(int i=0;i<4;i++){
                        cabecera[24+i]=regresar[i];
                    }
                    break;
                case 2:
                    for(int i=0;i<4;i++){
                        cabecera[28+i]=regresar[i];
                    }
                    break;
                case 5:
                    for(int i=0;i<4;i++){
                        cabecera[40+i]=regresar[i];
                    }
                    break;
                case 6:
                    for(int i=0;i<4;i++){
                        cabecera[4+i]=regresar[i];
                    }
                    break;
                default:
                    break;
                }
            break;
        default:
            printf("Error posicion incorrecta!");
            break;
   }
}

void copiar_cabecera(unsigned char *cabecera,unsigned char *copia){
    for(int i=0;i<44;i++){
        copia[i]=cabecera[i];
    }
}

void buscar_picos(double *muestras,double *picos, int num_muestras){

    double max = 0.0;
    double max2 = 0.0;

    int ind = 0;
    int ind2 = 0;

   for(int i=0;i<(num_muestras/2);i++){
       if(muestras[i]>=max){
           max  = muestras[i];
           ind = i;
       }
   }

    for(int i=0;i<ind-50;i++){
      if(muestras[i]>=max2){
           max2  = muestras[i];
           ind2 = i;
       }
   }

   for(int i=(ind+50);i<(num_muestras/2);i++){
      if(muestras[i]>=max2){
           max2  = muestras[i];
           ind = i;
       }
   }

    if(ind<ind2){
        picos[0]=max;
        picos[1]=max2;

    }else{
        picos[0]=max2;
        picos[1]=max;

    }


}


void swap(double *x1,double *x2,int i,int j){
    double aux = x1[i];
    x1[i] = x2[j];
    x2[j] = aux;
}

int FFT(double *xr,double *xi,int N,int inverse){
    int i,j,k,j1,m,n;
    double arg,s,c,w,tempr,tempi;

    m=log((double) N) / log(2.0);
    for(i=0; i<N ; ++i){
        j=0;
        for(k=0; k<m ; ++k)
            j=(j<<1) | (1 & ( i>> k));
        if(j < i){   
            swap(xr,xr,i,j);  
            swap(xi,xi,i,j);  
        }
    }
    for(i=0;i<m;i++){
        n=w=pow(2.0,(double)i);
        w=M_PI/n;
        if (inverse) w=-w;
        k=0;
        while(k<N-1){
            for(j=0;j<n; j++){
                arg=-j*w; c=cos(arg); s=sin(arg);
                j1=k+j;
                tempr=xr[j1+n]*c-xi[j1+n]*s;
                tempi=xi[j1+n]*c+xr[j1+n]*s;

                xr[j1+n]=xr[j1]-tempr;
                xi[j1+n]=xi[j1]-tempi;

                xr[j1]=xr[j1]+tempr;
                xi[j1]=xi[j1]+tempi;

            }
            k+=2*n;
        }
    }
    arg=1.0/sqrt((double)N);
    for(i=0;i<N;i++){
        xr[i]*=arg; xi[i]*=arg;
    }

    return 1;
}
