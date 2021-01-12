bin/ex-08-airquality: TSL2561.cpp BME280.cpp integrate.cc main.cc
	@mkdir -p bin
	$(CXX) -std=c++11 -I$(C8Y_LIB_PATH)/include -I$(GROVEPI_SENSOR_PATH) $^ -L/usr/lib/ -L$(C8Y_LIB_PATH)/lib -lsera -lwiringPi -pthread -o $@ -Wno-narrowing
