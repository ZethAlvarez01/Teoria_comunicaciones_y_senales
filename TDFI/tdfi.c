/*
Zeth Alvarez Hernandez  2020
Teoria de comunicaciones y señales
*/

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

void tdfi(double *reales,double *imagin,double *salidaR, double *salidaI,int n);

int main(int argc, char* argv[]){

    unsigned char cabecera[44];
    int metadata_cabecera[7]={0,0,0,0,0,0,0}; 

    int imprimir=0;   

    if(argc<3){

        return 0;
    }else if (argc>3){

        return 0;
    }


    FILE *entrada=fopen(argv[1],"rb");
    FILE *salida=fopen(argv[2],"wb");

    if (entrada==NULL || salida==NULL){
        fputs ("Error de lectura de archivos",stderr); 
        return 0;
    }

    lectura_cabecera(entrada,cabecera,metadata_cabecera,imprimir);


    fwrite(cabecera,sizeof(unsigned char),44,salida);

    int num_muestras=metadata_cabecera[5];
    int num_muestras_hex=num_muestras*(metadata_cabecera[4]/8);

    double *arreglo_muestras_double=malloc(num_muestras * sizeof(double));
    char *arreglo_muestras_hex=malloc(num_muestras_hex * sizeof(char));

    double *resultado=malloc(num_muestras * sizeof(double));
    double *salidaR=malloc(num_muestras/2 * sizeof(double));
    double *salidaI=malloc(num_muestras/2 * sizeof(double));
    double *reales=malloc(num_muestras/2 * sizeof(double));
    double *imaginarios=malloc(num_muestras/2 * sizeof(double));

    
    lectura_muestras(entrada,arreglo_muestras_double,arreglo_muestras_hex,num_muestras,metadata_cabecera[4]);

    fclose(entrada);

    int iR=0;
    int iI=0;
    for(int i=0;i<num_muestras;i++){
        if(i%2==0){
            reales[iR]=arreglo_muestras_double[i];
            iR++;
        }else{
            imaginarios[iI]=arreglo_muestras_double[i];
            iI++;
        }
    }

    tdfi(reales,imaginarios,salidaR,salidaI,num_muestras/2);

    iR=0;
    iI=0;

    for(int i=0;i<num_muestras;i++){
        if(i%2==0){
            resultado[i]=salidaR[iR];
            iR++;
        }else{
            resultado[i]=salidaI[iI];
            iI++;
        }
    }

    regresar_arreglo_double(salida,resultado,num_muestras,metadata_cabecera[4]);


    fclose(salida);

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
    int bytes=tam_muestras/8;
    int leer_n_muestras=num_muestras*bytes;
    double potencia=((pow(2,(tam_muestras))/2)-1);
  
    unsigned char valorC=0x00;
 
    long int valorI=0x00;
    char aux=0x0;

    double muestra=0;
    

    while(ind<leer_n_muestras){
        arreglo_muestras_hex[ind]=fgetc(entrada);
        ind++;
    }

    ind=num_muestras-1;
         
    switch (bytes){
        case 1:
            for(int i=0;i<num_muestras;i++){
                valorC=arreglo_muestras_hex[i];
                muestra=(valorC-potencia)/potencia;
                if(muestra>1) muestra=1;
                if(muestra<-1) muestra=-1;
                arreglo_muestras_double[i]=muestra;
            }
            break;
        default:
            for(int i=leer_n_muestras-1;i>=0;i--){
                aux=arreglo_muestras_hex[i];
                valorI<<=8;
                valorI+=aux;
                if(i%bytes==0){
                    muestra=valorI/potencia;
                    if(muestra>1) muestra=1;
                    if(muestra<-1) muestra=-1;
                    arreglo_muestras_double[ind]=muestra;
                    ind--;
                    aux=0x0;
                    valorI=0x0;
                }
                
            }
            break;
    }

   
}

void regresar_arreglo_double(FILE* salida,double *arreglo_muestras_double,int num_muestras,int bits_muestra){
   
    double potencia=(pow(2,bits_muestra)/2)-1;
    int bytes=bits_muestra/8;

    unsigned char regresar[4]={0x00,0x00,0x00,0x00};

    char regreso[1]={0x00};

    switch (bytes)
    {
    case 1:
        for(int i=0;i<num_muestras;i++){
            regreso[0]=(arreglo_muestras_double[i]*potencia)+potencia;
            fwrite(regreso,1,bytes,salida);
        }
        break;
    default:
        for (int i=0;i<num_muestras;i++){
                unsigned long int aux=arreglo_muestras_double[i]*potencia;

                for(int j=0;j<bytes;j++){
                    regresar[j]=(unsigned char) (aux>>(8*j)); 
                }

                fwrite(regresar,1,bytes,salida);
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

            break;
   }
}

void copiar_cabecera(unsigned char *cabecera,unsigned char *copia){
    for(int i=0;i<44;i++){
        copia[i]=cabecera[i];
    }
}

void tdfi(double *reales,double *imagin,double *salidaR, double *salidaI,int numero_muestras){

   for (int i = 0; i < numero_muestras; i++) {  
        salidaR[i]=0;
        salidaI[i]=0;
        for (int j = 0; j < numero_muestras; j++) { 
            salidaR[i]+=reales[j]*cos( 2 * M_PI * i * j / numero_muestras)-imagin[j]*sin( 2 * M_PI * i * j / numero_muestras);
            salidaI[i]+=imagin[j]*cos( 2 * M_PI * i * j / numero_muestras)+reales[j]*sin( 2 * M_PI * i * j / numero_muestras);
        }

        if (salidaR[i]>1.0){
            salidaR[i]=1;
        }else if(salidaR[i]<-1.0){
            salidaR[i]=-1;
        }

        if (salidaI[i]>1.0){
            salidaI[i]=1;
        }else if(salidaI[i]<-1.0){
            salidaI[i]=-1;
        }
        
    }
} 	 
