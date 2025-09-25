
// include the library
#include <RadioLib.h>
#include <string.h>
// SX1278 has the following connections:
// NSS pin:   10
// DIO0 pin:  2
// RESET pin: 9
// DIO1 pin:  3
SX1278 radio = new Module(10, 2, 9, 3);

// or using RadioShield
// https://github.com/jgromes/RadioShield
//SX1278 fsk = RadioShield.ModuleA;

#include<stdlib.h>
#include <math.h>
#include <stdio.h>

int             m, n, length, k, t, d;
int             p[21];
int             alpha_to[50], index_of[50], g[50];
int             recd[50], data[50], bb[50];
int             seed;
int             numerr, errpos[50], decerror = 0;
char            *arrChar;





void setup() 
{
  Serial.begin(9600);

  // initialize SX1278 FSK modem with default settings
  Serial.print(F("[SX1278] Initializing ... "));
  int state = radio.beginFSK();
  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

  // if needed, you can switch between LoRa and FSK modes
  //
  // radio.begin()       start LoRa mode (and disable FSK)
  // radio.beginFSK()    start FSK mode (and disable LoRa)

  // the following settings can also
  // be modified at run-time
  state = radio.setFrequency(433.5);
  state = radio.setBitRate(100.0);
  state = radio.setFrequencyDeviation(10.0);
  state = radio.setRxBandwidth(250.0);
  state = radio.setOutputPower(10.0);
  state = radio.setCurrentLimit(100);
  state = radio.setDataShaping(RADIOLIB_SHAPING_0_5);
  uint8_t syncWord[] = {0x01, 0x23, 0x45, 0x67,
                        0x89, 0xAB, 0xCD, 0xEF};
  state = radio.setSyncWord(syncWord, 8);
  if (state != ERR_NONE) {
    Serial.print(F("Unable to set configuration, code "));
    Serial.println(state);
    while (true);
  }

}




 
void decode_bch()
/*
 * Simon Rockliff's implementation of Berlekamp's algorithm.
 *
 * Assume we have received bits in recd[i], i=0..(n-1).
 *
 * Compute the 2*t syndromes by substituting alpha^i into rec(X) and
 * evaluating, storing the syndromes in s[i], i=1..2t (leave s[0] zero) .
 * Then we use the Berlekamp algorithm to find the error location polynomial
 * elp[i].
 *
 * If the degree of the elp is >t, then we cannot correct all the errors, and
 * we have detected an uncorrectable error pattern. We output the information
 * bits uncorrected.
 *
 * If the degree of elp is <=t, we substitute alpha^i , i=1..n into the elp
 * to get the roots, hence the inverse roots, the error location numbers.
 * This step is usually called "Chien's search".
 *
 * If the number of errors located is not equal the degree of the elp, then
 * the decoder assumes that there are more than t errors and cannot correct
 * them, only detect them. We output the information bits uncorrected.
 */
{
  register int    i, j, u, q, t2, count = 0, syn_error = 0;
  int             elp[50][50], d[50], l[50], u_lu[50], s[50];
  int             root[50], loc[50], err[50], reg[50];

  t2 = 2 * t;

  /* first form the syndromes */
  printf("S(x) = ");
  for (i = 1; i <= t2; i++) {
    s[i] = 0;
    for (j = 0; j < length; j++)
      if (recd[j] != 0)
        s[i] ^= alpha_to[(i * j) % n];
    if (s[i] != 0)
      syn_error = 1; /* set error flag if non-zero syndrome */
/*
 * Note:    If the code is used only for ERROR DETECTION, then
 *          exit program here indicating the presence of errors.
 */
    /* convert syndrome from polynomial form to index form  */
    s[i] = index_of[s[i]];
    printf("%3d ", s[i]);
  }
  printf("\n");

  if (syn_error) {  /* if there are errors, try to correct them */
    /*
     * Compute the error location polynomial via the Berlekamp
     * iterative algorithm. Following the terminology of Lin and
     * Costello's book :   d[u] is the 'mu'th discrepancy, where
     * u='mu'+1 and 'mu' (the Greek letter!) is the step number
     * ranging from -1 to 2*t (see L&C),  l[u] is the degree of
     * the elp at that step, and u_l[u] is the difference between
     * the step number and the degree of the elp. 
     */
    /* initialise table entries */
    d[0] = 0;     /* index form */
    d[1] = s[1];    /* index form */
    elp[0][0] = 0;    /* index form */
    elp[1][0] = 1;    /* polynomial form */
    for (i = 1; i < t2; i++) {
      elp[0][i] = -1; /* index form */
      elp[1][i] = 0;  /* polynomial form */
    }
    l[0] = 0;
    l[1] = 0;
    u_lu[0] = -1;
    u_lu[1] = 0;
    u = 0;
 
    do {
      u++;
      if (d[u] == -1) {
        l[u + 1] = l[u];
        for (i = 0; i <= l[u]; i++) {
          elp[u + 1][i] = elp[u][i];
          elp[u][i] = index_of[elp[u][i]];
        }
      } else
        /*
         * search for words with greatest u_lu[q] for
         * which d[q]!=0 
         */
      {
        q = u - 1;
        while ((d[q] == -1) && (q > 0))
          q--;
        /* have found first non-zero d[q]  */
        if (q > 0) {
          j = q;
          do {
            j--;
            if ((d[j] != -1) && (u_lu[q] < u_lu[j]))
              q = j;
          } while (j > 0);
        }
 
        /*
         * have now found q such that d[u]!=0 and
         * u_lu[q] is maximum 
         */
        /* store degree of new elp polynomial */
        if (l[u] > l[q] + u - q)
          l[u + 1] = l[u];
        else
          l[u + 1] = l[q] + u - q;
 
        /* form new elp(x) */
        for (i = 0; i < t2; i++)
          elp[u + 1][i] = 0;
        for (i = 0; i <= l[q]; i++)
          if (elp[q][i] != -1)
            elp[u + 1][i + u - q] = 
                                   alpha_to[(d[u] + n - d[q] + elp[q][i]) % n];
        for (i = 0; i <= l[u]; i++) {
          elp[u + 1][i] ^= elp[u][i];
          elp[u][i] = index_of[elp[u][i]];
        }
      }
      u_lu[u + 1] = u - l[u + 1];
 
      /* form (u+1)th discrepancy */
      if (u < t2) { 
      /* no discrepancy computed on last iteration */
        if (s[u + 1] != -1)
          d[u + 1] = alpha_to[s[u + 1]];
        else
          d[u + 1] = 0;
          for (i = 1; i <= l[u + 1]; i++)
            if ((s[u + 1 - i] != -1) && (elp[u + 1][i] != 0))
              d[u + 1] ^= alpha_to[(s[u + 1 - i] 
                            + index_of[elp[u + 1][i]]) % n];
        /* put d[u+1] into index form */
        d[u + 1] = index_of[d[u + 1]];  
      }
    } while ((u < t2) && (l[u + 1] <= t));
 
    u++;
    if (l[u] <= t) {/* Can correct errors */
      /* put elp into index form */
      for (i = 0; i <= l[u]; i++)
        elp[u][i] = index_of[elp[u][i]];

      printf("sigma(x) = ");
      for (i = 0; i <= l[u]; i++)
        printf("%3d ", elp[u][i]);
      printf("\n");
      printf("Roots: ");

      /* Chien search: find roots of the error location polynomial */
      for (i = 1; i <= l[u]; i++)
        reg[i] = elp[u][i];
      count = 0;
      for (i = 1; i <= n; i++) {
        q = 1;
        for (j = 1; j <= l[u]; j++)
          if (reg[j] != -1) {
            reg[j] = (reg[j] + j) % n;
            q ^= alpha_to[reg[j]];
          }
        if (!q) { /* store root and error
             * location number indices */
          root[count] = i;
          loc[count] = n - i;
          count++;
          printf("%3d ", n - i);
        }
      }
      printf("\n");
      if (count == l[u])  
      /* no. roots = degree of elp hence <= t errors */
        for (i = 0; i < l[u]; i++)
          recd[loc[i]] ^= 1;
      else  /* elp has degree >t hence cannot solve */
        printf("Incomplete decoding: errors detected\n");
    }
  }
}

 


int count=1;

void loop() {
  // FSK modem can use the same transmit/receive methods
  // as the LoRa modem, even their interrupt-driven versions
  // NOTE: FSK modem maximum packet length is 63 bytes!

  int  i;

  //read_p();               /* Read m */
  //generate_gf();          /* Construct the Galois Field GF(2**m) */
  //gen_poly();             /* Compute the generator polynomial of BCH code */

  /* Randomly generate DATA */
  /*seed = 131073;
  srandom(seed);
  for (i = 0; i < k; i++)
    data[i] = ( random() & 65536 ) >> 16;*/
    
  int data[]={1, 1, 1, 0, 1};
             
  /*
   * recd[] are the coefficients of c(x) = x**(length-k)*data(x) + b(x)
   */
  for (i = 0; i < length - k; i++)
    recd[i] = bb[i];
  for (i = 0; i < k; i++)
    recd[i + length - k] = data[i];
  /*printf("Code polynomial:\nc(x) = ");
  for (i = 0; i < length; i++) {
    printf("%1d", recd[i]);
    if (i && ((i % 50) == 0))
      printf("\n");
  }
  printf("\n");*/
  
  
  


 
  // FSK modem can use the same transmit/receive methods
  // as the LoRa modem, even their interrupt-driven versions
  // NOTE: FSK modem maximum packet length is 63 bytes!

  // receive FSK packet
  String str;
  int state = radio.receive(str);
  /*
    byte byteArr[8];
    int state = radio.receive(byteArr, 8);
  */
  if (state == ERR_NONE) {
    Serial.println(F("[SX1278] Received packet!"));
    Serial.print(F("[SX1278] Data:\t"));
    Serial.println(str);

    // print the RSSI (Received Signal Strength Indicator)
    // of the last received packet
    Serial.print(F("[SX1278] RSSI:\t\t\t"));
    Serial.print(radio.getRSSI());
    Serial.println(F(" dBm"));

    // print the SNR (Signal-to-Noise Ratio)
    // of the last received packet
    Serial.print(F("[SX1278] SNR:\t\t\t"));
    Serial.print(radio.getSNR());
    Serial.println(F(" dB"));

    // print frequency error
    // of the last received packet
    Serial.print(F("[SX1278] Frequency error:\t"));
    Serial.print(radio.getFrequencyError());
    Serial.println(F(" Hz"));

  }
  else if (state == ERR_RX_TIMEOUT)
  {
    Serial.println(F("[SX1278] Timed out while waiting for packet!"));
  } else
  {
    Serial.println(F("[SX1278] Failed to receive packet, code "));
    Serial.println(state);
  }

  String str2=str;

  int  iarr [length];
    
  for(i = 0; i <length; i++)
  {
      int ich = str2[i] - 48;
      iarr[i] = ich;
      recd[i]=iarr[i];
  }



  
  decode_bch();             /* DECODE received codeword recv[] */

  /*
   * print out original and decoded data
   */
  Serial.print("Results:\n");
  Serial.print("original data  = ");
  for (i = 0; i < k; i++) {
    printf("%1d", data[i]);
    if (i && ((i % 50) == 0))
      printf("\n");
  }
  
  printf("\nrecovered data = ");
  for (i = length - k; i < length; i++) {
    Serial.print("%1d", recd[i]);
    if ((i-length+k) && (((i-length+k) % 50) == 0))
      Serial.print("\n");
  }
  printf("\n");

  /*
   * DECODING ERRORS? we compare only the data portion
   
  for (i = length - k; i < length; i++)
    if (data[i - length + k] != recd[i])
      decerror++;
  if (decerror)
     printf("There were %d decoding errors in message positions\n", decerror);
  else
     printf("Succesful decoding\n");
  
   */
  
  
}

  
