/*
 * Payload.h
 *
 *  Created on: 2017/12/10
 *      Author: tomoaki
 */

#ifndef PAYLOAD_H_
#define PAYLOAD_H_

#include <AppDefine.h>
#include <stdint.h>

namespace tomyApplication
{
class Payload
{
public:
    Payload(void);
    Payload(uint8_t length);
    ~Payload(void);

    void set_bool(bool);
    void  set_int4(int8_t);
    void set_int8(int8_t);
    void set_int16(int16_t);
    void set_int32(int32_t);
    void set_float(float);
    void set_uint4(uint8_t);
    void set_uint8(uint8_t);
    void set_uint16(uint16_t);
    void set_uint24(uint32_t);
    void set_uint32(uint32_t);
    void set_string(String);

    bool get_bool(void);
    int8_t get_int4(void);
    int8_t get_int8(void);
    int16_t get_int16(void);
    int32_t get_int32(void);
    float get_float(void);
    uint8_t get_uint4(void);
    uint8_t get_uint8(void);
    uint16_t get_uint16(void);
    uint32_t get_uint24(void);
    uint32_t get_uint32(void);
    String get_string(void);

    uint8_t* getRowData(void);
    uint8_t getLen(void);
    void clear(void);
    void reacquire(void);
    void create(uint8_t len);
    Payload& operator =(Payload& payload);
private:
    void setByte(uint8_t* data, uint8_t len);
    uint8_t* getData(uint8_t* val, uint8_t len);
    uint8_t* getByte(uint8_t* val, uint8_t len);
    void erase(void);
    uint8_t* _buff;
    uint8_t  _maxLen;
    uint8_t* _Bpos;
    uint8_t  _bPos;
    uint8_t  _gPos;
    uint8_t* _Gpos;
    uint8_t  _memDlt;
};

}
#endif /* PAYLOAD_H_ */
