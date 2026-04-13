// LoRaSRadio.h
#ifndef LORASRADIO_H
#define LORASRADIO_H

#include <RadioLib.h>

class LoRaSRadio
{
public:
    virtual ~LoRaSRadio() = default;

    virtual int begin(float freq, float bw, uint8_t sf, uint8_t cr,
                      uint8_t syncWord, int8_t power,
                      uint16_t preamble, uint8_t gain) = 0;

    virtual int explicitHeader() = 0;
    virtual int setCRC(bool enable) = 0;
    virtual int getIRQFlags() = 0;
    virtual int16_t clearIrqFlags(uint16_t irqFlags) = 0;
    virtual int startReceive() = 0;
    virtual int readData(uint8_t *buf, size_t len) = 0;
    virtual int transmit(uint8_t *buf, size_t len) = 0;
    virtual int16_t getRSSI() = 0;
    virtual float getSNR() = 0;
    virtual int getPacketLength() = 0;
    virtual int16_t scanChannel() = 0;
    virtual bool isRxDone() = 0;
    virtual void clearRxDone() = 0;
};

// --- SX1278 wrapper ---
class LoRaSRadioSX1278 : public LoRaSRadio
{
public:
    LoRaSRadioSX1278(SX1278 *m) : module(m) {}

    int begin(float freq, float bw, uint8_t sf, uint8_t cr,
              uint8_t syncWord, int8_t power,
              uint16_t preamble, uint8_t gain) override
    {
        return module->begin(freq, bw, sf, cr, syncWord, power, preamble, gain);
    }
    int explicitHeader() override { return module->explicitHeader(); }
    int setCRC(bool enable) override { return module->setCRC(enable); }
    int getIRQFlags() override { return module->getIRQFlags(); }
    int16_t clearIrqFlags(uint16_t irqFlags) override { return module->clearIrqFlags(irqFlags); }
    int startReceive() override { return module->startReceive(); }
    int readData(uint8_t *buf, size_t len) override { return module->readData(buf, len); }
    int transmit(uint8_t *buf, size_t len) override { return module->transmit(buf, len); }
    int16_t getRSSI() override { return module->getRSSI(); }
    float getSNR() override { return module->getSNR(); }
    int getPacketLength() override { return module->getPacketLength(); }
    int16_t scanChannel() override { return module->scanChannel(); }
    bool isRxDone() override
    {
        return module->getIRQFlags() & RADIOLIB_SX127X_CLEAR_IRQ_FLAG_RX_DONE;
    }
    void clearRxDone() override
    {
        uint16_t flags = module->getIRQFlags();
        module->clearIrqFlags(flags);
    }

private:
    SX1278 *module;
};

// --- SX1262 wrapper ---
class LoRaSRadioSX1262 : public LoRaSRadio
{
public:
    LoRaSRadioSX1262(SX1262 *m) : module(m) {}

    int begin(float freq, float bw, uint8_t sf, uint8_t cr,
              uint8_t syncWord, int8_t power,
              uint16_t preamble, uint8_t gain) override
    {
        // SX1262 begin() has no gain parameter
        return module->begin(freq, bw, sf, cr, syncWord, power, preamble);
    }
    int explicitHeader() override { return module->explicitHeader(); }
    int setCRC(bool enable) override { return module->setCRC(enable ? 2 : 0); } // SX1262 uses 0/2
    int getIRQFlags() override { return module->getIrqFlags(); }
    int16_t clearIrqFlags(uint16_t irqFlags) override { return module->clearIrqFlags(irqFlags); }
    int startReceive() override { return module->startReceive(); }
    int readData(uint8_t *buf, size_t len) override { return module->readData(buf, len); }
    int transmit(uint8_t *buf, size_t len) override { return module->transmit(buf, len); }
    int16_t getRSSI() override { return module->getRSSI(); }
    float getSNR() override { return module->getSNR(); }
    int getPacketLength() override { return module->getPacketLength(); }
    int16_t scanChannel() override { return module->scanChannel(); }
    bool isRxDone() override
    {
        return module->getIrqFlags() & RADIOLIB_SX126X_IRQ_RX_DONE;
    }
    void clearRxDone() override
    {
        uint16_t flags = module->getIrqFlags();
        module->clearIrqFlags(flags);
    }

private:
    SX1262 *module;
};

#endif // LORASRADIO_H