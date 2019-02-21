#include <iocc2530.h>
#include <stdio.h>
//#define RX
#define LED1 P1_0
#define LED2 P1_1
static  char buf[128];
static  int len=0;
unsigned char i; 

void Delay(unsigned int n)
{
  unsigned int i,tt;
  for(i=0;i<5;i++)
    for(tt=0;tt<n;tt++)
      ;
}
void rf_init()
{
  //Ӳ��CRC�Լ�AUTO_ACKʹ��
  FRMCTRL0 |= (0x20 | 0x40);

  //����TX������������Ի�ú��ʵĴ���
  TXFILTCFG = 0x09;
  //����AGCĿ��ֵ
  AGCCTRL1 = 0x15;
  //�����ѵ�EVM
  FSCAL1 = 0x00;
  // RXPKTDONE �ж�λʹ��
  RFIRQM0 |= (1<<6);
  //  RF �ж�ʹ��
  IEN2 |= (1<<0);
  //���ж�
  EA = 1;
  //�ŵ�ѡ��ѡ��11�ŵ�
  FREQCTRL = 0x0b; 
  //Ŀ���ַ�����ڼ�ʹ�õĶ̵�ַ
  SHORT_ADDR0 = 0x05;
  SHORT_ADDR1 = 0x00;
  //Ŀ���ַ�����ڼ�ʹ�õ�PANID
  PAN_ID0 = 0x22; 
  PAN_ID1 = 0x00;
  //���RXFIFO����������λ�����  
  RFST = 0xed; 
  //ΪRXʹ�ܲ�У׼Ƶ�ʺϳ���
  RFST = 0xe3; 
  //��ֹ֡����
  FRMFILT0 &= ~(1<<0);
}

void tx()
{
unsigned char i;
unsigned char tx[13]={'a','b','c','d','e','f','g','h','i','j','k','\r','\n'}; 
  //ΪRXʹ�ܲ�У׼Ƶ�ʺϳ���
  RFST = 0xe3;  
  // TX_ACTIVE | SFD 
  while (FSMSTAT1 & ((1<<1) | (1<<5))); 
  //��ֹRXPKTDONE�ж�
  RFIRQM0 &= ~(1<<6); 
  //��ֹRF�ж�
  IEN2 &= ~(1<<0); 
  // ���TXFIFO���� 
  RFST = 0xee; 
  // ��� TXDONE �ж� 
  RFIRQF1 = ~(1<<1);
  // ���͵ĵ�һ���ֽ��Ǵ����֡����  
  RFD = 13+2;//tx�а�����13���ַ�+2;��仺������������Ҫ����2�ֽڣ�CRCУ���Զ���� 
//��mac������д��RFD��
  for(i=0;i<13;i++)
  {
    RFD = tx[i];
  }
  // ��RX�ж� 
  RFIRQM0 |= (1<<6);
  //��RF�ж�
  IEN2 |= (1<<0);
  //У׼��ʹ��TX 
  RFST = 0xe9; 
  //�ȴ��������
  while (!(RFIRQF1 & (1<<1)));
  //��� TXDONE״̬  
  RFIRQF1 = ~(1<<1);
  //LED1��״̬�ı�
  LED1=~LED1;
  //��ʱ
  Delay(20000);
}
void initLed(void)
{
  //P1Ϊ��ͨ I/O ��
  P1SEL  &= ~(1<<0); 
  //P1.0  P1.1����Ϊ���
  P1DIR |= 0x03;             
  //�ر�LED1
  LED1=1;
  //�ر�LED2
  LED2=1;  
}
/****************************************************************
���ڳ�ʼ������
****************************************************************/
void initUARTSEND(void)
{
    PERCFG = 0x00;        //λ�� 1 P0 ��
    P0SEL = 0x3c;        //P0_2,P0_3,P0_4,P0_5��������
    P2DIR &= ~0xc0;        //P0 ������ΪUART0
    U0CSR |= 0x80;        //ѡ�� UART ģʽ
    U0GCR |= 11;                // ����� U0GCR �� U0BAUD
    U0BAUD = 216;               // 115200
    UTX0IF = 0;        //UART0 TX �жϱ�־��ʼ��λ 0
}

void initClock(void)
{
  /*��ؼĴ��������� cc2530�����ֲ��в���*/
  CLKCONCMD &= ~0x40;                           //����ϵͳʱ��ԴΪ 32MHZ����
  while(CLKCONSTA & 0x40);                      //�ȴ������ȶ�
  CLKCONCMD  &=  ~0x47;      //����ϵͳ��ʱ��Ƶ��Ϊ 32MHZ  X0XXX000
  
  SLEEPCMD |= 0x04; 
}

/****************************************************************
���ڷ����ַ�������
****************************************************************/
void UartTX_Send_String(char *Data,int len)
{
    int j;
    for(j=0; j<len; j++)
    {
        U0DBUF = *Data++;
        while(UTX0IF == 0);
        UTX0IF = 0;
    }
}
void main(void)
{  
  //�ر����ж�
  EA = 0;
  initLed();
  initClock(); 
  initUARTSEND();
  //��ʼ��RF
  rf_init();
  //�ж�ʹ��
  EA = 1; 
  //���ͻ�ȴ������ж�
  while(1) 
  {
    //�궨��RX
#ifndef RX
    //���û�ж���RX����ʼ����
    tx();
    //��ʱ
    Delay(20000);
    Delay(20000);
    //�������RX���ȴ������ж�
# else		
#endif			
  }  
}

//�����жϴ���
#pragma vector=RF_VECTOR
__interrupt void rf_isr(void) 
{
  unsigned char  i; 
  int rssi=0;
  char crc_ok=0;
  EA=0;
  //���ж�
  IEN2 &= ~0X01; 
  //����֡����
  if (RFIRQF0 & (1<<6)) 
  {
    //����֡����
    len = RFD ;    
    //printf("\nlen = %d\n***********\n",len);
    len &= 0x7f;
    //�����յ�����д��buf��
    for (i = 0; i < len - 2; i++) 
    {
      buf[i] = RFD;
      Delay(200);
      //�򴮿ڷ��ͽ��յ�������
      //UartTX_Send_String(&buf[i],1);
    }
    rssi = RFD - 73;  //��ȡRSSI���  
    crc_ok = RFD;
    printf("[%d],crcResult=%d\n",rssi,(crc_ok&0x80));////crc_ok&0x80��ȡCRCУ���� BIT7      
    //�򴮿ڷ��ͽ��յ�������
    UartTX_Send_String(buf,len-2);
    RFST = 0xED;
    // ��RF�ж�
    S1CON = 0; 
    //�� RXPKTDONE�ж�  
    RFIRQF0 &= ~(1<<6); 
    //LED1��״̬�ı�
    LED1 = ~LED1;
  }
  IEN2 |= (1<<0);
  EA=1;
}

