//Sequencia semaforo e interrupção

sbit LCD_RS at RC3_bit;
sbit LCD_EN at RC7_bit;
sbit LCD_D0 at RD7_bit;
sbit LCD_D1 at RB1_bit;
sbit LCD_D2 at RB2_bit;
sbit LCD_D3 at RB3_bit;
sbit LCD_D4 at RB4_bit;
sbit LCD_D5 at RB5_bit;
sbit LCD_D6 at RB6_bit;
sbit LCD_D7 at RB7_bit;
sbit LCD_RS_Direction at TRISC3_bit;
sbit LCD_EN_Direction at TRISC7_bit;
sbit LCD_D0_Direction at TRISD7_bit;
sbit LCD_D1_Direction at TRISB1_bit;
sbit LCD_D2_Direction at TRISB2_bit;
sbit LCD_D3_Direction at TRISB3_bit;
sbit LCD_D4_Direction at TRISB4_bit;
sbit LCD_D5_Direction at TRISB5_bit;
sbit LCD_D6_Direction at TRISB6_bit;
sbit LCD_D7_Direction at TRISB7_bit;

int  count = 0, atualiza_carros = 0, uni = 9, dez = 1, estado = 1, ped = 0, carros = 0;
char text[] = "Controle", carros_txt[8];

void interrupt()
{
     if(INTF_bit)                                     //Ocorreu interrupção externa (sensor)
     {
          atualiza_carros = 1;
          carros++;
          INTF_bit = 0;
     }

     if(TMR0IF_bit)                                        //Ocorreu inerrupção do Timer
     {
          count++;
          TMR0 = 0;
          TMR0IF_bit = 0;
     }
} //end interrupt

void main() {
  char display[10] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111};
  int a = 0;
  CMCON = 7;
  //TRISA = 0b0010000;
  //PORTA = 0;                                            
  ADCON1 = 0b0111;                                        //Set as Digital I/O

  TRISA.B0 = 1;
  TRISA.B1 = 1;
  TRISA.B2 = 0;
  TRISB.B0 = 1;
  TRISC = 0;
  TRISD = 0b10000000;
  TRISE = 0;
  PORTA.B2 = 0;
  PORTB = 0;
  PORTC.B3 = 0;
  PORTC.B7 = 0;
  PORTD = 0b01111111;
  PORTE = 0;
                                                          //Configurações do reg INTCON:
  INTE_bit =  1;                                      //Habilita a interrupção externa
  INTEDG_bit = 0;                                     //Habilita a interrupção na borda de subida
  GIE_bit = 1;                                        //Habilita a interrupção global
  TMR0IE_bit = 1;                                     //Habilita a interrupção por estouro do TMR0

  OPTION_REG = 0b10000010;                            //Desabilita resistores internos de pull up e prescaler = 1:8

  TMR0 = 0;                                           //Condição inicial

  Lcd_Init();
  Lcd_Cmd(_LCD_CLEAR);
  Lcd_Cmd(_LCD_CURSOR_OFF);
  
  Lcd_Out(1, 1, text);
  Lcd_Out(2, 1, "Carros: ");
  IntToStr(carros, carros_txt);
  Lcd_Out(2, 8, carros_txt);

   while(1) {
        
        switch(estado)
        {
             case 1:                                  //vermelho 1 e verde 2
             {
                 PORTC = 0b01000100;
                 if(count > 5000) {
                    estado = 2;
                    count = 0;
                 }
                 break;
             }

             case 2:                                  //vermelho 1 e amarelo 2
             {
                 PORTC = 0b00010100;
                 if(count > 3000) {
                    if(ped == 1)
                        estado = 5;
                    else
                        estado = 3;
                    count = 0;
                 }
                 break;
             }

             case 3:                                  //verde 1 e vermelho 2
             {
                 PORTC = 0b00100001;
                 if(count > 5000) {
                    estado = 4;
                    count = 0;
                 }
                 break;
             }

             case 4:                                  //amarelo 1 e vermelho 2
             {
                  PORTC = 0b00100010;
                  if(count > 3000) {
                     if(ped == 1)
                         estado = 5;
                     else
                         estado = 1;
                     count = 0;
                  }
                  break;
             }
             
             case 5:                                            //pedestres
             {
                  PORTC = 0b00100100;                           //vermelho 1 e vermelho 2
                  //PORTE.B2 = 0;                                 //desliga LED

                  if(count >  20000) {                          //terminou a contagem
                       PORTD = 0b01111111;                      //desliga display
                       PORTE.B0 = 0;
                       PORTE.B1 = 0;
                       PORTA.B2 = 0;
                       estado = 1;
                       ped = 0;
                       count = 0;
                       a = 0;
                  }
                  else {
                      dez = (20000-count)/10000;
                      uni = ((20000-count)/1000)%10;
                      
                      PORTE.B0 = 1;     //habilita dezena
                      PORTE.B1 = 0;     //desabilita unidade
                      PORTD = ~display[dez];
                      delay_ms(50);
                            
                      PORTE.B0 = 0;     //desabilita dezena
                      PORTE.B1 = 1;     //habilita unidade
                      PORTD = ~display[uni];
                      delay_ms(50);
                      
                      if(a%4 == 0)        //buzzer
                           PORTA.B2 = ~PORTA.B2;
                      a++;
                  }
                   break;
             }
             default: break;
        } //end switch
        
        if((atualiza_carros == 1) && (PORTB.B0 == 1)){
            IntToStr(carros, carros_txt);
            Lcd_Out(2, 8, carros_txt);
            atualiza_carros = 0;
            if(estado == 5) {
                 PORTE.B2 = 1;
                 delay_ms(20);
                 PORTE.B2 = 0;
            }
        }
        
        if(PORTA.B0 == 0)                                       //habilita botão pedestre
             ped = 1;
             
        if(PORTA.B1 == 0)
        {
             carros = 0;
             atualiza_carros = 1;
        }
                                                    //zera contagem de carros
        
   } //end while(1)
} //end main
