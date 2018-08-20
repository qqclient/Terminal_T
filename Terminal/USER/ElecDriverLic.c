/******************** (C) COPYRIGHT soarsky ********************
* File Name          :
* Author             :
* Version            :
* Date               :
* Description        :
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"


/***************本地常量定义*************/

/***************本局变量定义*************/
AllApplyLic_Struct ApplyLicense;		//申请人的驾照号volatile


/*******************************************************************************
* Function Name  : void ApplyLicInit(void)
* Description    : 初始化申请驾照的堆栈指针,在进入主循环之前需要调用该函数
******************************************************************************/
void ApplyLicInit(void)
{
    // 内存清0
    memset( (unsigned char*)&ApplyLicense.FirApplyLic.ApplyLicStack[0][0],
            0,
            (int)&ApplyLicense.NoneApplyLic.pointer -
            (int)&ApplyLicense.FirApplyLic.ApplyLicStack[0][0] +
            sizeof(ApplyLicense.NoneApplyLic.pointer) );

    ApplyLicense.FirApplyLic.pointer = -1;
    ApplyLicense.SecApplyLic.pointer = -1;
    ApplyLicense.ThiApplyLic.pointer = -1;
    ApplyLicense.FouApplyLic.pointer = -1;
    ApplyLicense.FifApplyLic.pointer = -1;
    ApplyLicense.NoneApplyLic.pointer = -1;

}


/*******************************************************************************
* Function Name  : void PushApplyStack(ApplyLic_Struct tmpLic, u8 *str,u8 len)
* Description    : 将申请人的驾驶证号、驾驶证型号压入堆栈中
******************************************************************************/
void PushApplyStack(ApplyLic_Struct *tmpLic, unsigned char* str, unsigned char len)
{
    Flag_ApplyResult = SET;
    if( is_full(tmpLic) )
    {
        tmpLic->pointer += 1;
        memcpy(&tmpLic->ApplyLicStack[tmpLic->pointer][0], str, len);
    }
}

/*******************************************************************************
* Function Name  : void PopApplyStack(ApplyLic_Struct tmpLic,u8 len)
* Description    : 将申请人的驾驶证号、驾驶证型号弹出堆栈
******************************************************************************/
static void PopApplyStack(ApplyLic_Struct *tmpLic)
{
    if(is_empty(tmpLic))
    {
        memset( tmpLic->ApplyLicStack[tmpLic->pointer], 0, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );
        tmpLic->pointer -= 1;
    }
}

/*******************************************************************************
* Function Name  : GetApplyData
* Description    : 获得申请人的驾驶证号、驾驶证型号
******************************************************************************/
void GetApplyData(AllApplyLic_Struct* alltmpLic, u8* idNo)
{
    if(is_empty(&alltmpLic->FirApplyLic))
    {
        memcpy(idNo, &alltmpLic->FirApplyLic.ApplyLicStack[alltmpLic->FirApplyLic.pointer][0], __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );
        PopApplyStack(&alltmpLic->FirApplyLic);
        return;
    }
    else if(is_empty(&alltmpLic->SecApplyLic))
    {
        memcpy(idNo, &alltmpLic->SecApplyLic.ApplyLicStack[alltmpLic->SecApplyLic.pointer][0], __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );
        PopApplyStack(&alltmpLic->SecApplyLic);
        return;
    }
    else if(is_empty(&alltmpLic->ThiApplyLic))
    {
        memcpy(idNo, &alltmpLic->ThiApplyLic.ApplyLicStack[alltmpLic->ThiApplyLic.pointer][0], __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );
        PopApplyStack(&alltmpLic->ThiApplyLic);
        return;
    }
    else if(is_empty(&alltmpLic->FouApplyLic))
    {
        memcpy(idNo, &alltmpLic->FouApplyLic.ApplyLicStack[alltmpLic->FouApplyLic.pointer][0], __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );
        PopApplyStack(&alltmpLic->FouApplyLic);
        return;
    }
    else if(is_empty(&alltmpLic->FifApplyLic))
    {
        memcpy(idNo, &alltmpLic->FifApplyLic.ApplyLicStack[alltmpLic->FifApplyLic.pointer][0], __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );
        PopApplyStack(&alltmpLic->FifApplyLic);
        return;
    }
    else if(is_empty(&alltmpLic->NoneApplyLic))
    {
        memcpy(idNo, &alltmpLic->NoneApplyLic.ApplyLicStack[alltmpLic->NoneApplyLic.pointer][0], __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );
        PopApplyStack(&alltmpLic->NoneApplyLic);
        return;
    }
}

/*******************************************************************************
* Function Name  : static int is_emptyApplyData(ApplyLic_Struct tmpLic)
* Description    : 判断申请人的堆栈是否为空
******************************************************************************/
int is_emptyApplyData(AllApplyLic_Struct *alltmpLic)
{
    if(is_empty(&alltmpLic->FirApplyLic) ||
            is_empty(&alltmpLic->SecApplyLic) ||
            is_empty(&alltmpLic->ThiApplyLic) ||
            is_empty(&alltmpLic->FouApplyLic) ||
            is_empty(&alltmpLic->FifApplyLic))
    {
        return 1;
    }
    else
        return 0;
}

/*******************************************************************************
* Function Name  : static int is_empty(ApplyLic_Struct tmpLic)
* Description    : 判断堆栈是否为空
******************************************************************************/
int is_empty(ApplyLic_Struct *tmpLic)
{
    if(tmpLic->pointer < 0)
        return 0;
    else
        return 1;
}

/*******************************************************************************
* Function Name  : static int is_empty(ApplyLic_Struct tmpLic)
* Description    : 判断堆栈是否为满
******************************************************************************/
static int is_full(ApplyLic_Struct *tmpLic)
{
    if(tmpLic->pointer == STACK_SIZE - 1)
        return 0;
    else
        return 1;
}

