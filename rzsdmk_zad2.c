#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

//Velicina prijemnog bafera (mora biti 2^n)
#define USART_RX_BUFFER_SIZE 64

char Rx_Buffer[USART_RX_BUFFER_SIZE];			//prijemni FIFO bafer
volatile unsigned char Rx_Buffer_Size = 0;	//broj karaktera u prijemnom baferu
volatile unsigned char Rx_Buffer_First = 0;
volatile unsigned char Rx_Buffer_Last = 0;

ISR(USART_RX_vect)
{
  	Rx_Buffer[Rx_Buffer_Last++] = UDR0;		//ucitavanje primljenog karaktera
	Rx_Buffer_Last &= USART_RX_BUFFER_SIZE - 1;	//povratak na pocetak u slucaju prekoracenja
	if (Rx_Buffer_Size < USART_RX_BUFFER_SIZE)
		Rx_Buffer_Size++;					//inkrement brojaca primljenih karaktera
}

void usartInit(unsigned long baud)
{
	UCSR0A = 0x00;	//inicijalizacija indikatora
					//U2Xn = 0: onemogucena dvostruka brzina
					//MPCMn = 0: onemogucen multiprocesorski rezim

	UCSR0B = 0x98;	//RXCIEn = 1: dozvola prekida izavanog okoncanjem prijema
					//RXENn = 1: dozvola prijema
					//TXENn = 1: dozvola slanja

	UCSR0C = 0x06;	//UMSELn[1:0] = 00: asinroni rezim
					//UPMn[1:0] = 00: bit pariteta se ne koristi
					//USBSn = 0: koristi se jedan stop bit
					//UCSzn[2:0] = 011: 8bitni prenos

	UBRR0 = F_CPU / (16 * baud) - 1;

	sei();	//I = 1 (dozvola prekida)
}

unsigned char usartAvailable()
{
	return Rx_Buffer_Size;		//ocitavanje broja karaktera u prijemnom baferu
}

void usartPutChar(char c)
{
	while(!(UCSR0A & 0x20));	//ceka da se setuje UDREn (indikacija da je predajni bafer prazan)
	UDR0 = c;					//upis karaktera u predajni bafer
}

void usartPutString(char *s)
{
	while(*s != 0)				//petlja se izvrsava do nailaska na nul-terminator
	{
		usartPutChar(*s);		//slanje tekuceg karaktera
		s++;					//azuriranje pokazivaca na tekuci karakter
	}
}

void usartPutString_P(const char *s)
{
	while (1)
	{
		char c = pgm_read_byte(s++);	//citanje sledeceg karaktera iz programske memorije
		if (c == '\0')					//izlazak iz petlje u slucaju
			return;						//nailaska na terminator
		usartPutChar(c);				//slanje karaktera
	}
}

char usartGetChar()
{
	char c;

	if (!Rx_Buffer_Size)						//bafer je prazan?
		return -1;
  	c = Rx_Buffer[Rx_Buffer_First++];			//citanje karaktera iz prijemnog bafera
	Rx_Buffer_First &= USART_RX_BUFFER_SIZE - 1;	//povratak na pocetak u slucaju prekoracenja
	Rx_Buffer_Size--;							//dekrement brojaca karaktera u prijemnom baferu

	return c;
}

unsigned char usartGetString(char *s)
{
	unsigned char len = 0;

	while(Rx_Buffer_Size) 			//ima karaktera u faferu?
		s[len++] = usartGetChar();	//ucitavanje novog karaktera

	s[len] = 0;						//terminacija stringa
	return len;						//vraca broj ocitanih karaktera
}

char korisnici[10][32]{
  "korisnik 1",
  "korisnik 2",
  "Marko Markovic",
  "Bojan Dangubic",
  "Mikelandjelo",
  "Ana Karenjina",
  "Lav Tolstoj",
  "Stefan Nemanjic",
  "Boda Tajson",
  "Stevan Sremac" 
};

char PIN[10][32]{
  "1111",
  "2222",
  "3333",
  "4444",
  "5555",
  "6666",
  "7777",
  "8888",
  "9999",
  "1234"
};

int main()
{ 
	usartInit(9600);
  while(1){
      	char ime[32] ;
  		int ok=0;
  		char *pinu ="1234";
    	usartPutString("Unesi ime i prezime:\r\n");
  		while(!usartAvailable());
      	_delay_ms(60);
  		usartGetString(ime);
 		int i=0;
        while(i<10){
          if(!strcmp(ime, korisnici[i])){
            usartPutString("Odgovor baze: Dobrodosli ");
            usartPutString(ime);
            usartPutString("\r\n");
            ok=1;
            break;
          }
          i++;
        }
  		if(ok){
            usartPutString("Unesite pin kod cifru po cifru\r\n");

            while(!usartAvailable());
            _delay_ms(60);
            pinu[0]=usartGetChar();
            usartPutString("*");

            while(!usartAvailable());
            _delay_ms(60);
            pinu[1]=usartGetChar();
            usartPutString("*");

            while(!usartAvailable());
            _delay_ms(60);
            pinu[2]=usartGetChar();
            usartPutString("*");

            while(!usartAvailable());
            _delay_ms(60);
            pinu[3]=usartGetChar();
            usartPutString("*\r\n");
        }
  		if(!strcmp(pinu, PIN[i])){
        	usartPutString("Pin: OK\r\n");
        }
        else{
			usartPutString("Pin: Fail\r\n");
        }

  		
  	
	
	}

	return 0;
}
