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

#include <iostream>
#include <sragent.h>
#include <srlogger.h>

using namespace std;

int main()
{
    const char* const server = "http://bbv-ch.cumulocity.com";
    const char* const credentialPath = "/tmp/helloc8y/ex01-hello";
    const char* const deviceID = "ex01-hello";   // unique device identifier

    srLogSetLevel(SRLOG_DEBUG);          // set log level to debug

    SrAgent agent(server, deviceID);     // instantiate SrAgent
    if (agent.bootstrap(credentialPath)) // bootstrap to Cumulocity
    {
        return 0;
    }

    cout << "Hello, Cumulocity!" << endl;

    return 0;
}
