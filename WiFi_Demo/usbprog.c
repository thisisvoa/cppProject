/****************************************************************************
*
*文件(File):         usbprog.c
*
*修改(Modify):       2011/8/9 11:37:56
*
*作者(Author):       USER
*
*编绎(Compile):      智能平台(Smart Platform)
*
*描述(Description):
*
*
*
----------------------------------------------------------------------------
|
| Version | Datetime             |   Author    | Description
| --------+----------------------+-------------+----------------------------
|
| V1.00  2011/8/9 11:37:56           USER 
----------------------------------------------------------------------------
****************************************************************************/

#include <API.h>
#include <usb.h>
//#include "D12ci.h"
//#include "db.h"
#include "usbprog.h"
#include "extra.h"
#include "d12.h"
#include "in_call.h"
//#include "Common.h"

//表结构的大小
extern unsigned short g_rec_size[NUMOFDBTABLE];
//表分配的块数据
extern char g_blocknum[NUMOFDBTABLE];	
//表的名称
extern unsigned char *g_strTableName[NUMOFDBTABLE];

struct TableInfo
{
	//存在多少条记录
	unsigned int TotalExist[NUMOFDBTABLE];
	//有效的记录数
	unsigned int TotalValid[NUMOFDBTABLE];
	//表记录的长度
	unsigned int RecordSize[NUMOFDBTABLE];	
	//表分配的块数据
	unsigned int RecordBlackCount[NUMOFDBTABLE];
};

//函数功能：
//	USB接口数据传输
//函数输入：
//	void
//函数返回：
//	0 成功
//	1 F1退出
//	2 没有USB接口
//	3 没有联接上PC
//	4 未知错误
//	5 记录的长度大小128
// 	6 数据库不能存在当前的记录
//	7 添加记录失败
int Usb_Data_Transmit(void)
{
	int fsid = 0;	
	char flag = 0;	
	int IntRet = 0;
	int RetValue = 0;
	int pos_type = 0;
	char CharTempA[20];	
	unsigned int DataLen = 0;	
	unsigned int RecordCount = 0;
	unsigned int RecordCountA = 0;
	struct TableInfo TableInfoObj;	
	unsigned char cmd_op = 0;
	unsigned char cmd_type = 0;
	USER_INFO *pTAB1_STRUCTObj = NULL;	
	unsigned char CharTemp[MAX_NUMBER_DT_DATA + 1];
	int g_MachineNo = 10;
	//char end_flag = 0;		//chen
	
	memset(&TableInfoObj, '\0', sizeof(struct TableInfo));

	pos_type = Sys_Get_POS_Type();
	if(pos_type == POS_ARM7_0528)
	{
		DispStr_CE(0,3,"没有USB接口",DISP_CENTER|DISP_CLRSCR);
		return(2);
	}

	if(USB_Init())
	{
		RetValue = 8;
		goto EndFlag;
		//return(8);
	}
	
	DispStr_CE(0, 2, "计算数据条数...", DISP_CENTER | DISP_CLRSCR);
	for(fsid = 0; fsid < NUMOFDBTABLE; fsid++)
	{
		count_db_toal(fsid,&TableInfoObj.TotalValid[fsid], &TableInfoObj.TotalExist[fsid]);	  
		TableInfoObj.RecordSize[fsid] = g_rec_size[fsid];
		TableInfoObj.RecordBlackCount[fsid] = g_blocknum[fsid];
	}

	DispStr_CE(0, 2, "USB数据传输...",DISP_CENTER|DISP_CLRSCR);
	while(1)
	{
		if(KEY_Read() == KEY_F1)	
		{
			RetValue = 1;
			goto EndFlag;
			//return(1);
		}		
		memset(CharTemp, '\0', sizeof(CharTemp));
		IntRet = USBDownLongString(CharTemp, &DataLen);
		if(IntRet == 0)
		{
			cmd_op = CharTemp[0];
			cmd_type = CharTemp[1];

			//询问手持设备与PC的联接状态
			if((cmd_op == USB_CMD_REQ) && (cmd_type == USB_CMD_DISCONNECT))
			{				
				memset(CharTemp, '\0', sizeof(CharTemp));
				CharTemp[0] = USB_CMD_RTC;
				CharTemp[1] = USB_CMD_DISCONNECT;
				IntRet = USBUpLongString(CharTemp, 2);
				if(IntRet == -1)
				{
					RetValue = 1;
					goto EndFlag;
					//return(1);
				}
				else if(IntRet == -2)
				{
					RetValue = 3;
					goto EndFlag;				
					//return(3);
				}
//				DispStr_CE(0, 2, "USB数据传输...",DISP_CENTER|DISP_CLRSCR);
			}					
			//返回当前表的信息
			else if((cmd_op == USB_CMD_REQ) && (cmd_type == USB_CMD_TABLE_INFO))
			{
				DispStr_CE(0, 2, "发送表信息C...", DISP_CENTER|DISP_CLRSCR);
				
				//表的序号
				fsid = CharTemp[2];
				memset(CharTemp, '\0', sizeof(CharTemp));
				CharTemp[0] = USB_CMD_RTC;
				CharTemp[1] = USB_CMD_TABLE_INFO;
				
				//表中存在的记录数据
				CharTemp[2] = (unsigned char)(TableInfoObj.TotalExist[fsid] >> 24);
				CharTemp[3] = (unsigned char)(TableInfoObj.TotalExist[fsid] >> 16);
				CharTemp[4] = (unsigned char)(TableInfoObj.TotalExist[fsid] >> 8);
				CharTemp[5] = (unsigned char)TableInfoObj.TotalExist[fsid];			

				//表中有效的记录数
				CharTemp[6] = (unsigned char)(TableInfoObj.TotalValid[fsid] >> 24);
				CharTemp[7] = (unsigned char)(TableInfoObj.TotalValid[fsid] >> 16);
				CharTemp[8] = (unsigned char)(TableInfoObj.TotalValid[fsid] >> 8);
				CharTemp[9] = (unsigned char)TableInfoObj.TotalValid[fsid];			

				//表记录的长度
				CharTemp[10] = (unsigned char)(TableInfoObj.RecordSize[fsid] >> 24);
				CharTemp[11] = (unsigned char)(TableInfoObj.RecordSize[fsid] >> 16);
				CharTemp[12] = (unsigned char)(TableInfoObj.RecordSize[fsid] >> 8);
				CharTemp[13] = (unsigned char)TableInfoObj.RecordSize[fsid];			

				//表分配的块数据
				CharTemp[14] = (unsigned char)(TableInfoObj.RecordBlackCount[fsid] >> 24);
				CharTemp[15] = (unsigned char)(TableInfoObj.RecordBlackCount[fsid] >> 16);
				CharTemp[16] = (unsigned char)(TableInfoObj.RecordBlackCount[fsid] >> 8);
				CharTemp[17] = (unsigned char)TableInfoObj.RecordBlackCount[fsid];			
				
				IntRet = USBUpLongString(CharTemp, 18);
				if(IntRet == -1)
				{
					RetValue = 1;
					goto EndFlag;				
					//return(1);
				}
				else if(IntRet == -2)
				{
					RetValue = 3;
					goto EndFlag;				
					//return(3);
				}					
				
//				DispStr_CE(0, 2, "USB数据传输...",DISP_CENTER|DISP_CLRSCR);
				
			}
			//能过USB把手持设备上的信息传到PC上
			else  if((cmd_op == USB_CMD_REQ) && (cmd_type == USB_CMD_SEND_DATA))
			{
				
				DispStr_CE(0, 2, "发送表信息A...",DISP_CENTER|DISP_CLRSCR);
				
				//表的序号
				fsid = CharTemp[3];
				if(TableInfoObj.RecordSize[fsid] > 128)
				{
					//发送准备就绪
					memset(CharTemp, '\0', sizeof(CharTemp));
					CharTemp[0] = USB_CMD_RTC;
					CharTemp[1] = USB_CMD_SEND_DATA;				
					CharTemp[2] = USB_CMD_OPER_ERR;
					IntRet = USBUpLongString(CharTemp, 3);
					if(IntRet == -1)
					{
						RetValue = 1;
						goto EndFlag;				
						//return(1);
					}
					else if(IntRet == -2)
					{
						RetValue = 3;
						goto EndFlag;				
						//return(3);
					}					
					else
					{
						RetValue = 5;
						goto EndFlag;									
						//return(5);
					}
				}

				//数据发送
				//以记录为单位发送数据，最大长度不能超过128个字节
				for(RecordCount = 0; RecordCount < TableInfoObj.TotalExist[fsid]; RecordCount++)
				{					
					DispStr_CE(0, 0, "正在发送表信息",DISP_CENTER|DISP_CLRSCR);			
					//发送准备就绪
					memset(CharTemp, '\0', sizeof(CharTemp));
					CharTemp[0] = USB_CMD_RTC;
					CharTemp[1] = USB_CMD_SEND_DATA;		
					CharTemp[2] = 0xff;		
					
					memset(CharTempA, '\0', sizeof(CharTempA));					
					sprintf(CharTempA, "总共%d条记录", TableInfoObj.TotalExist[fsid]);
					DispStr_CE(0, 2, CharTempA,DISP_CENTER);

					memset(CharTempA, '\0', sizeof(CharTempA));					
					sprintf(CharTempA, "正处理第%d条", RecordCount + 1);
					DispStr_CE(0, 4, CharTempA,DISP_CENTER);
					
					pTAB1_STRUCTObj = NULL;	
			    	 	pTAB1_STRUCTObj = (USER_INFO *)DB_jump_to_record(fsid, RecordCount, &flag);
			  	   	if(flag) 
					{
						continue;					
			     		}
										
					//发送准备就绪
					memcpy(&CharTemp[3], pTAB1_STRUCTObj, TableInfoObj.RecordSize[fsid]);
					//DispStr_CE(0, 0, CharTemp ,DISP_CENTER|DISP_CLRSCR);
					IntRet = USBUpLongString(CharTemp, TableInfoObj.RecordSize[fsid] + 3);
					if(IntRet == -1)
					{
						RetValue = 1;
						goto EndFlag;				
						//return(1);
					}
					else if(IntRet == -2)
					{
						RetValue = 3;
						goto EndFlag;				
						//return(3);
					}						

					if(RecordCount == (TableInfoObj.TotalExist[fsid] - 1))
					{
						RetValue = 0;		//debug
						//USBUpLongString("over", 4);	//debug
						goto EndFlag;		//debug
						break;
					}
					USBDownLongString(CharTemp, &DataLen);
					//Sys_Delay_MS(200);
				}												
//				if (RecordCount == TableInfoObj.TotalExist[fsid]) {
//					RetValue = 0;
//					goto EndFlag;
//				}
//				DispStr_CE(0, 2, "USB数据传输...",DISP_CENTER|DISP_CLRSCR);
			}
			else  if((cmd_op == USB_CMD_REQ) && (cmd_type == USB_CMD_RECV_DATA))
			{
				//表的序号
				fsid = CharTemp[2];										
				RecordCountA = (((int)CharTemp[3] << 24) & 0xFF000000) + (((int)CharTemp[4] << 16) & 0x00FF0000) + (((int)CharTemp[5] << 8) & 0x0000FF00) + (int)CharTemp[6]; 
				/*
				{
					char CharTempB[20];
					memset(CharTempB, '\0', sizeof(CharTempB));
					sprintf(CharTempB, "%d,%d,%d,%d", RecordCountA, TableInfoObj.TotalExist[fsid], RecordCountA, DB_capacity(fsid));
					DispStr_CE(0,1,CharTempB,DISP_CENTER|DISP_CLRSCR);

					memset(CharTempB, '\0', sizeof(CharTempB));
					sprintf(CharTempB, "%d,%d,%d,%d,%d", CharTemp[3], CharTemp[4], CharTemp[5], CharTemp[6], DataLen);
					DispStr_CE(0,4,CharTempB,DISP_CENTER);
					
					//delay_and_wait_key(20, EXIT_KEY_ALL, 30);								
				}
				*/
				
				//判断是否能全部保存到数据库
				if((TableInfoObj.TotalExist[fsid] + RecordCountA) >= DB_capacity(fsid))
				{
					//发送准备就绪
					memset(CharTemp, '\0', sizeof(CharTemp));
					CharTemp[0] = USB_CMD_RTC;
					CharTemp[1] = USB_CMD_RECV_DATA;		
					CharTemp[2] = USB_CMD_OPER_ERR;		
					IntRet = USBUpLongString(CharTemp, 3);
					if(IntRet == -1)
					{
						RetValue = 1;
						goto EndFlag;				
						//return(1);
					}
					else if(IntRet == -2)
					{
						RetValue = 3;
						goto EndFlag;				
						//return(3);
					}	
					else
					{
						RetValue = 6;
						goto EndFlag;							
						//return(6);
					}
				}

				//for(RecordCount = 0; RecordCount < RecordCountA; RecordCount++)
				{
					DispStr_CE(0, 4, "接收记录...",DISP_CENTER|DISP_CLRSCR);

					/*
					memset(CharTempA, '\0', sizeof(CharTempA));					
					sprintf(CharTempA, "总%d条记录", RecordCountA);
					DispStr_CE(0, 2, CharTempA,DISP_CENTER);

					memset(CharTempA, '\0', sizeof(CharTempA));					
					sprintf(CharTempA, "正处理第%d条", RecordCount + 1);
					DispStr_CE(0, 4, CharTempA,DISP_CENTER);
					*/
					
					//发送准备就绪
					memset(CharTemp, '\0', sizeof(CharTemp));
					CharTemp[0] = USB_CMD_RTC;
					CharTemp[1] = USB_CMD_RECV_DATA;				
					IntRet = USBUpLongString(CharTemp, 2);
					if(IntRet == -1)
					{
						RetValue = 1;
						goto EndFlag;				
						//return(1);
					}
					else if(IntRet == -2)
					{
						RetValue = 3;
						goto EndFlag;				
						//return(3);
					}					

					memset(CharTemp, '\0', sizeof(CharTemp));
			      	IntRet = USBDownLongString(CharTemp, &DataLen);
					if(IntRet == -1)
					{
						RetValue = 1;
						goto EndFlag;				
						//return(1);
					}
					else if(IntRet == -2)
					{
						RetValue = 3;
						goto EndFlag;				
						//return(3);
					}											
					
					if(!DB_add_record(fsid, (void*)CharTemp))
					{                     
						RetValue = 7;
						goto EndFlag;						
						//return(7);                                                           		
					}					
					TableInfoObj.TotalExist[fsid]++;
					TableInfoObj.TotalValid[fsid]++;                                                                             		
				}		
				DispStr_CE(0, 2, "USB数据传输...",DISP_CENTER|DISP_CLRSCR);
			}	
			else if((cmd_op == USB_CMD_REQ) && (cmd_type == USB_CMD_DEL_FSID))
			{									
				DispStr_CE(0, 3, "删除记录", DISP_CENTER | DISP_CLRSCR);
				RetValue = DB_erase_filesys(CharTemp[2]);
				memset(CharTemp, '\0', sizeof(CharTemp));
				CharTemp[0] = USB_CMD_RTC;
				CharTemp[1] = USB_CMD_DEL_FSID;				
				if(RetValue == 0)
				{					
					CharTemp[2] = 0;
				}
				else if(RetValue == -1)
				{
					CharTemp[2] = 1;
				}
				else 
				{
					CharTemp[2] = 1;					
				}
						
				IntRet = USBUpLongString(CharTemp, 2);
				if(IntRet == -1)
				{
					RetValue = 1;
					goto EndFlag;
				}
				else if(IntRet == -2)
				{
					RetValue = 3;
					goto EndFlag;				
				}
				DispStr_CE(0, 2, "USB数据传输...",DISP_CENTER|DISP_CLRSCR);
			}				
			else if((cmd_op == USB_CMD_REQ) && (cmd_type == USB_CMD_SENDMACHINEPARA))
			{	
				warning_beep(1);
				DispStr_CE(0, 3, "上传手持机参数", DISP_CENTER | DISP_CLRSCR);
				
				memset(CharTemp, '\0', sizeof(CharTemp));
				CharTemp[0] = USB_CMD_RTC;
				CharTemp[1] = USB_CMD_SENDMACHINEPARA;				
				CharTemp[2] = g_MachineNo & 0xFF;				
						
				IntRet = USBUpLongString(CharTemp, 3);
				if(IntRet == -1)
				{
					RetValue = 1;
					goto EndFlag;
				}
				else if(IntRet == -2)
				{
					RetValue = 3;
					goto EndFlag;				
				}
				DispStr_CE(0, 2, "USB数据传输...",DISP_CENTER|DISP_CLRSCR);
			}				
		}
									
		//F1 返回
		else if(IntRet == -1)
		{
			RetValue = 1;
			goto EndFlag;			
			//return(1);				
		}
		//没有联接上PC
		else if(IntRet == -2)
		{
			RetValue = 3;
			goto EndFlag;					
			//return(3);				
		}
		else
		{				
			RetValue = 4;
			goto EndFlag;					
			//return(4);		
		}				
	}
EndFlag:
	USB_Disconnect();
	return(RetValue);	
}


//函数功能：
//	接收从PC端传输过来的数据
//函数输入：
//	unsigned char *DataBuffer
//		保存数据的缓冲区
//	unsigned long *BufferLen
//		数据的长度
//函数返回：
//	0 操作成功
//	-1 F1键返回
//	-2 没有联接上PC
int USBDownLongString(unsigned char DataBuffer[], unsigned int *DataLen)
{
	short ret = -1;					// 函数返回值
	short cnt_now;
	unsigned short	i_st;						// 中断类型
	unsigned int	 expect_len;
	unsigned int real_len = 0;	
	unsigned char recvbuf[100];				// 接收缓冲区
	/*
	if(USB_Init())
	{
		return(-2);
	}
	*/
	for(;;)
	{
		if(KEY_Read() == KEY_F1)	
		{
			ret = -1;
			goto end;
		}
		i_st = D12_Read_Interrupt_Register();				// 读取PDIUSBD12中断寄存器值
		if(i_st != 0) 
		{
			if(i_st & D12_INT_SUSPENDCHANGE)
				USB_Suspend();								// 总线挂起改变
			if(i_st & D12_INT_ENDP0IN)
				USB_EP0_Txdone();							// 控制端点发送数据处理
			if(i_st & D12_INT_ENDP0OUT)
				USB_EP0_Rxdone();							// 控制端点接收数据处理
			if(i_st & D12_INT_ENDP1OUT)
			{
				D12_Read_Last_Status(2);
				D12_Read_Endpoint(2, 16, recvbuf);
				if(recvbuf[0] == 'E') 
				{
					break;
				}
				else if(recvbuf[0] != 'S')					// 如非上传模式
				{
					memcpy(&expect_len,recvbuf,4);
					D12_Write_Endpoint(3, 4, (unsigned char*)&expect_len); 			
				}			
			}
			if(i_st & D12_INT_ENDP2OUT)
			{
				D12_Read_Last_Status(4);
				cnt_now = D12_Read_Endpoint(4, 64, recvbuf);
				memcpy((DataBuffer + real_len), recvbuf, cnt_now);
				real_len += cnt_now;
			}
		}
	}
	*DataLen = real_len;
	//if(real_len == expect_len)
		ret = 0;
end:
//	USB_Disconnect();
	return ret;
}


//函数功能：
//	接收从PC端传输过来的数据
//函数输入：
//	unsigned char *DataBuffer
//		保存数据的缓冲区
//	unsigned long *BufferLen
//		数据的长度
//函数返回：
//	0 操作成功
//	-1 F1键返回
//	-2 没有联接上PC
//注：
//	此函数最长能处理128个字节的数据。
int USBUpLongString(unsigned char DataBuffer[], unsigned int DataLen)
{
	int blk_cnt;						// 块数量
	int blk_res;						// 不足1024字节的剩余字节数
	int blk_idx=0;					// 循环变量
	int cnt_now;						// 本次发送字节数
	int chr_idx;						// 块内循环变量
	short ret = -1;					// 函数返回值		
	unsigned short i_st;					// 中断类型
	unsigned char recvbuf[4];		// 接收缓冲区
	/*
	if(USB_Init())
	{
		ret = -2;
		return ret;
	}
	*/
	blk_cnt = (DataLen / 64);						// 计算参数
	blk_res = (DataLen % 64);
	blk_cnt += ( (blk_res>0) ? 1 : 0 );
	blk_res = ((blk_res==0) ? 64 : blk_res);
	
	for(;;)
	{
		if(KEY_Read() == KEY_F1)	
		{
			ret = -1;
			goto end;
		}
		i_st = D12_Read_Interrupt_Register();				// 读取PDIUSBD12中断寄存器值
		if(i_st != 0) 
		{
			if(i_st & D12_INT_SUSPENDCHANGE)
				USB_Suspend();								// 总线挂起改变
			if(i_st & D12_INT_ENDP0IN)
				USB_EP0_Txdone();							// 控制端点发送数据处理
			if(i_st & D12_INT_ENDP0OUT)
				USB_EP0_Rxdone();							// 控制端点接收数据处理
			if(i_st & D12_INT_ENDP1OUT)
			{
				D12_Read_Last_Status(2);
				D12_Read_Endpoint(2,1,recvbuf);
				if(recvbuf[0] == 'S')
				{					
					D12_Read_Last_Status(3);
					D12_Write_Endpoint(3, 4, (unsigned char*)&DataLen);			
				}
				if(recvbuf[0] == 'E') 
				{
					break;
				}				
			}
			if(i_st & D12_INT_ENDP2OUT)
			{	
				D12_Read_Last_Status(4);
				D12_Read_Endpoint(4, 1, recvbuf);
				if(recvbuf[0] == 'S')
				{
					for(chr_idx = 0; chr_idx < 2; chr_idx++)	// 一次发送 128 byte
					{
						if(blk_idx >= blk_cnt)
							break;
							 		
						cnt_now = ((blk_idx != (blk_cnt - 1)) ? 64 : blk_res );
						D12_Read_Last_Status(5);
						D12_Write_Endpoint(5, cnt_now, (DataBuffer + 64 * blk_idx));
						blk_idx++;	
					}
				}
			}
		}
	}
	ret = 0;
end:
//	USB_Disconnect();
	return ret;
}




