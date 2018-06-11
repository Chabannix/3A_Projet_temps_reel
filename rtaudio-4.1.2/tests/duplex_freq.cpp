/******************************************/
/*
  duplex.cpp
  by Gary P. Scavone, 2006-2007.

  This program opens a duplex stream and passes
  input directly through to the output.
*/
/******************************************/

#include "RtAudio.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "../../ressources_tstr_v1_1/c/somefunc.cpp"


typedef double MY_TYPE;
#define FORMAT RTAUDIO_FLOAT64


typedef struct {
      double tailleRepImp; 	//taille reponse impulsionnelle
      double taille_conv;	//taille du buffer contenant le résultat total de la convolution (L+M-1)
      double taille_rest_conv;	//taille du buffer contenant le reste de la convolution (M-1)

      double * impres;  //reponse impulsionnelle
      double * conv;  //buffer contenant le résultat de la convolution
      double * rest_conv;  //buffer contenant le reste de la conv

      int taille_fft;  //taille de la FFT
      double * fft_impres;  //FFT de la reponse imulsionnelle
      double * tmp_impres;  //pointeur tampon qui indique la fin de la partie reelle et le debut de la partie imaginaire
      double * fft_buff;  // FFT du buffer d'entrée

} MyData;


void usage( void ) {
      // Error function in case of incorrect command-line
      // argument specifications
      std::cout << "\nuseage: duplex N fs <iDevice> <oDevice> <iChannelOffset> <oChannelOffset>\n";
      std::cout << "    where N = number of channels,\n";
      std::cout << "    fs = the sample rate,\n";
      std::cout << "    iDevice = optional input device to use (default = 0),\n";
      std::cout << "    oDevice = optional output device to use (default = 0),\n";
      std::cout << "    iChannelOffset = an optional input channel offset (default = 0),\n";
      std::cout << "    and oChannelOffset = optional output channel offset (default = 0).\n\n";
      exit( 0 );
}


/* **************************************************************************
***********************    convolution frequentielle   **********************
*************************************************************************** */

void fconv(MyData * ptData, double* buffer, unsigned int nBufferFrames)
{
      int i;
      double x,y,a,b; //variables pour stocker les pointeurs sur les FFT

      for (i = 0; i < ptData->taille_fft; ++i)	//initialisation : on met les buffers des FFT à 0
      {
            *(ptData->fft_buff+i)=0;
            *(ptData->fft_impres+i)=0;
      }

      memcpy(ptData->fft_buff,buffer, nBufferFrames*sizeof(double));	//copie du signal à traiter dans le tableau

      double* tmp = ptData->fft_buff + ptData->taille_fft;	//pointeur tampon qui indique la fin de la partie reelle et le debut de la partie imaginaire


      fftr(ptData->fft_buff,tmp,ptData->taille_fft); //calcul de la FFT : ptData->fft_buff contient le resultat de la FFT

      //Calcul de la convolution
      for (i = 0; i < ptData->taille_fft; i++)
      {
            x = *(ptData->fft_buff+i);	//partie reelle de la FFT du buffer
            y = *(tmp+i);	//partie imaginaire de la FFT du buffer


            a = *(ptData->fft_impres+i);	//partie reelle de la FFT de la reponse impulsionnelle
            b = *(ptData->tmp_impres+i);	//partie imaginaire de la FFT de la reponse impulsionnelle


            *(ptData->conv+i) = a*x -b*y;	//partie reelle du resultat de la convolution
            *(ptData->conv+i+ptData->taille_fft) = a*y+b*x;	//partie imaginaire du resultat de la convolution
      }
	//ptData->conv contient le resultat de la convolution

      double* res_ifft = (double *)calloc(2*ptData->taille_fft,sizeof(double));	//variable pour stocker le résultat de la FFT inverse
      memcpy(res_ifft,ptData->conv, 2*ptData->taille_fft*sizeof(double));	//copie du signal à traiter (le resultat de la convolution)
     
      double* tmp3 = res_ifft + ptData->taille_fft;	//pointeur tampon qui indique la fin de la partie reelle et le debut de la partie imaginaire
      ifft(res_ifft,tmp3,ptData->taille_fft); // res_ifft contient maintenant la transformée de Fourier inverse
      memcpy(ptData->conv, res_ifft, 2*ptData->taille_fft*sizeof(double));	//copie du resultat final dans ptData->conv
}


/* **************************************************************************
*****************************    fonction inout   ****************************
*************************************************************************** */

int inout( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
           double /*streamTime*/, RtAudioStreamStatus status, void * ptData)
{
      double time1 = get_process_time();	//1er relevé temporel

      // Since the number of input and output channels is equal, we can do
      // a simple buffer copy operation here.
      if ( status ) std::cout << "Stream over/underflow detected." << std::endl;

      MyData * ptData2 = (MyData *) ptData;	//recast de la structure ptData passée en paramètre
      double * input = (double *) inputBuffer;

      fconv(ptData2, (double *) input, nBufferFrames);	//calcul de la convolution frequentielle du buffer d'entree avec la reponse impulsionnelle stockée dans ptData2



      // On somme le résultat avec le reste de la convolution précedente

      for(int i=0; i<ptData2->taille_rest_conv;i++)
                  *(ptData2->conv+i) = *(ptData2->conv+i)+*(ptData2->rest_conv+i);


      memcpy(outputBuffer, ptData2->conv, nBufferFrames*sizeof(double));	//copie du resultat de la convolution sur le buffer de sortie
      memcpy( (void *)(ptData2->rest_conv), (void *)(ptData2->conv+(int)(nBufferFrames)), ptData2->taille_rest_conv*sizeof(double));


      double time2 = get_process_time();	//deuxième relevé temporel

      // ****** décommenter la ligne suivante pour avoir le temps de calcul dans le terminal *******
      //std::cout << "\nProcess time = " << time2 - time1 << ".\n";

      return 0;
}

/* **************************************************************************
*****************************		Début du main		*****************************
*************************************************************************** */

int main( int argc, char *argv[] )
{
      std::cout << std::endl;
      std::cout << std::endl;
      system("clear");
      std::cout << "*******************************************************"<< std::endl;
      std::cout << "****    duplex.cpp modifié par Débora et Damien    ****"<< std::endl;
      std::cout << "****             projet temps réel 2017            ****"<< std::endl;
      std::cout << "****           convolution fréquentielle           ****"<< std::endl;
      std::cout << "*******************************************************"<< std::endl;
      std::cout << std::endl;


      unsigned int channels, fs, bufferBytes, oDevice = 0, iDevice = 0, iOffset = 0, oOffset = 0;

      // Minimal command-line checking
      if (argc < 3 || argc > 7 ) usage();

      RtAudio adac;
      if ( adac.getDeviceCount() < 1 ) {
            std::cout << "\nNo audio devices found!\n";
            exit( 1 );
      }

      channels = (unsigned int) atoi(argv[1]);
      fs = (unsigned int) atoi(argv[2]);
      if ( argc > 3 )
            iDevice = (unsigned int) atoi(argv[3]);
      if ( argc > 4 )
            oDevice = (unsigned int) atoi(argv[4]);
      if ( argc > 5 )
            iOffset = (unsigned int) atoi(argv[5]);
      if ( argc > 6 )
            oOffset = (unsigned int) atoi(argv[6]);

      // Let RtAudio print messages to stderr.
      adac.showWarnings( true );

      // Set the same number of channels for both input and output.
      unsigned int bufferFrames = 512;  //en frames
      RtAudio::StreamParameters iParams, oParams;
      iParams.deviceId = iDevice;
      iParams.nChannels = channels;
      iParams.firstChannel = iOffset;
      oParams.deviceId = oDevice;
      oParams.nChannels = channels;
      oParams.firstChannel = oOffset;

      /***********************************************************************************************
      *********   Initialisation de la structre et chargement de la réponse impulsionnelle  ********** 
      ***********************************************************************************************/

      //Allocation de la structure MyData
      MyData *ptData = (MyData *) malloc (sizeof(MyData));
      if (ptData == NULL) std::cout << "erreur à l'allocation pour myData\n" << std::endl; 

      //Chargement de la reponse impulsionnelle
      FILE * pFile;
      pFile = fopen ( "../../ressources_tstr_v1_1/c/impres" , "rb" );
      if (pFile==NULL) {fputs ("File error",stderr); exit (1);}	

      ptData->tailleRepImp = 20000;	//choix du nombre d'échantillons de la réponse impulsionnelle que l'on garde
      ptData->impres = (double*)calloc (ptData->tailleRepImp,sizeof(double));
      fread(ptData->impres, sizeof(double), ptData->tailleRepImp, pFile);

      ptData->taille_fft = get_nextpow2(ptData->tailleRepImp + bufferFrames - 1);

      ptData->taille_conv = bufferFrames+ptData->tailleRepImp -1;     // en frames
      ptData->conv=(double *)calloc(ptData->taille_fft*2,sizeof(double));	//allocation de memoire pour la convolution
 
      ptData->taille_rest_conv = ptData->taille_conv - bufferFrames;
      ptData->rest_conv = (double*)calloc (ptData->taille_rest_conv,sizeof(double));

      ptData->fft_buff=(double *)calloc(ptData->taille_fft*2,sizeof(double));	//allocation de memoire pour la FFT du buffer
      ptData->fft_impres=(double *)calloc(ptData->taille_fft*2,sizeof(double));	//allocation de memoire pour la FFT de la reponse impulsionnelle

      //Calcul de la FFT de la reponse impulsionnelle
      memcpy(ptData->fft_impres,ptData->impres,ptData->tailleRepImp*sizeof(double));
      ptData->tmp_impres = ptData->fft_impres + ptData->taille_fft;
      fftr(ptData->fft_impres,ptData->tmp_impres,ptData->taille_fft); // ptData->fft_impres contient maintenant le résultat de la FFT


      /***********************************************************************************************
      ******************    Visualisation de la structure dans le terminal   ************************ 
      ***********************************************************************************************/

      std::cout << "****************  Paramètres du main : ****************" << std::endl; 
      std::cout << "bufferFrames = " << bufferFrames << std::endl; 
      std::cout << "nChannels = " << channels << std::endl; 

      std::cout << std::endl;

      std::cout << "****************  Structure ptData : ******************" << std::endl;

      std::cout << "ptData->tailleRepImp = " << ptData->tailleRepImp << " frames (" << ptData->tailleRepImp*8 << " octets)" << std::endl;
      std::cout << "ptData->taille_conv = " << ptData->taille_conv << " frames (" << ptData->taille_conv*8 << " octets)" << std::endl;
      std::cout << "ptData->taille_rest_conv = " << ptData->taille_rest_conv << " frames (" << ptData->taille_rest_conv*8 << " octets)" << std::endl;
      std::cout << "ptData->taille_fft = " << ptData->taille_fft << " frames (" << ptData->taille_fft*8 << " octets)" << std::endl;
  

      std::cout << std::endl;

      std::cout << "ptData->impres    :" << ptData->impres << std::endl;
      std::cout << "ptData->conv      :" << ptData->conv << std::endl;
      std::cout << "*******************************************************"<< std::endl; 

 
      /***********************************************************************************************
      *********************             Choix des périphériques           *************************** 
      *******************            si iDevice == 0 => micro par défaut           ******************* 
      *******************       si oDevice == 0 => haut-parleurs par défaut        ******************* 
      ***********************************************************************************************/

      if ( iDevice == 0 )
            iParams.deviceId = adac.getDefaultInputDevice();
      if ( oDevice == 0 )
            oParams.deviceId = adac.getDefaultOutputDevice();

      RtAudio::StreamOptions options;
      //options.flags |= RTAUDIO_NONINTERLEAVED;


      /***********************************************************************************************
      *********************                Fonction Callback              *************************** 
      ***********************************************************************************************/

      try {
            adac.openStream( &oParams, &iParams, FORMAT, fs, &bufferFrames, &inout, (void *)(ptData), &options );
      }
      catch ( RtAudioError& e ) {
            std::cout << '\n' << e.getMessage() << '\n' << std::endl;
      exit( 1 );
      }


      /***********************************************************************************************
      ***********************************************************************************************/

      bufferBytes = bufferFrames * channels * sizeof( MY_TYPE );

      // Test RtAudio functionality for reporting latency.
      std::cout << "\nStream latency = " << adac.getStreamLatency() << " frames" << std::endl;

      try {
            adac.startStream();

            char input;
            std::cout << "\nRunning ... press <enter> to quit (buffer frames = " << bufferFrames << ").\n";
            std::cin.get(input);

            // Stop the stream.
            adac.stopStream();
      }
      catch ( RtAudioError& e ) {
            std::cout << '\n' << e.getMessage() << '\n' << std::endl;
            goto cleanup;
      }

      cleanup:
      if ( adac.isStreamOpen() ) adac.closeStream();


      /***********************************************************************************************
      *************         fermeture du fichier et libération de la mémoire           ************** 
      ***********************************************************************************************/

      fclose (pFile);
      free(ptData->conv);
      free(ptData->impres);
      free(ptData->fft_buff);
      free(ptData->fft_impres);
      free(ptData);


      return 0;
}
