/*
 * Payload.cpp
 *
 *  Created on: 2017/12/10
 *      Author: tomoaki
 */
#include <Application.h>
#include <Payload.h>

using namespace tomyApplication;

uint16_t getUint16(const uint8_t* pos){
    uint16_t val = ((uint16_t)*pos++ << 8);
    return val += *pos;
}

void setUint16(uint8_t* pos, uint16_t val){
    *pos++ = (val >> 8) & 0xff;
    *pos   = val & 0xff;
}

uint32_t getUint32(const uint8_t* pos){
    uint32_t val = uint32_t(*pos++) <<  24;
    val += uint32_t(*pos++) << 16;
    val += uint32_t(*pos++) <<  8;
    return val += *pos++;
}

void setUint32(uint8_t* pos, uint32_t val){
    *pos++ = (val >> 24) & 0xff;
    *pos++ = (val >> 16) & 0xff;
    *pos++ = (val >>  8) & 0xff;
    *pos   =  val & 0xff;
}

float getFloat32(const uint8_t* pos){
    union{
        float flt;
        uint8_t d[4];
    }val;
    val.d[3] = *pos++;
    val.d[2] = *pos++;
    val.d[1] = *pos++;
    val.d[0] = *pos;
    return val.flt;
}

void setFloat32(uint8_t* pos, float flt){
    union{
        float flt;
        uint8_t d[4];
    }val;
    val.flt = flt;
    *pos++ = val.d[3];
    *pos++ = val.d[2];
    *pos++ = val.d[1];
    *pos   = val.d[0];
}


/*=====================================
        Class Payload
  =====================================*/
Payload::Payload():
    _buff{0},  _maxLen{0}, _Bpos{0}, _bPos{7}, _gPos{7}, _memDlt{0}
{
    _Gpos = 0;
}

Payload::Payload(uint8_t len):
        _buff{0},  _maxLen{0}, _Bpos{0}, _bPos{7}, _gPos{7}, _memDlt{0}
{
    create(len);
}

Payload::~Payload()
{
    erase();
}

void Payload::erase(void)
{
    if(_memDlt)
    {
        free(_buff);
    }
}

void Payload::create(uint8_t len)
{
    erase();
    _buff = (uint8_t*)calloc(len, sizeof(uint8_t));
    if(_buff == 0)
    {
        ConsolePrint(F("can't allocate a payload\n"));
        for ( ; ; );
    }
    _Bpos = _buff;
    _Gpos = _buff;
    _bPos = 7;
    _gPos = 7;
    _maxLen = len;
    _memDlt = 1;
}

void Payload::clear(void)
{
    memset( _buff, 0, _maxLen);
    _Bpos = _Gpos = _buff;
    _bPos = _gPos = 7;
}

Payload& Payload::operator =(Payload& payload)
{
    this->erase();
    this->create(payload._maxLen);
    memcpy(this->_buff, payload._buff, this->_maxLen);
    this->_bPos = payload._bPos;
    this->_gPos = payload._gPos;
    return *this;
}

uint8_t Payload::getLen(void)
{
    if ( _bPos < 7 )
    {
        return _Bpos - _buff + 1;
    }
    else
    {
        return _Bpos - _buff;
    }
}

uint8_t* Payload::getRowData(void)
{
    return _buff;
}

void Payload::reacquire(void)
{
    _Gpos = _buff;
    _gPos = 7;
}

//
// setters
//
void Payload::set_bool(bool flg)
{
    uint8_t data = flg ? 1 : 0;
    setByte( &data, 1);
}

void Payload::set_int4(int8_t val)
{
    if ( val > 8 || val < -7 )
    {
        val = 0;
    }
    uint8_t data = val >= 0 and val < 8 ? val : 16 + val;
    setByte((uint8_t*)&data, 4);
}

void Payload::set_int8(int8_t val)
{
    set_uint8(val);
}

void Payload::set_int16(int16_t val)
{
    set_uint16((uint16_t)val);
}

void Payload::set_int32(int32_t val)
{
    set_uint32(val);
}

void Payload::set_float(float val)
{
    uint8_t data[4];
    setFloat32(data, val);
    for ( uint8_t i = 0;  i < 4; i++ )
    {
    setByte(data + i, 8);
    }
}

void Payload::set_uint4(uint8_t val)
{
    uint8_t data = val;
    setByte(&data, 4);
}

void Payload::set_uint8(uint8_t val)
{
    uint8_t data = val;
     setByte(&data, 8);
}

void Payload::set_uint16(uint16_t val)
{
    uint8_t data[2];
     setUint16(  data, val);
     setByte(data, 8);
     setByte(data + 1, 8);
}

void Payload::set_uint24(uint32_t val)
{
    uint8_t data[4];
     setUint32(data, val);
     setByte(data + 1, 8);
     setByte(data + 2, 8);
     setByte(data + 3, 8);
}

void Payload::set_uint32(uint32_t val)
{
    uint8_t data[4];
     setUint32( data, val);
     for ( uint8_t i = 0; i < 4; i++ )
     {
     setByte(data + i, 8);
     }
}

void Payload::set_string(String str)
{
    uint8_t len = str.length();
    if ( len > 15 )
    {
        len = 15;
    }
    setByte(&len,4);
    for ( uint8_t i = 0; i < len; i++ )
    {
        uint8_t ch = (uint8_t)str.charAt(i);
        setByte(&ch, 8);
    }
}

//
// getters
//
bool Payload::get_bool(void)
{
    uint8_t val;
    return (bool)*getData(&val, 1);
}

int8_t Payload::get_int4(void)
{
    uint8_t val = get_uint4();
    return val < 8 ? val : val - 16;
}

int8_t Payload::get_int8(void)
{
    return (int8_t) get_uint8();
}

int16_t Payload::get_int16(void)
{
    return (int16_t)get_uint16();
}

int32_t Payload::get_int32(void)
{
    return (int32_t)get_uint32();
}

float Payload::get_float(void)
{
    uint8_t buf[4];
    float val = getFloat32((const uint8_t*)getData(buf, 32));
    return val;
}

uint8_t Payload::get_uint4(void)
{
    uint8_t val;
    getData(&val, 4);
    return val;
}

uint8_t Payload::get_uint8(void)
{
    uint8_t val;
   getData(&val, 8);
   return val;
}

uint16_t Payload::get_uint16(void)
{
    uint8_t buf[2];
    uint16_t val = getUint16((const uint8_t*)getData(buf, 16));
    return val;
}

uint32_t Payload::get_uint24(void)
{
    uint8_t buf[4];
    uint8_t* p = buf + 1;
    buf[0] = 0;
     getByte(p++, 8);
     getByte(p++, 8);
     getByte(p, 8);
    uint32_t val = getUint32(buf);
    return val;
}

uint32_t Payload::get_uint32(void)
{
    uint8_t buf[4];
    uint32_t val = getUint32((const uint8_t*)getData(buf, 32));
    return val;
}

String Payload::get_string(void)
{
    String str ;
    char val;
    uint8_t len = get_uint4();
    for(uint8_t i = 0; i < len; i++)
    {
        getByte((uint8_t*)&val, 8);
        str += String(val);
    }
    return str;
}


//
//
//
uint8_t* Payload::getData(uint8_t* val, uint8_t len)
{
    if ( len < 16 )
    {
        return getByte(val, len);
    }
    else if ( len < 32 )
    {
        getByte(val, 8);
        getByte( val + 1, 8);
        return val;
    }
    else
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            getByte(val + i, 8);
        }
        return val;
    }
}

uint8_t* Payload::getByte(uint8_t* val, uint8_t len)
{
    if ( len >  _gPos + 1 )
    {
        uint8_t  len0 = len - 1 -  _gPos;

        *val = *_Gpos << (7 - _gPos);
        *val = * val >> (8 - len);
        _Gpos++;
        *val |= *_Gpos >> (8 - len0);
        _gPos = 7 - len0;
    }
    else
    {
        *val = *_Gpos << ( 7 - _gPos );
        *val = *val >> (8 - len);
        _gPos -= len;
        if ( _gPos == 255 )
        {
            _gPos = 7;
            _Gpos++;
        }
    }
    return val;
}

void Payload::setByte(uint8_t* data, uint8_t len)
{
    uint8_t buf;

    if ( (_Bpos - _buff) > _maxLen )
    {
        goto err_exit;
    }

    if ( len >  _bPos + 1 )
    {
        uint8_t len0 = len - 1 - _bPos;

        buf  = *data >> len0;
        *_Bpos++ |= buf;
        if ( (_Bpos - _buff) > _maxLen )
        {
            goto err_exit;
        }
        buf = *data << (8 - len0);
        *_Bpos |= buf;
        _bPos = 7 - len0;
    }
    else
    {
         buf  = *data << ( _bPos + 1 - len);
        *_Bpos |= buf;
        _bPos -= len;
        if ( _bPos == 255 )
        {
            _bPos = 7;
            _Bpos++;
        }
    }
    return;

err_exit:
    ConsolePrint(F("Payload over flow\n"));
    return;
}
