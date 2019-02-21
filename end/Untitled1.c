#include <iocc2530.h>
#include <stdio.h>
/*�궨��*****************************************/
#define uchar unsigned char
#define uint unsigned int
#define int16 short 
#define uint16 unsigned short 
      
/*��������*******************************/
//��ʼ��ʱ��
void initclock(void)
{
  CLKCONCMD &=0XBF;//ϵͳʱ��Դѡ��32MHZ XOSC
  CLKCONCMD &=0XF8;//ϵͳʱ��Ƶ��Ϊ32MHZ
  
  CLKCONCMD |=0X28;
  CLKCONCMD &=0XEF;//��ʱʱ��Ƶ��Ϊ1MHZ
  while(CLKCONSTA & 0X40);//�ȴ�ʱ���ȶ�
}
//��ʼ������0����
void inituart0(void)
{
  PERCFG &=0XFE;//��USART0��ALT 1
  P0SEL |=0X3C;//P0��2��3��4��5������
  P2DIR &=0X3F;//P0�������ȼ�USART0���
  U0CSR |=0X80;//��USART0 ������ʽΪ UART
  U0GCR |=9;
  U0BAUD |=59;//�貨����,������Ӧ��Ϊ19200�����Դ���ȴҪ��Ϊ9600����֣�
  UTX0IF=0;//UART0 TX�жϱ�־λ��0
  U0CSR |=0X40;//USART0 ����ʹ��
  IEN0 |=0X04;//usart0 �����ж�ʹ��
  IEN0 |=0X80;//���ж�ʹ��
}
//���ڷ����ַ�������
void uarttx_send_string(char *data,int len)
{
  int j;
  for(j=0;j<len;j++)
  {
    U0DBUF=*data++;
    while(UTX0IF==0);
    UTX0IF=0;
  }
  U0DBUF=0X0A;//����
  while(UTX0IF==0);
  UTX0IF=0;  
}
//��ʱ����
void delay(uint n)
{
  uint i;
  for(i=0;i<n;i++);
  for(i=0;i<n;i++);
  for(i=0;i<n;i++);
  for(i=0;i<n;i++);
  for(i=0;i<n;i++);
}
//��ȡ�¶ȴ�����ADֵ����
uint gettemperature(void)
{
  uint i;
  uint16 adcvalue;
  uint16 value;
  
  adcvalue=0;
  for(i=0;i<4;i++)
  {
    ADCCON3 |=0X0E;//��ͨ��ADת��ԴΪ�¶ȴ�����--���ֲ�
    ADCCON3 &=0X3F;//��ͨ��ADת���ο���ѹΪ1.25�ڲ���ѹ
    ADCCON3 |=0X30;//��ͨ��ADת���ֱ���Ϊ512DEC��12��Ч
    ADCCON1 |=0X30;//ADC������ʽѡ��ΪADCCON1.ST=1�¼�
    ADCCON1 |=0X40;//ADC����ת��
        
    while(!ADCCON1&0X80);//�ȴ�ADת�����
    value =  ADCL >> 2; //ADCL�Ĵ�����2λ��Ч
    value |=(((uint16)ADCH)<<6);//����ADCH��ADCL������ֵ��value
    adcvalue +=value;//adcvalue����ֵΪ4��ADֵ֮��
  }
  value=adcvalue>>2;//�ۼӳ���4���õ�ƽ��ֵ
  return ((value) >> 4) - 315;     //����ADֵ�������ʵ�ʵ��¶�
}   
   
/*��������͸�ֵ*******************************/
int16 avgtemp;/*������*******************************/
void main(void)
{
  char i;
  char tempvalue[10];
  
  initclock();//��ʼ��ʱ��  
  inituart0();//��ʼ������
  IEN0=IEN1=IEN2=0X00;//�ر������ж�
  ATEST=0X01;//�����¶ȴ�����
  TR0=0X01;//���¶ȴ�������ADC��������
  
  while(1)
  {
    avgtemp=0;
    for(i=0;i<64;i++)
    {
      avgtemp +=gettemperature();//ȡ���¶���������ԭƽ��ֵ
      avgtemp>>=1;//����2��ȡ��ƽ��ֵ
    }
    
    sprintf(tempvalue,(char *)"%dC/r",(int)avgtemp);
    uarttx_send_string(tempvalue,4);
    delay(50000);
  }
}
