//#include <avr/eeprom.h>

#define SIG 2       //pin, kjer je priklopljen signal DATA iz receiverja
#define RELE1v 0    //pin, kjer vklopimo Rele (set)
#define RELE1i 1    //pin, kjer izklopimo Rele (reset)
#define RELE2v 4    //pin, kjer vklopimo Rele (set)
#define RELE2i 5    //pin, kjer izklopimo Rele (reset)
#define RELE3v 6    //pin, kjer vklopimo Rele (set)
#define RELE3i 7    //pin, kjer izklopimo Rele (reset)

#define NICEL 35  //število ničel za začetni signal, ko je LOW dolg sedem milisekund 35
#define ENK 15  //število ničel za začetni signal, ko je LOW dolg sedem milisekund 15
#define POCAKAJ 150 //timeout za enakomerno semplanje signala v us
#define CIFER 300   /*število vzorcev, ki jih naredimo na signalu. En bit je dolg ca 1,5 ms. Semplamo na 120us. 
                    Bitov je 24. 24*1,5ms=36ms
                    36000us/120us=300 semplov
                    vzamemo 330, da je ziher dost
*/
#define DELAYRELE 500 //timeout za vklop releja v ms pred izklopom tuljave (ker je bistabilen)


unsigned int i=0; 
unsigned int k=0;
unsigned int zacetek=0;
unsigned int nicle=0;

unsigned char z[CIFER];
int zPrej=1;
int zZdaj;
unsigned int stZ0=0;
unsigned int stZ1=0;

unsigned int ukaz1=0x000;
unsigned int ukaz2=0x000;

int stanjeRele1=0; 
int stanjeRele2=0;
int stanjeRele3=0;

void setup()
{
  bitClear(DDRB, SIG);  //input

  bitSet(DDRB, RELE1v);  //output
  bitSet(DDRB, RELE1i);
  bitSet(DDRA, RELE2v);  
  bitSet(DDRA, RELE2i);
  bitSet(DDRA, RELE3v);  
  bitSet(DDRA, RELE3i);

  bitSet(DDRA, 0);
  
  bitClear(PORTB, RELE1v);  //damo vse izhode za rele na LOW
  bitClear(PORTB, RELE1i);
  bitClear(PORTA, RELE2v);
  bitClear(PORTA, RELE2i);
  bitClear(PORTA, RELE3v);
  bitClear(PORTA, RELE3i);

  

}


void loop()
{
  
  while(1){                             //iščemo začeten signal - na začetku je nič oz low dolg 7 ms
    zZdaj=bitRead(PINB, SIG);
    delayMicroseconds(POCAKAJ);
    if(zPrej==1 && zZdaj==1){
      stZ1++;
    }
    else if(stZ1>=ENK && zPrej==1 && zZdaj==0){
      stZ0=0;
    }
    else if(zPrej==0 && zZdaj==0) stZ0++;
    else if(stZ1>=ENK && stZ0>=NICEL && zPrej==0 && zZdaj==1) {
//          bitSet(PORTA, 0);
//          delay(DELAYRELE);
//          bitClear(PORTA, 0);
      break;
      }  //semplamo v nedogled, dokler ni dovolj ničel in hkrati zaznamo edge oz skok na 1
    else{
      stZ0=0;
      stZ1=0;// če ni nič od zgornjega, potem samo dajemo na 0, dokler se ne pojavi zanimiv vzorec
    }

    zPrej=zZdaj;
  }
  stZ0=0;
  stZ1=0;

  for(int i=0;i<CIFER;i++){ //semplamo signal - zaradi delay je št branj manjše
    //    z[i]=digitalRead(SIG);
    z[i]=bitRead(PINB, SIG);
    delayMicroseconds(POCAKAJ);

  }

i=0;

  

    while(z[i]!=1){
      i++; 

    }


    ukaz1=beri_ukaz(i);

    if(ukaz1==0xAAF || ukaz1==0xA3A || ukaz1==0xA8D || ukaz1==0xA44){
                                                            /*  1010 1010 1111 1111 1110 0101 to je gumb 2-ON 
                                                                1010 0011 1010 0110 0011 0101                 
                                                                1010 1000 1101 0001 1111 0101                 
                                                                1010 0100 0100 1101 0000 0101                 
                                                                0xAAFFE5 || 0xA3A635 || 0xA8D1F5 || 0xA44D05
                                                                */

      ukaz2=beri_ukaz(i);
      if(ukaz2==0xFE5 || ukaz2==0x635 || ukaz2==0x1F5 || ukaz2==0xD05){
        if(stanjeRele1==0){
          bitSet(PORTB, RELE1v);
          delay(DELAYRELE);
          bitClear(PORTB, RELE1v);
          stanjeRele1=0x1;
        }
        else if(stanjeRele1==1){
          bitSet(PORTB, RELE1i);
          delay(DELAYRELE);
          bitClear(PORTB, RELE1i);
          stanjeRele1=0x0;
        }
 
        ukaz2=0;
      }
      ukaz1=0;
    }
    else if (ukaz1==0xAF6 || ukaz1==0xAD8 || ukaz1==0xA9C || ukaz1==0xAC0){    
                                                            /*  1010 1111 0110 0010 1001 0101 to je gumb 2-OFF  
                                                                1010 1101 1000 0100 1100 0101                       
                                                                1010 1001 1100 1001 0100 0101                       
                                                                1010 1100 0000 0111 1101 0101                       
                                                                0xAF6295 || 0xAD84C5 || 0xA9C945 || 0xAC07D5
                                                                */  
      ukaz2=beri_ukaz(i);
      if (ukaz2==0x295 || ukaz2==0x4C5 || ukaz2==0x945 || ukaz2==0x7D5){

        if(stanjeRele2==0){
          bitSet(PORTA, RELE2v);
          delay(DELAYRELE);
          bitClear(PORTA, RELE2v);
          stanjeRele2=0x1;
        }
        else if(stanjeRele2==1){
          bitSet(PORTA, RELE2i);
          delay(DELAYRELE);
          bitClear(PORTA, RELE2i);
          stanjeRele2=0x0;
          
        }
        ukaz2=0;
      }
      ukaz1=0;

    }
    else if (ukaz1==0xAB9 || ukaz1==0xA53 || ukaz1==0xA17 || ukaz1==0xA75){    
                                                            /*  1010 1011 1001 0101 1011 1110 to je gumb 3-ON 
                                                                1010 0101 0011 0011 0010 1110                       
                                                                1010 0001 0111 1100 1000 1110                       
                                                                1010 0111 0101 1010 0111 1110                       
                                                                0xAB95BE || 0xA5332E || 0xA17C8E || 0xA75A7E
                                                                */  
      ukaz2=beri_ukaz(i);
      if (ukaz2==0x5BE || ukaz2==0x32E || ukaz2==0xC8E || ukaz2==0xA7E){

        if(stanjeRele3==0){
          bitSet(PORTA, RELE3v);
          delay(DELAYRELE);
          bitClear(PORTA, RELE3v);
          stanjeRele3=0x1;
        }
        else if(stanjeRele3==1){
          bitSet(PORTA, RELE3i);
          delay(DELAYRELE);
          bitClear(PORTA, RELE3i);
          stanjeRele3=0x0;
          
        }
        ukaz2=0;
      }
      ukaz1=0;

    }

  i=0;
}


int beri_ukaz(int zctk){
  int vst=0;    //vsota vseh enk v enem bitu
  int indx=zctk;
  int r1=0;

  unsigned long prim=1;

  unsigned long koda=0x000;
  unsigned long r=0x000;
    
  for(int k=0;k<12;k++){  //preberemo prvih 12 številk v tem primeru
    
    while(1){
           
      vst+=z[indx];
      indx++;
      if(z[indx]==1 && z[indx-1]==0) break;
      
    }
    r1=(indx-zctk)/2;


    if(vst>r1){
      koda|=1<<(11-k);  //zapišemo vsak bit v binarni zapis z OR funkcijo. Enko shiftamo do svojega mesta
    }
    
    vst=0;
    zctk=indx;
  }
  r=koda;
  i=indx;
  koda=0;

  return r;
}

