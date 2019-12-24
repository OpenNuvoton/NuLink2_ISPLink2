#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

void Voltage_Init(void);
void Voltage_SupplyTargetPower(int32_t bEnable, uint32_t u32Voltage_mv);
void Voltage_OpenPin(void);
void Voltage_ShutDownAllPin(void);

#ifdef __cplusplus
}
#endif
