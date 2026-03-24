#include "Config.h"

#ifdef LORA_ADDRESS
uint8_t myNodeId = LORA_ADDRESS;
#endif

#ifdef INFORMATION_NODE
bool informationNode = true;
#else
bool informationNode = false;
#endif