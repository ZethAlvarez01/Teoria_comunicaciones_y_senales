#include<stdio.h>
#include<math.h>

/*
Zeth Alvarez Hernandez 
Teoria de comunicaciones y señales
2020
*/

void lectura_muestras(FILE *entrada,float *arreglo_muestras_float,char *arreglo_muestras_hex,int num_bytes_totales,int num_muestras,int num_bits);
void regresar_arreglo_float(FILE* salida,float *arreglo_muestras_float,int num_muestras,int num_bytes_por_muestra,double normalizar);
int lectura_cabecera(FILE *entrada,unsigned char *cabecera,int *metadata_cabecera,int flg);

//Dividir una señal n veces  
void dividir_senal(float *arreglo_muestras,float *resultado,int num_muestras,int n);

// Link de esta funcion el cual nos muestra los algoritmos para la convolucion discreta
// http://www.songho.ca/dsp/convolution/convolution.html?fbclid=IwAR2HsCXDnYEzytz8pdeQhGSD5r6FX3d-5EoHVtrvTavEBCqjxsemOuvcD2A#cpp_conv1d
double convolucion1D(float* in, float* out, int dataSize, float* kernel, int kernelSize);


int main(int argc, char* argv[]){

    unsigned char cabecera[44];
    int metadata_cabecera[7]={0,0,0,0,0,0,0}; 
    /*
        Metadata de la cabecera
        [0] = canales
        [1] = frecuencia de muestreo
        [2] = byte_rate
        [3] = block_align
        [4] = tamaño en bytes de cada una de las muestras
        [5] = numero de muestras
        [6] = tamaño del archivo
    */
    int imprimir=1;   // 0 = imprime ; 1 = No imprime

    if(argc!=3){
        printf("Error! \nFaltan argumentos\n");
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
    --> Aqui modificar la cabecera (arregllo cabecera)<--
        Las 44 posiciones del arreglo cabecera coindicen con cada uno de los 44 bytes
        que la componen 
    */

    //Imprime en el archivo de salida la cabecera (el arreglo con o sin modificaciones)
    //Esta linea se puede comentar si quieres el archivo RAW
    fwrite(cabecera,sizeof(unsigned char),44,salida);


    int num_muestras=metadata_cabecera[5];
    float arreglo_muestras_float[num_muestras];
    char arreglo_muestras_hex[metadata_cabecera[2]];

    //Guarda las muestras en dos arreglos
    //arreglo_muestras_float -> Guarda las muestras con su valor en float dependiendo su configuracion 8,16 o 32 bits
    //arreglo_muestras_hex -> Guarda las muestras en su valor hexadecimal (1 byte en cada posicion del arreglo)
    lectura_muestras(entrada,arreglo_muestras_float,arreglo_muestras_hex,metadata_cabecera[2],num_muestras,metadata_cabecera[4]);

    //Cierro el archivo de entrada
    fclose(entrada);

    //Creamos un arreglo para los resultados, asi no modificamos o perdemos el arreglo original
    float arreglo_resultado[num_muestras];
    // Normalizar el valor entre 0 y 1 dependiendo el valor mayor que se consiga despues de operar cada una de las muestras
    double normalizar=1; 

    //Partir señal a la mitad
    //Arreglo de muestras - Arreglo para almacenar el resultadom - Numero de muestras - Por cuanto dividir la señal

    
    //dividir_senal(arreglo_muestras_float,arreglo_resultado,num_muestras,2);
    

    //Convolucion 1D
    /*
    Respuesta al impulso de un circuito RC con frecuencia de corte de 2000 hz y frecuencia de muestreo de 44100
    Ver Calculos_convolucion.png  
    */
     float convolucion[100]={1.000000, 0.752051, 0.565580, 0.425345, 0.319881, 0.240567, 0.180918, 0.136060, 0.102324, 0.076953, 
                            0.057872, 0.043523, 0.032731, 0.024616, 0.018512, 0.013922, 0.010470, 0.007874, 0.005922, 0.004453,
                            0.003349, 0.002519, 0.001894, 0.001425, 0.001071, 0.000806, 0.000606, 0.000456, 0.000343, 0.000258,
                            0.000194, 0.000146, 0.000110, 0.000082, 0.000062, 0.000047, 0.000035, 0.000026, 0.000020, 0.000015,
                            0.000011, 0.000008, 0.000006, 0.000005, 0.000004, 0.000003, 0.000002, 0.000002, 0.000001, 0.000001,
                            0.000001, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
                            0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
                            0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
                            0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
                            0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000};

    normalizar=convolucion1D(arreglo_muestras_float,arreglo_resultado,num_muestras,convolucion,100);

    //Regresa el arreglo_resultado al archivo de salida
    regresar_arreglo_float(salida,arreglo_resultado,num_muestras,metadata_cabecera[4],normalizar);

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
        metadata_cabecera[6]=tamano_archivo;

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

        //Numero de  muestras
        ind=43;
        long unsigned int num_muestras=0x0000;
        while(ind>=40){
            num_muestras<<=8;
            num_muestras+=cabecera[ind--];
        }
        metadata_cabecera[5]=num_muestras/(tamano_muestras/8);

        if(flg==1){
            printf("\t\n->Archivo de ENTRADA");
            printf("\t\nTamano del archivo:  %d bytes",tamano_archivo);
            printf("\t\nCanales: %d",canales);
            printf("\t\nFrecuencia de muestreo (Sample rate): %d Hz",frecuencia_muestreo); 
            printf("\t\nTasa de bytes (Byte rate): %d",byte_rate);
            printf("\t\nBlock Align: %d",block_align);
            printf("\t\nTamano de las muestras: %d bytes",tamano_muestras);
            printf("\t\nNumero de muestras: %d ",num_muestras/(tamano_muestras/8));
            printf("\n\n");    
        }

    return 0;
}

void lectura_muestras(FILE *entrada,float *arreglo_muestras_float,char *arreglo_muestras_hex,int num_bytes_totales,int num_muestras,int num_bits){

    int ind=0;
    while(ind<num_bytes_totales){
        arreglo_muestras_hex[ind]=fgetc(entrada);
        ind++;
    }   

    long int valor=0x00;
    char aux=0x0;
    ind=0;
    
    for(int i=0;i<=num_bytes_totales;i++){
        if((i+1)%(num_bits/8)==0){
            valor=arreglo_muestras_hex[i];
            valor<<=8;
            valor+=aux;
            arreglo_muestras_float[ind]=valor/((pow(2,num_bits)/2)-1);
            valor=0x00; aux=0x0; ind++;
        }else{
            aux=arreglo_muestras_hex[i];
        }
    }


}

void regresar_arreglo_float(FILE* salida,float *arreglo_muestras_float,int num_muestras,int num_bytes_por_muestra,double normalizar){

   for (int i=0;i<num_muestras;i++){
        unsigned long int aux=((arreglo_muestras_float[i])*((pow(2,(num_bytes_por_muestra))/2)-1))/normalizar;
        unsigned char regresar[4]={0x00,0x00,0x00,0x00};
        for(int j=0;j<num_bytes_por_muestra/8;j++){
            regresar[j]=(unsigned char) (aux>>(8*j));
        }
        fwrite(regresar,1,num_bytes_por_muestra/8,salida);
    }
   
}


void dividir_senal(float *arreglo_muestras,float *resultado,int num_muestras,int n){
    for(int i=0;i<num_muestras;i++){
        resultado[i]=arreglo_muestras[i]/n;
    }
}


double convolucion1D(float* in, float* out, int dataSize, float* kernel, int kernelSize){
    int i, j, k;
    double maximo=0, val_abs=0;

    // check validity of params
    if(!in || !out || !kernel) return 0;
    if(dataSize <=0 || kernelSize <= 0) return 0;

    // start convolution from out[kernelSize-1] to out[dataSize-1] (last)
    for(i = kernelSize-1; i < dataSize; ++i)
    {
        out[i] = 0;                             // init to 0 before accumulate

        for(j = i, k = 0; k < kernelSize; --j, ++k)
            out[i] += in[j] * kernel[k];

        val_abs=fabs(out[i]);
        if(val_abs>maximo){
            maximo=val_abs;
        }
    }

    // convolution from out[0] to out[kernelSize-2]
    for(i = 0; i < kernelSize - 1; ++i)
    {
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
