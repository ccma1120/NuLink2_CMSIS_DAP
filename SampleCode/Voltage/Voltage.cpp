#include "Voltage.h"
//#include "stdio.h"
#include "NuMicro.h"
#include "Voltage_IO.h"
#include "EasyScale.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum DESIGN_ID_E
{
    DESIGN_ID_NULINK2_WO_CONVERTER    = 0,
    DESIGN_ID_NULINK2_W_CONVERTER     = 1,
    DESIGN_ID_NUTRACE                 = 2,
    DESIGN_ID_NULINK3                 = 3
};

DESIGN_ID_E Voltage_GetDesignID()
{
/*
    Nu-Trace    (ID0, ID1, ID2) = (1, 1, 1)
    Nu-Link2    (ID0, ID1, ID2) = (0, 1, 1)
    Nu-Link2-Me (ID0, ID1, ID2) = (0, 0, 1) w/o voltage converter
    Nu-Link3    (ID0, ID1, ID2) = (0, 0, 0)
*/
    GPIO_SetMode(ID0_GPIO_GRP, (1 << ID0_GPIO_BIT), GPIO_MODE_INPUT);
    GPIO_SetMode(ID1_GPIO_GRP, (1 << ID1_GPIO_BIT), GPIO_MODE_INPUT);
    GPIO_SetMode(ID2_GPIO_GRP, (1 << ID2_GPIO_BIT), GPIO_MODE_QUASI); //new added on Nu-Link3

    if(ID0_GPIO_IO == 1 && ID1_GPIO_IO == 1 && ID2_GPIO_IO == 1)
        return DESIGN_ID_NUTRACE;
    else if(ID0_GPIO_IO == 0 && ID1_GPIO_IO == 1 && ID2_GPIO_IO == 1)
        return DESIGN_ID_NULINK2_W_CONVERTER;
    else if(ID0_GPIO_IO == 0 && ID1_GPIO_IO == 0 && ID2_GPIO_IO == 0)
        return DESIGN_ID_NULINK3;
    else 
        return DESIGN_ID_NULINK2_WO_CONVERTER;
}

const uint16_t TPS62400[] = 
{
    850,    // 0x41
    900,    // 0x42
    950,    // 0x43
    1000,   // 0x44
    1050,   // 0x45
    1100,   // 0x46
    1150,   // 0x47
    1200,   // 0x48
    1250,   // 0x49
    1300,   // 0x4A
    1350,   // 0x4B
    1400,   // 0x4C
    1450,   // 0x4D
    1500,   // 0x4E
    1550,   // 0x4F
    1600,   // 0x50
    1700,   // 0x51
    1800,   // 0x52
    1850,   // 0x53
    2000,   // 0x54
    2100,   // 0x55
    2200,   // 0x56
    2300,   // 0x57
    2400,   // 0x58
    2500,   // 0x59
    2600,   // 0x5A
    2700,   // 0x5B
    2800,   // 0x5C
    2850,   // 0x5D
    3000,   // 0x5E
    3300    // 0x5F
};

void Voltage_Init(void)
{
    /* SWDH_CLK, SWDH_RST, SWDH_DAT_O */
    GPIO_SetMode(LS_DO_DIR0_GRP, (1 << LS_DO_DIR0_BIT), GPIO_MODE_OUTPUT);
    LS_DO_DIR0_IO = 0;

    /* SWDH_DAT_I */
    GPIO_SetMode(LS_DI_DIR1_GRP, (1 << LS_DI_DIR1_BIT), GPIO_MODE_OUTPUT);
    LS_DI_DIR1_IO = 0;

    /* Target POWER ON control */
    GPIO_SetMode(A_TGP_EN_GRP, (1 << A_TGP_EN_BIT), GPIO_MODE_OUTPUT);
    A_TGP_EN_IO = 1;

    /* TPS62400 convert enable */
    GPIO_SetMode(A_LSP_CS_GRP, (1 << A_LSP_CS_BIT), GPIO_MODE_OUTPUT);
    A_LSP_CS_IO = 0;

    /* TPS62400 data pin */
    GPIO_SetMode(LSP_DA_GRP, (1 << LSP_DA_BIT), GPIO_MODE_OUTPUT);
    LSP_DA_IO = 0;

    /* TPS62400 output enable */
    if(Voltage_GetDesignID() == DESIGN_ID_NULINK3)
    {
        //...
    }
    else
    {
        GPIO_SetMode(LSP_EN_GRP, (1 << LSP_EN_BIT), GPIO_MODE_OUTPUT);
        LSP_EN_IO = 1;
    }

    /* V3.3 output control */
    GPIO_SetMode(TPLS_DIR_GRP, (1 << TPLS_DIR_BIT), GPIO_MODE_OUTPUT);
    TPLS_DIR_IO = 1;

    Voltage_ShutDownAllPin();
    Voltage_SupplyTargetPower(0, 3300);
}

static int32_t Voltage_SetIOVoltage(uint32_t u32Voltage_mv)
{
    int32_t bDirectIO = 0;

    do
    {
        //if (u32Voltage_mv > 3300)
        if (u32Voltage_mv > 3500)    /* We look on voltage 3.3V ~ 3.5V is 3.3V */
        {
            /* Use direct IO */
            bDirectIO = 1;
            /* TVCC_O => VDDIO 1.6~3.6V*/
            u32Voltage_mv = 3300;
        }
        else
        {
            /* Use EasyScale */
            bDirectIO = 0;
        }

        EasyScale::SendCmd(0x4E);

        int32_t i;
        for (i = 0; i < sizeof(TPS62400) / sizeof(TPS62400[0]); ++i)
        {
            if (u32Voltage_mv <= TPS62400[i])
                break;
        }

        if (!(i < sizeof(TPS62400) / sizeof(TPS62400[0])))
            --i;

        EasyScale::SendCmd(0x41 + i);
    }
    while(0);

    return bDirectIO;
}

void Voltage_SupplyTargetPower(int32_t bEnable, uint32_t u32Voltage_mv)
{
    A_TGP_EN_IO = 0;
    A_LSP_CS_IO = 0;

    int32_t bDirectIO = Voltage_SetIOVoltage(u32Voltage_mv);

    if (bEnable)
    {
        if (bDirectIO)
        {
            A_TGP_EN_IO = 0;
            A_LSP_CS_IO = 1;
        }
        else
        {
            A_LSP_CS_IO = 0;
            A_TGP_EN_IO = 1;
        }
    }
}

void Voltage_OpenPin(void)
{
    /* SWDH_CLK, SWDH_RST, SWDH_DAT_O */
    LS_DO_DIR0_IO = 1;

    /* SWDH_DAT_I */
    LS_DI_DIR1_IO = 0;
}

void Voltage_ShutDownAllPin(void)
{
    /* SWDH_CLK, SWDH_RST, SWDH_DAT_O */
    LS_DO_DIR0_IO = 0;

    /* SWDH_DAT_I */
    LS_DI_DIR1_IO = 0;
}

#ifdef __cplusplus
}
#endif
