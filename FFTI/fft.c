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
void rellenarFFI(double *arreglo_muestras_double,double *arreglo_FFI_muestras_double,int num_muestras,int pot);
void fft(double *xr,double *xi,int N,int inverse);
void swap(double *x1,double *x2,int i,int j);

int main(int argc, char* argv[]){

    unsigned char cabecera[44];
    int metadata_cabecera[7]={0,0,0,0,0,0,0}; 

    int imprimir=0;  

    if(argc<3){
        printf("Error! \nFaltan argumentos\n");
        return 0;
    }else if (argc>3){
        printf("Error! \nSobran argumentos\n");
        return 0;
    }

    FILE *entrada=fopen(argv[1],"rb");
    FILE *salida=fopen(argv[2],"wb");

    if (entrada==NULL || salida==NULL){
        fputs ("Error de lectura de archivos",stderr); 
        return 0;
    }

 
    lectura_cabecera(entrada,cabecera,metadata_cabecera,imprimir);


    unsigned char cabecera_copia[44];

    copiar_cabecera(cabecera,cabecera_copia);


    if(metadata_cabecera[0] != 2){

        editar_cabecera(cabecera_copia,0,2);


        editar_cabecera(cabecera_copia,3,2*(metadata_cabecera[4]/8));

        editar_cabecera(cabecera_copia,5,(metadata_cabecera[5]*metadata_cabecera[4]/8)*2); 

        editar_cabecera(cabecera_copia,6,metadata_cabecera[6]+(metadata_cabecera[5]*(metadata_cabecera[4]/8)));

        editar_cabecera(cabecera_copia,2,metadata_cabecera[1]*2*(metadata_cabecera[4]/8));
    }


    fwrite(cabecera_copia,sizeof(unsigned char),44,salida);

    int num_muestras=metadata_cabecera[5];
    int num_muestras_hex=num_muestras*(metadata_cabecera[4]/8);

    double *arreglo_muestras_double=malloc(num_muestras * sizeof(double));
    char *arreglo_muestras_hex=malloc(num_muestras_hex * sizeof(char));
   
    lectura_muestras(entrada,arreglo_muestras_double,arreglo_muestras_hex,num_muestras,metadata_cabecera[4]);

    int pot=0;
    while(1){
        if(num_muestras>pow(2,pot)) pot++;
        else    break;
    }
    int num_m_pot_2=pow(2,pot);


    fclose(entrada);



    double *arreglo_FFI_muestras_real=malloc(pow(2,pot) * sizeof(double));
    double *arreglo_FFI_muestras_imag=malloc(pow(2,pot) * sizeof(double));

    
    for(int i=0;i<num_m_pot_2;i++){
        arreglo_FFI_muestras_imag[i]=0.0;
    }

    rellenarFFI(arreglo_muestras_double,arreglo_FFI_muestras_real,num_muestras,num_m_pot_2);

    fft(arreglo_FFI_muestras_real, arreglo_FFI_muestras_imag, num_m_pot_2, 1);

   
    int m=0;
    int n=0;
    for(int i=0;i<num_muestras*2;i++){
        double muestra[1];
        if(i%2==0){
                muestra[0]=arreglo_FFI_muestras_real[m];
            m++;
        }else{
                muestra[0]=arreglo_FFI_muestras_imag[n];
            n++;
        }
        regresar_arreglo_double(salida,muestra,1,metadata_cabecera[4]);
    }
    
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
            printf("Error posicion incorrecta!");
            break;
   }
}

void copiar_cabecera(unsigned char *cabecera,unsigned char *copia){
    for(int i=0;i<44;i++){
        copia[i]=cabecera[i];
    }
}

void rellenarFFI(double *arreglo_muestras_double,double *arreglo_FFI_muestras_double,int num_muestras,int num_m_pot){
    for(int i=0;i<num_m_pot;i++){
        if(i<num_muestras){
            arreglo_FFI_muestras_double[i]=arreglo_muestras_double[i];
        }else{
            arreglo_FFI_muestras_double[i]=0.0;
        }
    }

}

void swap(double *x1,double *x2,int i,int j){
    double aux = x1[i];
    x1[i] = x2[j];
    x2[j] = aux;
}

void fft(double *xr,double *xi,int N,int inverse) {
	int i,j,k,j1,m,n;
	float arg,s,c,w,tempr,tempi;
	m=log((float)N)/log(2.0);
	for (i=0; i<N;++i){
		j=0;
		for (k=0;k<m;++k)
			j=(j<<1)| (1 & (i>>k));
		if (j<i){
		swap(xr,xr,i,j); swap(xi,xi,i,j);	
			}
	}
	for (i=0;i<m;i++){
		n=w=pow(2.0,(float)i);
		w=M_PI/n;
		if (inverse) w=-w;
		k=0;
		while (k<N-1){
			for (j=0;j<n;j++){
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
	for (i=0;i<N;i++){
		if (xi[i]>1.0)xi[i]=1;
		if (xr[i]>1.0)xr[i]=1;
		if (xi[i]<-1.0)xi[i]=-1;
		if (xr[i]<-1.0)xr[i]=-1;
	}	
	
}