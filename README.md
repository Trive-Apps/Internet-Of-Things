# TRIVE - IoT Part

This repository contains code for a prototype electric car (EV) we created. This prototype will act as a miniature EV whose task is to collect data that is generally found in an EV such as battery temperature, voltage, current, and also geo-location data. This data will then be thrown into a database which will later be consumed by the Android application and also the machine learning model.

![Trive Car](./docs/car_image.jpg)

## Folder structure
```
trive_car/
├─── data_controller/           # Contains esp32 code to collect data on the sensor
│    ├── credential.cpp
│    ├── credential.cpp.example
│    ├── credential.h
│    └── data_controller.ino
│       
├─── docs/                      # Contains documents about schematic diagrams
│    ├── car_image.jpg
│    ├── TRIVE CAR PROTOTYPE HARDWARE CONNECTIONS.pdf
│    └── TRIVE CAR PROTOTYPE HARDWARE CONNECTIONS.png
│
├─── motor_controller/          # Contains esp32 code to control the motor
│    ├── credential.cpp
│    ├── credential.h
│    ├── motor_controller.html
│    └── motor_controller.ino
|
├─── .gitignore
└──- README.md
```

## How to Use this code

1. Clone this Repository.
2. Change the file `credential.cpp.example` in the `data_controller` and `motor_controller` directories to `credential.cpp`.
3. Fill in the credentials in the file.
4. Assemble a mobile prototype. Follow the schematic diagram in the `docs` directory.
5. Just upload the code. Make sure to upload the correct code to esp32.