#ifndef __ELECDRIVERLIC_H
#define __ELECDRIVERLIC_H

#include "stm32f10x.h"

#define STACK_SIZE 5	//堆栈大小


/* ---某一优先级的申请登录驾驶员的驾照号 数据结构 --------------------------------------------*/
typedef struct {
    unsigned char ApplyLicStack[STACK_SIZE][ __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ ];//
    int pointer;
} ApplyLic_Struct;

/* ---所有优先级的申请登录驾驶员的驾照号 数据结构 --------------------------------------------*/
typedef struct {
    ApplyLic_Struct FirApplyLic;		//第一优先级的申请人驾照信息
    ApplyLic_Struct SecApplyLic;		//第二优先级的申请人驾照信息
    ApplyLic_Struct ThiApplyLic;		//第三优先级的申请人驾照信息
    ApplyLic_Struct FouApplyLic;		//第四优先级的申请人驾照信息
    ApplyLic_Struct FifApplyLic;		//第五优先级的申请人驾照信息
    ApplyLic_Struct NoneApplyLic;		//无优先级的申请人驾照信息
} AllApplyLic_Struct;

/***************本地常量声明*************/
//#define STACK_TYPE (unsigned char) /* 堆栈所存储的值的数据类型 */

/***************本地函数声明*************/
//void PopApplyStack(ApplyLic_Struct tmpLic);
void  ApplyLicInit(void);
void  PushApplyStack(ApplyLic_Struct *tmpLic, u8 *str,u8 len);
void  GetApplyData(AllApplyLic_Struct* alltmpLic, u8 * idNo);
int   is_emptyApplyData(AllApplyLic_Struct *alltmpLic);
int   is_full(ApplyLic_Struct *tmpLic);//判断堆栈是否满栈
int   is_empty(ApplyLic_Struct *tmpLic);//判断堆栈是否为空
void  PopApplyStack(ApplyLic_Struct *tmpLic);//将数据弹出堆栈


/***************全局变量声明*************/
extern AllApplyLic_Struct ApplyLicense;		//申请人的驾照号volatile


#endif
