//�����¶�����
#include <ioCC2530.h>

#define uint unsigned int
//�������LED�ƵĶ˿�
#define LED1 P1_0	//����LED1ΪP10�ڿ���
#define LED2 P1_1	//����LED2ΪP11�ڿ���



float avgtemp;
char tempvalue[10]; 

/****************************
//��ʱ����
*****************************/
void Delay(uint n)
{
    uint i,t;
    for(i = 0;i<5;i++)
    for(t = 0;t<n;t++);
}
//��ʼ��ʱ��
void initClock(void)
{
  /*��ؼĴ��������� cc2530�����ֲ��в���*/
  CLKCONCMD &= ~0x40;                           //����ϵͳʱ��ԴΪ 32MHZ����
  while(CLKCONSTA & 0x40);                      //�ȴ������ȶ�
  CLKCONCMD  &=  ~0x47;      //����ϵͳ��ʱ��Ƶ��Ϊ 32MHZ  X0XXX000
  
  SLEEPCMD |= 0x04; 
}
//��ʼ��Led
void InitLed(void)
{
    P1DIR |= 0x03;   //P1_0��P1_1����Ϊ���
    LED1 = 1;       //LED1��Ϩ��
    LED2 = 1;	    //LED2��Ϩ��
}
/****************************************************************
���ڷ����ַ�������			
****************************************************************/
void UartTX_Send_String(char *Data,int len)
{
  int j;
  for(j=0;j<len;j++)
  { 
    U0DBUF = *Data++;
    while(UTX0IF == 0) //UTX0IF����1�������жϣ�����һֱѭ��
      ;
    UTX0IF = 0;
  }
}
void InitUart()
{
    CLKCONCMD &= ~0x40;//����ϵͳʱ��ԴΪ32MHZ����
    while(CLKCONSTA & 0x40);//�ȴ������ȶ�
    CLKCONCMD &= ~0x47;//����ϵͳ��ʱ��Ƶ��Ϊ32MHZ

    PERCFG = 0x00;    //λ��1 P0��
    P0SEL = 0x3c;    //P0��������
    P2DIR &= ~0XC0;//P0��������

    U0CSR |= 0x80;    //UART��ʽ
    U0GCR |= 11;    //baud_e = 11;
    U0BAUD |= 216;    //��������Ϊ115200
    UTX0IF = 1;

    
    IEN0 |= 0x80;//�����ж�
    IEN2 |= 0x04;//�����ж�ʹ��
    IEN0 |= 0x04;//�������ж�ʹ��
    URX0IE = 1;
    U0CSR |= 0X40;    //�������
}
//����Ƭ���¶ȴ������¶�
float getTemperature(void)
{
  uint value;
  ADCCON3 = 0x3e; //ѡ��1.25VΪ�ο���ѹ��14λ�ֱ��ʣ���Ƭ���¶ȴ���������
  ADCCON1 |= 0x30;//ADC������ʽѡ��ΪADCCON1.ST=1�¼�
  ADCCON1 |= 0x40;//ADC����ת��
  
  while(!(ADCCON1 & 0x80))//�ȴ�ADת�����
    ;
  value = ADCL >>4;/*ADCL�Ĵ�����4λ��Ч��ADC���ֻ�ܴﵽ12λ��Чλ�������Ϻܶ�������ﶼ��������λ�����ǲ��Ե�*/
  value |= (((uint)ADCH)<<4);//����ADCH��ADCL������ֵ��value
  return (value - 1367.5)/4.5;
  //return ((value) >> 4) - 315;     //����ADֵ�������ʵ�ʵ��¶�
  //return value*0.06229-311.43;
  //return (value-1367.5)/4.5-4;         //��������������֪�����Ǹ� ����ָ��һ��
  //return value*0.06229-303.3-4;/*�¶ȵļ��㹫ʽΪ���¶�=����������ѹ-ĳһ��ѹ��/�¶�ϵ����-�¶ȵ����ֵ*/
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

void tx(char *tx,int len)
{
  unsigned char i; 
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
  RFD = len+2;//tx�а�����13���ַ�+2;��仺������������Ҫ����2�ֽڣ�CRCУ���Զ���� 
//��mac������д��RFD��
  for(i=0;i<len;i++)
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
 
  //��ʱ
  Delay(20000);
}

 
void main(void)
{  
  initClock();
  InitUart();
  InitLed();
  
  ATEST = 0x01;//�����¶ȴ�����
  TR0 = 0X01;  //���¶ȴ�������ADC��������
  while(1)
  {
    avgtemp = getTemperature();//ȡ���¶��� 
    tempvalue[0] = (unsigned char)(avgtemp)/10 + 48;          //ʮλ
    tempvalue[1] = (unsigned char)(avgtemp)%10 + 48;          //��λ

    
    UartTX_Send_String(tempvalue,2);
    tx(tempvalue,2);
    Delay(50000);                      //��ʱ
    if((tempvalue[0]*10+tempvalue[1])>24)
    {
      LED1=0;                        //��־����״̬
      Delay(50000);
      Delay(50000);
    }
    else
      LED1=1;
  }
}