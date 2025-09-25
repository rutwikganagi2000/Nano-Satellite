
// include the library
#include <RadioLib.h>
#include <string.h>
// SX1278 has the following connections:
// NSS pin:   10
// DIO0 pin:  2
// RESET pin: 4
// DIO1 pin:  3
SX1278 radio = new Module(10, 2, 4, 3);

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



void read_p()
      
/*
 *  Read m, the degree of a primitive polynomial p(x) used to compute the
 *  Galois field GF(2**m). Get precomputed coefficients p[] of p(x). Read
 *  the code length.
 */
{
  int     i, ninf;
  /*
  printf("bch3: An encoder/decoder for binary BCH codes\n");
  printf("Copyright (c) 1994-7. Robert Morelos-Zaragoza.\n");
  printf("This program is free, please read first the copyright notice.\n");
  printf("\nFirst, enter a value of m such that the code length is\n");
  printf("2**(m-1) - 1 < length <= 2**m - 1\n\n");*/
    do {
     //printf("Enter m (between 2 and 20): ");
     //scanf("%d", &m);
      m=4;
    } while ( !(m>1) || !(m<21) );
  for (i=1; i<m; i++)
    p[i] = 0;
  p[0] = p[m] = 1;
  if (m == 2)     p[1] = 1;
  else if (m == 3)  p[1] = 1;
  else if (m == 4)  p[1] = 1;
  else if (m == 5)  p[2] = 1;
  else if (m == 6)  p[1] = 1;
  else if (m == 7)  p[1] = 1;
  else if (m == 8)  p[4] = p[5] = p[6] = 1;
  else if (m == 9)  p[4] = 1;
  else if (m == 10) p[3] = 1;
  else if (m == 11) p[2] = 1;
  else if (m == 12) p[3] = p[4] = p[7] = 1;
  else if (m == 13) p[1] = p[3] = p[4] = 1;
  else if (m == 14) p[1] = p[11] = p[12] = 1;
  else if (m == 15) p[1] = 1;
  else if (m == 16) p[2] = p[3] = p[5] = 1;
  else if (m == 17) p[3] = 1;
  else if (m == 18) p[7] = 1;
  else if (m == 19) p[1] = p[5] = p[6] = 1;
  else if (m == 20) p[3] = 1;
  printf("p(x) = ");
    n = 1;
  for (i = 0; i <= m; i++) {
        n *= 2;
    printf("%1d", p[i]);
        }
  printf("\n");
  n = n / 2 - 1;
  ninf = (n + 1) / 2 - 1;
  do  {
    //printf("Enter code length (%d < length <= %d): ", ninf, n);
    //scanf("%d", &length);
    length=15;
  } while ( !((length <= n)&&(length>ninf)) );
}


void generate_gf()

/*
 * Generate field GF(2**m) from the irreducible polynomial p(X) with
 * coefficients in p[0]..p[m].
 *
 * Lookup tables:
 *   index->polynomial form: alpha_to[] contains j=alpha^i;
 *   polynomial form -> index form: index_of[j=alpha^i] = i
 *
 * alpha=2 is the primitive element of GF(2**m) 
 */
{
  register int    i, mask;

  mask = 1;
  alpha_to[m] = 0;
  for (i = 0; i < m; i++) {
    alpha_to[i] = mask;
    index_of[alpha_to[i]] = i;
    if (p[i] != 0)
      alpha_to[m] ^= mask;
    mask <<= 1;
  }
  index_of[alpha_to[m]] = m;
  mask >>= 1;
  for (i = m + 1; i < n; i++) {
    if (alpha_to[i - 1] >= mask)
      alpha_to[i] = alpha_to[m] ^ ((alpha_to[i - 1] ^ mask) << 1);
    else
      alpha_to[i] = alpha_to[i - 1] << 1;
    index_of[alpha_to[i]] = i;
  }
  index_of[0] = -1;
}


void gen_poly()

/*
 * Compute the generator polynomial of a binary BCH code. Fist generate the
 * cycle sets modulo 2**m - 1, cycle[][] =  (i, 2*i, 4*i, ..., 2^l*i). Then
 * determine those cycle sets that contain integers in the set of (d-1)
 * consecutive integers {1..(d-1)}. The generator polynomial is calculated
 * as the product of linear factors of the form (x+alpha^i), for every i in
 * the above cycle sets.
 */
{
  register int  ii, jj, ll, kaux;
  register int  test, aux, nocycles, root, noterms, rdncy;
  int             cycle[80][21], size[80], min[80], zeros[80];

  /* Generate cycle sets modulo n, n = 2**m - 1 */
  cycle[0][0] = 0;
  size[0] = 1;
  cycle[1][0] = 1;
  size[1] = 1;
  jj = 1;     /* cycle set index */
  if (m > 9)  {
    //printf("Computing cycle sets modulo %d\n", n);
    //printf("(This may take some time)...\n");
  }
  do {
    /* Generate the jj-th cycle set */
    ii = 0;
    do {
      ii++;
      cycle[jj][ii] = (cycle[jj][ii - 1] * 2) % n;
      size[jj]++;
      aux = (cycle[jj][ii] * 2) % n;
    } while (aux != cycle[jj][0]);
    /* Next cycle set representative */
    ll = 0;
    do {
      ll++;
      test = 0;
      for (ii = 1; ((ii <= jj) && (!test)); ii++) 
      /* Examine previous cycle sets */
        for (kaux = 0; ((kaux < size[ii]) && (!test)); kaux++)
           if (ll == cycle[ii][kaux])
              test = 1;
    } while ((test) && (ll < (n - 1)));
    if (!(test)) {
      jj++; /* next cycle set index */
      cycle[jj][0] = ll;
      size[jj] = 1;
    }
  } while (ll < (n - 1));
  nocycles = jj;    /* number of cycle sets modulo n */

  //printf("Enter the error correcting capability, t: ");
  //scanf("%d", &t);
  t=3;
  
  d = 2 * t + 1;

  /* Search for roots 1, 2, ..., d-1 in cycle sets */
  kaux = 0;
  rdncy = 0;
  for (ii = 1; ii <= nocycles; ii++) {
    min[kaux] = 0;
    test = 0;
    for (jj = 0; ((jj < size[ii]) && (!test)); jj++)
      for (root = 1; ((root < d) && (!test)); root++)
        if (root == cycle[ii][jj])  {
          test = 1;
          min[kaux] = ii;
        }
    if (min[kaux]) {
      rdncy += size[min[kaux]];
      kaux++;
    }
  }
  noterms = kaux;
  kaux = 1;
  for (ii = 0; ii < noterms; ii++)
    for (jj = 0; jj < size[min[ii]]; jj++) {
      zeros[kaux] = cycle[min[ii]][jj];
      kaux++;
    }

  k = length - rdncy;

    if (k<0)
      {
         //printf("Parameters invalid!\n");
         exit(0);
      }

  //printf("This is a (%d, %d, %d) binary BCH code\n", length, k, d);
  
  
  //Serial.println("This is a (%d, %d, %d) binary BCH code\n", length, k, d);

  /* Compute the generator polynomial */
  g[0] = alpha_to[zeros[1]];
  g[1] = 1;   /* g(x) = (X + zeros[1]) initially */
  for (ii = 2; ii <= rdncy; ii++) {
    g[ii] = 1;
    for (jj = ii - 1; jj > 0; jj--)
      if (g[jj] != 0)
        g[jj] = g[jj - 1] ^ alpha_to[(index_of[g[jj]] + zeros[ii]) % n];
      else
        g[jj] = g[jj - 1];
    g[0] = alpha_to[(index_of[g[0]] + zeros[ii]) % n];
  }
  printf("Generator polynomial:\ng(x) = ");
  for (ii = 0; ii <= rdncy; ii++) {
    printf("%d", g[ii]);
    if (ii && ((ii % 50) == 0))
      printf("\n");
  }
  printf("\n");
}


 
void encode_bch()
/*
 * Compute redundacy bb[], the coefficients of b(x). The redundancy
 * polynomial b(x) is the remainder after dividing x^(length-k)*data(x)
 * by the generator polynomial g(x).
 */
{
  register int    i, j;
  register int    feedback;

  for (i = 0; i < length - k; i++)
    bb[i] = 0;
  for (i = k - 1; i >= 0; i--) {
    feedback = data[i] ^ bb[length - k - 1];
    if (feedback != 0) {
      for (j = length - k - 1; j > 0; j--)
        if (g[j] != 0)
          bb[j] = bb[j - 1] ^ feedback;
        else
          bb[j] = bb[j - 1];
      bb[0] = g[0] && feedback;
    } else {
      for (j = length - k - 1; j > 0; j--)
        bb[j] = bb[j - 1];
      bb[0] = 0;
    }
  }
}

int count=1;

void loop() {
  // FSK modem can use the same transmit/receive methods
  // as the LoRa modem, even their interrupt-driven versions
  // NOTE: FSK modem maximum packet length is 63 bytes!

  int  i;

  read_p();               /* Read m */
  generate_gf();          /* Construct the Galois Field GF(2**m) */
  gen_poly();             /* Compute the generator polynomial of BCH code */

  /* Randomly generate DATA */
  /*seed = 131073;
  srandom(seed);
  for (i = 0; i < k; i++)
    data[i] = ( random() & 65536 ) >> 16;*/
    
  int data[]={1, 1, 1, 0, 1};
  encode_bch();           /* encode data */

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
  
  
  

    
  arrChar = (char *) malloc (sizeof(char) * (length+1));
  

  
  // converting integer array to char array
  for(i=0;i<length;i++)
    arrChar[i] = recd[i] + '0';
  
  
  
  //printf("\n");
  
  // adding '\0' at the end of character array
  arrChar[length] = '\0';
  //printf("String: %s", arrChar);
  
  //printf("\n");

 


  
  // transmit FSK packet
  
  String s = String(count);
  
  Serial.println(arrChar+s);
  int state = radio.transmit(arrChar);
  /*
    byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
                      0x89, 0xAB, 0xCD, 0xEF};
    int state = radio.transmit(byteArr, 8);
  */
  
  if (state == ERR_NONE) {
    Serial.print(F("[SX1278] Packet transmitted successfully! No. :"));
    Serial.println(s);
  } else if (state == ERR_PACKET_TOO_LONG) {
    Serial.println(F("[SX1278] Packet too long!"));
  } else if (state == ERR_TX_TIMEOUT) {
    Serial.println(F("[SX1278] Timed out while transmitting!"));
  } else {
    Serial.println(F("[SX1278] Failed to transmit packet, code "));
    Serial.println(state);
  }
  count=count+1;
  delay(1000);
}

  
