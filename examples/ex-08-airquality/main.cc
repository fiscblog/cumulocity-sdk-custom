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
#include <srdevicepush.h>
#include <unistd.h>

#include "integrate.h"
#include "messagehandler.h"
#include "srutils.h"
#include "wiringPiI2C.h"


#define SENSOR_ID_BME           0x76
#define SENSOR_ID_TSL           0x29


int main()
{
    const char* const server = "http://bbv-ch.cumulocity.com";
    const char* const credentialPath = "/var/helloc8y/bbv-airquality";
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

    std::string srversion, srtemplate;
    if (readSrTemplate("srtemplate.txt", srversion, srtemplate) != 0)
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

    RestartHandler restartHandler;
    agent.addMsgHandler(502, &restartHandler);

    ConfigurationHandler configurationHandler;
    agent.addMsgHandler(503, &configurationHandler);

    SoftwareHandler softwareHandler;
    agent.addMsgHandler(504, &softwareHandler);

    FirmwareHandler firmwareHandler;
    agent.addMsgHandler(505, &firmwareHandler);

    SrDevicePush push(server, agent.XID(), agent.auth(), agent.ID(), agent.ingress);
    if (push.start() != 0)      // Start the device push thread
    {
        return 0;
    }

    agent.loop();

    return 0;
}
