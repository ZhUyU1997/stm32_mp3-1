#include "string.h"
#include "dev_gpio.h"
#include "gpio.h"


group_global group_status[19];


//��gpx����
const u8 get_gpio_grop[MAXIO] = {
	0,0,0,0,0,0,		//gpio0-gpio5
	1,1,1,1,			//gpio6-gpio9
	2,2,2,2,			//gpio10-gpio13
	3,3,3,				//gpio14-gpio16
	4,4,4,4,4,			//gpio17-gpio21
	5,5,5,				//gpio22-gpio24
	6,6,6,				//gpio25-gpio27
	7,7,				//gpio28-gpio29
	8,8,				//gpio30-gpio31
	9,9,				//gpio32-gpio33
	10,10,10,10,10,10,10,10,10,10,		//gpio34-gpio43
	11,11,				//gpio44-gpio45
	12,12,				//gpio46-gpio47
	13,13,				//gpio48-gpio49
	14,					//gpio50
	15,					//gpio51
	16,					//gpio52
	17,17,  			//gpio53-gpio54
	18,                 ////gpio55
	19,19,19,19,19,19	//gpio56-gpio61
};

const u8 get_grop_ionum[MAXGROP+1] = {
	0,				//gpio0-gpio5
	6,				//gpio6-gpio9
	10,				//gpio10-gpio13
	14,				//gpio14-gpio16
	17,				//gpio17-gpio21
	22,				//gpio22-gpio24
	25,				//gpio25-gpio27
	28,				//gpio28-gpio29
	30,				//gpio30-gpio31
	32,				//gpio32-gpio33
	34,				//gpio34-gpio43
	44,				//gpio44-gpio45
	46,				//gpio46-gpio47
	48,				//gpio48-gpio49
	50,				//gpio50
	51,				//gpio51
	52,				//gpio52
	53,				//gpio53-gpio54
	55,
	56,				//gpio56-gpio61
	62				//end,�����ڴ�IO����Ϊ�˷���������һ���IO����
};

u32 GpioInterruptFun[62];	//�жϺ�����ڱ�

int_en GpioInterruptEn;

/******************************************************************
*** �� �� ��:   void gpio_status_init(void)
*** ����������  �ϵ��ʼ��GPIO״̬
*** ��    ��:   	
*** ��    �أ�   �ɹ�����0
***	                ����-1
******************************************************************/	
void gpio_status_init(void)
{
	memset(group_status, 0, sizeof(group_status));	//��ʼ���������IO״̬Ϊδʹ��״̬
	
	memset(GpioInterruptFun, 0, sizeof(GpioInterruptFun));	//��ʼ���������ָ��Ϊ��
	
	GpioInterruptEn.GpioIntEn64 = 0;	//����GPIO���жϽ�ֹ
	
	gpioClearAllExtInt();	//��������ж�״̬
}

/******************************************************************
*** �� �� ��:   s32 dev_iomutex_getsta(void)
*** ����������  ��ȡĳ���GPIO״̬
*** ��    ��:   	
*** ��    �أ�   ����0��ʾδռ��
***	             ����1��ʾ��ռ��
******************************************************************/
s32 dev_iomutex_getsta(u8 group)
{
	u8 cnt, i;
	u8 status=0;
	//0.�����ж�
	if(group>=MAXGROP)
	{
		return -1;		
	}
	
	//1.�����м���IO��
	cnt = get_grop_ionum[group+1]-get_grop_ionum[group];
	
	//2.��������IO��û�б�ռ�õ�
	for(i=0; i<cnt; i++)
	{
		status += group_status[group].gpx[i];
	}
	//3.����״̬
	if(status || group_status[group].status)
	{
		return USED;
	}
	else
	{
		return UNUSED;
	}
}

/******************************************************************
*** �� �� ��:   s32 dev_iomutex_setsta(u8 group, u16 status)
*** ����������  ����ĳ���GPIO�и����ŵ�״̬��ÿ1bit��Ӧһ��IO��
***              gpio0-gpio5��Ӧbit0-bit5�������鶼ռ���������Ϊ0xFFFF
*** ��    ��:   	
*** ��    �أ�   ����0��ʾδռ��
***	             ����1��ʾ��ռ��
******************************************************************/
s32 dev_iomutex_setsta(u8 group, u16 status)
{
	//0.�����ж�
	if(group>=MAXGROP)
	{
		return -1;		
	}
	
	//1.���ô����еĶ�Ӧ������״̬�����õĽŶ�Ӧbit����Ϊ1��δ�õĽŶ�ӦbitΪ0��
	group_status[group].status = status;
	
	//2.����״̬
	return SUCCESS;
}

/******************************************************************
*** �� �� ��:   s32 dev_gpio_open(u8 gpx, u8 pull, u8 dir)
*** ����������  ������
*** ��    ��:   	����: 	u8 gpx, ����
*** 						u8 pull, ������
***						u8 dir, ����
***						u32 *func,�Ƿ������ж�
***     	         �ɹ�����0
***	                ����-1
******************************************************************/
s32 dev_gpio_open(u8 gpx, u8 pull, u8 dir)
{
	u8 group,grp_io;

	//0.�����ж�
	if(gpx>=MAXIO)
	{
		return -1;		
	}
	
	//1.���жϣ������ж�
	group = get_gpio_grop[gpx];
	grp_io = gpx - get_grop_ionum[group];//��group�ĵ�grp_io��

	//if(/*USED == */group_status[group].status & (0x1<<grp_io))	//����IO�Ƿ���ռ�ã��˴����ǵ�SPI��Ƭѡ��Ҫ����SPI�飬���ǿ��Ե�����IO����
	if(/*USED == */group_status[group].status)	//THM3100��ֻҪ����Ϊ��IO���ܣ�������IO�����ܱ�ʹ��
	{
		return -1;
	}

	if(/*USED == */group_status[group].gpx[grp_io])
	{
		return -1;
	}
	
	//2.����GPIO/FUNCTION
	gpioSetGropProp(group, GROP_GPIO);

	//3.���÷���
	if(GPIO_IN == dir)
	{
		gpioSetDirIn(gpx);
	}
	else
	{
		gpioSetDirOut(gpx);
		
	}
	
	//4.����������
	if(pull == GPIO_PD)
	{
		/*set pull-down*/
		gpioSetPullDownBit(gpx);
		/*reset pull-up*/
		gpioResetPullUpBit(gpx);
	}
	else if(pull == GPIO_PU)
	{
		/*set pull-up*/
		gpioSetPullUpBit(gpx);
		/*reset pull-down*/
		gpioResetPullDownBit(gpx);
	}
	else//����
	{
		/*reset pull-up*/
		gpioResetPullUpBit(gpx);
		/*reset pull-down*/
		gpioResetPullDownBit(gpx);
	}	

	//4.���Ż���
	group_status[group].gpx[grp_io] = USED;
	
#if 0	//��GPIOʹ��ʱ������Ҫ������״̬��������ͬ��IO�е���һ��IO�޷����ã������ù���ʱ����Ҫ����״̬��
	group_status[group].status = USED;
#endif
	
	return SUCCESS;
}

/******************************************************************
*** �� �� ��:   s32 dev_gpio_close(u8 gpx)
*** ����������  �ر�����
*** ��    ��:   	����: 	u8 gpx, ����
***     	         �ɹ�����0
***	                ����-1
******************************************************************/
s32 dev_gpio_close(u8 gpx)
{
	u8 group,grp_io;
	
	//1.�����ж�
	if(gpx>=MAXIO)
	{
		return -1;		
	}

	//2.�жϴ˶˿��Ƿ�򿪣��򿪲���Ҫ�ر�
	group = get_gpio_grop[gpx];
	grp_io = gpx - get_grop_ionum[group];//��group�ĵ�grp_io��
	if(UNUSED == group_status[group].gpx[grp_io])
	{
		return -1;
	}
	
	gpioSetDirIn(gpx);
	//gpioResetBit(gpx);
	/*set pull-up*/
	gpioSetPullUpBit(gpx);
	/*reset pull-down*/
	gpioResetPullDownBit(gpx);
	
	//3.�رճɹ�������£���ռ�е�IO�ͷ�Ϊunused��
	group_status[group].gpx[grp_io] = UNUSED;
	group_status[group].status = UNUSED;
	
	return SUCCESS;
}

/******************************************************************
*** �� �� ��:   s32 dev_gpio_read(u8 gpx)
*** ����������  ������ֵ
*** ��    ��:   	����: 	u8 gpx, ����
***     	         �ɹ�����0
***	                ����-1
******************************************************************/
s32 dev_gpio_read(u8 gpx)
{
	u8 ret;
	
	//1.�����ж�
	if(gpx>=MAXIO)
	{
		return -1;		
	}

	//2.�жϴ˶˿��Ƿ�򿪣��򿪲��������
	
	//3.��ȡ����
	ret = gpioReadInputDataBit(gpx);
	
	return ret;
}

/******************************************************************
*** �� �� ��:   s32 dev_gpio_write(u8 gpx, u8 level)
*** ����������  �����Ÿ�ֵ
*** ��    ��:   	����: 	u8 gpx, ����
***						u8 level,��ƽ
***     	         �ɹ�����0
***	                ����-1
******************************************************************/
s32 dev_gpio_write(u8 gpx, u8 level)
{
	//1.�����ж�
	if(gpx>=MAXIO)
	{
		return -1;		
	}
	
	//2.�жϴ˶˿��Ƿ�򿪣��򿪲��������
	
	//3.���ö˿�״̬
	if(GPIO_LOW == level)
	{
		gpioResetBit(gpx);
	}
	else
	{
		gpioSetBit(gpx);
	}
	
	return SUCCESS;
}

static void GpioSetCallbackFunction(u8 gpx, u32 *func)
{
	s64 st_bit=0x1;
	st_bit <<= gpx;
	GpioInterruptFun[gpx] = (u32)func;

	if(func==NULL)
	{
		GpioInterruptEn.GpioIntEn64 &= ~st_bit;
	}
	else
	{
		GpioInterruptEn.GpioIntEn64  |= st_bit;
	}
}

/******************************************************************
*** �� �� ��:   s32 dev_gpio_set_irq(u8 gpx, u8 type, u32 *func)
*** ����������  �����Ÿ�ֵ
*** ��    ��:   	����: 	u8 gpx, ����
***							u8 type, �������ͣ�0-�͵�ƽ������1-�ߵ�ƽ������2-�½��ش�����3-�����ش���
***                         u32 *func, �жϵĻص�����
***     	         �ɹ�����0
***	                ����-1
******************************************************************/
s32 dev_gpio_set_irq(u8 gpx, u8 type, u32 *func)
{
	u8 group,grp_io;
	//1.�����ж�
	if(gpx>=MAXIO)
	{
		return -1;		
	}
	
	//2.�жϴ˶˿��Ƿ�򿪣�����Ϊinput����������Ϊ�ж�
	group = get_gpio_grop[gpx];
	grp_io = gpx - get_grop_ionum[group];//��group�ĵ�grp_io��
	if(UNUSED == group_status[group].gpx[grp_io])
	{
		return -1;
	}
	
	//3.�ж϶˿�������Ϊ����
	if(GPIO_OUT==gpioGetDir(gpx))
	{
		return -1;
	}
	
	//����ɲ����ж�
	if((func == NULL)||(type > 3))
	{
		return -1;
	}
	else
	{
		//�����ж�
		gpioSetExtInt(gpx, type);
		//�����жϺ���
		GpioSetCallbackFunction(gpx, func);

		return SUCCESS;
	}
	
}

s32 dev_gpio_free_irq(u8 gpx)
{
	//�ͷ��ж�
	gpioClrExtInt(gpx);
	//���жϺ���
	GpioSetCallbackFunction(gpx, NULL);
	
	return SUCCESS;
}

