#include <Complex.h>    /* Standard Library of Complex Numbers */
#include <math.h>

#define l_muestras 128     //Numero de bytes que tiene la muestra
#define pi 3.1416
#define threshold 300000
#define l_tiempo 20
//----------------------    ---------------------
//FFT:
//-------------------------------------------
//Variables globales:
complex r_pos=0;
complex r_neg=0;

complex muestras[l_muestras];
complex resultado[l_muestras];  

uint8_t max_it=0;
uint8_t N=l_muestras;
uint8_t indice[l_muestras];   //Indice para mariposas
//Sampling register:
uint8_t samples[l_muestras][l_tiempo];

//Funciones:
void mariposa(double complex v_pos,double complex v_neg,int N_m,int f_m);

void setup(){
    //-------------------------------------------
    //Indicador:
    //-------------------------------------------
    pinMode(22, OUTPUT);

    //-------------------------------------------
    //ADC:
    //-------------------------------------------
    analogReadResolution(9);
    analogSetWidth(9);
    
    //-------------------------------------------
    //Serial:
    //-------------------------------------------
    Serial.begin(500000);

    //-------------------------------------------
    //Inicializacion de la FFT:
    //-------------------------------------------
    
    //Define el numero de muestras:
    N=l_muestras;

    //Inicializa el indice con 0
    indice[0]=0;

    float l_ob = l_muestras;
    max_it=0;

    while (l_ob >= 2){  // Obtencion de reordenamiento
        
        l_ob = l_ob/2;

        if(l_ob >= 2){
            max_it++;
        }
    }
    
    
    for(int i=0; i<=max_it; i++){  //Repite esto hasta el numero de iteraciones maximo
        int div=pow(2,i+1);
        int lim_ac = N/pow(2,max_it+1-i);
        
        for(int j=0; j<lim_ac; j++){
            indice[j+lim_ac]=indice[j]+N/div;
        }
    }

}

void loop(){
    //Await for incoming byte
    while(Serial.available() == 0); 

    //Checks for the initialization word
    uint8_t key=Serial.read();    
    
    if(key == 'B'){
        //Serial.println("Beggining...");

        //-------------------------------------------
        //Sampling
        //-------------------------------------------

        digitalWrite(22,HIGH);

        //Sampling:
        for(uint8_t j_s=0; j_s < l_tiempo; j_s++){
            for(uint8_t i_s=0; i_s < l_muestras; i_s++){
                samples[i_s][j_s] = analogRead(34) >> 1;
                delayMicroseconds(1000);
            }
        }

        digitalWrite(22,LOW);

        //FFT para todas las muestras en el tiempo
        for(uint8_t k = 0; k<l_tiempo; k++){
            
            //Muestras to complex muestras
            for(uint8_t i=0; i<l_muestras; i++){
                muestras[i] = samples[i][k];
            }
            
            //FFT:
            for(int i=0; i <= max_it; i++){ //Numero de series de mariposas a realizar (3 para 8 bits, 4 para 16 bits y 5 para 32 bits)
                uint8_t div=pow(2,max_it-i);
                uint8_t N_m=N/div;
                
                uint8_t i_pos=0;          //Indice normal positivo
                uint8_t i_neg=pow(2,i);   //Indice normal negativo

                for(int j=0; j < N/(2*pow(2,i)); j++){

                    for(int k=0; k < pow(2,i); k++){
                        mariposa(muestras[indice[i_pos+k]],muestras[indice[i_neg+k]],N,div*k);  //Puede modificarse para que N_m sea constante
                        resultado[indice[i_pos+k]] = r_pos;
                        resultado[indice[i_neg+k]] = r_neg;
                    }

                    i_pos += pow(2,i+1);
                    i_neg += pow(2,i+1);

                }

                for(int j=0; j<N; j++){
                    muestras[j]=resultado[j];
                }

            }

            
            //Shift de los registros y normalizacion a decibeles:
            for(int i=0; i < N; i++){
                if(resultado[indice[i]]==0){
                   samples[i][k]==255; 
                }else{
                   samples[i][k]=abs(20*log(cabs(resultado[indice[i]]/N)/255));
                }
                
            }

        }//End de la FFT general

        //Serial.println("Done...");

        //Print the values to the Serial:
        for(uint8_t j=0; j<l_tiempo; j++){
            for(uint8_t i=0; i<l_muestras; i++){
                Serial.println(samples[i][j]);
            }
        }
       
    }
}

void mariposa(double complex v_pos,double complex v_neg,int N_m,int f_m){
  r_pos = v_pos + v_neg*cexp(-2*pi*I*f_m/N_m);
  r_neg = v_pos - v_neg*cexp(-2*pi*I*f_m/N_m);
}
