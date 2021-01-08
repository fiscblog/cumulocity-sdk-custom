/*
 * Copyright (C) 2015-2017 Cumulocity GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <cstdlib>
#include <iostream>
#include <sragent.h>
#include <srreporter.h>
#include <srlogger.h>
#include <unistd.h>
#include "integrate.h"
#include "wiringPiI2C.h"
#include "BME280.h"
#include "TSL2561.h"

#define SENSOR_ID_BME           0x76
#define SENSOR_ID_TSL           0x29

using namespace std;

static const char *srversion = "helloc8y_4";
static const char *srtemplate =
        "10,100,GET,/identity/externalIds/c8y_Serial/%%,,"
        "application/json,%%,STRING,\n"

        "10,101,POST,/inventory/managedObjects,application/json,"
        "application/json,%%,,\"{\"\"name\"\":\"\"bbv-airquality\"\","
        "\"\"type\"\":\"\"c8y_hello\"\",\"\"c8y_IsDevice\"\":{},"
        "\"\"com_cumulocity_model_Agent\"\":{}}\"\n"

        "10,102,POST,/identity/globalIds/%%/externalIds,application/json,,%%,"
        "STRING STRING,\"{\"\"externalId\"\":\"\"%%\"\","
        "\"\"type\"\":\"\"c8y_Serial\"\"}\"\n"

        "10,103,POST,/measurement/measurements,application/json,,%%,"
        "NOW UNSIGNED NUMBER,\"{\"\"time\"\":\"\"%%\"\","
        "\"\"source\"\":{\"\"id\"\":\"\"%%\"\"},"
        "\"\"type\"\":\"\"c8y_TemperatureMeasurement\"\","
        "\"\"c8y_TemperatureMeasurement\"\":{\"\"Temperature\"\":"
        "{\"\"value\"\":%%,\"\"unit\"\":\"\"C\"\"}}}\"\n"

        "10,104,POST,/measurement/measurements,application/json,,%%,"
        "NOW UNSIGNED NUMBER,\"{\"\"time\"\":\"\"%%\"\","
        "\"\"source\"\":{\"\"id\"\":\"\"%%\"\"},"
        "\"\"type\"\":\"\"c8y_HumidityMeasurement\"\","
        "\"\"c8y_HumidityMeasurement\"\":{\"\"Humidity\"\":"
        "{\"\"value\"\":%%,\"\"unit\"\":\"\"%\"\"}}}\"\n"

        "10,105,POST,/measurement/measurements,application/json,,%%,"
        "NOW UNSIGNED NUMBER,\"{\"\"time\"\":\"\"%%\"\","
        "\"\"source\"\":{\"\"id\"\":\"\"%%\"\"},"
        "\"\"type\"\":\"\"c8y_PressureMeasurement\"\","
        "\"\"c8y_PressureMeasurement\"\":{\"\"Pressure\"\":"
        "{\"\"value\"\":%%,\"\"unit\"\":\"\"hPa\"\"}}}\"\n"

        "10,106,POST,/measurement/measurements,application/json,,%%,"
        "NOW UNSIGNED NUMBER,\"{\"\"time\"\":\"\"%%\"\","
        "\"\"source\"\":{\"\"id\"\":\"\"%%\"\"},"
        "\"\"type\"\":\"\"c8y_LuminosityMeasurement\"\","
        "\"\"c8y_LuminosityMeasurement\"\":{\"\"Luminosity\"\":"
        "{\"\"value\"\":%%,\"\"unit\"\":\"\"lux\"\"}}}\"\n"


        "11,500,$.managedObject,,$.id\n"
        "11,501,,$.c8y_IsDevice,$.id\n";


class TemperatureMeasurement: public SrTimerHandler
{
public:
    TemperatureMeasurement(BME280 &_bme) : bme(_bme) {}

    virtual void operator()(SrTimer &timer, SrAgent &agent)
    {
         bme.read();
         const int temperature = bme.getTemperature();
         agent.send("103," + agent.ID() + "," + to_string(temperature));
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
         agent.send("104," + agent.ID() + "," + to_string(humidity));
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
         agent.send("105," + agent.ID() + "," + to_string(pressure));
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
        agent.send("106," + agent.ID() + "," + to_string(luminosity));
    }
private:
    TSL2561 &tsl;
};

int main()
{
    const char* const server = "http://bbv-ch.cumulocity.com";
    const char* const credentialPath = "/tmp/helloc8y/bbv-airquality";
    const char* const deviceID = "bbv-airquality"; // unique device identifier

    srLogSetLevel(SRLOG_DEBUG);        // set log level to debug

    int bme_fd;
    if ((bme_fd = wiringPiI2CSetup(SENSOR_ID_BME)) == 0) {
        std::cerr << "Cannot open I2C connection to BME280 sensor";
        return 1;
    }
    BME280 bme;
    if (not bme.init(bme_fd)) {
        std::cerr << "Failed to initialize BME280 sensor";
        return 1;
    }

    int tsl_fd;
    if ((tsl_fd = wiringPiI2CSetup(SENSOR_ID_TSL)) == 0) {
        std::cerr << "Cannot open I2C connection to TSL2561 sensor";
        return 1;
    }
    TSL2561 tsl;
    if (not tsl.init(tsl_fd)) {
        std::cerr << "Failed to initialize TSL2561 sensor";
        return 1;
    }

    // Discard first two readings
    tsl.powerOn(true);
    usleep(50000);
    bme.read();
    tsl.read();
    usleep(50000);
    bme.read();
    tsl.read();
    tsl.powerOn(false);

    Integrate igt;
    SrAgent agent(server, deviceID, &igt); // instantiate SrAgent
    if (agent.bootstrap(credentialPath))   // bootstrap to Cumulocity
    {
        return 0;
    }

    if (agent.integrate(srversion, srtemplate)) // integrate to Cumulocity
    {
        return 0;
    }

    TemperatureMeasurement temperature(bme);
    SrTimer timer_temperature(3 * 1000, &temperature);
    agent.addTimer(timer_temperature);
    timer_temperature.start();

    HumidityMeasurement humidity(bme);
    SrTimer timer_humidity(3 * 1000, &humidity);
    agent.addTimer(timer_humidity);
    timer_humidity.start();

    PressureMeasurement pressure(bme);
    SrTimer timer_pressure(3 * 1000, &pressure);
    agent.addTimer(timer_pressure);
    timer_pressure.start();

    LuminosityMeasurement luminosity(tsl);
    SrTimer timer_luminosity(3 * 1000, &luminosity);
    agent.addTimer(timer_luminosity);
    timer_luminosity.start();

    SrReporter reporter(server, agent.XID(), agent.auth(), agent.egress, agent.ingress);
    if (reporter.start() != 0)      // Start the reporter thread
    {
        return 0;
    }

    agent.loop();

    return 0;
}
