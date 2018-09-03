/*
 * LoRaWANDef.h
 *
 *                       The MIT License
 *
 *   Copyright (c) 2017-2018     Tomoaki Yamaguchi    tomoaki@tomy-tech.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef LORAWANDEF_H_
#define LORAWANDEF_H_
namespace tomyApplication
{

//
//  LoRaWAN defines
//
#define LoRa_DEFAULT_PAYLOAD_SIZE      11

#define LoRa_INIT_WAIT_TIME      1000
#define LoRa_SERIAL_WAIT_TIME    2000
#define LoRa_RECEIVE_DELAY2      5000
#define JOIN__WAIT_TIME         30000

#define LORA_RC_SUCCESS            0
#define LORA_RC_DATA_TOO_LONG     -1
#define LORA_RC_NOT_JOINED        -2
#define LORA_RC_NO_FREE_CH   -3
#define LORA_RC_BUSY    -4
#define LORA_RC_ERROR             -5

#define ADR_ACK_LIMIT     5
#define ADR_ACK_DELAY   2

#ifdef LORA_DEBUG
#define LoRaDebug(...)  DebugPrint( __VA_ARGS__)
#define ECHOFLAG  true
#else
#define LoRaDebug(...)
#define ECHOFLAG  false
#endif

#ifdef TEST_ADR
#define ADRDebug(...)  DebugPrint( __VA_ARGS__)
#else
#define ADRDebug(...)
#endif

#define MAX_NO_FREE_CH_CNT  5
#ifndef PORT_LIST
#define PORT_LIST   PortList_t  thePortList[]
#define PORT(...)         {__VA_ARGS__}
#define END_OF_PORT_LIST  {0, 0}

typedef enum
{
    DR0, DR1, DR2, DR3, DR4, DR5
}LoRaDR;

typedef enum
{
    CH0, CH1, CH2, CH3, CH4, CH5, CH6, CH7, CH8, CH9, CH10, CH11, CH12, CH13, CH14, CH15, CH16
}CHID;

typedef  enum {
    joined, not_joined
}JoineStatus;

typedef struct PortList
{
    uint8_t port;
    void (*callback)(void);
} PortList_t;
#endif

#define LORA_TYPES

}

#endif /* LORAWANDEF_H_ */
