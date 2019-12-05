#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

class EasyScale
{
public:
    static void SendCmd(uint8_t u8Value);
protected:
    static void WriteStart();
    static void WriteEOS();
    static void WriteLogic0();
    static void WriteLogic1();
    static void WriteByte(uint8_t u8Value);
};

#ifdef __cplusplus
}
#endif
