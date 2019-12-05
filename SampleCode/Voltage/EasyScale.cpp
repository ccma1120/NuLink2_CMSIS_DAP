#include "EasyScale.h"
#include "NuMicro.h"
#include "Voltage_IO.h"

#ifdef __cplusplus
extern "C"
{
#endif

static int32_t DisableIRQ()
{
    int32_t nPRIMASK = __get_PRIMASK();
    int32_t bPrevousEnabled = ((nPRIMASK & 1) == 0);
    if (bPrevousEnabled)
        __set_PRIMASK(1);    /* Disable IRQ */

    return bPrevousEnabled;
}

static void RestoreIRQ(int32_t bPrevousEnabled)
{
    if (bPrevousEnabled)
        __set_PRIMASK(0);
}

void EasyScale::SendCmd(uint8_t u8Value)
{
    EasyScale::WriteStart();
    EasyScale::WriteByte(u8Value);
    EasyScale::WriteEOS();
}

void EasyScale::WriteStart()
{
    LSP_DA_IO = 1;
    CLK_SysTickDelay(10);
    LSP_DA_IO = 0;
}

void EasyScale::WriteEOS()
{
    LSP_DA_IO = 0;
    CLK_SysTickDelay(10);
}

void EasyScale::WriteLogic0()
{
    CLK_SysTickDelay(50);

    //Disable IRQ since the delay time should be exactly very short
    int32_t nPrevEnabled = DisableIRQ();

    LSP_DA_IO = 1;
    CLK_SysTickDelay(10);
    LSP_DA_IO = 0;

    //Restore IRQ state
    RestoreIRQ(nPrevEnabled);
}

void EasyScale::WriteLogic1()
{
    CLK_SysTickDelay(10);
    LSP_DA_IO = 1;
    CLK_SysTickDelay(50);
    LSP_DA_IO = 0;
}

void EasyScale::WriteByte(uint8_t u8Value)
{
    int32_t i;
    for (i = 0; i < 8; ++i)
    {
        if ((u8Value & (uint8_t)0x80) == 0)
            EasyScale::WriteLogic0();
        else
            EasyScale::WriteLogic1();

        u8Value <<= 1;
    }
}

#ifdef __cplusplus
}
#endif
