/*
Zeth Alvarez Hernandez  2020
Teoria de comunicaciones y señales
*/

#include<stdio.h>  
#include<stdlib.h>
#include<math.h>

void lectura_muestras(FILE *entrada,double *arreglo_muestras_double,char *arreglo_muestras_hex,int num_muestras,int num_bits);
void regresar_arreglo_double(FILE* salida,double *arreglo_muestras_double,int num_muestras,int num_bytes_por_muestra);
int lectura_cabecera(FILE *entrada,unsigned char *cabecera,int *metadata_cabecera,char *nombre,int flg);
void editar_cabecera(unsigned char *cabecera,int pos, unsigned long int nuevo_valor);
void copiar_cabecera(unsigned char *cabecera,unsigned char *copia);
void multiplicarReal(double *archivo_1,double *archivo_2,double *salida, int tam);
void multiplicarCmpjs(double *a, double *b, double *c, double *d,double *salidaR,double *salidaI, int tam);

int main(int argc, char* argv[]){

    unsigned char cabecera_1[44];
    unsigned char cabecera_2[44];
    int metadata_cabecera_1[7]={0,0,0,0,0,0,0}; 
    int metadata_cabecera_2[7]={0,0,0,0,0,0,0}; 
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

    if(argc<4){
        printf("Error! \nFaltan argumentos\n");
        return 0;
    }else if (argc>4){
        printf("Error! \nSobran argumentos\n");
        return 0;
    }

    //Abrir archivos de entrada1, entrada2 y salida
    FILE *entrada_1=fopen(argv[1],"rb");
    FILE *entrada_2=fopen(argv[2],"rb");
    FILE *salida=fopen(argv[3],"wb");

    if (entrada_1==NULL || entrada_2==NULL || salida==NULL){
        fputs ("Error de lectura de archivos",stderr); 
        return 0;
    }

    // Lee los 44 caracteres de la cabecera del archivo WAV
    // Guarda los datos de la cabecera en int (arreglo metadatos_cabecera) para poderlos usar
    //Entrada1
    lectura_cabecera(entrada_1,cabecera_1,metadata_cabecera_1,argv[1],imprimir);
    //Entrada2
    lectura_cabecera(entrada_2,cabecera_2,metadata_cabecera_2,argv[2],imprimir);

    /*
    --> Aqui modificar la cabecera (arreglo cabecera)<--
        Las 44 posiciones del arreglo cabecera coindicen con cada uno de los 44 bytes
        que la componen 
    */
    unsigned char nueva_cabecera[44];
    int canal_1=metadata_cabecera_1[0];
    int canal_2=metadata_cabecera_2[0];

     //Imprime en el archivo de salida la cabecera (el arreglo con o sin modificaciones)
    int num_muestras_1=metadata_cabecera_1[5];
    int num_muestras_2=metadata_cabecera_2[5];
    int mayor;              //  0 = 1 = 2;  1 = 1 > 2;  2 = 1 < 2;  
    int canales;            //  0 = 1 y 1;  1 = 1 y 2;  2 = 2 y 1;  3 = 2 y 2; 

    //Marca que archivo tiene mas muestras
    if(num_muestras_1<num_muestras_2){
        mayor=2;
    }else if(num_muestras_2==num_muestras_1){
        mayor=0;
    }else{
        mayor=1;
    }

    //Marca que combinacion de canales corresponde 
    if(canal_1==2 && canal_2==2){
            canales = 3;
    }else if((canal_1==1 && canal_2==2) || (canal_1==2 && canal_2==1)){
        if(canal_1==1){
            canales = 1;
        }else
        {
            canales = 2;
        }
    }else if(canal_1==canal_2){
            canales = 0;
    }

    //Switch Edita y escribe la cabecera correspondiente en el archivo de salida
    switch (canales){
    // Archivos con canales iguales 1 y 1 o 2 y 2
    case 0:
    case 3:
        switch (mayor){
            case 1:
                fwrite(cabecera_1,sizeof(unsigned char),44,salida);
                break;
            case 2:
                fwrite(cabecera_2,sizeof(unsigned char),44,salida);
                break;
            default:
                fwrite(cabecera_1,sizeof(unsigned char),44,salida);
            break;
        }
        break;
    // Archivos con canales 1 y 2
    case 1:
        switch (mayor){
            case 1:
                // Archivos de diferentes tamaños se guardan como RAW
                printf("Archivos de diferente tamaño, se guardan como RAW");
                break;
            case 2:
                printf("Archivos de diferente tamaño, se guardan como RAW");
                // Archivos de diferentes tamaños se guardan como RAW
                break;
            default:
                //Archivos iguales
                fwrite(cabecera_2,sizeof(unsigned char),44,salida);
            break;
        }
        break;
    // Archivos con canales 2 y 1
    case 2:
        switch (mayor){
            case 1:
                // Archivos de diferentes tamaños se guardan como RAW
                printf("Archivos de diferente tamaño, se guardan como RAW");
                break;
            case 2:
                printf("Archivos de diferente tamaño, se guardan como RAW");
                // Archivos de diferentes tamaños se guardan como RAW
                break;
            default:
                //Archivos iguales
                fwrite(cabecera_1,sizeof(unsigned char),44,salida);
            break;
        }
        break;
    }


    // Normalizar el valor entre 0 y 1 dependiendo el valor mayor que se consiga despues de operar cada una de las muestras
    //double normalizar=1; 

    //Guarda las muestras en dos arreglos
    //arreglo_muestras_double -> Guarda las muestras con su valor en double dependiendo su configuracion 8,16 o 32 bits
    //arreglo_muestras_hex -> Guarda las muestras en su valor hexadecimal (1 byte en cada posicion del arreglo)

    int num_muestras_g=0;

    switch (mayor){
            case 1:
                num_muestras_g = num_muestras_1;
                break;
            case 2:
                num_muestras_g = num_muestras_2;
                break;
            default:
                num_muestras_g = num_muestras_1;
            break;
        }

    //Numero de muestras en hexadecimal
    int num_muestras_hex_1=num_muestras_1*(metadata_cabecera_1[4]/8);
    int num_muestras_hex_2=num_muestras_2*(metadata_cabecera_2[4]/8);

    //Arreglo general de muestras (Archivo 1)
    double *arreglo_muestras_1=malloc(num_muestras_g * sizeof(double));
    char *arreglo_muestras_hex_1=malloc(num_muestras_hex_1 * sizeof(char));

    //Arreglo general de muestras (Archivo 2)
    double *arreglo_muestras_2=malloc(num_muestras_g * sizeof(double));
    char *arreglo_muestras_hex_2=malloc(num_muestras_hex_2 * sizeof(char));

    //Arreglos para muestras complejas (Archivo 1)
    double *arreglo_muestras_1_R=malloc(num_muestras_g/2 * sizeof(double));
    double *arreglo_muestras_1_I=malloc(num_muestras_g/2 * sizeof(double));
        
    //Arreglos para muestras complejas (Archivo 2)
    double *arreglo_muestras_2_R=malloc(num_muestras_g/2 * sizeof(double));
    double *arreglo_muestras_2_I=malloc(num_muestras_g/2 * sizeof(double));

    switch (mayor){
        case 1:
            for(int i=0;i<num_muestras_g;i++){
                arreglo_muestras_2[i]=0;
            }
            for(int i=0;i<num_muestras_g/2;i++){
                arreglo_muestras_2_R[i]=0;
                arreglo_muestras_2_I[i]=0;
            }
        break;
        case 2:
            for(int i=0;i<num_muestras_g;i++){
                arreglo_muestras_1[i]=0;
            }
            for(int i=0;i<num_muestras_g/2;i++){
                arreglo_muestras_1_R[i]=0;
                arreglo_muestras_1_I[i]=0;
            }
        break;
        default:
        break;
    }
    

    lectura_muestras(entrada_1,arreglo_muestras_1,arreglo_muestras_hex_1,num_muestras_1,metadata_cabecera_1[4]);
    lectura_muestras(entrada_2,arreglo_muestras_2,arreglo_muestras_hex_2,num_muestras_2,metadata_cabecera_2[4]);
    //Cierro el archivo de entrada
    fclose(entrada_1);

    //Separar muestras complejas

    if(canales != 0){
        int m=0,n=0;
        for(int i=0;i<num_muestras_1;i++){
            if(i%2==0){
                arreglo_muestras_1_R[m]=arreglo_muestras_1[i];
                arreglo_muestras_2_R[m]=arreglo_muestras_2[i];
                m++;
            }else{
                arreglo_muestras_1_I[n]=arreglo_muestras_1[i];
                arreglo_muestras_2_I[n]=arreglo_muestras_2[i];
                n++;
            }
        }
    }

    //==========================================================================================

    //Aqui meter la funcion que le vamos a aplicar la señal
    //Dividir señal, Convolucion, TDF, TDFI, FFT, FFTI, DTMF, Multiplicacion
    
    double *arreglo_salidaR=malloc(num_muestras_1 * sizeof(double));
    double *arreglo_salidaI=malloc(num_muestras_1 * sizeof(double));

    //Funcion Multiplicar
    switch (canales){
    // Archivos con canales iguales 1 y 1 o 2 y 2
    case 0:
        multiplicarReal(arreglo_muestras_1,arreglo_muestras_2,arreglo_salidaR,num_muestras_1);
        break;
    case 3:
        multiplicarCmpjs(arreglo_muestras_1_R,arreglo_muestras_1_I,
                         arreglo_muestras_2_R,arreglo_muestras_2_I,
                         arreglo_salidaR,arreglo_salidaI,num_muestras_1/2);
        
        break;
    // Archivos con canales 1 y 2
    case 1:
         multiplicarCmpjs(arreglo_muestras_1_R,arreglo_muestras_1_I,
                         arreglo_muestras_2_R,arreglo_muestras_2_I,
                         arreglo_salidaR,arreglo_salidaI,num_muestras_g/2);
        break;
    // Archivos con canales 2 y 1
    case 2:
         multiplicarCmpjs(arreglo_muestras_1_R,arreglo_muestras_1_I,
                         arreglo_muestras_2_R,arreglo_muestras_2_I,
                         arreglo_salidaR,arreglo_salidaI,num_muestras_g/2);
        break;
    }
    

    //Regresar el arreglo resultado al archivo salida

    if(canales != 0){

        int m=0;
        int n=0;
        for(int i=0;i<num_muestras_1;i++){
            double muestra[1];
            if(i%2==0){
                muestra[0]=arreglo_salidaR[m];
                m++;
            }else{
                muestra[0]=arreglo_salidaI[n];
                n++;
            }
            regresar_arreglo_double(salida,muestra,1,metadata_cabecera_1[4]);
        }

    }else{
        regresar_arreglo_double(salida,arreglo_salidaR,num_muestras_1,metadata_cabecera_1[4]);

    }

    fclose(salida);

    return 0;
}

///Funciones generales para tratar el archivo

int lectura_cabecera(FILE *entrada,unsigned char *cabecera,int *metadata_cabecera,char *nombre,int flg){
    
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
        printf("\t\n->Archivo de ENTRADA %s\n",nombre);
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

void multiplicarReal(double *archivo_1,double *archivo_2,double *salida, int tam){
    for(int i=0;i<tam;i++){
        salida[i] = archivo_1[i]*archivo_2[i];
    }
}

// Multiplicar número complejos en forma binómica
// (a + bi) * (c + di) = (ac − bd) + ( ad + bc)i
void multiplicarCmpjs(double *a, double *b, double *c, double *d,double *salidaR,double *salidaI, int tam){
    for(int i=0;i<tam;i++){
        salidaR[i] = (a[i] * c[i]) - (b[i] * d[i]);
        salidaR[i]/=2;
        if(salidaR[i]>1){
            salidaR[i]=1;
        }
        if(salidaR[i]<-1){
            salidaR[i]=-1;
        }
        salidaI[i] = (a[i] * d[i]) + (b[i] * c[i]);
        salidaI[i]/=2;
        if(salidaI[i]>1){
            salidaI[i]=1;
        }
        if(salidaI[i]<-1){
            salidaI[i]=-1;
        }
    }
}