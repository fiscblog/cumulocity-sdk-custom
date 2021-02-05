bin/ex-10-sensorhub: sensorhub.c
	@mkdir -p bin
	$(CC) -I$(C8Y_LIB_PATH)/include $^ -L$(C8Y_LIB_PATH)/lib -lpaho-mqtt3c -lwiringPi -o $@
clean:
	@rm -rf bin
	@echo Clean done