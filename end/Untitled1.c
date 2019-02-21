#include <iocc2530.h>
#include <stdio.h>
/*宏定义*****************************************/
#define uchar unsigned char
#define uint unsigned int
#define int16 short 
#define uint16 unsigned short 
      
/*函数定义*******************************/
//初始化时钟
void initclock(void)
{
  CLKCONCMD &=0XBF;//系统时钟源选择32MHZ XOSC
  CLKCONCMD &=0XF8;//系统时钟频率为32MHZ
  
  CLKCONCMD |=0X28;
  CLKCONCMD &=0XEF;//定时时钟频率为1MHZ
  while(CLKCONSTA & 0X40);//等待时钟稳定
}
//初始化串口0函数
void inituart0(void)
{
  PERCFG &=0XFE;//设USART0的ALT 1
  P0SEL |=0X3C;//P0口2、3、4、5做外设
  P2DIR &=0X3F;//P0外设优先级USART0最高
  U0CSR |=0X80;//设USART0 工作方式为 UART
  U0GCR |=9;
  U0BAUD |=59;//设波特率,理论上应该为19200，调试串口却要设为9600，奇怪？
  UTX0IF=0;//UART0 TX中断标志位清0
  U0CSR |=0X40;//USART0 接收使能
  IEN0 |=0X04;//usart0 接收中断使能
  IEN0 |=0X80;//总中断使能
}
//串口发送字符串函数
void uarttx_send_string(char *data,int len)
{
  int j;
  for(j=0;j<len;j++)
  {
    U0DBUF=*data++;
    while(UTX0IF==0);
    UTX0IF=0;
  }
  U0DBUF=0X0A;//换行
  while(UTX0IF==0);
  UTX0IF=0;  
}
//延时函数
void delay(uint n)
{
  uint i;
  for(i=0;i<n;i++);
  for(i=0;i<n;i++);
  for(i=0;i<n;i++);
  for(i=0;i<n;i++);
  for(i=0;i<n;i++);
}
//读取温度传感器AD值函数
uint gettemperature(void)
{
  uint i;
  uint16 adcvalue;
  uint16 value;
  
  adcvalue=0;
  for(i=0;i<4;i++)
  {
    ADCCON3 |=0X0E;//单通道AD转换源为温度传感器--看手册
    ADCCON3 &=0X3F;//单通道AD转换参考电压为1.25内部电压
    ADCCON3 |=0X30;//单通道AD转换分辨率为512DEC，12有效
    ADCCON1 |=0X30;//ADC启动方式选择为ADCCON1.ST=1事件
    ADCCON1 |=0X40;//ADC启动转换
        
    while(!ADCCON1&0X80);//等待AD转换完成
    value =  ADCL >> 2; //ADCL寄存器低2位无效
    value |=(((uint16)ADCH)<<6);//连接ADCH和ADCL，并赋值给value
    adcvalue +=value;//adcvalue被赋值为4次AD值之和
  }
  value=adcvalue>>2;//累加除以4，得到平均值
  return ((value) >> 4) - 315;     //根据AD值，计算出实际的温度
}   
   
/*变量定义和赋值*******************************/
int16 avgtemp;/*主函数*******************************/
void main(void)
{
  char i;
  char tempvalue[10];
  
  initclock();//初始化时钟  
  inituart0();//初始化串口
  IEN0=IEN1=IEN2=0X00;//关闭所有中断
  ATEST=0X01;//开启温度传感器
  TR0=0X01;//将温度传感器与ADC连接起来
  
  while(1)
  {
    avgtemp=0;
    for(i=0;i<64;i++)
    {
      avgtemp +=gettemperature();//取得温度数，加上原平均值
      avgtemp>>=1;//除以2，取得平均值
    }
    
    sprintf(tempvalue,(char *)"%dC/r",(int)avgtemp);
    uarttx_send_string(tempvalue,4);
    delay(50000);
  }
}
