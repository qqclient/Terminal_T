/******************** (C) COPYRIGHT soarsky ********************
* File Name          :
* Author             :
* Version            :
* Date               :
* Description        :
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"


/***************���س�������*************/

/***************���ֱ�������*************/
AllApplyLic_Struct ApplyLicense;		//�����˵ļ��պ�volatile


/*******************************************************************************
* Function Name  : void ApplyLicInit(void)
* Description    : ��ʼ��������յĶ�ջָ��,�ڽ�����ѭ��֮ǰ��Ҫ���øú���
******************************************************************************/
void ApplyLicInit(void)
{
    // �ڴ���0
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
* Description    : �������˵ļ�ʻ֤�š���ʻ֤�ͺ�ѹ���ջ��
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
* Description    : �������˵ļ�ʻ֤�š���ʻ֤�ͺŵ�����ջ
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
* Description    : ��������˵ļ�ʻ֤�š���ʻ֤�ͺ�
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
* Description    : �ж������˵Ķ�ջ�Ƿ�Ϊ��
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
* Description    : �ж϶�ջ�Ƿ�Ϊ��
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
* Description    : �ж϶�ջ�Ƿ�Ϊ��
******************************************************************************/
static int is_full(ApplyLic_Struct *tmpLic)
{
    if(tmpLic->pointer == STACK_SIZE - 1)
        return 0;
    else
        return 1;
}

