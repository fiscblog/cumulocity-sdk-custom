bin/ex-09-mqtt: main.c
	@mkdir -p bin
	$(CXX) -std=c++11 -I$(C8Y_LIB_PATH)/include $^ -L/usr/lib/ -L$(C8Y_LIB_PATH)/lib -lpaho-mqtt3c -o $@