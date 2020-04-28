/*
Zeth Alvarez Hernandez  2020
Teoria de comunicaciones y señales
*/

#include<stdio.h>  
#include<stdlib.h>
#include<math.h>


void lectura_muestras(FILE *entrada,double *arreglo_muestras_double,char *arreglo_muestras_hex,int num_muestras,int num_bits);
void regresar_arreglo_double(FILE* salida,double *arreglo_muestras_double,int num_muestras,int num_bytes_por_muestra,double normalizar);
int lectura_cabecera(FILE *entrada,unsigned char *cabecera,int *metadata_cabecera,int flg);
void editar_cabecera(unsigned char *cabecera,int pos, unsigned long int nuevo_valor);
void copiar_cabecera(unsigned char *cabecera,unsigned char *copia);

// Link de esta funcion el cual nos muestra los algoritmos para la convolucion discreta
// http://www.songho.ca/dsp/convolution/convolution.html?fbclid=IwAR2HsCXDnYEzytz8pdeQhGSD5r6FX3d-5EoHVtrvTavEBCqjxsemOuvcD2A#cpp_conv1d
double convolucion1D(double* in, double* out, int dataSize, double* kernel, int kernelSize);

int main(int argc, char* argv[]){

    unsigned char cabecera[44];
    int metadata_cabecera[7]={0,0,0,0,0,0,0}; 
    /*
        Metadata de la cabecera         
        Posicion                                       Posicion
        metadata_cabecera         Valor                cabecera       
                    
        [0]                      Canales               [22 - 23]          
        [1]               Frecuencia de muestreo       [24 - 27]
        [2]                      Byte_rate             [28 - 31]
        [3]                     Block_align            [32 - 33] 
        [4]          Tamaño en bytes de cada muestra   [34 - 35]
        [5]                  Numero de muestras        [40 - 43]
        [6]                  Tamaño del archivo        [ 4 -  7]
    */
    //Imprime o no imprime los datos de la cabecera
    int imprimir=1;   // 0 = imprime ; 1 = No imprime

    if(argc<3){
        printf("Error! \nFaltan argumentos\n");
        return 0;
    }else if (argc>3){
        printf("Error! \nSobran argumentos\n");
        return 0;
    }

    //Abrir archivos de entrada y salida
    FILE *entrada=fopen(argv[1],"rb");
    FILE *salida=fopen(argv[2],"wb");

    if (entrada==NULL || salida==NULL){
        fputs ("Error de lectura de archivos",stderr); 
        return 0;
    }

    // Lee los 44 caracteres de la cabecera del archivo WAV
    // Guarda los datos de la cabecera en int (arreglo metadatos_cabecera) para poderlos usar
    lectura_cabecera(entrada,cabecera,metadata_cabecera,imprimir);

    /*
    --> Aqui modificar la cabecera (arreglo cabecera)<--
        Las 44 posiciones del arreglo cabecera coindicen con cada uno de los 44 bytes
        que la componen 
    */

    //Imprime en el archivo de salida la cabecera (el arreglo con o sin modificaciones)
    //Esta linea se puede comentar si quieres el archivo RAW

    fwrite(cabecera,sizeof(unsigned char),44,salida);

    int num_muestras=metadata_cabecera[5];
    int num_muestras_hex=num_muestras*(metadata_cabecera[4]/8);

    double *arreglo_muestras_double=malloc(num_muestras * sizeof(double));
    char *arreglo_muestras_hex=malloc(num_muestras_hex * sizeof(char));
    double *resultado=malloc(num_muestras * sizeof(double));

    // Normalizar el valor entre 0 y 1 dependiendo el valor mayor que se consiga despues de operar cada una de las muestras
    double normalizar=0; 

    //Guarda las muestras en dos arreglos
    //arreglo_muestras_double -> Guarda las muestras con su valor en double dependiendo su configuracion 8,16 o 32 bits
    //arreglo_muestras_hex -> Guarda las muestras en su valor hexadecimal (1 byte en cada posicion del arreglo)
    //printf("entrada, arreglo double, arreglo hex, %d byterate, %d num_muestras, %d tamaño bits muestras",metadata_cabecera[2],num_muestras,metadata_cabecera[4]);
    lectura_muestras(entrada,arreglo_muestras_double,arreglo_muestras_hex,num_muestras,metadata_cabecera[4]);

    //Cierro el archivo de entrada
    fclose(entrada);

    //Aqui meter la funcion que le vamos a aplicar la señal
    //Dividir señal, Convolucion, TDF, TDFI, FFT, FFTI, DTMF, Multiplicacion
  
    double convolucion[100]={1.000000, 0.752051, 0.565580, 0.425345, 0.319881, 0.240567, 0.180918, 0.136060, 0.102324, 0.076953, 
                            0.057872, 0.043523, 0.032731, 0.024616, 0.018512, 0.013922, 0.010470, 0.007874, 0.005922, 0.004453,
                            0.003349, 0.002519, 0.001894, 0.001425, 0.001071, 0.000806, 0.000606, 0.000456, 0.000343, 0.000258,
                            0.000194, 0.000146, 0.000110, 0.000082, 0.000062, 0.000047, 0.000035, 0.000026, 0.000020, 0.000015,
                            0.000011, 0.000008, 0.000006, 0.000005, 0.000004, 0.000003, 0.000002, 0.000002, 0.000001, 0.000001,
                            0.000001, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
                            0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
                            0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
                            0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
                            0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000};
    
    normalizar=convolucion1D(arreglo_muestras_double,resultado,num_muestras,convolucion,100);


    //Regresar el arreglo resultado al archivo salida
    regresar_arreglo_double(salida,resultado,num_muestras,metadata_cabecera[4],normalizar);


    fclose(salida);

    return 0;
}

///Funciones generales para tratar el archivo

int lectura_cabecera(FILE *entrada,unsigned char *cabecera,int *metadata_cabecera,int flg){
    
    int posicion_archivo=0;

    while(posicion_archivo<44){
        cabecera[posicion_archivo] = fgetc(entrada);
        posicion_archivo++;
    }

    //Tamaño archivo
    int ind=7;
    long unsigned int tamano_archivo=0x0000;
    while(ind>=4){
        tamano_archivo<<=8;
        tamano_archivo+=cabecera[ind--];
    }
    

    //Canales que trabaja el archivo
    ind=22;
    char b1=cabecera[ind];
    short canales=cabecera[++ind];
    canales<<=8;
    canales+=b1;
    metadata_cabecera[0]=canales;
    
    //Frecuencia de muestreo
    ind=27;
    long unsigned int frecuencia_muestreo=0x0000;
    while(ind>=24){
        frecuencia_muestreo<<=8;
        frecuencia_muestreo+=cabecera[ind--];
    }
    metadata_cabecera[1]=frecuencia_muestreo;

    //Tasa de bytes
    ind=31;
    long unsigned int byte_rate=0x0000;
    while(ind>=28){
        byte_rate<<=8;
        byte_rate+=cabecera[ind--];
    }
    metadata_cabecera[2]=byte_rate;

    // Block align
    ind=32;
    b1=cabecera[ind];
    short block_align=cabecera[++ind];
    block_align<<=8;
    block_align+=b1;
    metadata_cabecera[3]=block_align;

    // Bytes per samble (Tamaño de las muestras)
    ind=34;
    b1=cabecera[ind];
    short tamano_muestras=cabecera[++ind];
    tamano_muestras<<=8;
    tamano_muestras+=b1;
    metadata_cabecera[4]=tamano_muestras;

    int bytes_x_muestra=tamano_muestras/8;

    //Numero de  muestras
    ind=43;
    unsigned long int num_muestras=0x0000;
    
    while(ind>=40){
        num_muestras<<=8;
        num_muestras+=cabecera[ind--];
    }
    metadata_cabecera[5]=num_muestras/bytes_x_muestra;

    metadata_cabecera[6]=tamano_archivo;

    if(flg==1){
        printf("\t\n->Archivo de ENTRADA\n");
        printf("\t\nTamano del archivo:  %ld bytes",tamano_archivo);
        printf("\t\nCanales: %d",canales);
        printf("\t\nFrecuencia de muestreo (Sample rate): %ld Hz",frecuencia_muestreo); 
        printf("\t\nTasa de bytes (Byte rate): %ld",byte_rate);
        printf("\t\nBlock Align: %d",block_align);
        printf("\t\nTamano de las muestras: %d bytes",tamano_muestras);
        printf("\t\nNumero de muestras: %ld ",num_muestras/bytes_x_muestra);
        printf("\n\n");    
    }

    return 0;
}


void lectura_muestras(FILE *entrada,double *arreglo_muestras_double,char *arreglo_muestras_hex,int num_muestras,int tam_muestras){

    int ind=0;
    int bytes=tam_muestras/8;
    int leer_n_muestras=num_muestras*bytes;
    double potencia=((pow(2,(tam_muestras))/2)-1);
    //Variables para muestras de 8 bits
    unsigned char valorC=0x00;
    //Variables para muestras de mas de 8 bits
    long int valorI=0x00;
    char aux=0x0;
    //Muestra
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

void regresar_arreglo_double(FILE* salida,double *arreglo_muestras_double,int num_muestras,int bits_muestra,double normalizar){
   
    double potencia=(pow(2,bits_muestra)/2)-1;
    int bytes=bits_muestra/8;
    //Arreglo de regreso para muestras de mas de 8 bits
    unsigned char regresar[4]={0x00,0x00,0x00,0x00};
    //Arreglo de regreso para muestras de 8 bits
    unsigned char regreso[1]={0x00};

    switch (bytes)
    {
    case 1:
        for(int i=0;i<num_muestras;i++){
            regreso[0]=((arreglo_muestras_double[i]*potencia/normalizar)+potencia);
            fwrite(regreso,1,bytes,salida);
        }
        break;
    default:
        for (int i=0;i<num_muestras;i++){
                unsigned long int aux=(arreglo_muestras_double[i]*potencia)/normalizar;

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
        // 16 bits
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

        // 32 bits
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

//Funcion para la Convolucion
double convolucion1D(double* in, double* out, int dataSize, double* kernel, int kernelSize){
    int i, j, k;
    double maximo=0, val_abs=0;

    // check validity of params
    if(!in || !out || !kernel) return 0;
    if(dataSize <=0 || kernelSize <= 0) return 0;

    // start convolution from out[kernelSize-1] to out[dataSize-1] (last)
    for(i = kernelSize-1; i < dataSize; ++i){
        out[i] = 0;                             // init to 0 before accumulate

        for(j = i, k = 0; k < kernelSize; --j, ++k)
            out[i] += in[j] * kernel[k];

        val_abs=fabs(out[i]);
        if(val_abs>maximo){
            maximo=val_abs;
        }
    }

    // convolution from out[0] to out[kernelSize-2]
    for(i = 0; i < kernelSize - 1; ++i){
        out[i] = 0;                             // init to 0 before sum

        for(j = i, k = 0; j >= 0; --j, ++k)
            out[i] += in[j] * kernel[k];

        val_abs=fabs(out[i]);
        if(val_abs>maximo){
            maximo=val_abs;
        }
    }

    return maximo;
}
