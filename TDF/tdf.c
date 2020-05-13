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

//Aplicar TDF a una señal
void tdf(double *arreglo_muestras,double *reales,double *imagin,int num_muestras);

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

    unsigned char cabecera_copia[44];

    copiar_cabecera(cabecera,cabecera_copia);

    //editar_cabecera(arreglo cabecera original, posicion valor a cambiar, nuevo valor);

        if(metadata_cabecera[0] != 2){
        //Edito el canal
        editar_cabecera(cabecera_copia,0,2);

        //Edito el blockAlign
        editar_cabecera(cabecera_copia,3,2*(metadata_cabecera[4]/8)); // canales * Tamaño en bytes de cada muestra / 8

        //Edito el numero de muestras
        editar_cabecera(cabecera_copia,5,(metadata_cabecera[5]*metadata_cabecera[4]/8)*2); //Numero de muestras * Tamaño en bytes de cada muestra / 8 * 2 //( dos porque son el doble de muestras)  

        //Edito el chucksize
        editar_cabecera(cabecera_copia,6,metadata_cabecera[6]+(metadata_cabecera[5]*(metadata_cabecera[4]/8))); //Tamaño del archivo + (Numero de muestras * Tamaño en bytes de cada muestra / 8) numero de muestras nuevas en bytes

        //Edito ByteRate
        editar_cabecera(cabecera_copia,2,metadata_cabecera[1]*2*(metadata_cabecera[4]/8));  //Frecuencia de muestreo * canales * Tamaño en bytes de cada muestra / 8
    }

    //Imprime en el archivo de salida la cabecera (el arreglo con o sin modificaciones)
    //Esta linea se puede comentar si quieres el archivo RAW

    fwrite(cabecera_copia,sizeof(unsigned char),44,salida);

    int num_muestras=metadata_cabecera[5];
    int num_muestras_hex=num_muestras*(metadata_cabecera[4]/8);

    double *arreglo_muestras_double=malloc(num_muestras * sizeof(double));
    char *arreglo_muestras_hex=malloc(num_muestras_hex * sizeof(char));
    //Arreglo para los resultados
    //double *resultado=malloc(num_muestras * sizeof(double));
    double *arreglo_reales=malloc(num_muestras * sizeof(double));
    double *arreglo_imaginarios=malloc(num_muestras * sizeof(double));

    // Normalizar el valor entre 0 y 1 dependiendo el valor mayor que se consiga despues de operar cada una de las muestras
    //double normalizar=1; 

    //Guarda las muestras en dos arreglos
    //arreglo_muestras_double -> Guarda las muestras con su valor en double dependiendo su configuracion 8,16 o 32 bits
    //arreglo_muestras_hex -> Guarda las muestras en su valor hexadecimal (1 byte en cada posicion del arreglo)
    //printf("entrada, arreglo double, arreglo hex, %d byterate, %d num_muestras, %d tamaño bits muestras",metadata_cabecera[2],num_muestras,metadata_cabecera[4]);
    lectura_muestras(entrada,arreglo_muestras_double,arreglo_muestras_hex,num_muestras,metadata_cabecera[4]);

    //Cierro el archivo de entrada
    fclose(entrada);

    //Aqui meter la funcion que le vamos a aplicar la señal
    //Dividir señal, Convolucion, TDF, TDFI, FFT, FFTI, DTMF, Multiplicacion


    tdf(arreglo_muestras_double,arreglo_reales,arreglo_imaginarios,num_muestras);
    

    //Regresar el arreglo resultado al archivo salida
    //regresar_arreglo_double(salida,arreglo_muestras_double,num_muestras,metadata_cabecera[4]);
    //Regresar arreglo de archivos estereo
    int m=0;
    int n=0;
    for(int i=0;i<((num_muestras)*2);i++){
        double muestra[1];
        if(i%2==0){
            muestra[0]=arreglo_reales[m];
            m++;
        }else{
            muestra[0]=arreglo_imaginarios[n];
            n++;
        }
        regresar_arreglo_double(salida,muestra,1,metadata_cabecera[4]);
    }


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

void regresar_arreglo_double(FILE* salida,double *arreglo_muestras_double,int num_muestras,int bits_muestra){
   
    double potencia=(pow(2,bits_muestra)/2)-1;
    int bytes=bits_muestra/8;
    //Arreglo de regreso para muestras de mas de 8 bits
    unsigned char regresar[4]={0x00,0x00,0x00,0x00};
    //Arreglo de regreso para muestras de 8 bits
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

//Funcion de TDF
void tdf(double *arreglo_muestras,double *reales,double *imagin,int num_muestras){

    int pos=0;

    for(int i=0;i<num_muestras;i++){
        for(int j=0;j<num_muestras;j++){
            reales[i]+=((arreglo_muestras[j])*cos((2*M_PI*i*j)/num_muestras))/num_muestras; // Reales
            imagin[i]+=((-1)*((arreglo_muestras[j])*sin((2*M_PI*i*j)/num_muestras))/num_muestras); // Imaginarios
        }
        if(reales[i] != reales[i]){
            reales[i]=0.0;
        }        
        if(imagin[i] != imagin[i]){
            imagin[i]=0.0;
        }

        int aux=(i*100)/num_muestras;
        
        if(pos!=aux){
            system("cls");
            printf("\nCargando TDF... %d %%\n",aux+1);
            pos=aux;
        }
        
        //printf("%d = { %lf + %lf j }  \n",i,reales[i], imagin[i]);
    }
}