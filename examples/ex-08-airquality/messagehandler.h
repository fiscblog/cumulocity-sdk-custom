#include <iostream>
#include <sragent.h>
#include <srreporter.h>
#include <srlogger.h>

#include "BME280.h"
#include "TSL2561.h"


class TemperatureMeasurement: public SrTimerHandler
{
public:
    TemperatureMeasurement(BME280 &_bme) : bme(_bme) {}

    virtual void operator()(SrTimer &timer, SrAgent &agent)
    {
         bme.read();
         const int temperature = bme.getTemperature();
         agent.send("103," + agent.ID() + "," + std::to_string(temperature));
    }
private:
    BME280 &bme;
};

class HumidityMeasurement: public SrTimerHandler
{
public:
     HumidityMeasurement(BME280 &_bme) : bme(_bme) {}

     virtual void operator()(SrTimer &timer, SrAgent &agent)
     {
         bme.read();
         const int humidity = bme.getHumidity();
         agent.send("104," + agent.ID() + "," + std::to_string(humidity));
     }
private:
    BME280 &bme;
};

class PressureMeasurement: public SrTimerHandler
{
public:
     PressureMeasurement(BME280 &_bme) : bme(_bme) {}

     virtual void operator()(SrTimer &timer, SrAgent &agent)
     {
         bme.read();
         const int pressure = bme.getPressure() / 100;
         agent.send("105," + agent.ID() + "," + std::to_string(pressure));
     }
private:
     BME280 &bme;
};

class LuminosityMeasurement: public SrTimerHandler
{
public:
    LuminosityMeasurement(TSL2561 &_tsl) : tsl(_tsl) {}

    virtual void operator()(SrTimer &timer, SrAgent &agent)
    {
        tsl.powerOn(true);
        usleep(500000);
        tsl.read();
        const int luminosity = tsl.getLux();
        tsl.powerOn(false);
        agent.send("106," + agent.ID() + "," + std::to_string(luminosity));
    }
private:
    TSL2561 &tsl;
};

class RestartHandler: public SrMsgHandler
{
public:

    virtual void operator()(SrRecord &r, SrAgent &agent)
    {
        agent.send("108," + r.value(2) + ",EXECUTING");
        for (int i = 0; i < r.size(); ++i)
        {
            std::cerr << r.value(i) << " ";
        }
        std::cerr << std::endl;

        agent.send("108," + r.value(2) + ",SUCCESSFUL");
    }
};

class ConfigurationHandler: public SrMsgHandler
{
public:

    virtual void operator()(SrRecord &r, SrAgent &agent)
    {
        agent.send("109," + r.value(2) + ",EXECUTING");
        for (int i = 0; i < r.size(); ++i)
        {
            std::cerr << r.value(i) << " ";
        }
        std::cerr << std::endl;

        agent.send("109," + r.value(2) + ",SUCCESSFUL");
    }
};

class SoftwareHandler: public SrMsgHandler
{
public:

    virtual void operator()(SrRecord &r, SrAgent &agent)
    {
        agent.send("110," + r.value(2) + ",EXECUTING");
        for (int i = 0; i < r.size(); ++i)
        {
            std::cerr << r.value(i) << " ";
        }
        std::cerr << std::endl;

        agent.send("110," + r.value(2) + ",SUCCESSFUL");
    }
};

class FirmwareHandler: public SrMsgHandler
{
public:

    virtual void operator()(SrRecord &r, SrAgent &agent)
    {
        agent.send("111," + r.value(2) + ",EXECUTING");
        for (int i = 0; i < r.size(); ++i)
        {
            std::cerr << r.value(i) << " ";
        }
        std::cerr << std::endl;

        agent.send("111," + r.value(2) + ",SUCCESSFUL");
    }
};
