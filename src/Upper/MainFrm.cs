using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO.Ports;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace UpperDemo
{
    public partial class MainFrm : Form
    {
        // 当前连接套接字
        private Socket currentSocket;
        // 当前串口
        private SerialPort currentSerial;
        // socket是否连接
        private bool isConnect = false;
        // 串口是否打开
        private bool isOpen = false;

        public MainFrm()
        {
            InitializeComponent();
            // 禁用开关风扇按钮
            btnControl.Enabled = false;
            // 取消跨线程检查
            Control.CheckForIllegalCrossThreadCalls = false;
        }

        // 连接socket服务器
        private void btnConnect_Click(object sender, EventArgs e)
        {
            txtData.AppendText("连接服务器...\r\n");
            try
            {
                // 创建一个负责通信的Socket
                currentSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                // 获得要连接的远程应用程序IP地址和端口号
                IPAddress ip = IPAddress.Parse(txtIp.Text);
                IPEndPoint point = new IPEndPoint(ip, Int32.Parse(txtPort.Text));
                // 连接
                currentSocket.Connect(point);
                txtData.AppendText("连接服务器成功！\r\n");
                isConnect = true;
                if (isConnect || isOpen)
                {
                    btnControl.Enabled = true;
                }
                // 开启一个新线程
                // 用于接收服务器发送过来的消息
                Thread th = new Thread(Recive);
                th.IsBackground = true;
                th.Start();

                btnConnect.Enabled = false;
            }
            catch (Exception)
            {
                //
                txtData.AppendText("连接服务器失败！\r\n");
            }
        }

        // 打开串口
        private void btnOpen_Click(object sender, EventArgs e)
        {
            txtData.AppendText("打开串口...\r\n");
            try
            {
                currentSerial = new SerialPort();
                // 波特率
                currentSerial.BaudRate = Int32.Parse(txtBaudRate.Text);
                // 串口名
                currentSerial.PortName = txtCom.Text;
                // 数据位
                currentSerial.DataBits = 8;
                // 打开
                currentSerial.Open();

                txtData.AppendText("打开串口成功!\r\n");
                isOpen = true;
                if (isOpen || isConnect)
                {
                    btnControl.Enabled = true;
                }

                // 两个字节触发接收事件
                currentSerial.ReceivedBytesThreshold = 2;
                // 订阅读取事件
                currentSerial.DataReceived += new SerialDataReceivedEventHandler(readSerial);
                txtData.AppendText("监听串口成功...\r\n");

                btnOpen.Enabled = false;
            }
            catch (Exception)
            {
                //
                txtData.AppendText("打开串口失败!\r\n");
            }
        }

        // 开关风扇
        private void btnControl_Click(object sender, EventArgs e)
        {
            if (btnControl.Text == "开风扇")
            {
                txtData.AppendText("PC开风扇...\r\n向串口发送开风扇指令...\r\n向服务器发送风扇状态...\r\n");
                // 控制
                FanOffOn("1", "01");

                btnControl.Text = "关风扇";
            }
            else
            {
                txtData.AppendText("PC关风扇...\r\n向串口发送关风扇指令...\r\n向服务器发送风扇状态...\r\n");
                // 控制
                FanOffOn("0", "00");

                btnControl.Text = "开风扇";
            }
        }

        /// <summary>
        /// 开关风扇
        /// </summary>
        /// <param name="serialCommand">发往串口的风扇控制命令</param>
        /// <param name="socketState">发往服务器的风扇状态</param>
        private void FanOffOn(string serialCommand, string socketState)
        {
            // 向串口发送指令
            if (currentSerial != null && currentSerial.IsOpen)
            {
                currentSerial.Write(serialCommand);
                txtData.AppendText("发送开风扇指令成功!\r\n");
            }
            else
            {
                txtData.AppendText("发送开风扇指令失败!串口未打开.\r\n");
            }
            // 向服务器发送风扇状态
            if (currentSocket != null)
            {
                currentSocket.Send(Encoding.ASCII.GetBytes(socketState));
                txtData.AppendText("发送风扇状态成功!\r\n");
            }
            else
            {
                txtData.AppendText("发送风扇状态失败!服务器未连接.\r\n");
            }
        }

        // 循环接受服务器的消息
        private void Recive()
        {
            txtData.AppendText("监听服务器成功...\r\n");
            while (true)
            {
                try
                {
                    byte[] buffer = new byte[1];
                    int res = currentSocket.Receive(buffer);
                    if (res == 0)
                    {
                        txtData.AppendText("服务器断开连接!请重新连接!!!\r\n");
                        btnConnect.Text = "重新连接";
                        currentSocket.Shutdown(SocketShutdown.Both);
                        currentSocket.Close();
                        currentSocket = null;
                        btnConnect.Enabled = true;
                        break;
                    }
                    if (res != 1)
                    {
                        txtData.AppendText("收到服务器错误数据!\r\n");
                        break;
                    }

                    txtData.AppendText("收到服务器命令...\r\n");
                    if (buffer[0] == 1)
                    {
                        txtData.AppendText("服务器开风扇...\r\n");
                        btnControl.Text = "关风扇";
                        // 开风扇
                        txtData.AppendText("将服务器命令转发到串口...\r\n");
                        if (currentSerial != null && currentSerial.IsOpen)
                        {
                            currentSerial.Write("1");
                            txtData.AppendText("转发成功!\r\n");
                        }
                        else
                        {
                            txtData.AppendText("处理失败!串口未打开.\r\n");
                        }
                    }
                    else if (buffer[0] == 0)
                    {
                        txtData.AppendText("服务器关风扇...\r\n");
                        btnControl.Text = "开风扇";
                        // 关风扇
                        txtData.AppendText("将服务器命令转发到串口...\r\n");
                        if (currentSerial != null && currentSerial.IsOpen)
                        {
                            currentSerial.Write("0");
                            txtData.AppendText("转发成功!\r\n");
                        }
                        else
                        {
                            txtData.AppendText("处理失败!串口未打开.\r\n");
                        }
                    }
                    else
                    {
                        txtData.AppendText("错误命令：" + buffer[0] + "\r\n");
                    }
                }
                catch (Exception)
                {
                    //
                }
            }
        }

        // 循环接受串口的数据
        private void readSerial(object sender, SerialDataReceivedEventArgs e)
        {
            if (currentSerial != null && currentSerial.IsOpen)
            {
                byte[] readbuffer = new byte[currentSerial.BytesToRead];
                try
                {
                    // 接收温度数据
                    int r = currentSerial.Read(readbuffer, 0, readbuffer.Length);
                    if (r != 2)
                        return;
                    currentSerial.DiscardInBuffer();
                    labTemp.Text = Encoding.ASCII.GetString(readbuffer);
                    txtData.AppendText("收到串口温度数据：" + labTemp.Text + "\r\n");
                    // 发送温度数据到服务器
                    txtData.AppendText("将温度数据转发到服务器...\r\n");
                    if (currentSocket != null)
                    {
                        currentSocket.Send(Encoding.ASCII.GetBytes("1" + labTemp.Text));
                        txtData.AppendText("温度数据转发成功!\r\n");
                    }
                    else
                    {
                        txtData.AppendText("处理失败!服务器未连接.\r\n");
                    }
                    //TODO:自动控制温度
                }
                catch (TimeoutException)
                {
                    //
                    txtData.AppendText("接收串口数据失败!\r\n");
                }
            }
            else
            {
                Thread.Sleep(100);
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            // 窗口关闭时关闭连接
            if (currentSocket != null)
            {
                currentSocket.Shutdown(SocketShutdown.Both);
                currentSocket.Close();
            }
        }
    }
}
