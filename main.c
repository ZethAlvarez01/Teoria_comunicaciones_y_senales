#include<stdio.h>  
#include <stdlib.h>
#include<math.h>

#ifndef MPI
#define M_PI 3.14159265358979323846
#endif

/*
Zeth Alvarez Hernandez 
Teoria de comunicaciones y señales
2020
*/

void lectura_muestras(FILE *entrada,double *arreglo_muestras_double,char *arreglo_muestras_hex,int num_muestras,int num_bits);
void regresar_arreglo_double(FILE* salida,double *arreglo_muestras_double,int num_muestras,int num_bytes_por_muestra);
int lectura_cabecera(FILE *entrada,unsigned char *cabecera,int *metadata_cabecera,int flg);
void editar_cabecera(unsigned char *cabecera,int pos, unsigned long int nuevo_valor);
void copiar_cabecera(unsigned char *cabecera,unsigned char *copia);

//Dividir una señal n veces  
void dividir_senal(double *arreglo_muestras,double *resultado,int num_muestras,int n);

// Link de esta funcion el cual nos muestra los algoritmos para la convolucion discreta
// http://www.songho.ca/dsp/convolution/convolution.html?fbclid=IwAR2HsCXDnYEzytz8pdeQhGSD5r6FX3d-5EoHVtrvTavEBCqjxsemOuvcD2A#cpp_conv1d
double convolucion1D(double* in, double* out, int dataSize, double* kernel, int kernelSize);

//Aplicar TDF a una señal
void tdf(double *arreglo_muestras,double *reales,double *imagin,int num_muestras);

//Aplicar TDFI a una señal
void tdfi(double *reales,double *imagin,double *regreso_tdfi,int num_muestras);

int main(int argc, char* argv[]){

    unsigned char cabecera[44];
    int metadata_cabecera[7]={0,0,0,0,0,0,0}; 
    /*
        Metadata de la cabecera         
        Posicion                                      Posicion
        metadata_cabecera         Valor               cabecera       
                    
        [0]                      canales              [22 - 23]          
        [1]               frecuencia de muestreo      [24 - 27]
        [2]                      byte_rate            [28 - 31]
        [3]                     block_align           [32 - 33] 
        [4]          tamaño en bytes de cada muestra  [34 - 35]
        [5]                  numero de muestras       [40 - 43]
        [6]                  tamaño del archivo       [ 4 -  7]
    */
    

   //Imprime o no imprime los datos de la cabecera
    int imprimir=0;   // 0 = imprime ; 1 = No imprime

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

    //Aqui modifico la cabecera para TDF y TDFI
    // Si es canal mono la cambia a stereo si es stereo la deja igual
    unsigned char cabecera_copia[44];

    copiar_cabecera(cabecera,cabecera_copia);

    if(metadata_cabecera[0] != 2){
        //Edito el canal
        editar_cabecera(cabecera_copia,0,2);

        //Edito el blockAlign
        editar_cabecera(cabecera_copia,3,2*(metadata_cabecera[4]/8));

        //Edito el numero de muestras
        editar_cabecera(cabecera_copia,5,(metadata_cabecera[5]*metadata_cabecera[4]/8)*2);

        //Edito el chucksize
        editar_cabecera(cabecera_copia,6,metadata_cabecera[6]+(metadata_cabecera[5]*(metadata_cabecera[4]/8)));

        //Edito ByteRate
        editar_cabecera(cabecera_copia,2,metadata_cabecera[1]*2*(metadata_cabecera[4]/8));
    }


    //Imprime en el archivo de salida la cabecera (el arreglo con o sin modificaciones)
    //Esta linea se puede comentar si quieres el archivo RAW

    fwrite(cabecera_copia,sizeof(unsigned char),44,salida);

    int num_muestras=metadata_cabecera[5];
    double arreglo_muestras_double[num_muestras];
    int num_muestras_hex=num_muestras*(metadata_cabecera[4]/8);
    char arreglo_muestras_hex[num_muestras_hex];

    //Guarda las muestras en dos arreglos
    //arreglo_muestras_double -> Guarda las muestras con su valor en double dependiendo su configuracion 8,16 o 32 bits
    //arreglo_muestras_hex -> Guarda las muestras en su valor hexadecimal (1 byte en cada posicion del arreglo)
    //printf("entrada, arreglo double, arreglo hex, %d byterate, %d num_muestras, %d tamaño bits muestras",metadata_cabecera[2],num_muestras,metadata_cabecera[4]);
    lectura_muestras(entrada,arreglo_muestras_double,arreglo_muestras_hex,num_muestras,metadata_cabecera[4]);

    //Cierro el archivo de entrada
    fclose(entrada);
    
    //Creamos un arreglo para los resultados, asi no modificamos o perdemos el arreglo original
    double arreglo_resultado[num_muestras];
    //Arrego resultados para reales e imaginarios
    double arreglo_reales[num_muestras];
    double arreglo_imaginarios[num_muestras];

    // Normalizar el valor entre 0 y 1 dependiendo el valor mayor que se consiga despues de operar cada una de las muestras
    double normalizar=1; 

    //Partir señal a la mitad
    //Arreglo de muestras - Arreglo para almacenar el resultadom - Numero de muestras - Por cuanto dividir la señal

    
    //dividir_senal(arreglo_muestras_double,arreglo_resultado,num_muestras,2);
    

    //Convolucion 1D
    
    //Respuesta al impulso de un circuito RC con frecuencia de corte de 2000 hz y frecuencia de muestreo de 44100
    //Ver Calculos_convolucion.png  
    
     /*double convolucion[100]={1.000000, 0.752051, 0.565580, 0.425345, 0.319881, 0.240567, 0.180918, 0.136060, 0.102324, 0.076953, 
                            0.057872, 0.043523, 0.032731, 0.024616, 0.018512, 0.013922, 0.010470, 0.007874, 0.005922, 0.004453,
                            0.003349, 0.002519, 0.001894, 0.001425, 0.001071, 0.000806, 0.000606, 0.000456, 0.000343, 0.000258,
                            0.000194, 0.000146, 0.000110, 0.000082, 0.000062, 0.000047, 0.000035, 0.000026, 0.000020, 0.000015,
                            0.000011, 0.000008, 0.000006, 0.000005, 0.000004, 0.000003, 0.000002, 0.000002, 0.000001, 0.000001,
                            0.000001, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
                            0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
                            0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
                            0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
                            0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000};

    normalizar=convolucion1D(arreglo_muestras_double,arreglo_resultado,num_muestras,convolucion,100);
*/

    //Transformada discreta de Fourier y Trasnsformada discreta de Fourier Inversa
    // TDF y TDFI pa' los cuates

    tdf(arreglo_muestras_double,arreglo_reales,arreglo_imaginarios,num_muestras);

    // int m=0;
    // int n=0;

    // for(int i=0;i<num_muestras;i++){
    //     if(i%2==0){
    //         arreglo_reales[m]=arreglo_muestras_double[i];
    //         m++;
    //     }else{
    //         arreglo_imaginarios[n]=arreglo_muestras_double[i];
    //         n++;
    //     }
    // }
    
    // tdfi(arreglo_reales,arreglo_imaginarios,arreglo_resultado,num_muestras);

    // // //Mete todo a reales
    // for(int i=0;i<num_muestras;i++){
    //     arreglo_reales[i]=arreglo_resultado[i];
    //     arreglo_imaginarios[i]=0.0;
    // }
    
    //Regresar valores reales e imaginarios intercalandolos
    int m=0;
    int n=0;
    int total_muestras_stereo=num_muestras*2;
    double muestra[1];

    for(int i=0;i<total_muestras_stereo;i++){
        if(i%2==0){
            muestra[0]=arreglo_reales[m];
            m++;
        }else{
            muestra[0]=arreglo_imaginarios[n];
            n++;
        }
        regresar_arreglo_double(salida,muestra,1,metadata_cabecera[4]);
    }
   
    //Cierro el archivo de salida
    fclose(salida);


    return 0;   
}


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
        printf("\t\nTamano del archivo:  %d bytes",tamano_archivo);
        printf("\t\nCanales: %d",canales);
        printf("\t\nFrecuencia de muestreo (Sample rate): %d Hz",frecuencia_muestreo); 
        printf("\t\nTasa de bytes (Byte rate): %d",byte_rate);
        printf("\t\nBlock Align: %d",block_align);
        printf("\t\nTamano de las muestras: %d bytes",tamano_muestras);
        printf("\t\nNumero de muestras: %d ",num_muestras/bytes_x_muestra);
        printf("\n\n");    
    }

    return 0;
}

void lectura_muestras(FILE *entrada,double *arreglo_muestras_double,char *arreglo_muestras_hex,int num_muestras,int tam_muestras){

    int ind=0;
    int bytes_x_muestra=tam_muestras/8;
    int leer_muestras=num_muestras*bytes_x_muestra;
    int total_bytes=num_muestras*bytes_x_muestra;
    double normalizar_signed=((pow(2,tam_muestras)/2)-1);

    while(ind<leer_muestras){
        arreglo_muestras_hex[ind]=fgetc(entrada);
        ind++;
    }   

    long int valor=0x00;
    char aux=0x0;
    ind=0;
    
    for(int i=0;i<=total_bytes;i++){
        if((i+1)%(bytes_x_muestra)==0){
            valor=arreglo_muestras_hex[i];
            valor<<=8;
            valor+=aux;
            arreglo_muestras_double[ind]=valor/normalizar_signed;
            valor=0x00; aux=0x0; ind++;
        }else{
            aux=arreglo_muestras_hex[i];
        }
    }


}

void regresar_arreglo_double(FILE* salida,double *arreglo_muestras_double,int num_muestras,int num_bytes_por_muestra){
   
   double potencia=((pow(2,(num_bytes_por_muestra))/2)-1);
   int bytes_x_muestra=num_bytes_por_muestra/8;
   unsigned char regresar[4]={0x00,0x00,0x00,0x00};

   for (int i=0;i<num_muestras;i++){
        unsigned long int aux=arreglo_muestras_double[i]*potencia;

        for(int j=0;j<bytes_x_muestra;j++){
            regresar[j]=(unsigned char) (aux>>(8*j)); 
        }

        fwrite(regresar,1,bytes_x_muestra,salida);
    }
   
}


void dividir_senal(double *arreglo_muestras,double *resultado,int num_muestras,int n){
    for(int i=0;i<num_muestras;i++){
        resultado[i]=arreglo_muestras[i]/n;
    }
}


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

void tdf(double *arreglo_muestras,double *reales,double *imagin,int num_muestras){
    
    int pos=0;

    for(int i=0;i<num_muestras;i++){
        for(int j=0;j<num_muestras;j++){
            reales[i]+=((arreglo_muestras[j])*cos((2*M_PI*i*j)/num_muestras))/num_muestras; // Reales
            imagin[i]-=((arreglo_muestras[j])*sin((2*M_PI*i*j)/num_muestras))/num_muestras; // Imaginarios
        }
        //Por si el calculo resulta en un valor mayor a 1
        if(reales[i] != reales[i]){
            reales[i]=0.0;
        }        
        //Por si el calculo resulta en un valor menor a -1
        if(imagin[i] != imagin[i]){
            imagin[i]=0.0;
        }

        //Porcentaje de carga para el calculo de la TDF
        //Borrar al presentar con Aldana
        int aux=(i*100)/num_muestras;
        if(pos!=aux){
            system("cls");
            printf("\nCargando TDF... %d %%\n",aux+1);
            pos=aux;
        }
        //Imprime los valores reales e imaginarios de cada una de las muestras
        //printf("%d = { %lf + %lf j }  \n",i,reales[i], imagin[i]);
    }
}

void tdfi(double *reales,double *imagin,double *regreso_tdfi,int num_muestras){
    int pos=0;
    for(int i=0;i<num_muestras;i++){
        for(int j=0;j<num_muestras;j++){
            regreso_tdfi[i]+=((((reales[j])*cos((2*M_PI*i*j)/num_muestras))+((imagin[j])*sin((2*M_PI*i*j)/num_muestras))));
        }
        //Por si el calculo resulta en un valor NaN
        if(regreso_tdfi[i] != regreso_tdfi[i]){
            regreso_tdfi[i]=0.0;
        }
        //Por si el calculo resulta en un valor mayor a 1
        if(regreso_tdfi[i]>1){
            regreso_tdfi[i]=1;
        }
        //Por si el calculo resulta en un valor menor a -1
        if(regreso_tdfi[i]<-1){
            regreso_tdfi[i]=-1;
        }
        //Porcentaje de carga para el calculo de la TDF
        //Borrar al presentar con Aldana
        int aux=(i*100)/num_muestras;
        if(pos!=aux){
            system("cls");
            printf("\nCargando TDFI... %d %%\n",aux+1);
            pos=aux;
        }
        //Imprime los valores reales e imaginarios de cada una de las muestras
        //printf("%d = { %lf }  \n",i,regreso_tdfi[i]);
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